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

#include "network/socket_address.hpp"
#include "network/network_config.hpp"
#include "network/stk_ipv6.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#ifdef WIN32
#  undef _WIN32_WINNT
#  define _WIN32_WINNT 0x600
#  include <iphlpapi.h>
#else
#ifndef __SWITCH__
#  include <ifaddrs.h>
#else
extern "C" {
  #define u64 uint64_t
  #define u32 uint32_t
  #define s64 int64_t
  #define s32 int32_t
  #include <switch/services/nifm.h>
  #undef u64
  #undef u32
  #undef s32
  #undef s64
}
#endif
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

#include <sys/types.h>

// ----------------------------------------------------------------------------
bool SocketAddress::g_ignore_error_message = false;
// ----------------------------------------------------------------------------
/** IPv4 Constructor. */
SocketAddress::SocketAddress(uint32_t ip, uint16_t port)
{
    clear();
    setIP(ip);
    setPort(port);
}   // SocketAddress

// ----------------------------------------------------------------------------
/** IPv4 Constructor (4 bytes). */
SocketAddress::SocketAddress(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4,
                             uint16_t port)
             : SocketAddress((b1 << 24) + (b2 << 16) + (b3 << 8) + b4, port)
{
}   // SocketAddress(uint8_t,...)

// ----------------------------------------------------------------------------
/** ENetAddress constructor. */
SocketAddress::SocketAddress(const ENetAddress& ea)
{
#ifdef ENABLE_IPV6
    if (isIPv6Socket())
    {
        m_family = AF_INET6;
        m_sockaddr = {};
        struct sockaddr_in6* in6 = (struct sockaddr_in6*)m_sockaddr.data();
        in6->sin6_family = AF_INET6;
        in6->sin6_port = htons(ea.port);
        // We modify host to use 5 uint32_t (see enet.h)
        memcpy(in6->sin6_addr.s6_addr, &ea.host.p0, 16);
        in6->sin6_scope_id = ea.host.p4;
    }
    else
    {
        m_family = AF_INET;
        setIP(htonl(ea.host.p0));
        setPort(ea.port);
    }
#else
    m_family = AF_INET;
#ifdef __SWITCH__
    setIP(htonl(ea.host.p0));
#else
    setIP(htonl(ea.host));
#endif
    setPort(ea.port);
#endif
}   // SocketAddress(const ENetAddress&)

// ----------------------------------------------------------------------------
/** String initialization (Can be IPv4, IPv6 or domain).
 *  \param str The address (can have a port with :)
 *  \param port_number The port number, default is 0 or overwritten if
 *  specified in str
 *  \param family AF_UNSPEC, AF_INET or AF_INET6 for example
 */
void SocketAddress::init(const std::string& str, uint16_t port_number,
                         short family)
{
    clear();
    if (str.empty())
    {
        Log::error("SocketAddress", "Empty address is specified.");
        return;
    }

    std::string addr_str;
    std::string port_str;

    bool only_1_colon = false;
    size_t colon_pos = std::string::npos;
    // Check if only 1 colon, if so then it's either domain:port or IPv4:port
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == ':')
        {
            if (colon_pos == std::string::npos)
            {
                colon_pos = i;
                only_1_colon = true;
            }
            else
                only_1_colon = false;
        }
    }
    if (only_1_colon)
    {
        addr_str = std::string(str, 0, colon_pos);
        if (colon_pos + 1 < str.size())
            port_str = std::string(str, colon_pos + 1, str.size());
        else
            port_str = StringUtils::toString(port_number);
    }
#ifdef ENABLE_IPV6
    else if (str[0] == '[')
    {
        // Handle [IPv6 address]:port format
        size_t pos = str.find(']');
        if (pos == std::string::npos || pos - 1 == 0)
        {
            Log::error("SocketAddress", "Invalid IPv6 address is specified.");
            return;
        }
        addr_str = std::string(str, 1, pos - 1);
        if (pos + 2 < str.size() && str[pos + 1] == ':')
            port_str = std::string(str, pos + 2, str.size());
        else
            port_str = StringUtils::toString(port_number);
    }
