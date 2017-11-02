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
    m_arrival_time = (double)StkTime::getTimeSinceEpoch();

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
        m_data = new NetworkString(event->packet->data, 
                                   (int)event->packet->dataLength);
    }
    else
        m_data = NULL;

    if (event->packet)
    {
        // we got all we need, just remove the data.
        enet_packet_destroy(event->packet);
    }

    m_peer = STKHost::get()->getPeer(event->peer);
    if(m_type == EVENT_TYPE_MESSAGE && m_peer->isClientServerTokenSet() &&
        m_data->getToken()!=m_peer->getClientServerToken() )
    {
        Log::error("Event", "Received event with invalid token!");
        Log::error("Event", "HostID %d Token %d message token %d",
            m_peer->getHostId(), m_peer->getClientServerToken(),
            m_data->getToken());
        Log::error("Event", m_data->getLogMessage().c_str());
    }
}   // Event(ENetEvent)

// ----------------------------------------------------------------------------
/** \brief Destructor that frees the memory of the package.
 */
Event::~Event()
{
    // Do not delete m_peer, it's a pointer to the enet data structure
    // which is persistent.
    m_peer = NULL;
    delete m_data;
}   // ~Event

