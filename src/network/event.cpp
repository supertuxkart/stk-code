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
#include "network/network_manager.hpp"

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

    std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
    m_peer = NULL;
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        if (peers[i]->m_peer == event->peer)
        {
            m_peer = peers[i];
            Log::verbose("Event", "The peer you sought has been found on %p",
                         m_peer);
            return;
        }
    }
    if (m_peer == NULL) // peer does not exist, create him
    {
        STKPeer* new_peer = new STKPeer();
        new_peer->m_peer = event->peer;
        m_peer = new_peer;
        Log::debug("Event", 
                   "Creating a new peer, address are STKPeer:%p, Peer:%p",
                    new_peer, event->peer);
    }
}   // Event(ENetEvent)

// ----------------------------------------------------------------------------
/** \brief Constructor
 *  \param event : The event to copy.
 */
Event::Event(const Event& event)
{
    m_type   = event.m_type;
    m_packet = NULL;
    m_data   = event.m_data;
    m_peer   = event.m_peer;
}   // Event(Event)

// ----------------------------------------------------------------------------
/** \brief Destructor that frees the memory of the package.
 */
Event::~Event()
{
    delete m_peer;
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