#else
    // Ignore ipv6, otherwise we end up querying for them which can be slow
    else if (colon_pos != std::string::npos)
    {
      Log::debug("SocketAddress", "Ignoring ipv6 address: %s", str.c_str());
      return;
    }
#endif
    else
    {
        addr_str = str;
        port_str = StringUtils::toString(port_number);
    }

    struct addrinfo hints;
    struct addrinfo* res = NULL;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo_compat(addr_str.c_str(), port_str.c_str(), &hints,
        &res);
    if (status != 0)
    {
#ifdef WIN32
        if (!g_ignore_error_message)
        {
            wchar_t msgbuf[256] = {};
            DWORD flags =
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
                FORMAT_MESSAGE_MAX_WIDTH_MASK;
            FormatMessage(flags, NULL, WSAGetLastError(), 0, msgbuf,
                sizeof(msgbuf) / sizeof(wchar_t), NULL);
            Log::error("SocketAddress", "Error in getaddrinfo for "
                "SocketAddress (str constructor) %s: %s",
                str.c_str(), StringUtils::wideToUtf8(msgbuf).c_str());
        }
#else
        if (!g_ignore_error_message)
        {
            Log::error("SocketAddress", "Error in getaddrinfo for "
                "SocketAddress (str constructor) %s: %s",
                str.c_str(), gai_strerror(status));
        }
#endif
        return;
    }
    if (res == NULL)
    {
        if (!g_ignore_error_message)
            Log::error("SocketAddress", "No address is resolved.");
        return;
    }

    bool found = false;
    bool ipv4_mapped = str.size() > 7 && str.compare(0, 7, "::ffff:") == 0;
    for (struct addrinfo* addr = res; addr != NULL; addr = addr->ai_next)
    {
        switch (addr->ai_family)
        {
        case AF_INET:
            memcpy(m_sockaddr.data(), addr->ai_addr, sizeof(sockaddr_in));
            found = true;
            break;
        case AF_INET6:
            if (ipv4_mapped ||
                !isIPv4MappedAddress((const struct sockaddr_in6*)addr->ai_addr))
            {
                // OSX and iOS can return AF_INET6 with ::ffff:x.y.z.w for server
                // with A record, skip them and make it only get real AAAA record
                m_family = AF_INET6;
                memcpy(m_sockaddr.data(), addr->ai_addr, sizeof(sockaddr_in6));
                found = true;
            }
            break;
        default:
            break;
        }
        if (found)
            break;
    }
    freeaddrinfo(res);
}   // init

// ----------------------------------------------------------------------------
/** Returns the IPv4 address in decimal, it will handle IPv4 mapped IPv6
 *  address too. */
uint32_t SocketAddress::getIP() const
{
    if (m_family == AF_INET6)
    {
        sockaddr_in6* in6 = (sockaddr_in6*)m_sockaddr.data();
        if (isIPv4MappedAddress(in6))
            return ntohl(((in_addr*)(in6->sin6_addr.s6_addr + 12))->s_addr);
        return 0;
    }
    else if (m_family == AF_INET)
    {
        sockaddr_in* in = (sockaddr_in*)m_sockaddr.data();
        return ntohl(in->sin_addr.s_addr);
    }
    return 0;
}   // getIP

// ----------------------------------------------------------------------------
/** Returns the port number. */
uint16_t SocketAddress::getPort() const
{
    if (m_family == AF_INET)
    {
        sockaddr_in* in = (sockaddr_in*)m_sockaddr.data();
        return ntohs(in->sin_port);
    }
    else if (m_family == AF_INET6)
    {
        sockaddr_in6* in6 = (sockaddr_in6*)m_sockaddr.data();
        return ntohs(in6->sin6_port);
    }
    return 0;
}   // getPort

// ----------------------------------------------------------------------------
/** Sets the ip address. */
void SocketAddress::setIP(uint32_t ip)
{
    if (m_family != AF_INET)
    {
        // Reset the structure if different family is used
        clear();
    }
    sockaddr_in* in = (sockaddr_in*)m_sockaddr.data();
    in->sin_addr.s_addr = htonl(ip);
}   // setIP

