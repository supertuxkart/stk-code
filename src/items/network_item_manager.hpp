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

#include "items/item_manager.hpp"
#include "network/rewinder.hpp"
#include "utils/cpp2011.hpp"
#include "utils/synchronised.hpp"

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

    /** Time at which m_confirmed_state was taken. */
    int m_confirmed_state_time;

    /** Stores on the server the latest confirmed tick from each client. */
    Synchronised< std::vector<int> > m_last_confirmed_item_ticks;

    // ------------------------------------------------------------------------
    /** This class stores a delta, i.e. an item event (either collection of
     *  an item, adding a new item, or an item switch being activated). All
     *  those deltas will be applied to the confirmed state to get a new state.
     */
    class ItemEventInfo
    {
    public:
        /** Time at which this event happens. */
        int m_ticks;

        /** Index of this item in the item list. Only used when creating
         *  new items (e.g. bubble gum). */
        int m_index;

        /** Additional info about the item: -1 if a switch is activated.
         *  Otherwise it contains information about what item the kart gets
         *  (e.g. banana --> anchor or bomb). */
        int m_item_info;

        /** The kart id that collected an item ,
        *  otherwise undefined. */
        int m_kart_id;

        /** In case of new items the position of the new item. */
        Vec3 m_xyz;

        /** Constructor for collecting an existing item.
         *  \param ticks Time of the event.
         *  \param item_id The index of the item that was collected.
         *  \param kart_id the kart that collected the item. */
        ItemEventInfo(int ticks, int index, int kart_id, int item_info)
                    : m_ticks(ticks), m_index(index), m_kart_id(kart_id),
                      m_item_info(item_info)
        {
        }   // ItemEventInfo(collected existing item)

        // --------------------------------------------------------------------
        /** Constructor for creating a new item (i.e. a bubble gum is dropped).
         */
        ItemEventInfo(int ticks, ItemState::ItemType type, int index,
                      const Vec3 &xyz)            
        {
            m_ticks = ticks;
            m_index = index;
            m_xyz   = xyz;
        }   // ItemEventInfo(new item)
        // --------------------------------------------------------------------
        /** Constructor for switching items. */
        ItemEventInfo(int ticks) 
                    : m_ticks(ticks), m_kart_id(-1), m_item_info(-1)
        {
        }   // ItemEventInfo(switch)

    };   // class ItemEventInfo

    // ------------------------------------------------------------------------
    /** List of all items events. */
    Synchronised< std::vector<ItemEventInfo> > m_item_events;

    void forwardTime(int ticks);
    virtual unsigned int insertItem(Item *item) OVERRIDE;

    NetworkItemManager();
    virtual ~NetworkItemManager();

public:
    static void create();

    void setSwitchItems(const std::vector<int> &switch_items);
    void sendItemUpdate();
    void saveInitialState();

    virtual void reset();
    virtual void setItemConfirmationTime(int host_id, int ticks) OVERRIDE;
    virtual void collectedItem(Item *item, AbstractKart *kart) OVERRIDE;
    virtual Item* dropNewItem(ItemState::ItemType type,
                              AbstractKart *kart) OVERRIDE;
    virtual BareNetworkString* saveState() OVERRIDE;
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
};   // NetworkItemManager

#endif
