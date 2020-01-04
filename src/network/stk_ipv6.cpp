//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2019 SuperTuxKart-Team
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

#include <string.h>
#ifdef WIN32
#ifdef __GNUC__
#    include <ws2tcpip.h>    // Mingw / gcc on windows
#    undef _WIN32_WINNT
#    define _WIN32_WINNT 0x501
#    include <winsock2.h>
#    include <ws2tcpip.h>

extern "C"
{
   WINSOCK_API_LINKAGE  PCSTR WSAAPI inet_ntop(INT Family, PVOID pAddr, PSTR pStringBuf, size_t StringBufSize);
   WINSOCK_API_LINKAGE  INT WSAAPI inet_pton(INT Family, PCSTR pszAddrString, PVOID pAddrBuf);
}

#else
#    include <winsock2.h>
#    include <in6addr.h>
#    include <ws2tcpip.h>
#endif

#else

#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#endif

#include <string>

// ----------------------------------------------------------------------------
std::string getIPV6ReadableFromIn6(const struct sockaddr_in6* in)
{
    std::string result;
    char ipv6[INET6_ADDRSTRLEN] = {};
    struct in6_addr ipv6_addr = in->sin6_addr;
    inet_ntop(AF_INET6, &ipv6_addr, ipv6, INET6_ADDRSTRLEN);
    result = ipv6;
    return result;
}   // getIPV6ReadableFromIn6

// ----------------------------------------------------------------------------
bool sameIPV6(const struct sockaddr_in6* in_1, const struct sockaddr_in6* in_2)
{
    // Check port first, then address
    if (in_1->sin6_port != in_2->sin6_port)
        return false;

    const struct in6_addr* a = &(in_1->sin6_addr);
    const struct in6_addr* b = &(in_2->sin6_addr);
    for (unsigned i = 0; i < sizeof(struct in6_addr); i++)
    {
        if (a->s6_addr[i] != b->s6_addr[i])
            return false;
    }
    return true;
}   // sameIPV6

// ----------------------------------------------------------------------------
/** Workaround of a bug in iOS 9 where port number is not written. */
extern "C" int getaddrinfo_compat(const char* hostname,
                                  const char* servname,
                                  const struct addrinfo* hints,
                                  struct addrinfo** res)
{
#ifdef IOS_STK
    int err;
    int numericPort;

    // If we're given a service name and it's a numeric string,
    // set `numericPort` to that, otherwise it ends up as 0.
    numericPort = servname != NULL ? atoi(servname) : 0;

    // Call `getaddrinfo` with our input parameters.
    err = getaddrinfo(hostname, servname, hints, res);

    // Post-process the results of `getaddrinfo` to work around
    if ((err == 0) && (numericPort != 0))
    {
        for (const struct addrinfo* addr = *res; addr != NULL;
             addr = addr->ai_next)
        {
            in_port_t* portPtr;
            switch (addr->ai_family)
            {
                case AF_INET:
                {
                    portPtr = &((struct sockaddr_in*)addr->ai_addr)->sin_port;
                }
                break;
                case AF_INET6:
                {
                    portPtr = &((struct sockaddr_in6*)addr->ai_addr)->sin6_port;
                }
                break;
                default:
                {
                    portPtr = NULL;
                }
                break;
            }
            if ((portPtr != NULL) && (*portPtr == 0))
            {
                *portPtr = htons(numericPort);
            }
        }
    }
    return err;
#else
    return getaddrinfo(hostname, servname, hints, res);
#endif
}   // getaddrinfo_compat

// ----------------------------------------------------------------------------
void andIPv6(struct in6_addr* ipv6, const struct in6_addr* mask)
{
    for (unsigned i = 0; i < sizeof(struct in6_addr); i++)
        ipv6->s6_addr[i] &= mask->s6_addr[i];
}   // andIPv6