// ----------------------------------------------------------------------------
/** Set the port. */
void SocketAddress::setPort(uint16_t port)
{
    if (m_family == AF_INET)
    {
        sockaddr_in* in = (sockaddr_in*)m_sockaddr.data();
        in->sin_port = htons(port);
    }
    else if (m_family == AF_INET6)
    {
        sockaddr_in6* in6 = (sockaddr_in6*)m_sockaddr.data();
        in6->sin6_port = htons(port);
    }
}   // setPort

// ----------------------------------------------------------------------------
/** Returns this IP address is localhost which its equal to any interface
 *  address. */
bool SocketAddress::isPublicAddressLocalhost() const
{
    if (m_family == AF_INET && getIP() == 0)
        return false;
    if (isLoopback())
        return true;
#ifdef __SWITCH__
    if (m_family == AF_INET)
    {
        uint32_t currentIp = 0;
        if(R_FAILED(nifmGetCurrentIpAddress(&currentIp)))
        {
          Log::warn("SocketAddress", "Failed to get current address!");
        }
        else if(currentIp)
            return htonl(currentIp) == getIP();
    }
    return false;
#elif !defined(WIN32)
    struct ifaddrs *addresses, *p;

    if (getifaddrs(&addresses) == -1)
    {
        Log::warn("SocketAddress", "Error in getifaddrs");
        return false;
    }
    bool is_local_host = false;
    for (p = addresses; p; p = p->ifa_next)
    {
        if (p->ifa_addr == NULL)
            continue;
        if (p->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *sa = (struct sockaddr_in*)p->ifa_addr;
            if (htonl(sa->sin_addr.s_addr) == getIP())
            {
                is_local_host = true;
                break;
            }
        }
        else if (p->ifa_addr->sa_family == AF_INET6 && getFamily() == AF_INET6)
        {
            struct sockaddr_in6 addr = {};
            memcpy(&addr, p->ifa_addr, sizeof(sockaddr_in6));
            sockaddr_in6* my_in6 = (sockaddr_in6*)m_sockaddr.data();
            addr.sin6_port = my_in6->sin6_port;
            if (sameIPV6(my_in6, &addr))
            {
                is_local_host = true;
                break;
            }
        }
    }
    freeifaddrs(addresses);
    return is_local_host;
#else
    // From docs from microsoft it recommends 15k size
    const int WORKING_BUFFER_SIZE = 15000;
    PIP_ADAPTER_ADDRESSES paddr = NULL;
    unsigned long len = WORKING_BUFFER_SIZE;
    int return_code = 0;
    int iteration = 0;
    do
    {
        paddr = (IP_ADAPTER_ADDRESSES*)malloc(len);
        if (paddr == NULL)
            return false;
        long flags = 0;
        return_code = GetAdaptersAddresses(AF_UNSPEC, flags, NULL, paddr,
            &len);
        if (return_code == ERROR_BUFFER_OVERFLOW)
        {
            free(paddr);
            paddr = NULL;
        }
        else
            break;
        iteration++;
    } while ((return_code == ERROR_BUFFER_OVERFLOW) && (iteration < 10));

    if (return_code == ERROR_BUFFER_OVERFLOW)
        return false;

    bool is_local_host = false;
    for (IP_ADAPTER_ADDRESSES *p = paddr; p; p = p->Next)
    {
        if (is_local_host)
            break;
        if (p->OperStatus != IfOperStatusUp)
            continue;

        for (PIP_ADAPTER_UNICAST_ADDRESS unicast = p->FirstUnicastAddress;
            unicast != NULL; unicast = unicast->Next)
        {
            if (unicast->Address.lpSockaddr->sa_family == AF_INET)
            {
                const sockaddr_in *sa = (sockaddr_in*)unicast->Address.lpSockaddr;
                if (htonl(sa->sin_addr.s_addr) == getIP())
                {
                    is_local_host = true;
                    break;
                }
            }
            else if (unicast->Address.lpSockaddr->sa_family == AF_INET6 &&
                getFamily() == AF_INET6)
            {
                struct sockaddr_in6 addr = {};
                memcpy(&addr, unicast->Address.lpSockaddr, sizeof(sockaddr_in6));
                sockaddr_in6* my_in6 = (sockaddr_in6*)m_sockaddr.data();
                addr.sin6_port = my_in6->sin6_port;
                if (sameIPV6(my_in6, &addr))
                {
                    is_local_host = true;
                    break;
                }
            }
        }
    }
    free(paddr);
    return is_local_host;
#endif
}   // isPublicAddressLocalhost

