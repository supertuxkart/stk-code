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
#include "network/network_string.hpp"
#include "network/protocol_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"

/** \brief Constructor
 *  Sets the basic protocol parameters, as the callback object and the
 *  protocol type.
 *  \param callback_object The callback object that will be used by the
 *          protocol. Protocols that do not use callback objects must set
 *          it to NULL.
 *  \param type The type of the protocol.
 */
Protocol::Protocol(ProtocolType type, CallbackObject* callback_object)
{
    m_callback_object       = callback_object;
    m_type                  = type;
    m_state                 = PROTOCOL_STATE_INITIALISING;
    m_id                    = 0;
    m_handle_connections    = false;
    m_handle_disconnections = false;
}   // Protocol

// ----------------------------------------------------------------------------
/** \brief Destructor.
 */
Protocol::~Protocol()
{
}   // ~Protocol

// ----------------------------------------------------------------------------
/** Returns a network string with the given type.
 *  \capacity Default preallocated size for the message.
 */
NetworkString* Protocol::getNetworkString(size_t capacity)
{
    return new NetworkString(m_type, (int)capacity);
}   // getNetworkString

// ----------------------------------------------------------------------------
/** Checks if the message has at least the specified size, and if not prints
 *  a warning message including the message content.
 *  \return True if the message is long enough, false otherwise.
 */
bool Protocol::checkDataSize(Event* event, unsigned int minimum_size)
{
    const NetworkString &data = event->data();
    if (data.size() < minimum_size)
    {
        Log::warn("Protocol", "Receiving a badly formated message:");
        Log::warn("Protocol", data.getLogMessage().c_str());
        return false;
    }
    return true;
}   // checkDataSize

// ----------------------------------------------------------------------------
/** Starts a request in the protocol manager to start this protocol. 
 */
void Protocol::requestStart()
{
    ProtocolManager::getInstance()->requestStart(this);
}   // requestStart

// ----------------------------------------------------------------------------
/** Submits a request to the ProtocolManager to pause this protocol.
 */
void Protocol::requestPause()
{
    ProtocolManager::getInstance()->requestPause(this);
}   // requestPause

// ----------------------------------------------------------------------------
/** Submits a request to the ProtocolManager to unpause this protocol.
 */
void Protocol::requestUnpause()
{
    ProtocolManager::getInstance()->requestUnpause(this);
}   // requestUnpause

// ----------------------------------------------------------------------------
/** Submits a request to the ProtocolManager to terminate this protocol.
 */
void Protocol::requestTerminate()
{
    ProtocolManager::getInstance()->requestTerminate(this);
}   // requestTerminate

// ----------------------------------------------------------------------------
/** Finds a protocol with the given type and requests it to be terminated.
 *  If no such protocol exist, log an error message.
 *  \param type The protocol type to delete.
 */
void Protocol::findAndTerminateProtocol(ProtocolType type)
{
    Protocol* protocol = ProtocolManager::getInstance()->getProtocol(type);
    if (protocol)
        protocol->requestTerminate();
    else
        Log::error("Protocol", "No protocol %d registered.", type);
}   // findAndTerminateProtocol

// ----------------------------------------------------------------------------
/** Sends a message to all peers, inserting the peer's token into the message.
 *  The message is composed of a 1-byte message (usually the message type)
 *  followed by the token of this client and then actual message).
 *  \param message The actual message content.
*/
void Protocol::sendMessageToPeersChangingToken(NetworkString *message,
                                               bool reliable)
{
    const std::vector<STKPeer*> &peers = STKHost::get()->getPeers();
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        peers[i]->sendPacket(message, reliable);
    }
}   // sendMessageToPeersChangingToken

// ----------------------------------------------------------------------------
/** Sends a message from a client to the server.
 */
void Protocol::sendToServer(NetworkString *message, bool reliable)
{
    STKHost::get()->sendToServer(message, reliable);
}   // sendMessage
