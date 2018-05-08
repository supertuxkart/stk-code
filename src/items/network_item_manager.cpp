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
    m_last_confirmed_event = 0;
    ItemManager::reset();
}   // reset

//-----------------------------------------------------------------------------
/** Copies the initial state at the start of a race as confirmed state.
 */
void NetworkItemManager::saveInitialState()
{
    m_confirmed_state_time = 0;
    m_last_confirmed_event = 0;

    m_confirmed_state.clear();
    for(auto i : m_all_items)
    {
        m_confirmed_state.emplace_back(*i);
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
        m_item_events.emplace_back(item->getType(), item->getItemId(), 
                                   World::getWorld()->getTimeTicks(),
                                   kart->getWorldKartId()             );
        ItemManager::collectedItem(item, kart, add_info);
    }
}   // collectedItem

//-----------------------------------------------------------------------------
void NetworkItemManager::setItemConfirmationTime(int host_id, int ticks)
{
    m_last_confirmed_item_ticks.lock();
    if (ticks > m_last_confirmed_item_ticks.getData()[host_id])
        m_last_confirmed_item_ticks.getData()[host_id] = ticks;
    m_last_confirmed_item_ticks.unlock();
}   // setItemConfirmationTime

//-----------------------------------------------------------------------------
BareNetworkString* NetworkItemManager::saveState()
{
    if(NetworkConfig::get()->isClient())
    {
        saveInitialState();
        return NULL;
    }

    // On the server:
    // ==============
    // First discard unneeded events, i.e. all events that have
    // been confirmed by all clients:
    int min_time = World::getWorld()->getTimeTicks() + 1;
    m_last_confirmed_item_ticks.lock();
    for (auto i : m_last_confirmed_item_ticks.getData())
        if (i < min_time) min_time = i;
    m_last_confirmed_item_ticks.unlock();

    auto p = m_item_events.begin();
    while (p != m_item_events.end() && p->m_ticks < min_time)
        p++;
    m_item_events.erase(m_item_events.begin(), p);

    uint16_t n = (uint16_t)m_item_events.size();
    if(n==0)
    {
        BareNetworkString *s =
            new BareNetworkString();
        return s;
    }

    BareNetworkString *s = 
        new BareNetworkString(n * (sizeof(int) + sizeof(uint16_t) +
                                               + sizeof(uint8_t)   )  );
    for (auto p : m_item_events)
    {
        s->addTime(p.m_ticks).addUInt16(p.m_item_id);
        if (p.m_item_id > -1) s->addUInt8(p.m_kart_id);
    }
    Log::verbose("NIM", "Including %d item update at %d",
                 n, World::getWorld()->getTimeTicks());
    return s;
}   // saveState

//-----------------------------------------------------------------------------
/** Restores a state, using exactly 'count' bytes of the message. 
 *  \param buffer the state content.
 *  \param count Number of bytes used for this state.
 */
void NetworkItemManager::restoreState(BareNetworkString *buffer, int count) 
{
    while(count > 0)
    {
        int ticks   = buffer->getTime();
        int item_id = buffer->getUInt16();
        int kart_id = -1;
        count -= 6;
        if(item_id>-1)
        {
            // Not a global event, so we have a kart id
            kart_id = buffer->getUInt8();
            count --;
        }   // item_id>-1
        // This event has already been received, and can be ignored now.
        if(ticks <= m_last_confirmed_event) continue;
        
        // Now we certainly have a new event that needs to be added:
        m_item_events.emplace_back(m_all_items[item_id]->getType(), item_id,
                                   ticks, kart_id                           );
        Log::info("NIM", "Received new event at %d", ticks);
    }   // while count >0

    // At le
    if(!m_item_events.empty())
        m_last_confirmed_event = m_item_events.back().m_ticks;
    m_last_confirmed_event = std::max(World::getWorld()->getTimeTicks(), 
                                      m_last_confirmed_event             );
    GameProtocol::lock()->sendItemEventConfirmation(m_last_confirmed_event);
}   // undoState

//-----------------------------------------------------------------------------
void NetworkItemManager::rewindToEvent(BareNetworkString *bns)
{
}
//-----------------------------------------------------------------------------
void NetworkItemManager::restoreStateAt(int ticks)
{
    if (ticks < m_confirmed_state_time)
    {
        Log::error("ItemManager",
            "Trying to restore state at t %d, but confirmed state is from %d.",
            ticks, m_confirmed_state_time);
        // Nothing much we can do about this - it should never happeb
        m_confirmed_state_time = ticks;
    }


    for(auto i : m_confirmed_state)
    {

      Item *it = m_all_items[i.m_item_id];
    }

}   // restoreStateAt