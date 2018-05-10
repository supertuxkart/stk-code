//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 Joerg Henrichs
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

#include "items/network_item_manager.hpp"

#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/protocols/game_protocol.hpp"
#include "network/rewind_manager.hpp"
#include "network/stk_host.hpp"


//-----------------------------------------------------------------------------
/** Creates one instance of the item manager. */
void NetworkItemManager::create()
{
    assert(!m_item_manager);
    m_item_manager = new NetworkItemManager();
}   // create


// ============================================================================
/** Creates a new instance of the item manager. This is done at startup
 *  of each race. */
NetworkItemManager::NetworkItemManager()
                  : Rewinder(/*can be deleted*/false),
                    ItemManager()
{
    m_last_confirmed_item_ticks.lock();
    m_last_confirmed_item_ticks.getData().clear();

    if (NetworkConfig::get()->isServer())
        m_last_confirmed_item_ticks.getData().resize(STKHost::get()->getPeerCount(), 0);

    m_last_confirmed_item_ticks.unlock();
}   // NetworkItemManager

//-----------------------------------------------------------------------------
/** Destructor. Cleans up all items and meshes stored.
 */
NetworkItemManager::~NetworkItemManager()
{
}   // ~NetworkItemManager

//-----------------------------------------------------------------------------
void NetworkItemManager::reset()
{
    ItemManager::reset();
}   // reset

//-----------------------------------------------------------------------------
/** Copies the initial state at the start of a race as confirmed state.
 */
void NetworkItemManager::saveInitialState()
{
    m_confirmed_state_time = 0;

    m_confirmed_state.clear();
    for(auto i : m_all_items)
    {
        ItemState *is = new ItemState(*i);
        m_confirmed_state.push_back(is);
    }
}   // saveInitialState

//-----------------------------------------------------------------------------
/** Called when a kart collects an item. In network games only the server
 *  acts on this event.
 *  \param item The item that was collected.
 *  \param kart The kart that collected the item.
 *  \param add_info 
 */
void NetworkItemManager::collectedItem(Item *item, AbstractKart *kart,
                                       int add_info)
{
    if(NetworkConfig::get()->isServer())
    {
        m_item_events.lock();
        m_item_events.getData().emplace_back(World::getWorld()->getTimeTicks(), 
                                             item->getItemId(),
                                             kart->getWorldKartId(),
                                             /*item_info*/0);
        m_item_events.unlock();
        ItemManager::collectedItem(item, kart, add_info);
    }
    else if (!RewindManager::get()->isRewinding())
    {
        // If we are predicting (i.e. not rewinding), the client
        // predicts item collection:
        ItemManager::collectedItem(item, kart, add_info);
    }
}   // collectedItem

//-----------------------------------------------------------------------------
/** Called by the GameProtocol when a confirmation for an item event is 
 *  received by a host. Once all hosts have confirmed an event, it can be
 *  deleted and won't be send to any clients again.
 *  \param host_id Host identification of the host confirming the latest
 *         event time received.
 *  \param ticks Time at which the last event was received.
 */
void NetworkItemManager::setItemConfirmationTime(int host_id, int ticks)
{
    assert(NetworkConfig::get()->isServer());
    m_last_confirmed_item_ticks.lock();
    if (ticks > m_last_confirmed_item_ticks.getData()[host_id])
        m_last_confirmed_item_ticks.getData()[host_id] = ticks;

    // Now discard unneeded events, i.e. all events that have
    // been confirmed by all clients:
    int min_time = 999999;
    for (auto i : m_last_confirmed_item_ticks.getData())
        if (i < min_time) min_time = i;
    m_last_confirmed_item_ticks.unlock();

    // Find the last entry before the minimal confirmed time.
    // Since the event list is sorted, all events up to this
    // entry can be deleted.
    m_item_events.lock();
    auto p = m_item_events.getData().begin();
    while (p != m_item_events.getData().end() && p->m_ticks < min_time)
        p++;
    m_item_events.getData().erase(m_item_events.getData().begin(), p);
    m_item_events.unlock();

    // TODO: Get informed when a client drops out!!!
}   // setItemConfirmationTime