// ----------------------------------------------------------------------------
extern "C" int insideIPv6CIDR(const char* ipv6_cidr, const char* ipv6_in)
{
    const char* mask_location = strchr(ipv6_cidr, '/');
    if (mask_location == NULL)
        return 0;
    struct in6_addr v6_in;
    if (inet_pton(AF_INET6, ipv6_in, &v6_in) != 1)
        return 0;

    char ipv6[INET6_ADDRSTRLEN] = {};
    memcpy(ipv6, ipv6_cidr, mask_location - ipv6_cidr);
    struct in6_addr cidr;
    if (inet_pton(AF_INET6, ipv6, &cidr) != 1)
        return 0;

    int mask_length = atoi(mask_location + 1);
    if (mask_length > 128 || mask_length <= 0)
        return 0;

    struct in6_addr mask = {};
    for (int i = mask_length, j = 0; i > 0; i -= 8, j++)
    {
        if (i >= 8)
            mask.s6_addr[j] = 0xff;
        else
            mask.s6_addr[j] = (unsigned long)(0xffU << (8 - i));
    }

    andIPv6(&cidr, &mask);
    andIPv6(&v6_in, &mask);
    for (unsigned i = 0; i < sizeof(struct in6_addr); i++)
    {
        if (cidr.s6_addr[i] != v6_in.s6_addr[i])
            return 0;
    }
    return 1;
}   // andIPv6

#ifndef ENABLE_IPV6
#include "network/stk_ipv6.hpp"
// ----------------------------------------------------------------------------
int isIPV6()
{
    return 0;
}   // isIPV6

// ----------------------------------------------------------------------------
void setIPV6(int val)
{
}   // setIPV6

// ----------------------------------------------------------------------------
std::string getIPV6ReadableFromMappedAddress(const ENetAddress* ea)
{
    return "";
}   // getIPV6ReadableFromMappedAddress

// ----------------------------------------------------------------------------
void removeDisconnectedMappedAddress()
{
}   // removeDisconnectedMappedAddress

// ----------------------------------------------------------------------------
void addMappedAddress(const ENetAddress* ea, const struct sockaddr_in6* in6)
{
}   // addMappedAddress

#else

#include "network/stk_ipv6.hpp"
#include "network/ios_ipv6.hpp"
#include "network/transport_address.hpp"
#include "utils/string_utils.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"
#include "utils/types.hpp"

#include <algorithm>
#include <vector>

// ============================================================================
uint32_t g_mapped_ipv6_used;
int g_ipv6;
struct MappedAddress
{
    ENetAddress m_addr;
    struct sockaddr_in6 m_in6;
    uint64_t m_last_activity;
    MappedAddress(const ENetAddress& addr, const struct sockaddr_in6& in6)
    {
        m_addr = addr;
        m_in6 = in6;
        m_last_activity = StkTime::getMonoTimeMs();
    }
};
std::vector<MappedAddress> g_mapped_ips;
// ============================================================================
int isIPV6()
{
    return g_ipv6;
}   // isIPV6

// ----------------------------------------------------------------------------
void setIPV6(int val)
{
    g_ipv6 = val;
}   // setIPV6

// ----------------------------------------------------------------------------
void stkInitialize()
{
    // Clear previous setting, in case user changed wifi or mobile data
    g_mapped_ipv6_used = 0;
    g_ipv6 = 0;
    g_mapped_ips.clear();
#ifdef IOS_STK
    if (isIPV6Only())
        g_ipv6 = 1;
#endif
}   // stkInitialize

// ----------------------------------------------------------------------------
std::string getIPV6ReadableFromMappedAddress(const ENetAddress* ea)
{
    std::string result;
    auto it = std::find_if(g_mapped_ips.begin(), g_mapped_ips.end(),
        [ea](const MappedAddress& addr)
        {
            return ea->host == addr.m_addr.host &&
                ea->port == addr.m_addr.port;
        });
    if (it != g_mapped_ips.end())
        result = getIPV6ReadableFromIn6(&it->m_in6);
    return result;
}   // getIPV6ReadableFromMappedAddress

// ----------------------------------------------------------------------------
/** Add a (fake or synthesized by ios / osx) IPv4 address and map it to an IPv6
 *  one, used in client to set the game server address or server to initialize
 *  host.
 */
void addMappedAddress(const ENetAddress* ea, const struct sockaddr_in6* in6)
{
    g_mapped_ips.emplace_back(*ea, *in6);
}   // addMappedAddress

// ----------------------------------------------------------------------------
/* This is called when enet needs to sent to an mapped IPv4 address, we look up
 * the map here and get the real IPv6 address, so you need to call
 * addMappedAddress above first (for client mostly).
 */