// ----------------------------------------------------------------------------
/** Returns if this IP is loopback (ie for IPv4 127.0.0.0/8, IPv6 ::1/128)
 */
bool SocketAddress::isLoopback() const
{
    uint32_t ip = getIP();
    if (ip != 0)
    {
        if (ip >> 24 == 0x7f)
            return true;
    }
    else if (m_family == AF_INET6)
    {
        sockaddr_in6* in6 = (sockaddr_in6*)m_sockaddr.data();
        for (int i = 0; i < 15; i++)
        {
            uint8_t w_i = in6->sin6_addr.s6_addr[i];
            if (w_i != 0)
            {
                return false;
            }
        } // for  (int i = 0; i < 15; i++)
        // ::1/128 Loopback
        uint8_t w15 = in6->sin6_addr.s6_addr[15];
        if (w15 == 1)
        {
            return true;
        }
    }
    return false;
}   // isLoopback

// ----------------------------------------------------------------------------
/** Returns if this IP address belongs to a LAN, i.e. is in 192.168* or
 *  10*, 172.16-31.*, or is on the same host, i.e. 127*.
 */
bool SocketAddress::isLAN() const
{
    if (isLoopback())
        return true;
    uint32_t ip = getIP();
    if (ip != 0)
    {
        // IPv4 test
        if (ip >> 16 == 0xc0a8)         // Check for 192.168.*
            return true;
        else if (ip >> 20 == 0xac1 )    // 172.16-31.*
            return true;
        else if (ip >> 24 == 0x0a  )    // 10.*
            return true;
    }
    else if (m_family == AF_INET6)
    {
        sockaddr_in6* in6 = (sockaddr_in6*)m_sockaddr.data();
        uint8_t w0 = in6->sin6_addr.s6_addr[0];
        uint8_t w1 = in6->sin6_addr.s6_addr[1];
        uint16_t _16 = ((uint16_t)w0) << 8 | w1;
        if (_16 >= 0xfc00 && _16 <= 0xfdff)
        {
            // fc00::/7 Unique Local Address (ULA)
            return true;
        }
        if (_16 >= 0xfe80 && _16 <= 0xfebf)
        {
            // fe80::/10 Link-Local Address
            return true;
        }
    }
    return false;
}   // isLAN

// ----------------------------------------------------------------------------
/** Compares if ip address and port are identical. */
bool SocketAddress::operator==(const SocketAddress& other) const
{
    if (m_family == AF_INET && other.m_family == AF_INET)
    {
        sockaddr_in* in_a = (sockaddr_in*)m_sockaddr.data();
        sockaddr_in* in_b = (sockaddr_in*)(other.m_sockaddr.data());
        return in_a->sin_addr.s_addr == in_b->sin_addr.s_addr &&
            in_a->sin_port == in_b->sin_port;
    }
    else if (m_family == AF_INET6 && other.m_family == AF_INET6)
    {
        sockaddr_in6* in6_a = (sockaddr_in6*)m_sockaddr.data();
        sockaddr_in6* in6_b = (sockaddr_in6*)(other.m_sockaddr.data());
        return sameIPV6(in6_a, in6_b);
    }
    return false;
}   // operator==

