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
#include "modes/soccer_world.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/protocols/game_protocol.hpp"
#include "network/rewinder.hpp"
#include "network/rewind_info.hpp"
#include "network/smooth_network_body.hpp"
#include "physics/physics.hpp"
#include "race/history.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/log.hpp"
#include "utils/profiler.hpp"

#include <algorithm>

RewindManager* RewindManager::m_rewind_manager[PT_COUNT];
std::atomic_bool RewindManager::m_enable_rewind_manager(false);

/** Creates the singleton. */
RewindManager *RewindManager::create()
{
    ProcessType pt = STKProcess::getType();
    assert(!m_rewind_manager[pt]);
    m_rewind_manager[pt] = new RewindManager();
    return m_rewind_manager[pt];
}   // create

// ----------------------------------------------------------------------------
/** Destroys the singleton. */
void RewindManager::destroy()
{
    ProcessType pt = STKProcess::getType();
    assert(m_rewind_manager[pt]);
    delete m_rewind_manager[pt];
    m_rewind_manager[pt] = NULL;
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
    for (RewindInfoEventFunction* rief : m_pending_rief)
        delete rief;
    m_pending_rief.clear();
}   // ~RewindManager

// ----------------------------------------------------------------------------
/** Frees all saved state information and all destroyable rewinder.
 */
void RewindManager::reset()
{
    m_schedule_reset_network_body = false;
    m_is_rewinding = false;
    m_not_rewound_ticks.store(0);
    m_overall_state_size = 0;
    m_state_frequency = stk_config->getPhysicsFPS() /
        NetworkConfig::get()->getStateFrequency();

    if (!m_enable_rewind_manager) return;

    clearExpiredRewinder();
    m_rewind_queue.reset();
    m_missing_rewinders.clear();
}   // reset

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
        ticks = World::getWorld()->getTicksSinceStart();
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
/** Saves a state using the GameProtocol function to combine several
 *  independent rewinders to write one state.
 */
void RewindManager::saveState()
{
    PROFILER_PUSH_CPU_MARKER("RewindManager - save state", 0x20, 0x7F, 0x20);
    auto gp = GameProtocol::lock();
    if (!gp)
        return;
    gp->startNewState();

    m_overall_state_size = 0;
    std::vector<std::string> rewinder_using;

    for (auto& p : m_all_rewinder)
    {
        // TODO: check if it's worth passing in a sufficiently large buffer from
        // GameProtocol - this would save the copy operation.
        BareNetworkString* buffer = NULL;
        if (auto r = p.second.lock())
            buffer = r->saveState(&rewinder_using);
        if (buffer != NULL)
        {
            m_overall_state_size += buffer->size();
            gp->addState(buffer);
        }
        delete buffer;    // buffer can be freed
    }
    gp->finalizeState(rewinder_using);
    PROFILER_POP_CPU_MARKER();
}   // saveState

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

    int ticks = World::getWorld()->getTicksSinceStart();

    m_not_rewound_ticks.store(ticks, std::memory_order_relaxed);

    if (!shouldSaveState(ticks))
        return;

    // Save state, remove expired rewinder first
    clearExpiredRewinder();
    if (NetworkConfig::get()->isClient())
    {
        auto& ret = m_local_state[ticks];
        for (auto& p : m_all_rewinder)
        {
            if (auto r = p.second.lock())
                ret.push_back(r->getLocalStateRestoreFunction());
        }
    }
    else
    {
        saveState();
        PROFILER_PUSH_CPU_MARKER("RewindManager - send state", 0x20, 0x7F, 0x40);
        if (auto gp = GameProtocol::lock())
            gp->sendState();
    }
    PROFILER_POP_CPU_MARKER();
}   // update

// ----------------------------------------------------------------------------
/** Replays all events from the last event played till the specified time.
 *  \param world_ticks Up to (and inclusive) which time events will be replayed.
 *  \param fast_forward If true, then only rewinders in network will be
 *  updated, but not the physics.
 */