//-----------------------------------------------------------------------------
/** Saves the state of all items. This is done by using a state that has
 *  been confirmed by all clients as a base, and then only adding any
 *  changes applied to that state later. As clients keep on confirming events
 *  the confirmed event will be moved forward in time, and older events can
 *  be deleted (and not sent to the clients anymore).
 *  This function is also called on the client in the first frame of a race
 *  to save the initial state, which is the first confirmed state by all
 *  clients.
 */
BareNetworkString* NetworkItemManager::saveState()
{
    if(NetworkConfig::get()->isClient())
    {
        saveInitialState();
        return NULL;
    }

    // On the server:
    // ==============
    m_item_events.lock();
    uint16_t n = (uint16_t)m_item_events.getData().size();
    if(n==0)
    {
        BareNetworkString *s =
            new BareNetworkString();
        m_item_events.unlock();
        return s;
    }

    BareNetworkString *s = 
        new BareNetworkString(n * (sizeof(int) + sizeof(uint16_t) +
                                               + sizeof(uint8_t)   )  );
    for (auto p : m_item_events.getData())
    {
        s->addTime(p.m_ticks).addUInt16(p.m_index);
        if (p.m_index > -1) s->addUInt8(p.m_kart_id);
    }
    m_item_events.unlock();
    Log::verbose("NIM", "Including %d item update at %d",
                 n, World::getWorld()->getTimeTicks());
    return s;
}   // saveState

//-----------------------------------------------------------------------------
/** Progresses the time for all item by the given number of ticks. Used
 *  when computing a new state from a confirmed state.
 *  \param ticks Number of ticks that need to be simulated.
 */
void NetworkItemManager::forwardTime(int ticks)
{
    for(auto &i : m_confirmed_state)
    {
        if (i) i->update(ticks);
    }   // for m_all_items
}   // forwardTime

//-----------------------------------------------------------------------------
/** Restores the state of the items to the current world time. It takes the
 *  last saved

 *  using exactly 'count' bytes of the message. 
 *  \param buffer the state content.
 *  \param count Number of bytes used for this state.
 */
void NetworkItemManager::restoreState(BareNetworkString *buffer, int count) 
{
    // The state at World::getTimeTicks() needs to be restored. The confirmed
    // state in this instance was taken at m_confirmed_state_time. So first
    // apply the new events to the confirmed state till we reach the end of
    // all new events (which must be <= getTimeTicks() (since the server will
    // only send events till the specified state time). Then forward the
    // state to getTimeTicks().

    int current_time = m_confirmed_state_time;
    while(count > 0)
    {
        // 1) Decode the event in the message
        // ----------------------------------
        int ticks      = buffer->getTime();
        int item_index = buffer->getUInt16();
        int kart_id    = -1;
        count         -= 6;
        if(item_index>-1)
        {
            kart_id = buffer->getUInt8();
            count --;
        }   // item_id>-1

        // 2) If the event needs to be applied, forward
        //    the time to the time of this event:
        // --------------------------------------------
        int dt = ticks - current_time;
        // Skip an event that are 'in the past' (i.e. have been sent again by
        // the server because it has not yet received confirmation from all
        // clients.
        if(dt<0) continue;

        // Forward the saved state:
        if (dt>0) forwardTime(dt);

        // TODO: apply the various events types, atm only collection is supported:
        ItemState *item_state = m_confirmed_state[item_index];
        if (item_index > -1)
        {
            // An item on the track was collected:
            AbstractKart *kart = World::getWorld()->getKart(kart_id);
            m_confirmed_state[item_index]->collected(kart);
        }

        current_time = ticks;
    }   // while count >0

    // Inform the server which events have been received.
    GameProtocol::lock()->sendItemEventConfirmation(World::getWorld()->getTimeTicks());

    // Forward the confirmed item state till the world time:
    int dt = World::getWorld()->getTimeTicks() - current_time;
    if(dt>0) forwardTime(dt);

    // Restore the state to the current world time:
    // ============================================

    for(unsigned int i=0; i<m_confirmed_state.size(); i++)
    {
        Item *item = m_all_items[i];
        const ItemState *is = m_confirmed_state[i];
        item->setTicksTillReturn(is->getTicksTillReturn());
        *(ItemState*)item = *is;
    }

    // Now we save the current local
    m_confirmed_state_time = World::getWorld()->getTimeTicks();
}   // restoreState

