//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

#include "network/event.hpp"

#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "utils/log.hpp"

#include <string.h>

/** \brief Constructor
 *  \param event : The event that needs to be translated.
 */
Event::Event(ENetEvent* event)
{
    switch (event->type)
    {
    case ENET_EVENT_TYPE_CONNECT:
        m_type = EVENT_TYPE_CONNECTED;
        break;
    case ENET_EVENT_TYPE_DISCONNECT:
        m_type = EVENT_TYPE_DISCONNECTED;
        break;
    case ENET_EVENT_TYPE_RECEIVE:
        m_type = EVENT_TYPE_MESSAGE;
        break;
    case ENET_EVENT_TYPE_NONE:
        return;
        break;
    }
    if (m_type == EVENT_TYPE_MESSAGE)
    {
        m_data = NetworkString(std::string((char*)(event->packet->data),
                               event->packet->dataLength-1));
    }

    m_packet = NULL;
    if (event->packet)
    {
        m_packet = event->packet;
        // we got all we need, just remove the data.
        enet_packet_destroy(m_packet);
    }
    m_packet = NULL;

    m_peer = STKHost::get()->getPeer(event->peer);
}   // Event(ENetEvent)

// ----------------------------------------------------------------------------
/** \brief Destructor that frees the memory of the package.
 */
Event::~Event()
{
    // Do not delete m_peer, it's a pointer to the enet data structure
    // which is persistent.
    m_peer = NULL;
    m_packet = NULL;
}   // ~Event

// ----------------------------------------------------------------------------
/** \brief Remove bytes at the beginning of data.
 *  \param size : The number of bytes to remove.
 */
void Event::removeFront(int size)
{
    m_data.removeFront(size);
}   // removeFront

