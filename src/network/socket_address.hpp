//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2020 SuperTuxKart-Team
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
#ifndef HEADER_SOCKET_ADDRESS_HPP
#define HEADER_SOCKET_ADDRESS_HPP

#include "utils/types.hpp"

#ifdef WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <netdb.h>
#  include <netinet/in.h>
#  include <sys/socket.h>
#endif
#include <enet/enet.h>

#include <array>
#include <cstring>
#include <string>
#include <vector>

// ============================================================================
/*! \class SocketAddress
 *  \brief Describes a IPv4 or IPv6 address in sockaddr_in(6) format, suitable
 *  in using with sendto.
 */
class SocketAddress
{
private:
    std::array<char, sizeof(sockaddr_storage)> m_sockaddr;

    short m_family;
public:
    // ------------------------------------------------------------------------
    static bool g_ignore_error_message;
    // ------------------------------------------------------------------------
    static void unitTesting();
    // ------------------------------------------------------------------------
    SocketAddress(uint32_t ip = 0, uint16_t port = 0);
    // ------------------------------------------------------------------------
    /** IPv4 Constructor (4 bytes). */
    SocketAddress(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4,
                  uint16_t port = 0);
    // ------------------------------------------------------------------------
    SocketAddress(const std::string& str, uint16_t port_number = 0,
                  short family = AF_UNSPEC)
    {
        init(str, port_number, family);
    }
    // ------------------------------------------------------------------------
    SocketAddress(const ENetAddress& ea);
    // ------------------------------------------------------------------------
    bool operator==(const SocketAddress& other) const;
    // ------------------------------------------------------------------------
    bool operator!=(const SocketAddress& other) const;
    // ------------------------------------------------------------------------
    void init(const std::string& str, uint16_t port_number = 0,
              short family = AF_UNSPEC);
    // ------------------------------------------------------------------------
    bool isLAN() const;
    // ------------------------------------------------------------------------
    bool isUnset() const
    {
        if (getPort() == 0)
            return true;
        if (m_family == AF_INET && getIP() == 0)
            return true;
        return false;
    }
    // ------------------------------------------------------------------------
    /** Resets ip and port to 0. */
    void clear()
    {
        m_family = AF_INET;
        m_sockaddr.fill(0);
        sockaddr_in* in = (sockaddr_in*)m_sockaddr.data();
        in->sin_family = AF_INET;
    }   // clear
    // ------------------------------------------------------------------------
    uint32_t getIP() const;
    // ------------------------------------------------------------------------
    uint16_t getPort() const;
    // ------------------------------------------------------------------------
    void setIP(uint32_t ip);
    // ------------------------------------------------------------------------
    void setPort(uint16_t port);
    // ------------------------------------------------------------------------
    std::string toString(bool show_port = true) const;
    // ------------------------------------------------------------------------
    sockaddr* getSockaddr() const      { return (sockaddr*)m_sockaddr.data(); }
    // ------------------------------------------------------------------------
    socklen_t getSocklen() const
    {
        if (m_family == AF_INET)
            return sizeof(sockaddr_in);
        else if (m_family == AF_INET6)
            return sizeof(sockaddr_in6);
        return 0;
    }
    // ------------------------------------------------------------------------
    void setSockAddrIn(short family, sockaddr* new_sockaddr, socklen_t len)
    {
        m_family = family;
        m_sockaddr.fill(0);
        memcpy(m_sockaddr.data(), new_sockaddr, len);
    }
    // ------------------------------------------------------------------------
    short getFamily() const                                { return m_family; }
    // ------------------------------------------------------------------------
    void setIPv6(const uint8_t* bytes, uint16_t port = 0)
    {
        m_family = AF_INET6;
        m_sockaddr.fill(0);
        sockaddr_in6* in6 = (sockaddr_in6*)m_sockaddr.data();
        in6->sin6_family = AF_INET6;
        memcpy(in6->sin6_addr.s6_addr, bytes, 16);
        setPort(port);
    }
    // ------------------------------------------------------------------------
    bool isPublicAddressLocalhost() const;
    // ------------------------------------------------------------------------
    /** Return true if this is an IPv6 address, if it's an IPv4 mapped IPv6
     *  address it will return false (::ffff:x.y.x.w). */
    bool isIPv6() const
    {
        if (m_family == AF_INET6)
        {
            if (getIP() != 0)
                return false;
            return true;
        }
        return false;
    }
    // ------------------------------------------------------------------------
    bool isLoopback() const;
    // ------------------------------------------------------------------------
    void convertForIPv6Socket(bool ipv6);
    // ------------------------------------------------------------------------
    ENetAddress toENetAddress() const;
};   // SocketAddress

#endif // HEADER_SOCKET_ADDRESS_HPP
