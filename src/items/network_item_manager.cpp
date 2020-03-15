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
#include "network/network_string.hpp"
#include "network/protocols/game_protocol.hpp"
#include "network/rewind_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"

bool NetworkItemManager::m_network_item_debugging = false;
// ============================================================================
/** Creates a new instance of the item manager. This is done at startup
 *  of each race.
 *  We must save the item state first (so that it is restored first), otherwise
 *  state updates for a kart could be overwritten by e.g. simulating the item
 *  collection later (which resets bubblegum counter), so a rewinder uid of
 *  "I" which is less than "Kx" (kart rewinder with id x)
 */
NetworkItemManager::NetworkItemManager()
                  : Rewinder({RN_ITEM_MANAGER}), ItemManager()
{
    m_confirmed_switch_ticks = -1;
    m_last_confirmed_item_ticks.clear();
    initServer();
}   // NetworkItemManager

//-----------------------------------------------------------------------------
/** If this is a server, initializing the peers in game
*/
void NetworkItemManager::initServer()
{
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
}   // initServer

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
    m_confirmed_switch_ticks = -1;
    ItemManager::reset();
}   // reset

//-----------------------------------------------------------------------------
/** Called when a kart collects an item. In network games only the server
 *  acts on this event.
 *  \param item The item that was collected.
 *  \param kart The kart that collected the item.
 */
void NetworkItemManager::collectedItem(ItemState *item, AbstractKart *kart)
{
    if (m_network_item_debugging)
        Log::info("NIM", "collectedItem at %d index %d type %d ttr %d",
                  World::getWorld()->getTicksSinceStart(),
                  item->getItemId(), item->getType(), item->getTicksTillReturn());

    if(NetworkConfig::get()->isServer())
    {
        ItemManager::collectedItem(item, kart);
        // The server saves the collected item as item event info
        m_item_events.lock();
        m_item_events.getData().emplace_back(World::getWorld()->getTicksSinceStart(),
                                             item->getItemId(),
                                             kart->getWorldKartId(),
                                             item->getTicksTillReturn());
        m_item_events.unlock();
    }
    else
    {
        // The client predicts item collection:
        ItemManager::collectedItem(item, kart);
    }
}   // collectedItem

// ----------------------------------------------------------------------------
/** Called when a switch is activated. On the server adds this information to
 *  the item state so it can be sent to all clients.
 */
void NetworkItemManager::switchItems()
{
    if (NetworkConfig::get()->isServer())
    {
        // The server saves the collected item as item event info
        m_item_events.lock();
        // Create a switch event - the constructor called determines
        // the type of the event automatically.
        m_item_events.getData()
                     .emplace_back(World::getWorld()->getTicksSinceStart());
        m_item_events.unlock();
    }
    ItemManager::switchItems();
}   // switchItems

// ----------------------------------------------------------------------------
/** Called when a new item is created, e.g. bubble gum.
 *  \param type Type of the item.
 *  \param kart In case of a dropped item used to avoid that a kart
 *         is affected by its own items.
 *  \param server_xyz In case of rewind the server's position of this item.
 *  \param server_normal In case of rewind the server's normal of this item.
 */
Item* NetworkItemManager::dropNewItem(ItemState::ItemType type,
                                      const AbstractKart *kart,
                                      const Vec3 *server_xyz,
                                      const Vec3 *server_normal)
{
    Item *item = ItemManager::dropNewItem(type, kart, server_xyz, server_normal);

    if(!item) return NULL;

    // Nothing else to do for client
    if (NetworkConfig::get()->isClient()) return item;

    assert(!server_xyz);
    // Server: store the data for this event:
    m_item_events.lock();
    m_item_events.getData().emplace_back(World::getWorld()->getTicksSinceStart(),
                                         type, item->getItemId(),
                                         kart->getWorldKartId(),
                                         item->getXYZ(),
                                         item->getNormal());
    m_item_events.unlock();
    return item;
}   // dropNewItem

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
    std::unique_lock<std::mutex> ul(m_live_players_mutex);
    // Peer may get removed earlier if peer request to go back to lobby
    if (m_last_confirmed_item_ticks.find(peer) !=
        m_last_confirmed_item_ticks.end() &&
        ticks > m_last_confirmed_item_ticks.at(peer))
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
    ul.unlock();

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
    int new_ticks = World::getWorld()->getTicksSinceStart() + ticks;
    World::getWorld()->setTicksForRewind(new_ticks);

    for(auto &i : m_confirmed_state)
    {
        if (i) i->update(ticks);
    }   // for m_all_items
    if(m_switch_ticks>ticks)
        m_switch_ticks -= ticks;
    else if (m_switch_ticks >= 0)
    {
        switchItemsInternal(m_confirmed_state);
        m_switch_ticks = -1;
    }
}   // forwardTime

