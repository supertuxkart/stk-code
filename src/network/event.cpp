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

#include "network/crypto.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/stk_peer.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"

#include <string.h>

/** \brief Constructor
 *  \param event : The event that needs to be translated.
 */

// ============================================================================
constexpr bool isConnectionRequestPacket(unsigned char* data, size_t length)
{
    // Connection request is not synchronous
    return length < 2 ? false : (uint8_t)data[0] == PROTOCOL_LOBBY_ROOM &&
        (uint8_t)data[1] == LobbyProtocol::LE_CONNECTION_REQUESTED;
}   // isConnectionRequestPacket

// ============================================================================
Event::Event(ENetEvent* event, std::shared_ptr<STKPeer> peer)
{
    m_arrival_time = StkTime::getMonoTimeMs();
    m_pdi = PDI_TIMEOUT;
    m_peer = peer;

    switch (event->type)
    {
    case ENET_EVENT_TYPE_CONNECT:
        m_type = EVENT_TYPE_CONNECTED;
        break;
    case ENET_EVENT_TYPE_DISCONNECT:
        m_type = EVENT_TYPE_DISCONNECTED;
        m_pdi = (PeerDisconnectInfo)event->data;
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
        if (!m_peer->isValidated() && !isConnectionRequestPacket
            (event->packet->data, event->packet->dataLength))
        {
            throw std::runtime_error("Invalid packet before validation.");
        }

        auto cl = LobbyProtocol::get<ClientLobby>();
        if (event->channelID == EVENT_CHANNEL_UNENCRYPTED && (!cl ||
            (cl && !cl->waitingForServerRespond())))
        {
            throw std::runtime_error("Unencrypted content at wrong state.");
        }
        if (m_peer->getCrypto() && (event->channelID == EVENT_CHANNEL_NORMAL ||
            event->channelID == EVENT_CHANNEL_DATA_TRANSFER))
        {
            m_data = m_peer->getCrypto()->decryptRecieve(event->packet);
        }
        else
        {
            m_data = new NetworkString(event->packet->data, 
                (int)event->packet->dataLength);
        }
    }
    else
        m_data = NULL;

    if (event->packet)
    {
        // we got all we need, just remove the data.
        enet_packet_destroy(event->packet);
    }

}   // Event(ENetEvent)

// ----------------------------------------------------------------------------
/** \brief Destructor that frees the memory of the package.
 */
Event::~Event()
{
    delete m_data;
}   // ~Event

