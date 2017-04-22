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

/*! \file stk_peer.hpp
 *  \brief Defines functions to easily manipulate 8-bit network destinated strings.
 */

#ifndef STK_PEER_HPP
#define STK_PEER_HPP

#include "utils/no_copy.hpp"
#include "utils/types.hpp"

#include <enet/enet.h>

#include <vector>

class NetworkPlayerProfile;
class NetworkString;
class TransportAddress;

/*! \class STKPeer
 *  \brief Represents a peer.
 *  This class is used to interface the ENetPeer structure.
 */
class STKPeer : public NoCopy
{
protected:
    /** Pointer to the corresponding ENet peer data structure. */
    ENetPeer* m_enet_peer;

    /** The token of this client. */
    uint32_t m_client_server_token;

    /** True if the token for this peer has been set. */
    bool m_token_set;

    /** Host id of this peer. */
    int m_host_id;

    /** True if this peer is authorised to control a server. */
    bool m_is_authorised;
public:
             STKPeer(ENetPeer *enet_peer);
    virtual ~STKPeer();

    virtual void sendPacket(NetworkString *data,
                            bool reliable = true);
    void disconnect();
    bool isConnected() const;
    bool exists() const;
    uint32_t getAddress() const;
    uint16_t getPort() const;
    bool isSamePeer(const STKPeer* peer) const;
    bool isSamePeer(const ENetPeer* peer) const;
    std::vector<NetworkPlayerProfile*> getAllPlayerProfiles();
    // ------------------------------------------------------------------------
    /** Sets the token for this client. */
    void setClientServerToken(const uint32_t& token)
    {
        m_client_server_token = token; 
        m_token_set = true; 
    }   // setClientServerToken
    // ------------------------------------------------------------------------
    void unsetClientServerToken() { m_token_set = false; }
    // ------------------------------------------------------------------------
    /** Returns the token of this client. */
    uint32_t getClientServerToken() const { return m_client_server_token; }
    // ------------------------------------------------------------------------
    /** Returns if the token for this client is known. */
    bool isClientServerTokenSet() const { return m_token_set; }
    // ------------------------------------------------------------------------
    /** Sets the host if of this peer. */
    void setHostId(int host_id) { m_host_id = host_id; }
    // ------------------------------------------------------------------------
    /** Returns the host id of this peer. */
    int getHostId() const { return m_host_id; }
    // ------------------------------------------------------------------------
    /** Sets if this peer is authorised to control the server. */
    void setAuthorised(bool authorised) { m_is_authorised = authorised; }
    // ------------------------------------------------------------------------
    /** Returns if this peer is authorised to control the server. The server
     *  uses this to check if a peer is allowed certain commands; and a client
     *  uses this function (in which case this peer is actually the server
     *  peer) to see if this client is allowed certain command (i.e. to
     *  display additional GUI elements). */
    bool isAuthorised() const { return m_is_authorised; }
};   // STKPeer

#endif // STK_PEER_HPP
