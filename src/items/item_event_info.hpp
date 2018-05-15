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

#ifndef HEADER_ITEM_EVENT_INFO_HPP
#define HEADER_ITEM_EVENT_INFO_HPP

#include "items/item.hpp"
#include "utils/vec3.hpp"

#include <assert.h>

class BareNetworkString;

// ------------------------------------------------------------------------
/** This class stores a delta, i.e. an item event (either collection of
 *  an item, adding a new item, or an item switch being activated). All
 *  those deltas will be applied to the confirmed state to get a new state.
 */
class ItemEventInfo
{
private:
    /** Time at which this event happens. */
    int m_ticks;

    /** Index of this item in the item list. Only used when creating
     *  new items (e.g. bubble gum). */
    int m_index;

    /** The kart id that collected an item if >=0; if -1 it indicates
     *  a new item, and a -2 indicates a switch being used. */
    int m_kart_id;

    /** In case of new items the position of the new item. */
    Vec3 m_xyz;

public:
    /** Constructor for collecting an existing item.
     *  \param ticks Time of the event.
     *  \param item_id The index of the item that was collected.
     *  \param kart_id the kart that collected the item. */
    ItemEventInfo(int ticks, int index, int kart_id)
        : m_ticks(ticks), m_index(index), m_kart_id(kart_id)
    {
        assert(kart_id >= 0);
    }   // ItemEventInfo(collected existing item)

    // --------------------------------------------------------------------
    /** Constructor for creating a new item (i.e. a bubble gum is dropped).
     */
    ItemEventInfo(int ticks, ItemState::ItemType type, int index,
        const Vec3 &xyz)
        : m_ticks(ticks), m_index(index), m_kart_id(-1), m_xyz(xyz)
    {
    }   // ItemEventInfo(new item)
    // --------------------------------------------------------------------
    /** Constructor for switching items. */
    ItemEventInfo(int ticks) : m_ticks(ticks), m_kart_id(-2)
    {
    }   // ItemEventInfo(switch)
    // --------------------------------------------------------------------
    ItemEventInfo(BareNetworkString *buffer, int *count);
    // --------------------------------------------------------------------
    void saveState(BareNetworkString *buffer);

    /** Returns if this event represents a new item. */
    bool isNewItem() const { return m_kart_id == -1; }
    // --------------------------------------------------------------------
    /** Returns true if this event represents collection of an item. */
    bool isItemCollection() const { return m_kart_id >= 0; }
    // --------------------------------------------------------------------
    /** Returns true if this event represent a switch usage. */
    bool isSwitch() const { return m_kart_id == -2; }
    // --------------------------------------------------------------------
    /** Returns the index of this item. */
    int getIndex() const { return m_index; }
    // --------------------------------------------------------------------
    /** Returns the time of the event in ticks. */
    int getTicks() const { return m_ticks; }
    // --------------------------------------------------------------------
    /** Returns the id of the kart that collected an item. Only allowed
     *  to be called when this event is an item collection. */
    int getKartId() const
    {
        assert(isItemCollection());
        return m_kart_id;
    }   // getKartId
    // --------------------------------------------------------------------
    /** Returns the location of a new item. Only allowed to be called when
     *  this is a new item event. */
    const Vec3& getXYZ() const
    {
        assert(isNewItem());
        return m_xyz;
    }   // getXYZ

};   // class ItemEventInfo


#endif
