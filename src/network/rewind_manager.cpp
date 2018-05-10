//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "network/rewind_manager.hpp"

#include "graphics/irr_driver.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/protocols/game_protocol.hpp"
#include "network/rewinder.hpp"
#include "network/rewind_info.hpp"
#include "physics/physics.hpp"
#include "race/history.hpp"
#include "utils/log.hpp"
#include "utils/profiler.hpp"

#include <algorithm>

RewindManager* RewindManager::m_rewind_manager = NULL;
bool           RewindManager::m_enable_rewind_manager = false;

/** Creates the singleton. */
RewindManager *RewindManager::create()
{
    assert(!m_rewind_manager);
    m_rewind_manager = new RewindManager();
    return m_rewind_manager;
}   // create

// ----------------------------------------------------------------------------
/** Destroys the singleton. */
void RewindManager::destroy()
{
    assert(m_rewind_manager);
    delete m_rewind_manager;
    m_rewind_manager = NULL;
}   // destroy

// ============================================================================
/** The constructor.
 */
RewindManager::RewindManager()
{
    reset();
}   // RewindManager

// ----------------------------------------------------------------------------
/** Frees all saved state information. Note that the Rewinder data must be
 *  freed elsewhere.
 */
RewindManager::~RewindManager()
{
}   // ~RewindManager

// ----------------------------------------------------------------------------
/** Frees all saved state information and all destroyable rewinder.
 */
void RewindManager::reset()
{
    m_is_rewinding = false;
    m_not_rewound_ticks = 0;
    m_overall_state_size = 0;
    m_last_saved_state = -1;  // forces initial state save
    m_state_frequency =
        stk_config->getPhysicsFPS() / stk_config->m_network_state_frequeny;

    if (!m_enable_rewind_manager) return;

    AllRewinder::iterator r = m_all_rewinder.begin();
    while (r != m_all_rewinder.end())
    {
        if (!(*r)->canBeDestroyed())
        {
            r++;
            continue;
        }
        Rewinder *rewinder = *r;
        r = m_all_rewinder.erase(r);
        delete rewinder;
    }

    m_rewind_queue.reset();
}   // reset

// ----------------------------------------------------------------------------
/** Adds a new TimeStep entry. Only exception is time=0 (which happens during
 *  all of 'ready, set, go') - for which only one entry is created.
 */
void RewindManager::addNextTimeStep(int time, float dt)
{
    // Add a timestep entry each timestep, except at 'ready, set, go'
    // at which time is 0 - we add only one entry there
    if ((time > 0 || m_rewind_queue.isEmpty()) &&
        World::getWorld()->getPhase() != WorldStatus::IN_GAME_MENU_PHASE)
    {
//        m_rewind_queue.addNewTimeStep(time, dt);
    }
}   // addNextTimeStep

// ----------------------------------------------------------------------------    
/** Adds an event to the rewind data. The data to be stored must be allocated
 *  and not freed by the caller!
 *  \param time Time at which the event was recorded. If time is not specified
 *          (or set to -1), the current world time is used.
 *  \param buffer Pointer to the event data.
 */
void RewindManager::addEvent(EventRewinder *event_rewinder,
                             BareNetworkString *buffer, bool confirmed,
                             int ticks)
{
    if (m_is_rewinding)
    {
        delete buffer;
        Log::error("RewindManager", "Adding event when rewinding");
        return;
    }

    if (ticks < 0)
        ticks = World::getWorld()->getTimeTicks();
    m_rewind_queue.addLocalEvent(event_rewinder, buffer, confirmed, ticks);
}   // addEvent

// ----------------------------------------------------------------------------
/** Adds an event to the list of network rewind data. This function is
 *  threadsafe so can be called by the network thread. The data is synched
 *  to m_rewind_info by the main thread. The data to be stored must be
 *  allocated and not freed by the caller!
 *  \param time Time at which the event was recorded.
 *  \param buffer Pointer to the event data.
 */
void RewindManager::addNetworkEvent(EventRewinder *event_rewinder,
                                     BareNetworkString *buffer, int ticks)
{
    m_rewind_queue.addNetworkEvent(event_rewinder, buffer, ticks);
}   // addNetworkEvent