//-----------------------------------------------------------------------------
/** Restores the state of the items to the current world time. It takes the
 *  last saved confirmed state, applies any updates from the server, and
 *  then syncs up the confirmed state to the in-race items.
 *  It uses exactly 'count' bytes of the message.
 *  \param buffer the state content.
 *  \param count Number of bytes used for this state.
 */
void NetworkItemManager::restoreState(BareNetworkString *buffer, int count)
{
    assert(NetworkConfig::get()->isClient());
    // The state at World::getTicksSinceStart() needs to be restored. The
    // confirmed state saved here was taken at m_confirmed_state_time
    // (which is before getTicksSinceStart()). So first forward the confirmed
    // state to the time we need to rewind to (i.e. getTicksSinceStart()).
    // Then copy the new confirmed state to the current items. Any new
    // items (dropped bubblegums) that have been predicted on a client
    // and not been confirmed by the server data will be removed automatically
    // at this stage. In more detail:
    //
    // 1) Apply all events included in this state to the confirmed state.
    //    It is possible that the server inludes events that have happened
    //    before the confirmed time on this client (the server keeps on 
    //    sending all event updates till all clients have confirmed that
    //    that they have received them!!) - which will simply be ignored/
    //    This phase will only act on the confirmed ItemState in the
    //    NetworkItemManager, nothing in the ItemManager will be changed.
    //    a) When a collection event is found, adjust the confirmed item state
    //       only (this state will later be copied to the current items).
    //       It still call collected() in the item, which will update e.g.
    //       the previous owner, use up counter etc. for that item.
    //    b) When a new item is created, create an ItemState instance for
    //       this item, and put it in the confirmed state. Make sure the
    //       same index position is used.
    //    c) If a switch is used, this will be recorded in
    //       m_confirmed_switch_ticks.
    //
    // 2) Inform the server that those item events have been received.
    //    Once the server has received confirmation from all clients 
    //    for events, they will not be resent anymore.
    // 
    // 3) Once all new events have been applied to the confirmed state the
    //    time must be <= world time. Forward the confirmed state to 
    //    world time (i.e. all confirmed items will get their ticksTillReturn
    //    value updated), and update m_confirmed_state_time to the world time.
    //
    // 4) Finally update the ItemManager state from the confirmed state:
    //    Any items that exist in both data structures will be updated.
    //    If an item exist in the ItemManager but not in the confirmed state
    //    of the NetworkItemManager, the item in the item manager will be 
    //    delete - it was a predict item, which has not been confirmed by
    //    the server (e.g. a kart drops a bubble gum, and either the server
    //    has not received that event yet to confirm it, or perhaps the
    //    server detects that the kart did not even have a bubble gum).
    //    Similary, if an item is not in the confirmed state anymore,
    //    but in the ItemManager, it can mean that an item was collected
    //    on the server, which was not predicted. The item in the ItemManager
    //    will be deleted (if a kart has collected this item, the kart state
    //    will include the changed state information for the kart, so no need
    //    to update the kart for this).
    //
    // From here the replay can happen.


    // 1) Apply all events to current confirmed state:
    // -----------------------------------------------
	World *world = World::getWorld();

	// The world clock was set by the RewindManager to be the time
	// of the state we are rewinding to. But the confirmed state of
    // the network manager is before this (we are replaying item events
    // since the last confirmed state in order to get a new confirmed state).
	// So we need to reset the clock to the time of the confirmed state,
    // and then forward this time accordingly. Getting the world time right
    // during forwarding the item state is important since e.g. the bubble
    // gum torque depends on the time. If the world time would be incorrect
    // at the time the collection event happens, the torque would be
    // predicted incorrectly, resulting in stuttering.

    int current_time = m_confirmed_state_time;
    // Reset the ItemManager's switch ticks to the value it had a the
    // confirmed time:
    m_switch_ticks     = m_confirmed_switch_ticks;
    int rewind_to_time = world->getTicksSinceStart();   // Save time we rewind to
    world->setTicksForRewind(current_time);
    bool has_state     = count > 0;

    // Note that the actual ItemManager states must NOT be changed here, only
    // the confirmed states in the Network manager are allowed to be modified.
    // They will all be copied to the ItemManager states after the loop.
    while(count > 0)
    {
        // 1.1) Decode the event in the message
        // ------------------------------------
        ItemEventInfo iei(buffer, &count);
        if(m_network_item_debugging)
            Log::info("NIM", "Rewindto %d current %d iei.index %d iei tick %d iei.coll %d iei.new %d iei.ttr %d confirmed %lx",
                      rewind_to_time, current_time,
                      iei.getIndex(),
                      iei.getTicks(), iei.isItemCollection(), iei.isNewItem(),
                      iei.getTicksTillReturn(),
                      iei.getIndex() < (int)m_confirmed_state.size() && iei.getIndex() != -1 ?
                      m_confirmed_state[iei.getIndex()] : NULL);
        // 1.2) If the event needs to be applied, forward
        //      the time to the time of this event:
        // ----------------------------------------------
        int dt = iei.getTicks() - current_time;

        // Skip any events that are 'in the past' (i.e. have been sent again by
        // the server because it has not yet received confirmation from all
        // clients).
        if(dt<0) continue;

        // Forward the saved state:
        if (dt>0) forwardTime(dt);

        if(iei.isItemCollection())
        {
            int index = iei.getIndex();
            // An item on the track was collected:
            AbstractKart *kart = world->getKart(iei.getKartId());

            assert(m_confirmed_state[index] != NULL);
            m_confirmed_state[index]->collected(kart); // Collect item
            // Reset till ticks return from state (required for eating banana with bomb)
            int ttr = iei.getTicksTillReturn();
            m_confirmed_state[index]->setTicksTillReturn(ttr);

            if (m_confirmed_state[index]->isUsedUp())
            {
                delete m_confirmed_state[index];
                m_confirmed_state[index] = NULL;
            }
        }
        else if(iei.isNewItem())
        {
            AbstractKart *kart = world->getKart(iei.getKartId());
            ItemState *is = new ItemState(iei.getNewItemType(), kart,
                                          iei.getIndex()             );
            is->initItem(iei.getNewItemType(), iei.getXYZ(), iei.getNormal());
            if (m_switch_ticks >= 0)
            {
                ItemState::ItemType new_type = m_switch_to[is->getType()];
                is->switchTo(new_type);
            }

            // A new confirmed item must either be inserted at the end of all
            // items, or in an existing unused entry.
            if (m_confirmed_state.size() <= is->getItemId())
            {
                // In case that the server should send items in the wrong
                // order, e.g. it sends an item for index n+2, then the item
                // for index n -> we might need to add NULL item states
                // into the state array to make sure the indices are correct.
                while(m_confirmed_state.size()<is->getItemId())
                    m_confirmed_state.push_back(NULL);
                m_confirmed_state.push_back(is);
            }
            else
            {
                // If the new item has an already existing index,
                // the slot in the confirmed state array must be free
                assert(m_confirmed_state[is->getItemId()] == NULL);
                m_confirmed_state[is->getItemId()] = is;
            }
        }
        else if(iei.isSwitch())
        {
            // Switch all confirmed items:
            ItemManager::switchItemsInternal(m_confirmed_state);
        }
        else
        {
            Log::error("NetworkItemManager",
                       "Received unknown event type at %d",
                       iei.getTicks());
        }
        current_time = iei.getTicks();
    }   // while count >0


    // 2. Update Server 
    // ================
    // Inform the server which events have been received (if there has
    // been any updates - no need to send messages if nothing has changed)

    if (has_state)
    {
        if (auto gp = GameProtocol::lock())
            gp->sendItemEventConfirmation(world->getTicksSinceStart());
    }

    // 3. Forward the confirmed item state to the world time
    // =====================================================
    int dt = rewind_to_time - current_time;
    if(dt>0) forwardTime(dt);

    // 4. Copy the confirmed state to the current item state
    // ======================================================
    
    // We need to test all items - and confirmed or all_items could
    // be the larger group (confirmed: when a new item was dropped
    // by a remote kart; all_items: if an item is predicted on
    // the client, but not yet confirmed). So 
    size_t max_index = std::max(m_confirmed_state.size(),
                                      m_all_items.size()        );
    m_all_items.resize(max_index, NULL);

    for(unsigned int i=0; i<max_index; i++)
    {
        ItemState *item     = m_all_items[i];
        const ItemState *is = i < m_confirmed_state.size() 
                            ? m_confirmed_state[i] : NULL;
        // For every *(ItemState*)item = *is, all deactivated ticks, item id
        // ... will be copied from item state to item
        if (is && item)
        {
            *(ItemState*)item = *is;
        }
        else if (is && !item)
        {
            // A new item was dropped according to the server that is not
            // yet part of the current state --> create new item
            Vec3 xyz = is->getXYZ();
            Vec3 normal = is->getNormal();
            Item *item_new = dropNewItem(is->getType(), is->getPreviousOwner(),
                                         &xyz, &normal );
            *((ItemState*)item_new) = *is;
            m_all_items[i] = item_new;
            insertItemInQuad(item_new);
        }
        else if (!is && item)
        {
            deleteItemInQuad(item);
            delete item;
            m_all_items[i] = NULL;
        }
    }   // for i < max_index
    // Clean up the rest
    m_all_items.resize(m_confirmed_state.size());

    // Now set the clock back to the 'rewindto' time:
    world->setTicksForRewind(rewind_to_time);

    // Save the current local time as confirmed time
    m_confirmed_state_time   = world->getTicksSinceStart();
    m_confirmed_switch_ticks = m_switch_ticks;
}   // restoreState

