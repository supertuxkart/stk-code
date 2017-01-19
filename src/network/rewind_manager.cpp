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

#include <algorithm>

RewindManager* RewindManager::m_rewind_manager        = NULL;
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
    m_is_rewinding         = false;
    m_overall_state_size   = 0;
    m_state_frequency      = 0.1f;   // save 10 states a second
    m_last_saved_state     = -9999.9f;  // forces initial state save

    if(!m_enable_rewind_manager) return;

    AllRewinder::iterator r = m_all_rewinder.begin();
    while(r!=m_all_rewinder.end())
    {
        if(!(*r)->canBeDestroyed())
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
/** Adds an event to the rewind data. The data to be stored must be allocated
 *  and not freed by the caller!
 *  \param time Time at which the event was recorded.
 *  \param buffer Pointer to the event data. 
 */
void RewindManager::addEvent(EventRewinder *event_rewinder,
                             BareNetworkString *buffer, bool confirmed,
                             float time                                   )
{
    if(m_is_rewinding) 
    {
        delete buffer;
        Log::error("RewindManager", "Adding event when rewinding");
        return;
    }

    Log::verbose("RewindManager", "Time world %f self-event %f",
                 World::getWorld()->getTime(), getCurrentTime());
    if (time < 0)
        time = getCurrentTime();
    m_rewind_queue.addEvent(event_rewinder, buffer, confirmed, time);
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
                                    BareNetworkString *buffer, float time)
{
    m_rewind_queue.addNetworkEvent(event_rewinder, buffer, time);
    Log::verbose("RewindManager", "Time world %f network-event %f",
                 World::getWorld()->getTime(), time);
}   // addEvent

// ----------------------------------------------------------------------------
/** Adds a state to the list of network rewind data. This function is
 *  threadsafe so can be called by the network thread. The data is synched
 *  to m_rewind_info by the main thread. The data to be stored must be
 *  allocated and not freed by the caller!
 *  \param time Time at which the event was recorded.
 *  \param buffer Pointer to the event data.
 */
void RewindManager::addNetworkState(int rewinder_index,
                                    BareNetworkString *buffer, float time)
{
    m_rewind_queue.addNetworkState(m_all_rewinder[rewinder_index], buffer,
                                   time);
    Log::verbose("RewindManager", "Time world %f network-state %f",
        World::getWorld()->getTime(), time);
}   // addState

// ----------------------------------------------------------------------------
/** Determines if a new state snapshot should be taken, and if so calls all
 *  rewinder to do so.
 *  \param dt Time step size.
 */
void RewindManager::saveStates()
{
    if(!m_enable_rewind_manager  || 
        m_all_rewinder.size()==0 ||
        m_is_rewinding              )  return;
    if (NetworkConfig::get()->isClient())
        return;

    float time = World::getWorld()->getTime();
    if(time - m_last_saved_state < m_state_frequency)
    {
        // Avoid saving a time event for the same time, which happens
        // with t=0.
        if (time > m_last_saved_state)
        {
            // No full state necessary, add a dummy entry for the time
            // which increases replay precision (same time step size)
            m_rewind_queue.addTimeEvent(getCurrentTime());
        }
        return;
    }

    // For now always create a snapshot.
    if (NetworkConfig::get()->isServer())
        GameProtocol::getInstance()->startNewState();
    AllRewinder::const_iterator i;
    for(i=m_all_rewinder.begin(); i!=m_all_rewinder.end(); ++i)
    {
        BareNetworkString *buffer = (*i)->saveState();
        if(buffer && buffer->size()>=0)
        {
            m_overall_state_size += buffer->size();
            m_rewind_queue.addState(*i, buffer, /*confirmed*/true,
                                    getCurrentTime());
            if(NetworkConfig::get()->isServer())
                GameProtocol::getInstance()->addState(buffer);
        }   // size >= 0
        else
            delete buffer;   // NULL or 0 byte buffer
    }
    if (NetworkConfig::get()->isServer())
        GameProtocol::getInstance()->sendState();

    Log::verbose("RewindManager", "%f allocated %ld",
                 World::getWorld()->getTime(), m_overall_state_size);

    m_last_saved_state = time;
}   // saveStates

// ----------------------------------------------------------------------------
/** Replays all events from the last event played till the specified time.
 *  \param time Up to (and inclusive) which time events will be replayed.
 */
void RewindManager::playEventsTill(float time)
{    
    // No events, nothing to do
    if (m_rewind_queue.isEmpty())
        return;

    bool needs_rewind;
    float rewind_time;

    m_rewind_queue.mergeNetworkData(&needs_rewind, &rewind_time);
    if(needs_rewind)
        Log::info("RewindManager", "At %f merging states from %f needs rewind",
                  World::getWorld()->getTime(), rewind_time);
    else
        Log::info("RewindManager", "At %f no need for rewind",
                  World::getWorld()->getTime());


    if (needs_rewind)
    {
        rewindTo(rewind_time);
    }

    assert(!m_is_rewinding);
    m_is_rewinding = true;

    while(m_rewind_queue.hasMoreRewindInfo())
    {
        RewindInfo *ri = m_rewind_queue.getNext();
        if (ri->getTime() > time)
        {
            m_is_rewinding = false;
            return;
        }
        ++m_rewind_queue;
        if(ri->isEvent())
            ri->rewind();
    }
    m_is_rewinding = false;
}   // playEventsTill

// ----------------------------------------------------------------------------
/** Rewinds to the specified time.
 *  \param t Time to rewind to.
 */
void RewindManager::rewindTo(float rewind_time)
{
    assert(!m_is_rewinding);
    Log::info("rewind", "Rewinding to %f at %f", rewind_time,
               StkTime::getRealTime());
    history->doReplayHistory(History::HISTORY_NONE);


    // Then undo the rewind infos going backwards in time
    // --------------------------------------------------
    m_is_rewinding = true;
    m_rewind_queue.undoUntil(rewind_time);

    // Rewind the required state(s)
    // ----------------------------
    World *world = World::getWorld();
    float current_time = world->getTime();

    // Get the (first) full state to which we have to rewind
    RewindInfoState *state =
                     dynamic_cast<RewindInfoState*>(m_rewind_queue.getNext());

    // Store the time to which we have to replay to
    float exact_rewind_time = state->getTime();

    // Now start the rewind with the full state:
    world->setTime(exact_rewind_time);
    float local_physics_time = state->getLocalPhysicsTime();
    Physics::getInstance()->getPhysicsWorld()->setLocalTime(local_physics_time);

    // Restore all states from the current time - the full state of a race
    // will be potentially stored in several state objects. State can be NULL
    // if the next event is not a state
    while(state && state->getTime()==exact_rewind_time)
    {
        state->rewind();
        ++m_rewind_queue;
        if(!m_rewind_queue.hasMoreRewindInfo()) break;
        state = dynamic_cast<RewindInfoState*>(m_rewind_queue.getNext());
    }

    // Now go forward through the list of rewind infos:
    // ------------------------------------------------
    // Need to exit loop if in-game menu is open, since world clock
    // will not be increased while the game is paused
    while( world->getTime() < current_time &&
           World::getWorld()->getPhase()!=WorldStatus::IN_GAME_MENU_PHASE)
    {
        // Now handle all states and events at the current time before
        // updating the world:
        if (m_rewind_queue.hasMoreRewindInfo())
        {
            RewindInfo *ri = m_rewind_queue.getNext();
            while (ri->getTime() <= world->getTime())
            {
                if (ri->isState())
                {
                    // TOOD: replace the old state with a new state. 
                    // For now just set it to confirmed
                    ri->setConfirmed(true);
                }
                else if (ri->isEvent())
                {
                    ri->rewind();
                }

                ++m_rewind_queue;
            }   // while ri->getTime() <= world->getTime()
        }   // if m_rewind_queue.hasMoreRewindInfo()

        float dt = m_rewind_queue.determineNextDT(current_time);
        world->updateWorld(dt);
#undef SHOW_ROLLBACK
#ifdef SHOW_ROLLBACK
        irr_driver->update(dt);
#endif
        world->updateTime(dt);

    }
    m_is_rewinding = false;
    Log::info("RewindManager", "Rewind from %f to %f finished at %f.",
              rewind_time, World::getWorld()->getTime(),
              StkTime::getRealTime());
}   // rewindTo

// ----------------------------------------------------------------------------