// ----------------------------------------------------------------------------
/** Adds a state to the list of network rewind data. This function is
 *  threadsafe so can be called by the network thread. The data is synched
 *  to m_rewind_info by the main thread. The data to be stored must be
 *  allocated and not freed by the caller!
 *  \param time Time at which the event was recorded.
 *  \param buffer Pointer to the event data.
 */
void RewindManager::addNetworkState(BareNetworkString *buffer, int ticks)
{
    assert(NetworkConfig::get()->isClient());
    m_rewind_queue.addNetworkState(buffer, ticks);
}   // addNetworkState

// ----------------------------------------------------------------------------
/** Saves a local state using the GameProtocol function to combine several
 *  independent rewinders to write one state. Typically only the server needs
 *  to save a state (which is then send to the clients), except that each
 *  client needs one initial state (in case that it receives an event from
 *  a client before a state from the serer).
 *  \param allow_local_save Do a local save.
 */
void RewindManager::saveState(bool local_save)
{
    PROFILER_PUSH_CPU_MARKER("RewindManager - save state", 0x20, 0x7F, 0x20);
    GameProtocol::lock()->startNewState(local_save);
    AllRewinder::const_iterator rewinder;
    for (rewinder = m_all_rewinder.begin(); rewinder != m_all_rewinder.end(); ++rewinder)
    {
        // TODO: check if it's worth passing in a sufficiently large buffer from
        // GameProtocol - this would save the copy operation.
        BareNetworkString *buffer = (*rewinder)->saveState();
        if (buffer && buffer->size() >= 0)
        {
            m_overall_state_size += buffer->size();
            GameProtocol::lock()->addState(buffer);
        }   // size >= 0
        delete buffer;    // buffer can be freed
    }
    PROFILER_POP_CPU_MARKER();
}   // saveState

// ----------------------------------------------------------------------------
/** Saves a state on the client. Used to save an initial state at t=0 for each
 *  client in case that we receive an event from another client (which will
 *  trigger a rewind) before a state from the server.
 */
void RewindManager::saveLocalState()
{
    int ticks = World::getWorld()->getTimeTicks();

    saveState(/*local_state*/true);
    NetworkString *state = GameProtocol::lock()->getState();

    // Copy the data to a new string, making the buffer in
    // GameProtocol availble for again.
    BareNetworkString *bns =
        new BareNetworkString(state->getCurrentData(),
                              state->size()           );
    m_rewind_queue.addLocalState(bns, /*confirmed*/true, ticks);
}   // saveLocalState

// ----------------------------------------------------------------------------
/** Restores a given state by calling rewindToState for each available rewinder
 *  with its correct data.
 *  \param data The data string used to store the whole game state.
 *
 */
void RewindManager::restoreState(BareNetworkString *data)
{
    data->reset();   
    int index = 0;
    //AllRewinder::const_iterator rewinder;
    for (auto rewinder = m_all_rewinder.begin(); rewinder != m_all_rewinder.end();
                                          ++rewinder)
    {
        uint16_t count = data->getUInt16();
        if (count > 0)
        {
            (*rewinder)->rewindToState(data);
        }
    }   // for all rewinder
}   // restoreState

// ----------------------------------------------------------------------------
/** Determines if a new state snapshot should be taken, and if so calls all
 *  rewinder to do so.
 *  \param ticks_not_used NUmber of physics time steps - should be 1.
 */
void RewindManager::update(int ticks_not_used)
{
    // FIXME: rename ticks_not_used
    if (!m_enable_rewind_manager ||
        m_all_rewinder.size() == 0 ||
        m_is_rewinding)  return;

    float time = World::getWorld()->getTime();
    int ticks = World::getWorld()->getTimeTicks();

    m_not_rewound_ticks = ticks;

    // Clients don't save state, so they just exit.
    if (NetworkConfig::get()->isClient() ||
        ticks - m_last_saved_state < m_state_frequency)
    {
        return;
    }

    // Save state
    saveState(/**allow_local_save*/false);
    PROFILER_PUSH_CPU_MARKER("RewindManager - send state", 0x20, 0x7F, 0x40);
    GameProtocol::lock()->sendState();
    PROFILER_POP_CPU_MARKER();
    m_last_saved_state = ticks;
}   // update

