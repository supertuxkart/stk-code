//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Joerg Henrichs
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

#include "network/transport_address.hpp"
#include "network/stk_ipv6.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#ifdef WIN32
#  include <iphlpapi.h>
#else
#  include <sys/ioctl.h>
#  include <net/if.h>
#  include <string.h>
#  include <errno.h>
#endif

#if defined(WIN32)
#  include "ws2tcpip.h"
#  define inet_ntop InetNtop
#else
#  include <arpa/inet.h>
#  include <errno.h>
#  include <sys/socket.h>
#endif

#ifdef __MINGW32__
#  undef _WIN32_WINNT
#  define _WIN32_WINNT 0x501
#endif

#ifdef WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <netdb.h>
#endif
#include <sys/types.h>

// ----------------------------------------------------------------------------
/* Return TransportAddress (with optionally port) from string, it can also be
 * used with an ipv4 string too. */
TransportAddress TransportAddress::fromDomain(const std::string& str)
{
    TransportAddress result;
    auto split = StringUtils::split(str, ':');
    if (split.empty())
        return result;
    struct addrinfo hints;
    struct addrinfo* res = NULL;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo_compat(split[0].c_str(),
        split.size() > 1 ? split[1].c_str() : NULL, &hints, &res);
    if (status != 0 || res == NULL)
    {
        Log::error("TransportAddress", "Error in getaddrinfo for fromDomain"
            " %s: %s", split[0].c_str(), gai_strerror(status));
        return result;
    }
    struct sockaddr_in* ipv4_addr = (struct sockaddr_in*)(res->ai_addr);
    result.setIP(ntohl(ipv4_addr->sin_addr.s_addr));
    result.setPort(ntohs(ipv4_addr->sin_port));
    freeaddrinfo(res);
    return result;
}   // fromDomain

// ----------------------------------------------------------------------------
/** Construct an IO address from a string in the format x.x.x.x with a
 *  port number. */
TransportAddress::TransportAddress(const std::string& str, uint16_t port_number)
{
    std::vector<uint32_t> ip = StringUtils::splitToUInt(str, '.');
    m_ip   = 0;
    m_port = 0;
    if (ip.size() >= 4)
        m_ip = (ip[0] << 24) + (ip[1] << 16) + (ip[2] << 8) + ip[3];

    m_port = port_number;
}   // TransportAddress(string of ip, port number)

// ----------------------------------------------------------------------------
/** Construct an IO address from a string in the format x.x.x.x:x. */
TransportAddress::TransportAddress(const std::string& str)
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

// ----------------------------------------------------------------------------
/** Returns if this IP address belongs to a LAN, i.e. is in 192.168* or
 *  10*, 172.16-31.*, or is on the same host, i.e. 127* or same public address.
 */
bool TransportAddress::isLAN() const
{
    uint32_t ip = getIP();
    if (ip >> 16 == 0xc0a8)         // Check for 192.168.*
        return true;
    else if (ip >> 20 == 0xac1 )    // 172.16-31.*
        return true;
    else if (ip >> 24 == 0x0a  )    // 10.*
        return true;
    else if (ip >> 24 == 0x7f  )    // 127.* localhost
        return true;
    return false;
}

// ----------------------------------------------------------------------------
/** Returns this IP address is localhost (127.0.0.1).
 */
bool TransportAddress::isPublicAddressLocalhost() const
{
#ifndef WIN32
    char buffer[2048] = {};
    struct ifconf ifc;
    ifc.ifc_len = sizeof(buffer);
    ifc.ifc_buf = (caddr_t)buffer;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        Log::error("TransportAddress","create socket failed, errno "
            "= %s (%d)\n", strerror(errno), errno);
        return false;
    }

    if (ioctl(fd, SIOCGIFCONF, &ifc) < 0)
    {
        if (fd >= 0)
            close(fd);
        Log::error("TransportAddress", "ioctl SIOCGIFCONF failed, "
            "errno = %s (%d)\n", strerror(errno), errno);
        return false;
    }
    bool is_local_host = false;
    for (int i = 0; i < ifc.ifc_len; i += sizeof(struct ifreq))
    {
        struct ifreq *ifr = (struct ifreq*)(buffer + i);
        if (ifr->ifr_addr.sa_family != AF_INET)
        {
            // only support IPv4
            continue;
        }
        struct sockaddr_in *addr = (struct sockaddr_in*)&ifr->ifr_addr;
        if (ntohl(addr->sin_addr.s_addr) == getIP())
        {
            is_local_host = true;
            break;
        }
    }
    if (fd >= 0 && close(fd) != 0)
    {
        Log::error("TransportAddress", "close fd %d failed, errno "
            "= %s (%d)\n", strerror(errno), errno);
    }
    return is_local_host;

