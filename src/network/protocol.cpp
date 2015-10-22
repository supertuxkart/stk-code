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

#include "network/protocol.hpp"

#include "network/event.hpp"
#include "network/network_manager.hpp"
#include "network/protocol_manager.hpp"

Protocol::Protocol(CallbackObject* callback_object, ProtocolType type)
{
    m_callback_object = callback_object;
    m_type = type;
}

Protocol::~Protocol()
{
}

void Protocol::pause()
{
    m_listener->requestPause(this);
}
void Protocol::unpause()
{
    m_listener->requestUnpause(this);
}

void Protocol::kill()
{
}

void Protocol::setListener(ProtocolManager* listener)
{
    m_listener = listener;
}

ProtocolType Protocol::getProtocolType()
{
    return m_type;
}

bool Protocol::checkDataSizeAndToken(Event* event, int minimum_size)
{
    NetworkString data = event->data();
    if (data.size() < minimum_size || data[0] != 4)
    {
        Log::warn("Protocol", "Receiving a badly "
                  "formated message. Size is %d and first byte %d",
                  data.size(), data[0]);
        return false;
    }
    STKPeer* peer = event->getPeer();
    uint32_t token = data.gui32(1);
    if (token != peer->getClientServerToken())
    {
        Log::warn("Protocol", "Peer sending bad token. Request "
                  "aborted.");
        return false;
    }
    return true;
}

bool Protocol::isByteCorrect(Event* event, int byte_nb, int value)
{
    NetworkString data = event->data();
    if (data[byte_nb] != value)
    {
        Log::info("Protocol", "Bad byte at pos %d. %d "
                "should be %d", byte_nb, data[byte_nb], value);
        return false;
    }
    return true;
}

void Protocol::sendMessageToPeersChangingToken(NetworkString prefix, NetworkString message)
{
    std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        prefix.ai8(4).ai32(peers[i]->getClientServerToken());
        prefix += message;
        m_listener->sendMessage(this, peers[i], prefix);
    }
}
