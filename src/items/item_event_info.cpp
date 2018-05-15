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

#include "items/item_event_info.hpp"

#include "network/network_config.hpp"
#include "network/protocols/game_protocol.hpp"
#include "network/rewind_manager.hpp"
#include "network/stk_host.hpp"


/** Loads an event from a server message. It helps encapsulate the encoding
 *  of events from and into a message buffer.
 *  \param buffer A network string with the event data.
 *  \param count The number of bytes read will be subtracted from this value.
 */
ItemEventInfo::ItemEventInfo(BareNetworkString *buffer, int *count)
{
    m_ticks   = buffer->getTime();
    m_kart_id = buffer->getUInt8();
    m_index   = buffer->getUInt16();
    *count -= 7;
}   // ItemEventInfo(BareNetworkString, int *count)

//-----------------------------------------------------------------------------
/** Stores this event into a network string.
 *  \param buffer The network string to which the data should be appended.
 */
void ItemEventInfo::saveState(BareNetworkString *buffer)
{
    assert(NetworkConfig::get()->isServer());
    buffer->addTime(m_ticks).addUInt8(m_kart_id).addUInt16(m_index);
    if(isNewItem())
    {
    }
}   // saveState

 
  