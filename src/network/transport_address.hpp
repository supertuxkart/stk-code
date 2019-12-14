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

/*! \file types.hpp
 *  \brief Declares the general types that are used by the network.
 */
#ifndef HEADER_TRANSPORT_ADDRESS_HPP
#define HEADER_TRANSPORT_ADDRESS_HPP

#include "utils/types.hpp"

#include "enet/enet.h"

#include <string>

// ============================================================================
/*! \class TransportAddress
 *  \brief Describes a transport-layer address.
 *  For IP networks, a transport address is the couple ip:port.
 */
class TransportAddress
{
private:
    uint32_t m_ip;    //!< The IPv4 address
    uint16_t m_port;  //!< The port number

public:
    /** Constructor. */
    TransportAddress(uint32_t ip = 0, uint16_t port = 0)
    {
        m_ip = ip;
        m_port = port;
    }   // TransportAddress

    // ------------------------------------------------------------------------
    TransportAddress(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4,
                     uint16_t port=0)
    {
        m_ip   = (b1 << 24) + (b2 << 16) + (b3 << 8) + b4;
        m_port = port;
    }   // TransportAddress(uint8_t,...)

    // ------------------------------------------------------------------------
    /** Construct an transport address from an ENetAddress. */
    TransportAddress(const ENetAddress &a)
    {
        m_ip   = htonl(a.host);
        m_port = a.port;
    }   // TransportAddress(EnetAddress)

    // ------------------------------------------------------------------------
    TransportAddress(const std::string& str, uint16_t port_number);
    // ------------------------------------------------------------------------
    TransportAddress(const std::string& str);
    // ------------------------------------------------------------------------
    ~TransportAddress() {}
    // ------------------------------------------------------------------------
    static void unitTesting();
public:
    // ------------------------------------------------------------------------
    static TransportAddress fromDomain(const std::string& str);
    // ------------------------------------------------------------------------
    bool isPublicAddressLocalhost() const;
    // ------------------------------------------------------------------------
    bool isLAN() const;
    // ------------------------------------------------------------------------
    bool isUnset() const { return m_ip == 0 || m_port == 0; }
    // ------------------------------------------------------------------------
    /** Resets ip and port to 0. */
    void clear()
    {
        m_ip   = 0;
        m_port = 0;
    }   // clear

    // ------------------------------------------------------------------------
    /** Returns the ip address. */
    uint32_t getIP() const { return m_ip; }

    // ------------------------------------------------------------------------
    /** Returns the port number. */
    uint16_t getPort() const { return m_port;  }

    // ------------------------------------------------------------------------
    /** Sets the ip address. */
    void setIP(uint32_t ip) { m_ip = ip;  }

    // ------------------------------------------------------------------------
    /** Set the port. */
    void setPort(uint16_t port) { m_port = port; }

    // ------------------------------------------------------------------------
    /** Converts the address to an enet address. */
    ENetAddress toEnetAddress() const
    {
        ENetAddress a;
        // because ENet wants little endian
        a.host = ((m_ip & 0xff000000) >> 24)
               + ((m_ip & 0x00ff0000) >> 8)
               + ((m_ip & 0x0000ff00) << 8)
               + ((m_ip & 0x000000ff) << 24);
        a.port = m_port;
        return a;
    }   // toEnetAddress

    // ------------------------------------------------------------------------
    /** Compares if ip address and port are identical. */
    bool operator==(const TransportAddress& other) const
    {
        return other.m_ip == m_ip && other.m_port == m_port;
    }   // operator==

    // ------------------------------------------------------------------------
    bool operator==(const ENetAddress& other)
    {
        return other.host == ntohl(m_ip) && other.port == m_port;
    }
    // ------------------------------------------------------------------------
    /** Compares if ip address or port are different. */
    bool operator!=(const TransportAddress& other) const
    {
        return other.m_ip != m_ip || other.m_port != m_port;
    }   // operator!=
    // ------------------------------------------------------------------------
    std::string toString(bool show_port = true) const;
};   // TransportAddress

#endif // TYPES_HPP