void getIPV6FromMappedAddress(const ENetAddress* ea, struct sockaddr_in6* in6)
{
    auto it = std::find_if(g_mapped_ips.begin(), g_mapped_ips.end(),
        [ea](const MappedAddress& addr)
        {
            return ea->host == addr.m_addr.host &&
                ea->port == addr.m_addr.port;
        });
    if (it != g_mapped_ips.end())
    {
        it->m_last_activity = StkTime::getMonoTimeMs();
        memcpy(in6, &it->m_in6, sizeof(struct sockaddr_in6));
    }
    else
        memset(in6, 0, sizeof(struct sockaddr_in6));
}   // getIPV6FromMappedAddress

// ----------------------------------------------------------------------------
/* This is called when enet recieved a packet from its socket, we create an
 * real IPv4 address out of it or a fake one if it's from IPv6 connection.
 */
void getMappedFromIPV6(const struct sockaddr_in6* in6, ENetAddress* ea)
{
    auto it = std::find_if(g_mapped_ips.begin(), g_mapped_ips.end(),
        [in6](const MappedAddress& addr)
        {
            return sameIPV6(in6, &addr.m_in6);
        });
    if (it != g_mapped_ips.end())
    {
        it->m_last_activity = StkTime::getMonoTimeMs();
        *ea = it->m_addr;
        return;
    }

    uint8_t w0 = in6->sin6_addr.s6_addr[0];
    uint8_t w1 = in6->sin6_addr.s6_addr[1];
    uint8_t w2 = in6->sin6_addr.s6_addr[2];
    uint8_t w3 = in6->sin6_addr.s6_addr[3];
    uint8_t w4 = in6->sin6_addr.s6_addr[4];
    uint8_t w5 = in6->sin6_addr.s6_addr[5];
    uint8_t w6 = in6->sin6_addr.s6_addr[6];
    uint8_t w7 = in6->sin6_addr.s6_addr[7];
    uint8_t w8 = in6->sin6_addr.s6_addr[8];
    uint8_t w9 = in6->sin6_addr.s6_addr[9];
    uint8_t w10 = in6->sin6_addr.s6_addr[10];
    uint8_t w11 = in6->sin6_addr.s6_addr[11];
    if (w0 == 0 && w1 == 0 && w2 == 0 && w3 == 0 && w4 == 0 &&
        w5 == 0 && w6 == 0 && w7 == 0 && w8 == 0 && w9 == 0 &&
        w10 == 0xff && w11 == 0xff)
    {
        ea->host = ((in_addr*)(in6->sin6_addr.s6_addr + 12))->s_addr;
        ea->port = ntohs(in6->sin6_port);
        TransportAddress addr(*ea);
        addMappedAddress(ea, in6);
    }
    else
    {
        // Create a fake IPv4 address of 0.x.x.x if it's a real IPv6 connection
        if (g_mapped_ipv6_used >= 16777215)
            g_mapped_ipv6_used = 0;
        TransportAddress addr(++g_mapped_ipv6_used, ntohs(in6->sin6_port));
        *ea = addr.toEnetAddress();
        Log::debug("IPv6", "Fake IPv4 address %s mapped to %s",
            addr.toString().c_str(), getIPV6ReadableFromIn6(in6).c_str());
        addMappedAddress(ea, in6);
    }
}   // getMappedFromIPV6

// ----------------------------------------------------------------------------
/* Called when a peer is expired (no activity for 20 seconds) and we remove its
 * reference to the IPv6.
 */
void removeDisconnectedMappedAddress()
{
    for (auto it = g_mapped_ips.begin(); it != g_mapped_ips.end();)
    {
        if (it->m_last_activity + 20000 < StkTime::getMonoTimeMs())
        {
            TransportAddress addr(it->m_addr);
            Log::debug("IPv6", "Removing expired %s, IPv4 address %s.",
                getIPV6ReadableFromIn6(&it->m_in6).c_str(),
                addr.toString().c_str());
            it = g_mapped_ips.erase(it);
            Log::debug("IPv6", "Mapped address size now: %d.",
                g_mapped_ips.size());
        }
        else
            it++;
    }
}   // removeDisconnectedMappedAddress

#endif
