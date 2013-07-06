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
    default:
        break;
    }
    if (type == EVENT_TYPE_MESSAGE)
        data = std::string((char*)(event->packet->data));
    else if (event->data)
    {
    }
    
    if (event->packet)
        m_packet = event->packet;

    std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
    peer = NULL;
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        if (*peers[i] == event->peer)
        {
            peer = peers[i];
            return;
        }
    }
    if (peer == NULL) // peer does not exist, create him
    {
        STKPeer* new_peer = new STKPeer();
        new_peer->m_peer = event->peer;
        peer = new_peer;
    }
}
    
Event::~Event()
{
    if (m_packet)
        enet_packet_destroy(m_packet);
}

void Event::removeFront(int size)
{
    data.removeFront(size);
}

