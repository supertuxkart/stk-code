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

/** \brief Constructor
 *  Sets the basic protocol parameters, as the callback object and the
 *  protocol type.
 *  \param callback_object The callback object that will be used by the
 *          protocol. Protocols that do not use callback objects must set
 *          it to NULL.
 *  \param type The type of the protocol.
 */
Protocol::Protocol(CallbackObject* callback_object, ProtocolType type)
{
    m_callback_object = callback_object;
    m_type            = type;
    m_state           = PROTOCOL_STATE_INITIALISING;
    m_id              = 0;
}   // Protocol

// ----------------------------------------------------------------------------
/** \brief Destructor.
 */
Protocol::~Protocol()
{
}   // ~Protocol

// ----------------------------------------------------------------------------
bool Protocol::checkDataSizeAndToken(Event* event, int minimum_size)
{
    const NetworkString &data = event->data();
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
}   // checkDataSizeAndToken

// ----------------------------------------------------------------------------
bool Protocol::isByteCorrect(Event* event, int byte_nb, int value)
{
    const NetworkString &data = event->data();
    if (data[byte_nb] != value)
    {
        Log::info("Protocol", "Bad byte at pos %d. %d "
                "should be %d", byte_nb, data[byte_nb], value);
        return false;
    }
    return true;
}   // isByteCorrect

// ----------------------------------------------------------------------------
/** Starts a request in the protocol manager to start this protocol. 
 */
void Protocol::requestStart()
{
    ProtocolManager::getInstance()->requestStart(this);
}   // requestStart

// ----------------------------------------------------------------------------
/** Submits a request to the ProtocolManager to terminate this protocol.
 */
void Protocol::requestTerminate()
{
    ProtocolManager::getInstance()->requestTerminate(this);
}   // requestTerminate

// ----------------------------------------------------------------------------
void Protocol::sendMessageToPeersChangingToken(NetworkString prefix,
                                               NetworkString message)
{
    std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        prefix.ai8(4).ai32(peers[i]->getClientServerToken());
        prefix += message;
        ProtocolManager::getInstance()->sendMessage(this, peers[i], prefix);
    }
}   // sendMessageToPeersChangingToken

// ----------------------------------------------------------------------------
void Protocol::sendMessage(const NetworkString& message, bool reliable)
{
    ProtocolManager::getInstance()->sendMessage(this, message, reliable);
}   // sendMessage

// ----------------------------------------------------------------------------
void Protocol::sendMessage(STKPeer* peer, const NetworkString& message,
                           bool reliable)
{
    ProtocolManager::getInstance()->sendMessage(this, peer, message, reliable);
}   // sendMessage