// ----------------------------------------------------------------------------
/** Replays all events from the last event played till the specified time.
 *  \param world_ticks Up to (and inclusive) which time events will be replayed.
 *  \param ticks Number of time steps - should be 1.
 */
void RewindManager::playEventsTill(int world_ticks, int *ticks)
{
    bool needs_rewind;
    int rewind_ticks;

    // Merge in all network events that have happened at the current
    // time step.
    // merge and that have happened before the current time (which will
    // be getTime()+dt - world time has not been updated yet).
    m_rewind_queue.mergeNetworkData(world_ticks, &needs_rewind, &rewind_ticks);

    if (needs_rewind)
    {
        Log::setPrefix("Rewind");
        PROFILER_PUSH_CPU_MARKER("Rewind", 128, 128, 128);
        rewindTo(rewind_ticks, world_ticks);
        // This should replay everything up to 'now'
        assert(World::getWorld()->getTimeTicks() == world_ticks);
        PROFILER_POP_CPU_MARKER();
        Log::setPrefix("");
    }

    assert(!m_is_rewinding);
    if (m_rewind_queue.isEmpty()) return;

    // This is necessary to avoid that rewinding an event will store the 
    // event again as a seemingly new event.
    m_is_rewinding = true;

    // Now play all events that happened at the current time stamp.
    m_rewind_queue.replayAllEvents(world_ticks);

    m_is_rewinding = false;
}   // playEventsTill

// ----------------------------------------------------------------------------
/** Rewinds to the specified time, then goes forward till the current
 *  World::getTime() is reached again: it will replay everything before
 *  World::getTime(), but not the events at World::getTime() (or later)/
 *  \param rewind_ticks Time to rewind to.
 *  \param now_ticks Up to which ticks events are replayed: up to but 
 *         EXCLUDING new_ticks (the event at now_ticks are played in
 *         the calling subroutine playEventsTill).
 */
void RewindManager::rewindTo(int rewind_ticks, int now_ticks)
{
    assert(!m_is_rewinding);
    bool is_history = history->replayHistory();
    history->setReplayHistory(false);

    // First save all current transforms so that the error
    // can be computed between the transforms before and after
    // the rewind.
    AllRewinder::iterator rewinder;
    for (rewinder = m_all_rewinder.begin();
        rewinder != m_all_rewinder.end(); ++rewinder)
    {
        (*rewinder)->saveTransform();
    }

    // Then undo the rewind infos going backwards in time
    // --------------------------------------------------
    m_is_rewinding = true;

    // This will go back till the first confirmed state is found before
    // the specified rewind ticks.
    int exact_rewind_ticks = m_rewind_queue.undoUntil(rewind_ticks);

    // Rewind the required state(s)
    // ----------------------------
    World *world = World::getWorld();

    // Now start the rewind with the full state:
    world->setTicks(exact_rewind_ticks);

    // Get the (first) full state to which we have to rewind
    RewindInfo *current = m_rewind_queue.getCurrent();
    assert(current->isState());

    // Restore states from the exact rewind time
    // -----------------------------------------
    // A loop in case that we should split states into several smaller ones:
    while (current && current->getTicks() == exact_rewind_ticks && 
          current->isState()                                        )
    {
        current->rewind();
        m_rewind_queue.next();
        current = m_rewind_queue.getCurrent();
    }

    // Now go forward through the list of rewind infos till we reach 'now':
    while(world->getTimeTicks() < now_ticks )
    { 
        m_rewind_queue.replayAllEvents(world->getTimeTicks());

        // Now simulate the next time step
        world->updateWorld(1);
#undef SHOW_ROLLBACK
#ifdef SHOW_ROLLBACK
        irr_driver->update(stk_config->ticks2Time(1));
#endif
        world->updateTime(1);

    }   // while (world->getTicks() < current_ticks)

    // Now compute the errors which need to be visually smoothed
    for (rewinder = m_all_rewinder.begin();
        rewinder != m_all_rewinder.end(); ++rewinder)
    {
        (*rewinder)->computeError();
    }

    history->setReplayHistory(is_history);
    m_is_rewinding = false;
}   // rewindTo
