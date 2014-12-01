//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 SuperTuxKart-Team
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

Event::Event(ENetEvent* event)
{
    switch (event->type)
    {
    case ENET_EVENT_TYPE_CONNECT:
        type = EVENT_TYPE_CONNECTED;
        break;
    case ENET_EVENT_TYPE_DISCONNECT:
        type = EVENT_TYPE_DISCONNECTED;
        break;
    case ENET_EVENT_TYPE_RECEIVE:
        type = EVENT_TYPE_MESSAGE;
        break;
    case ENET_EVENT_TYPE_NONE:
        return;
        break;
    }
    if (type == EVENT_TYPE_MESSAGE)
    {
        m_data = NetworkString(std::string((char*)(event->packet->data), event->packet->dataLength-1));
    }

    m_packet = NULL;
    if (event->packet)
        m_packet = event->packet;

    if (m_packet)
        enet_packet_destroy(m_packet); // we got all we need, just remove the data.
    m_packet = NULL;

    std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
    peer = new STKPeer*;
    *peer = NULL;
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        if (peers[i]->m_peer == event->peer)
        {
            *peer = peers[i];
            Log::verbose("Event", "The peer you sought has been found on %lx", (long int)(peer));
            return;
        }
    }
    if (*peer == NULL) // peer does not exist, create him
    {
        STKPeer* new_peer = new STKPeer();
        new_peer->m_peer = event->peer;
        *peer = new_peer;
        Log::debug("Event", "Creating a new peer, address are STKPeer:%lx, Peer:%lx", (long int)(new_peer), (long int)(event->peer));
    }
}

Event::Event(const Event& event):
    type(event.type),
    // copy the peer
    peer(event.peer),
    m_data(event.m_data),
    m_packet(NULL)
{
}

Event::~Event()
{
    delete peer;
    peer = NULL;
    m_packet = NULL;
}

void Event::removeFront(int size)
{
    m_data.removeFront(size);
}