// ----------------------------------------------------------------------------
/** Compares if ip address and port are not identical. */
bool SocketAddress::operator!=(const SocketAddress& other) const
{
    if (m_family == AF_INET && other.m_family == AF_INET)
    {
        sockaddr_in* in_a = (sockaddr_in*)m_sockaddr.data();
        sockaddr_in* in_b = (sockaddr_in*)(other.m_sockaddr.data());
        return in_a->sin_addr.s_addr != in_b->sin_addr.s_addr ||
            in_a->sin_port != in_b->sin_port;
    }
    else if (m_family == AF_INET6 && other.m_family == AF_INET6)
    {
        sockaddr_in6* in6_a = (sockaddr_in6*)m_sockaddr.data();
        sockaddr_in6* in6_b = (sockaddr_in6*)(other.m_sockaddr.data());
        return !sameIPV6(in6_a, in6_b);
    }
    return true;
}   // operator!=

// ----------------------------------------------------------------------------
std::string SocketAddress::toString(bool show_port) const
{
    std::string result;
    uint32_t ip = getIP();
    if (ip != 0 || m_family == AF_INET)
    {
        result = StringUtils::insertValues("%d.%d.%d.%d",
            ((ip >> 24) & 0xff), ((ip >> 16) & 0xff),
            ((ip >>  8) & 0xff), ((ip >>  0) & 0xff));
        if (show_port)
            result += ":" + StringUtils::toString(getPort());
    }
    else
    {
        result = getIPV6ReadableFromIn6((sockaddr_in6*)m_sockaddr.data());
        if (show_port)
        {
            result.insert (0, 1, '[');
            result += "]";
            result += ":" + StringUtils::toString(getPort());
        }
    }
    return result;
}   // toString

// ----------------------------------------------------------------------------
void SocketAddress::convertForIPv6Socket(bool ipv6)
{
#ifdef ENABLE_IPV6
    if (m_family == AF_INET && ipv6)
    {
        std::string ipv4 = toString(false/*show_port*/);
        uint16_t port = getPort();
        auto ip_type = NetworkConfig::get()->getIPType();
        if (ip_type == NetworkConfig::IP_V6_NAT64)
        {
            ipv4 = NetworkConfig::get()->getNAT64Prefix() + ipv4;
        }
        else
        {
            // Assume the system has dual stack if it uses an IPv6 socket
            ipv4 = std::string("::ffff:") + ipv4;
        }
        init(ipv4, port);
    }
#endif
}   // convertForIPv6Socket

// ----------------------------------------------------------------------------
/** Unit testing. Test various LAN patterns to verify that isLAN() works as
 *  expected.
 */
