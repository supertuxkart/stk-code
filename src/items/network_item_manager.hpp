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

#ifndef HEADER_NETWORK_ITEM_MANAGER_HPP
#define HEADER_NETWORK_ITEM_MANAGER_HPP

#include "items/item_event_info.hpp"
#include "items/item_manager.hpp"
#include "network/rewinder.hpp"
#include "utils/cpp2011.hpp"
#include "utils/synchronised.hpp"

#include <map>
#include <memory>
#include <mutex>

class STKPeer;

/** \ingroup items
 *  The network item manager is responsible for handling all network related
 *  item manager tasks - synchronisation between clients and servers. It 
 *  maintains one 'confirmed' state on the clients, based on the latest
 *  server update. The server sends updates that only contains the delta
 *  between the last confirmed and the current server state. Eash client
 *  confirms to the server which deltas it has received. Once all clients
 *  have received a delta, the server will remove it from the list of 
 *  deltas.
  */
class NetworkItemManager : public Rewinder, public ItemManager
{
private:

    /** A client stores a 'confirmed' item event state, which is based on the
      * server data. This is used in case of rewind. */
    std::vector<ItemState*> m_confirmed_state;

    /** The switch ticks value at the lime of the last confirmed state. */
    int m_confirmed_switch_ticks;

    /** Time at which m_confirmed_state was taken. */
    int m_confirmed_state_time;

    /** Allow remove or add peer live. */
    std::mutex m_live_players_mutex;

    /** Stores on the server the latest confirmed tick from each client. */
    std::map<std::weak_ptr<STKPeer>, int32_t,
        std::owner_less<std::weak_ptr<STKPeer> > > m_last_confirmed_item_ticks;

    /** List of all items events. */
    Synchronised< std::vector<ItemEventInfo> > m_item_events;

    void forwardTime(int ticks);
public:

    static bool m_network_item_debugging;

    NetworkItemManager();
    virtual ~NetworkItemManager();
    virtual void reset() OVERRIDE;
    virtual void setItemConfirmationTime(std::weak_ptr<STKPeer> peer,
                                         int ticks) OVERRIDE;
    virtual void  collectedItem(ItemState *item, AbstractKart *kart) OVERRIDE;
    virtual void  switchItems() OVERRIDE;
    virtual Item* dropNewItem(ItemState::ItemType type,
                              const AbstractKart *kart,
                              const Vec3 *server_xyz = NULL,
                              const Vec3 *server_normal = NULL) OVERRIDE;
    virtual BareNetworkString* saveState(std::vector<std::string>* ru)
        OVERRIDE;
    virtual void restoreState(BareNetworkString *buffer, int count) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void rewindToEvent(BareNetworkString *bns) OVERRIDE {};
    // ------------------------------------------------------------------------
    virtual void saveTransform() OVERRIDE {};
    // ------------------------------------------------------------------------
    virtual void computeError() OVERRIDE {};
    // ------------------------------------------------------------------------
    virtual void undoState(BareNetworkString *buffer) OVERRIDE {};
    // ------------------------------------------------------------------------
    virtual void undoEvent(BareNetworkString*) OVERRIDE {};
    // ------------------------------------------------------------------------
    void addLiveJoinPeer(std::weak_ptr<STKPeer> peer)
    {
        std::lock_guard<std::mutex> lock(m_live_players_mutex);
        m_last_confirmed_item_ticks[peer] = 0;
    }
    // ------------------------------------------------------------------------
    void erasePeerInGame(std::weak_ptr<STKPeer> peer)
    {
        std::lock_guard<std::mutex> lock(m_live_players_mutex);
        m_last_confirmed_item_ticks.erase(peer);
    }
    // ------------------------------------------------------------------------
    void saveCompleteState(BareNetworkString* buffer) const;
    // ------------------------------------------------------------------------
    void restoreCompleteState(const BareNetworkString& buffer);
    // ------------------------------------------------------------------------
    void initServer();

};   // NetworkItemManager

#endif