#else
    // Query the list of all IP addresses on the local host. First call to
    // GetIpAddrTable with 0 bytes buffer will return insufficient buffer
    // error, and size will contain the number of bytes needed for all data.
    // Repeat the process of querying the size using GetIpAddrTable in a while
    // loop since it can happen that an interface comes online between the
    // previous call to GetIpAddrTable and the next call.
    MIB_IPADDRTABLE *table = NULL;
    unsigned long size = 0;
    int error = GetIpAddrTable(table, &size, 0);
    // Also add a count to limit the while loop - in case that something
    // strange is going on.
    int count = 0;
    while (error == ERROR_INSUFFICIENT_BUFFER && count < 10)
    {
        delete[] table;   // deleting NULL is legal
        table = (MIB_IPADDRTABLE*)new char[size];
        error = GetIpAddrTable(table, &size, 0);
        count++;
    }   // while insufficient buffer
    for (unsigned int i = 0; i < table->dwNumEntries; i++)
    {
        unsigned int ip = ntohl(table->table[i].dwAddr);
        if (getIP() == ip) // this interface is ours
        {
            delete[] table;
            return true;
        }
    }
    delete[] table;
    return false;
#endif
}   // isPublicAddressLocalhost

// ----------------------------------------------------------------------------
/** Unit testing. Test various LAN patterns to verify that isLAN() works as
 *  expected.
 */
void TransportAddress::unitTesting()
{
    TransportAddress t1("192.168.0.0");
    assert(t1.getIP() == (192u << 24) + (168u << 16));
    assert(t1.isLAN());

    TransportAddress t2("192.168.255.255");
    assert(t2.getIP() == (192u << 24) + (168u << 16) + (255u << 8) + 255u);
    assert(t2.isLAN());

    TransportAddress t3("193.168.0.1");
    assert(t3.getIP() == (193u << 24) + (168u << 16) + 1);
    assert(!t3.isLAN());

    TransportAddress t4("192.167.255.255");
    assert(t4.getIP() == (192u << 24) + (167u << 16) + (255u << 8) + 255u);
    assert(!t4.isLAN());

    TransportAddress t5("192.169.0.0");
    assert(t5.getIP() == (192u << 24) + (169u << 16));
    assert(!t5.isLAN());

    TransportAddress t6("172.16.0.0");
    assert(t6.getIP() == (172u << 24) + (16u << 16));
    assert(t6.isLAN());

    TransportAddress t7("172.31.255.255");
    assert(t7.getIP() == (172u << 24) + (31u << 16) + (255u << 8) + 255u);
    assert(t7.isLAN());

    TransportAddress t8("172.15.255.255");
    assert(t8.getIP() == (172u << 24) + (15u << 16) + (255u << 8) + 255u);
    assert(!t8.isLAN());

    TransportAddress t9("172.32.0.0");
    assert(t9.getIP() == (172u << 24) + (32u << 16));
    assert(!t9.isLAN());

    TransportAddress t10("10.0.0.0");
    assert(t10.getIP() == (10u << 24));
    assert(t10.isLAN());

    TransportAddress t11("10.255.255.255");
    assert(t11.getIP() == (10u << 24) + (255u << 16) + (255u << 8) + 255u);
    assert(t11.isLAN());

    TransportAddress t12("9.255.255.255");
    assert(t12.getIP() == (9u << 24) + (255u << 16) + (255u << 8) + 255u);
    assert(!t12.isLAN());

    TransportAddress t13("11.0.0.0");
    assert(t13.getIP() == (11u << 24));
    assert(!t13.isLAN());

    TransportAddress t14("127.0.0.0");
    assert(t14.getIP() == (127u << 24));
    assert(t14.isLAN());

    TransportAddress t15("127.255.255.255");
    assert(t15.getIP() == (127u << 24) + (255u << 16) + (255u << 8) + 255u);
    assert(t15.isLAN());

    TransportAddress t16("126.255.255.255");
    assert(t16.getIP() == (126u << 24) + (255u << 16) + (255u << 8) + 255u);
    assert(!t16.isLAN());

    TransportAddress t17("128.0.0.0");
    assert(t17.getIP() == (128u << 24));
    assert(!t17.isLAN());

    // Test constructors
    TransportAddress t18("128.0.0.0");
    assert(t18.getIP() == (128u << 24));
    assert(t18.getPort() == 0);

    TransportAddress t19("128.0.0.0:1");
    assert(t19.getIP() == (128u << 24));
    assert(t19.getPort() == 1);

    TransportAddress t20("128.0.0.0", 123);
    assert(t20.getIP() == (128u << 24));
    assert(t20.getPort() == 123);

}   // unitTesting

// ----------------------------------------------------------------------------
/** Returns a std::string representing the ip address and port in human
 *  readable format.
 *  \param show_port True if the port should be shown as well, otherwise
 *         only the ip address will be returned.
 */
std::string TransportAddress::toString(bool show_port) const
{
    std::string s = 
        StringUtils::insertValues("%d.%d.%d.%d",
                                 ((m_ip >> 24) & 0xff), ((m_ip >> 16) & 0xff),
                                 ((m_ip >>  8) & 0xff), ((m_ip >>  0) & 0xff));
    if (show_port)
        s += StringUtils::insertValues(":%d", m_port);
    return s;
}   // toString