void SocketAddress::unitTesting()
{
    SocketAddress t1("192.168.0.0");
    assert(t1.getIP() == (192u << 24) + (168u << 16));
    assert(t1.isLAN());

    SocketAddress t2("192.168.255.255");
    assert(t2.getIP() == (192u << 24) + (168u << 16) + (255u << 8) + 255u);
    assert(t2.isLAN());

    SocketAddress t3("::ffff:193.168.0.1");
    assert(t3.getIP() == (193u << 24) + (168u << 16) + 1);
    assert(!t3.isLAN());

    SocketAddress t4("192.167.255.255");
    assert(t4.getIP() == (192u << 24) + (167u << 16) + (255u << 8) + 255u);
    assert(!t4.isLAN());

    SocketAddress t5("192.169.0.0");
    assert(t5.getIP() == (192u << 24) + (169u << 16));
    assert(!t5.isLAN());

    SocketAddress t6("172.16.0.0");
    assert(t6.getIP() == (172u << 24) + (16u << 16));
    assert(t6.isLAN());

    SocketAddress t7("172.31.255.255");
    assert(t7.getIP() == (172u << 24) + (31u << 16) + (255u << 8) + 255u);
    assert(t7.isLAN());

    SocketAddress t8("172.15.255.255");
    assert(t8.getIP() == (172u << 24) + (15u << 16) + (255u << 8) + 255u);
    assert(!t8.isLAN());

    SocketAddress t9("172.32.0.0");
    assert(t9.getIP() == (172u << 24) + (32u << 16));
    assert(!t9.isLAN());

    SocketAddress t10("10.0.0.0");
    assert(t10.getIP() == (10u << 24));
    assert(t10.isLAN());

    SocketAddress t11("10.255.255.255");
    assert(t11.getIP() == (10u << 24) + (255u << 16) + (255u << 8) + 255u);
    assert(t11.isLAN());

    SocketAddress t12("9.255.255.255");
    assert(t12.getIP() == (9u << 24) + (255u << 16) + (255u << 8) + 255u);
    assert(!t12.isLAN());

    SocketAddress t13("11.0.0.0");
    assert(t13.getIP() == (11u << 24));
    assert(!t13.isLAN());

    SocketAddress t14("127.0.0.0");
    assert(t14.getIP() == (127u << 24));
    assert(t14.isLAN());

    SocketAddress t15("::ffff:127.255.255.255");
    assert(t15.getIP() == (127u << 24) + (255u << 16) + (255u << 8) + 255u);
    assert(t15.isLAN());

    SocketAddress t16("126.255.255.255");
    assert(t16.getIP() == (126u << 24) + (255u << 16) + (255u << 8) + 255u);
    assert(!t16.isLAN());

    SocketAddress t17("128.0.0.0");
    assert(t17.getIP() == (128u << 24));
    assert(!t17.isLAN());

    // Test constructors
    SocketAddress t18("128.0.0.0");
    assert(t18.getIP() == (128u << 24));
    assert(t18.getPort() == 0);

    SocketAddress t19("128.0.0.0", 1);
    assert(t19.getIP() == (128u << 24));
    assert(t19.getPort() == 1);

    SocketAddress t20("128.0.0.0", 123);
    assert(t20.getIP() == (128u << 24));
    assert(t20.getPort() == 123);

    SocketAddress v6_1("0:0:0:0:0:0:0:1");
    assert(v6_1.isLAN());

    SocketAddress v6_2("fe80::221:86ff:fea0:ce84");
    assert(v6_2.isLAN());

    SocketAddress v6_3("fdf8:f53b:82e4::53");
    assert(v6_3.isLAN());

    // Boundary test
    // fc00::/7 Unique Local Address (ULA)
    SocketAddress v6_4("fc00::");
    assert(v6_4.isLAN());

    SocketAddress v6_5("fdff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
    assert(v6_5.isLAN());

    SocketAddress v6_6("fbff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
    assert(!v6_6.isLAN());

    SocketAddress v6_7("fe00::");
    assert(!v6_7.isLAN());

    // fe80::/10 Link-Local Address
    SocketAddress v6_8("fe80::");
    assert(v6_8.isLAN());

    SocketAddress v6_9("febf:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
    assert(v6_9.isLAN());

    SocketAddress v6_10("fe7f:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
    assert(!v6_10.isLAN());

    SocketAddress v6_11("fec0::");
    assert(!v6_11.isLAN());

    SocketAddress ipv4_port("0.0.0.1:1");
    assert(ipv4_port.getIP() == 1);
    assert(ipv4_port.getPort() == 1);

    SocketAddress ipv6_port("[::2]:1");
    assert(ipv6_port.toString(false) == "::2");
    assert(ipv6_port.getPort() == 1);

}   // unitTesting

// ----------------------------------------------------------------------------
ENetAddress SocketAddress::toENetAddress() const
{
    ENetAddress ea = {};
    uint32_t ip = getIP();
#if defined(ENABLE_IPV6) || defined(__SWITCH__)
    if (isIPv6Socket())
    {
        struct sockaddr_in6* in6 = (struct sockaddr_in6*)m_sockaddr.data();
        memcpy(&ea.host.p0, in6->sin6_addr.s6_addr, 16);
        ea.host.p4 = in6->sin6_scope_id;
    }
    else
    {
        // because ENet wants little endian
        ea.host.p0 = ((ip & 0xff000000) >> 24) +
            ((ip & 0x00ff0000) >> 8) + ((ip & 0x0000ff00) << 8) +
            ((ip & 0x000000ff) << 24);
    }
#else
    ea.host = ((ip & 0xff000000) >> 24) +
        ((ip & 0x00ff0000) >> 8) + ((ip & 0x0000ff00) << 8) +
        ((ip & 0x000000ff) << 24);
#endif
    ea.port = getPort();
    return ea;
}   // toENetAddress