void RewindManager::playEventsTill(int world_ticks, bool fast_forward)
{
    // We add the RewindInfoEventFunction to rewind queue before and after
    // possible rewind, some RewindInfoEventFunction can be created during
    // rewind
    mergeRewindInfoEventFunction();
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
        rewindTo(rewind_ticks, world_ticks, fast_forward);
        // This should replay everything up to 'now'
        assert(World::getWorld()->getTicksSinceStart() == world_ticks);
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
/** Adds a Rewinder to the list of all rewinders.
 *  \return true If successfully added, false otherwise.
 */
bool RewindManager::addRewinder(std::shared_ptr<Rewinder> rewinder)
{
    if (!m_enable_rewind_manager) return false;
    // Maximum 1 bit to store no of rewinder used
    if (m_all_rewinder.size() == 255)
        return false;
    m_all_rewinder[rewinder->getUniqueIdentity()] = rewinder;
    return true;
}   // addRewinder

// ----------------------------------------------------------------------------
/** Rewinds to the specified time, then goes forward till the current
 *  World::getTime() is reached again: it will replay everything before
 *  World::getTime(), but not the events at World::getTime() (or later)/
 *  \param rewind_ticks Time to rewind to.
 *  \param now_ticks Up to which ticks events are replayed: up to but 
 *         EXCLUDING new_ticks (the event at now_ticks are played in
 *         the calling subroutine playEventsTill).
 *  \param fast_forward If true, then only rewinders in network will be
 *  updated, but not the physics.
 */
void RewindManager::rewindTo(int rewind_ticks, int now_ticks,
                             bool fast_forward)
{
    assert(!m_is_rewinding);
    bool is_history = history->replayHistory();
    history->setReplayHistory(false);

    // First save all current transforms so that the error
    // can be computed between the transforms before and after
    // the rewind.
    for (auto& p : m_all_rewinder)
    {
        if (auto r = p.second.lock())
            r->saveTransform();
    }

    // Then undo the rewind infos going backwards in time
    // --------------------------------------------------
    m_is_rewinding = true;

    // This will go back till the first confirmed state is found before
    // the specified rewind ticks.
    int exact_rewind_ticks = m_rewind_queue.undoUntil(rewind_ticks);

    // Rewind the required state(s)
    // ----------------------------
    World* world = World::getWorld();

    // Now start the rewind with the full state. It is important that the
    // world time is set first, since e.g. the NetworkItem manager relies
    // on having the access to the 'confirmed' state time using 
    // the world timer.
    world->setTicksForRewind(exact_rewind_ticks);

    // Get the (first) full state to which we have to rewind
    RewindInfo *current = m_rewind_queue.getCurrent();
    assert(current->isState());

    // Restore states from the exact rewind time
    // -----------------------------------------
    auto it = m_local_state.find(exact_rewind_ticks);
    if (it != m_local_state.end())
    {
        for (auto& restore_local_state : it->second)
        {
            if (restore_local_state)
                restore_local_state();
        }
        for (auto it = m_local_state.begin(); it != m_local_state.end();)
        {
            if (it->first <= exact_rewind_ticks)
                it = m_local_state.erase(it);
            else
                break;
        }
    }
    else if (!fast_forward)
    {
        Log::warn("RewindManager", "Missing local state at ticks %d",
            exact_rewind_ticks);
    }

    // A loop in case that we should split states into several smaller ones:
    while (current && current->getTicks() == exact_rewind_ticks && 
           current->isState()                                        )
    {
        current->restore();
        m_rewind_queue.next();
        current = m_rewind_queue.getCurrent();
    }

    // Update check line, so the cannon animation can be replayed correctly
    Track::getCurrentTrack()->getCheckManager()->resetAfterRewind();

    if (exact_rewind_ticks >= 2)
    {
        // Restore all physical objects moved by 3d animation, as it only
        // set the motion state of physical bodies, it has 1 frame delay
        // the resetAfterRewind will do the saveKinematicState which needs
        // the previous frame transforms to calculate current linear and
        // angular velocities
        world->setTicksForRewind(exact_rewind_ticks - 2);
        Track::getCurrentTrack()->getTrackObjectManager()->resetAfterRewind();
        world->setTicksForRewind(exact_rewind_ticks - 1);
        Track::getCurrentTrack()->getTrackObjectManager()->resetAfterRewind();
        world->setTicksForRewind(exact_rewind_ticks);
    }

    // Now go forward through the list of rewind infos till we reach 'now':
    while (world->getTicksSinceStart() < now_ticks)
    { 
        m_rewind_queue.replayAllEvents(world->getTicksSinceStart());

        // Now simulate the next time step
        if (!fast_forward)
            world->updateWorld(1);
#undef SHOW_ROLLBACK
#ifdef SHOW_ROLLBACK
        irr_driver->update(stk_config->ticks2Time(1));
#endif
        world->updateTime(1);

    }   // while (world->getTicks() < current_ticks)

    // Now compute the errors which need to be visually smoothed
    for (auto& p : m_all_rewinder)
    {
        if (auto r = p.second.lock())
            r->computeError();
    }

    history->setReplayHistory(is_history);
    m_is_rewinding = false;
    mergeRewindInfoEventFunction();
}   // rewindTo

// ----------------------------------------------------------------------------
bool RewindManager::useLocalEvent() const
{
    return NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient() && !m_is_rewinding;
}   // useLocalEvent

// ----------------------------------------------------------------------------
void RewindManager::mergeRewindInfoEventFunction()
{
    for (RewindInfoEventFunction* rief : m_pending_rief)
        m_rewind_queue.insertRewindInfo(rief);
    m_pending_rief.clear();
}   // mergeRewindInfoEventFunction

// ----------------------------------------------------------------------------
/** Reset all smooth network body of rewinders so the rubber band effect of
 *  moveable does not exist during firstly live join.
 */
void RewindManager::handleResetSmoothNetworkBody()
{
    if (m_schedule_reset_network_body)
    {
        m_schedule_reset_network_body = false;
        for (auto& p : m_all_rewinder)
        {
            if (auto r = p.second.lock())
            {
                auto snb = std::dynamic_pointer_cast<SmoothNetworkBody>(r);
                if (snb)
                    snb->reset();
            }
        }
        // Make sure soccer ball is enabled after resuming
        // This happens when paused during goal
        SoccerWorld* sw = dynamic_cast<SoccerWorld*>(World::getWorld());
        if (sw)
            sw->getBall()->setEnabled(true);
    }
}   // handleResetSmoothNetworkBody
