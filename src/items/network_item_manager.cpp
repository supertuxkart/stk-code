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
#include "network/stk_peer.hpp"

//-----------------------------------------------------------------------------
/** Creates one instance of the item manager. */
void NetworkItemManager::create()
{
    assert(!m_item_manager);
    auto nim = std::shared_ptr<NetworkItemManager>(new NetworkItemManager());
    nim->rewinderAdd();
    m_item_manager = nim;
}   // create

// ============================================================================
/** Creates a new instance of the item manager. This is done at startup
 *  of each race. */
NetworkItemManager::NetworkItemManager()
                  : Rewinder("N"), ItemManager()
{
    m_last_confirmed_item_ticks.clear();

    if (NetworkConfig::get()->isServer())
    {
        auto peers = STKHost::get()->getPeers();
        for (auto& p : peers)
        {
            if (!p->isValidated() || p->isWaitingForGame())
                continue;
            m_last_confirmed_item_ticks[p] = 0;
        }
    }

}   // NetworkItemManager

//-----------------------------------------------------------------------------
/** Destructor. Cleans up all items and meshes stored.
 */
NetworkItemManager::~NetworkItemManager()
{
    for (ItemState* is : m_confirmed_state)
    {
        delete is;
    }
}   // ~NetworkItemManager

//-----------------------------------------------------------------------------
void NetworkItemManager::reset()
{
    ItemManager::reset();
}   // reset

//-----------------------------------------------------------------------------
/** Initialize state at the start of a race.
 */
void NetworkItemManager::initClientConfirmState()
{
    m_confirmed_state_time = 0;

    m_confirmed_state.clear();
    for(auto i : m_all_items)
    {
        ItemState *is = new ItemState(*i);
        m_confirmed_state.push_back(is);
    }
}   // initClientConfirmState

//-----------------------------------------------------------------------------
/** Called when a kart collects an item. In network games only the server
 *  acts on this event.
 *  \param item The item that was collected.
 *  \param kart The kart that collected the item.
 */
void NetworkItemManager::collectedItem(Item *item, AbstractKart *kart)
{
    if(NetworkConfig::get()->isServer())
    {
        // The server saves the collected item as item event info
        m_item_events.lock();
        m_item_events.getData().emplace_back(World::getWorld()->getTicksSinceStart(),
            item->getItemId(),
            kart->getWorldKartId());
        m_item_events.unlock();
        ItemManager::collectedItem(item, kart);
    }
    else
    {
        // The client predicts item collection:
        ItemManager::collectedItem(item, kart);
    }
}   // collectedItem

// ----------------------------------------------------------------------------
/** Called when a new item is created, e.g. bubble gum.
 *  \param type Type of the item.
 *  \param kart In case of a dropped item used to avoid that a kart
 *         is affected by its own items.
 */
Item* NetworkItemManager::dropNewItem(ItemState::ItemType type,
                                      const AbstractKart *kart, const Vec3 *xyz)
{
    Item *item = ItemManager::dropNewItem(type, kart, xyz);
    if(!item) return NULL;

    if (NetworkConfig::get()->isClient())
    {
        // If this is called when replaying a server event, the calling
        // function restoreState will set the item to be not-predicted.
        item->setPredicted(true);
        return item;
    }

    // Server: store the data for this event:
    m_item_events.lock();
    m_item_events.getData().emplace_back(World::getWorld()->getTicksSinceStart(),
                                         type, item->getItemId(),
                                         kart->getWorldKartId(),
                                         kart->getXYZ() );
    m_item_events.unlock();
    return item;
}   // newItem

// ----------------------------------------------------------------------------
/** Called by the GameProtocol when a confirmation for an item event is
 *  received by the server. Once all hosts have confirmed an event, it can be
 *  deleted and won't be sent to any clients again.
 *  \param peer Peer confirming the latest event time received.
 *  \param ticks Time at which the last event was received.
 */