//-----------------------------------------------------------------------------
/** Save all current items at current ticks in server for live join
 */
void NetworkItemManager::saveCompleteState(BareNetworkString* buffer) const
{
    const uint32_t all_items = (uint32_t)m_all_items.size();
    buffer->addUInt32(World::getWorld()->getTicksSinceStart())
        .addUInt32(m_switch_ticks).addUInt32(all_items);
    for (unsigned i = 0; i < all_items; i++)
    {
        if (m_all_items[i])
        {
            buffer->addUInt8(1);
            m_all_items[i]->saveCompleteState(buffer);
        }
        else
            buffer->addUInt8(0);
    }
}   // saveCompleteState

//-----------------------------------------------------------------------------
/** Restore all current items at current ticks in client for live join
 *  or at the start of a race.
 */
void NetworkItemManager::restoreCompleteState(const BareNetworkString& buffer)
{
    m_confirmed_state_time = buffer.getUInt32();
    m_confirmed_switch_ticks = buffer.getUInt32();
    uint32_t all_items = buffer.getUInt32();
    for (ItemState* is : m_confirmed_state)
    {
        delete is;
    }
    m_confirmed_state.clear();
    for (unsigned i = 0; i < all_items; i++)
    {
        const bool has_item = buffer.getUInt8() == 1;
        if (has_item)
        {
            ItemState* is = new ItemState(buffer);
            m_confirmed_state.push_back(is);
        }
        else
            m_confirmed_state.push_back(NULL);
    }
}   // restoreCompleteState
