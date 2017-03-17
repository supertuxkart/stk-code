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

#include "utils/no_copy.hpp"
#include "utils/string_utils.hpp"
#include "utils/types.hpp"

#include "enet/enet.h"

#include <string>

// ============================================================================
/*! \class TransportAddress
 *  \brief Describes a transport-layer address.
 *  For IP networks, a transport address is the couple ip:port.
 */
class TransportAddress : public NoCopy
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
    /** Construct an transport address from an ENetAddress. */
    TransportAddress(const ENetAddress &a)
    {
        m_ip   = htonl(a.host);
        m_port = a.port;
    }   // TransportAddress(EnetAddress)

    // ------------------------------------------------------------------------
    /** Construct an IO address from a string in the format x.x.x.x:xxx. */
    TransportAddress(const std::string& str)
    {
        std::string combined = StringUtils::replace(str, ":", ".");
        std::vector<uint32_t> ip = StringUtils::splitToUInt(combined, '.');
        m_ip   = 0;
        m_port = 0;
        if (ip.size() >= 4)
            m_ip = (ip[0] << 24) + (ip[1] << 16) + (ip[2] << 8) + ip[3];

        if(ip.size()==5)
            m_port = (uint16_t)(ip[4] < 65536 ? ip[4] : 0);
        else
            m_port = 0;
    }   // TransportAddress(string of ip)

    // ------------------------------------------------------------------------
    ~TransportAddress() {}
    // ------------------------------------------------------------------------
    static void unitTesting();
private:
    friend class NetworkConfig;
    /** The copy constructor is private, so that the friend class
     *  NetworkConfig can access it to create a copy (getMyAddress), but
     *  no other class can. */
    TransportAddress(const TransportAddress &other)
    {
        copy(other);
    }   // TransportAddress(const TransportAddress&)
public:
    bool isLAN() const;
    // ------------------------------------------------------------------------
    /** A copy function (to replace the copy constructor which is disabled
     *  using NoCopy): it copies the data from the argument into this object.*/
    void copy(const TransportAddress &other)
    {
        m_ip   = other.m_ip;
        m_port = other.m_port;
    }   // copy

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
    /** Returns a std::string representing the ip address and port in human
     *  readable format.
     *  \param show_port True if the port should be shown as well, otherwise
     *         only the ip address will be returned.
     */
    std::string toString(bool show_port = true) const
    {
        std::string s = 
            StringUtils::insertValues("%d.%d.%d.%d",
                                 ((m_ip >> 24) & 0xff), ((m_ip >> 16) & 0xff),
                                 ((m_ip >>  8) & 0xff), ((m_ip >>  0) & 0xff));
        if (show_port)
            s += StringUtils::insertValues(":%d", m_port);
        return s;
    }   // toString
};   // TransportAddress

#endif // TYPES_HPP