void NetworkItemManager::setItemConfirmationTime(std::weak_ptr<STKPeer> peer,
                                                 int ticks)
{
    assert(NetworkConfig::get()->isServer());
    if (ticks > m_last_confirmed_item_ticks.at(peer))
        m_last_confirmed_item_ticks.at(peer) = ticks;

    // Now discard unneeded events and expired (disconnected) peer, i.e. all
    // events that have been confirmed by all clients:
    int min_time = std::numeric_limits<int32_t>::max();
    for (auto it = m_last_confirmed_item_ticks.begin();
         it != m_last_confirmed_item_ticks.end();)
    {
        if (it->first.expired())
        {
            it = m_last_confirmed_item_ticks.erase(it);
        }
        else
        {
            if (it->second < min_time) min_time = it->second;
            it++;
        }
    }

    // Find the last entry before the minimal confirmed time.
    // Since the event list is sorted, all events up to this
    // entry can be deleted.
    m_item_events.lock();
    auto p = m_item_events.getData().begin();
    while (p != m_item_events.getData().end() && p->getTicks() < min_time)
        p++;
    m_item_events.getData().erase(m_item_events.getData().begin(), p);
    m_item_events.unlock();

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
BareNetworkString* NetworkItemManager::saveState(std::vector<std::string>* ru)
{
    ru->push_back(getUniqueIdentity());
    // On the server:
    // ==============
    m_item_events.lock();
    uint16_t n = (uint16_t)m_item_events.getData().size();
    if(n==0)
    {
        BareNetworkString *s = new BareNetworkString();
        m_item_events.unlock();
        return s;
    }

    BareNetworkString *s =
        new BareNetworkString(n * (  sizeof(int) + sizeof(uint16_t)
                                   + sizeof(uint8_t)              ) );
    for (auto p : m_item_events.getData())
    {
        p.saveState(s);
    }
    m_item_events.unlock();
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
    assert(NetworkConfig::get()->isClient());
    // The state at World::getTicksSinceStart() needs to be restored. The confirmed
    // state in this instance was taken at m_confirmed_state_time. First
    // forward this confirmed state to the current time (i.e. world time).
    // This is done in several steps:
    // 1) First remove all client-side predicted items from the list of all
    //    items. Predicted item only happen between m_confirmed_state_time
    //    and 'now'.
    // 2) Apply all events included in this state to the confirmed state.
    //    a) When a collection event is found, adjust the confirmed item state
    //       only (this state will later be copied to the current item state).
    //    b) When a new item is created, search in the item cache to see
    //       if a predicted item for this slot already exists to speed up
    //       item creation (if not, create a new item). Put this new item
    //       into the current item state as well as in the confirmed state
    //       (in the same index position).
    // 3) Once all new events have been applied to the confirmed state the
    //    time must be <= world time. Forward the confirmed state to 
    //    world time, and update m_confirmed_state_time to the world time.
    // From here the replay can happen.

    // 1) Remove predicted items:
    for (unsigned int i=0; i<m_all_items.size(); i++)
    {
        Item *item = m_all_items[i];
        if(item && item->isPredicted())
        {
            deleteItem(item);
        }
    }

    // 2) Apply all events to current confirmed state:
    int current_time = m_confirmed_state_time;
    bool has_state = count > 0;
    while(count > 0)
    {
        // 1) Decode the event in the message
        // ----------------------------------
        ItemEventInfo iei(buffer, &count);

        // 2) If the event needs to be applied, forward
        //    the time to the time of this event:
        // --------------------------------------------
        int dt = iei.getTicks() - current_time;
        // Skip an event that are 'in the past' (i.e. have been sent again by
        // the server because it has not yet received confirmation from all
        // clients.
        if(dt<0) continue;

        // Forward the saved state:
        if (dt>0) forwardTime(dt);

        // TODO: apply the various events types, atm only collection is supported:
        if(iei.isItemCollection())
        {
            int index = iei.getIndex();
            // An item on the track was collected:
            AbstractKart *kart = World::getWorld()->getKart(iei.getKartId());
            m_confirmed_state[index]->collected(kart);
            if (m_confirmed_state[index]->isUsedUp())
            {
                delete m_confirmed_state[index];
                m_confirmed_state[index] = NULL;
            }
        }
        else if(iei.isNewItem())
        {
            AbstractKart *kart = World::getWorld()->getKart(iei.getKartId());
            ItemState *is = new ItemState(iei.getNewItemType(), iei.getIndex(),
                                          kart);
            is->initItem(iei.getNewItemType(), iei.getXYZ());
            if (m_confirmed_state.size() <= is->getItemId())
            {
                m_confirmed_state.push_back(is);
            }
            else
            {
                if (m_confirmed_state[is->getItemId()] == NULL)
                    m_confirmed_state[is->getItemId()] = is;
                else
                    *m_confirmed_state[is->getItemId()] = *is;
            }
        }
        current_time = iei.getTicks();
    }   // while count >0

    // Inform the server which events have been received.
    if (has_state)
    {
        if (auto gp = GameProtocol::lock())
            gp->sendItemEventConfirmation(World::getWorld()->getTicksSinceStart());
    }

    // Forward the confirmed item state to the world time:
    int dt = World::getWorld()->getTicksSinceStart() - current_time;
    if(dt>0) forwardTime(dt);

    // Restore the state to the current world time:
    // ============================================

    for(unsigned int i=0; i<m_confirmed_state.size(); i++)
    {
        Item *item = m_all_items[i];
        const ItemState *is = m_confirmed_state[i];
        if (is && item)
            *(ItemState*)item = *is;
        else if (is && !item)
        {
            Vec3 xyz = is->getXYZ();
            Item *item_new = dropNewItem(is->getType(), is->getPreviousOwner(),
                                         &xyz);
            item_new->setPredicted(false);
            item_new->setItemId(i);
            m_all_items[i] = item_new;
            *((ItemState*)m_all_items[i]) = *is;
        }
        else if (!is && item)
        {
            deleteItem(m_all_items[i]);
        }
    }

    // Now we save the current local
    m_confirmed_state_time = World::getWorld()->getTicksSinceStart();
}   // restoreState

