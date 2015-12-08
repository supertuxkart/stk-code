//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 Joerg Henrichs
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

/*! \file stk_host.hpp
 *  \brief Defines an interface to use network low-level functions easily.
 */
#ifndef HEADER_NETWORK_CONFIG
#define HEADER_NETWORK_CONFIG

#include "network/transport_address.hpp"
#include "utils/synchronised.hpp"

#include "irrString.h"

class NetworkConfig
{
private:
    /** The singleton instance. */
    static NetworkConfig *m_network_config;

    enum NetworkType
    { NETWORK_NONE, NETWORK_WAN, NETWORK_LAN };

    /** Keeps the type of network connection: none (yet), LAN or WAN. */
    NetworkType m_network_type;

    /** True if this host is a server, false otherwise. */
    bool m_is_server;

    /** This is either this computer's public IP address, or the LAN
     *  address in case of a LAN game. With lock since it can
     *  be updated from a separate thread. */
    Synchronised<TransportAddress> m_my_address;

    /** Even if this is a WAN server, we also store the private (LAN)
     *  port number, to allow direct connection to clients on the same
     *  LAN. */
    uint16_t m_private_port;

    /** Maximum number of players on the server. */
    int m_max_players;

    /** If this is a server, it indicates if this server is registered
    *  with the stk server. */
    bool m_is_registered;

    /** If this is a server, the server name. */
    irr::core::stringw m_server_name;

    NetworkConfig();

public:
    /** Singleton get, which creates this object if necessary. */
    static NetworkConfig *get()
    {
        if(!m_network_config)
            m_network_config = new NetworkConfig();
        return m_network_config;
    }   // get

    // ------------------------------------------------------------------------
    static void destroy()
    {
        delete m_network_config;   // It's ok to delete NULL
        m_network_config = NULL;
    }   // destroy

    // ------------------------------------------------------------------------
    void setMyAddress(const TransportAddress& addr);

    // ------------------------------------------------------------------------
    /** Return if a network setting is happening. A network setting is active
     *  if a host (server or client) exists. */
    bool isNetworking() const { return m_network_type!=NETWORK_NONE; }
    // ------------------------------------------------------------------------
    /** Return true if it's a networked game with a LAN server. */
    bool isLAN() const { return m_network_type == NETWORK_LAN; }
    // ------------------------------------------------------------------------
    /** Return true if it's a networked game but with a WAN server. */
    bool isWAN() const { return m_network_type == NETWORK_WAN; }
    // ------------------------------------------------------------------------
    /** Set that this is a LAN networked game. */
    void setIsLAN() { m_network_type = NETWORK_LAN; }
    // ------------------------------------------------------------------------
    /** Set that this is a WAN networked game. */
    void setIsWAN() { m_network_type = NETWORK_WAN; }
    // ------------------------------------------------------------------------
    /** Set that this is not a networked game. */
    void unsetNetworking() { m_network_type = NETWORK_NONE; }
    // ------------------------------------------------------------------------
    /** Sets the maximum number of players for this server. */
    void setMaxPlayers(int n) { m_max_players = n; }
    // --------------------------------------------------------------------
    /** Returns the maximum number of players for this server. */
    int getMaxPlayers() const { return m_max_players; }
    // --------------------------------------------------------------------
    /** Sets if this instance is a server or client. */
    void setIsServer(bool b) { m_is_server = b; }
    // --------------------------------------------------------------------
    /** Returns if this instance is a server. */
    bool isServer() const { return m_is_server;  }
    // --------------------------------------------------------------------
    /** Returns if this instance is a client. */
    bool isClient() const { return !m_is_server; }
    // --------------------------------------------------------------------
    /** Sets the name of this server. */
    void setServerName(const irr::core::stringw &name)
    {
        m_server_name = name;
    }   // setServerName
    // --------------------------------------------------------------------
    /** Returns the server name. */
    const irr::core::stringw& getServerName() const
    {
        assert(isServer());
        return m_server_name;
    }   // getServerName
    // --------------------------------------------------------------------
    /** Sets if this server is registered with the stk server. */
    void setRegistered(bool registered)
    {
        assert(isServer());
        m_is_registered = registered; 
    }   // setRegistered
    // --------------------------------------------------------------------
    /** Returns if this server is registered with the stk server. */
    bool isRegistered() const
    {
        assert(isServer());
        return m_is_registered;
    }   // isRegistered

    // --------------------------------------------------------------------
    /** Returns the IP address of this host. We need to return a copy
     *  to make sure the address is thread safe (otherwise it could happen
     *  that e.g. data is taken when the IP address was written, but not
     *  yet the port). */
    const TransportAddress getMyAddress() const
    {
        TransportAddress a;
        m_my_address.lock();
        a.copy(m_my_address.getData());
        m_my_address.unlock();
        return a;
    }   // getMyAddress
    // ------------------------------------------------------------------------
    /** Sets the private (LAN) port for this instance. */
    void setPrivatePort(uint16_t port) { m_private_port = port; }
    // ------------------------------------------------------------------------
    /** Returns the private (LAN) port. */
    uint16_t getPrivatePort() const { return m_private_port; }

};   // class NetworkConfig

#endif // HEADER_NETWORK_CONFIG
