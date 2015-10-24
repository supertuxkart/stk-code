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

#include "network/network_manager.hpp"

#include "network/event.hpp"
#include "network/game_setup.hpp"
#include "network/protocol_manager.hpp"
#include "utils/log.hpp"

#include <pthread.h>
#include <signal.h>

//-----------------------------------------------------------------------------

NetworkManager::NetworkManager()
{
}   // NetworkManager

//-----------------------------------------------------------------------------

NetworkManager::~NetworkManager()
{
    ProtocolManager::kill();

    STKHost::destroy();
    while(!m_peers.empty())
    {
        delete m_peers.back();
        m_peers.pop_back();
    }
}   // ~Networkmanager

//----------------------------------------------------------------------------
/** \brief Function to start the Network Manager (start threads).
 */
void NetworkManager::run()
{
}   // run

//-----------------------------------------------------------------------------
/** \brief Function to reset the Network Manager.
 *  This function resets the peers and the listening host.
 */
void NetworkManager::reset()
{
    STKHost::destroy();
    while(!m_peers.empty())
    {
        delete m_peers.back();
        m_peers.pop_back();
    }
}   // reset

//-----------------------------------------------------------------------------
/** \brief Function that aborts the NetworkManager.
 *  This function will stop the listening, delete the host and stop
 *  threads that are related to networking.
 */
void NetworkManager::abort()
{
    STKHost::get()->stopListening();
    // FIXME: Why a reset here? This creates a new stk_host, which will open
    // a new packet_log file (and therefore delete the previous file)???
    // reset();
    ProtocolManager::getInstance()->abort();
}   // abort

//-----------------------------------------------------------------------------
/** \brief Try to establish a connection to a given transport address.
 *  \param peer : The transport address which you want to connect to.
 *  \return True if we're successfully connected. False elseway.
 */
bool NetworkManager::connect(const TransportAddress& address)
{
    if (peerExists(address))
        return isConnectedTo(address);

    return STKPeer::connectToHost(STKHost::get(), address, 2, 0);
}   // connect

//-----------------------------------------------------------------------------
/** Is called from STKHost when an event (i.e. a package) is received. If the
 *  event indicates a new connection, the peer is added to the list of peers.
 *  It logs the package, and propagates the event to the ProtocollManager,
 *  which in turn will notify individual protocols.
 *  \param event Pointer to the event to propagate.
 */
void NetworkManager::propagateEvent(Event* event)
{
    Log::verbose("NetworkManager", "EVENT received of type %d",
                 (int)(event->getType()));
    STKPeer* peer = event->getPeer();
    if (event->getType() == EVENT_TYPE_CONNECTED)
    {
        Log::info("NetworkManager", "A client has just connected. There are "
                  "now %lu peers.", m_peers.size() + 1);
        Log::debug("NetworkManager", "Addresses are : %lx, %lx",
                   event->getPeer(), peer);
        // create the new peer:
        m_peers.push_back(peer);
    }
    if (event->getType() == EVENT_TYPE_MESSAGE)
    {
        uint32_t addr = peer->getAddress();
        Log::verbose("NetworkManager",
                     "Message, Sender : %i.%i.%i.%i, message = \"%s\"",
                    ((addr>>24)&0xff),
                    ((addr>>16)&0xff),
                    ((addr>>8)&0xff),
                    (addr & 0xff), event->data().std_string().c_str());

    }

    // notify for the event now.
    ProtocolManager::getInstance()->propagateEvent(event);
}   // propagateEvent

//-----------------------------------------------------------------------------

void NetworkManager::sendPacket(STKPeer* peer, const NetworkString& data,
                                bool reliable)
{
    if (peer)
        peer->sendPacket(data, reliable);
}   // sendPacket

//-----------------------------------------------------------------------------

void NetworkManager::sendPacketExcept(STKPeer* peer, const NetworkString& data,
                                      bool reliable)
{
    for (unsigned int i = 0; i < m_peers.size(); i++)
    {
        STKPeer* p = m_peers[i];
        if (!p->isSamePeer(peer))
        {
            p->sendPacket(data, reliable);
        }
    }
}   // sendPacketExcept


//-----------------------------------------------------------------------------
/** Called when you leave a server.
 */
void NetworkManager::disconnected()
{

    // remove all peers
    for (unsigned int i = 0; i < m_peers.size(); i++)
    {
        delete m_peers[i];
        m_peers[i] = NULL;
    }
    m_peers.clear();
}   // disconnected

//-----------------------------------------------------------------------------

void NetworkManager::removePeer(STKPeer* peer)
{
    if (!peer || !peer->exists()) // peer does not exist (already removed)
        return;
    Log::debug("NetworkManager", "Disconnected host: %i.%i.%i.%i:%i",
               peer->getAddress()>>24&0xff,
               peer->getAddress()>>16&0xff,
               peer->getAddress()>>8&0xff,
               peer->getAddress()&0xff,
               peer->getPort());
    // remove the peer:
    bool removed = false;
    for (unsigned int i = 0; i < m_peers.size(); i++)
    {
        if (m_peers[i]->isSamePeer(peer) && !removed) // remove only one
        {
            delete m_peers[i];
            m_peers.erase(m_peers.begin()+i, m_peers.begin()+i+1);
            Log::verbose("NetworkManager",
                         "The peer has been removed from the Network Manager.");
            removed = true;
        }
        else if (m_peers[i]->isSamePeer(peer))
        {
            Log::fatal("NetworkManager",
                       "Multiple peers match the disconnected one.");
        }
    }
    if (!removed)
        Log::warn("NetworkManager", "The peer that has been disconnected was "
                                    "not registered by the Network Manager.");

    Log::info("NetworkManager",
              "Somebody is now disconnected. There are now %lu peers.",
              m_peers.size());
}   // removePeer

//-----------------------------------------------------------------------------
