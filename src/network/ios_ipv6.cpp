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

// Minimum code to allow iOS device to connect to IPv4 non-firewalled game server

#ifdef IOS_STK

#include "network/ios_ipv6.hpp"

#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <unistd.h>

#if TARGET_IPHONE_SIMULATOR
#include <net/route.h>
#else
#include "route.h"
#endif

#include <TargetConditionals.h>
#include <resolv.h>
#define RESOLV_CONFIG_PATH ("/etc/resolv.conf")

#define NAME_SVR ("nameserver")
#define NAME_SVR_LEN (10)

namespace Mars
{
// Based on Mars from tencent, licensed under MIT
#define SUCCESS (0)
#define FAILED  (-1)
#define ROUNDUP(a) \
((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))

    enum TLocalIPStack
    {
        ELocalIPStack_None = 0,
        ELocalIPStack_IPv4 = 1,
        ELocalIPStack_IPv6 = 2,
        ELocalIPStack_Dual = 3,
    };

    typedef union sockaddr_union
    {
        struct sockaddr generic;
        struct sockaddr_in in;
        struct sockaddr_in6 in6;
    } sockaddr_union;

    int getdefaultgateway(struct in_addr * addr)
    {
        /* net.route.0.inet.flags.gateway */
        int mib[] = {CTL_NET, PF_ROUTE, 0, AF_INET,
            NET_RT_FLAGS, RTF_GATEWAY};
        size_t l;
        char * buf, * p;
        struct rt_msghdr * rt;
        struct sockaddr * sa;
        struct sockaddr * sa_tab[RTAX_MAX];
        int i;
        int r = FAILED;
        if (sysctl(mib, sizeof(mib)/sizeof(int), 0, &l, 0, 0) < 0)
        {
            return FAILED;
        }
        if (l>0)
        {
            buf = (char*)malloc(l);
            if (sysctl(mib, sizeof(mib)/sizeof(int), buf, &l, 0, 0) < 0)
            {
                free(buf);
                return FAILED;
            }
            for(p=buf; p<buf+l; p+=rt->rtm_msglen)
            {
                rt = (struct rt_msghdr *)p;
                sa = (struct sockaddr *)(rt + 1);
                for(i=0; i<RTAX_MAX; i++)
                {
                    if (rt->rtm_addrs & (1 << i))
                    {
                        sa_tab[i] = sa;
                        sa = (struct sockaddr *)((char *)sa + ROUNDUP(sa->sa_len));
                    } else
                    {
                        sa_tab[i] = NULL;
                    }
                }
                if ( ((rt->rtm_addrs & (RTA_DST|RTA_GATEWAY)) == (RTA_DST|RTA_GATEWAY))
                    && sa_tab[RTAX_DST]->sa_family == AF_INET)
                {
                    //              && sa_tab[RTAX_GATEWAY]->sa_family == AF_INET) {
                    if (((struct sockaddr_in *)sa_tab[RTAX_DST])->sin_addr.s_addr == 0)
                    {
                        *addr = ((struct sockaddr_in *)(sa_tab[RTAX_GATEWAY]))->sin_addr;
                        r = SUCCESS;
                        break;
                    }
                }
            }
            free(buf);
        }
        return r;
    }

    int getdefaultgateway6(struct in6_addr * addr)
    {
        /* net.route.0.inet6.flags.gateway */
        int mib[] = {CTL_NET, PF_ROUTE, 0, AF_INET6,
            NET_RT_FLAGS, RTF_GATEWAY};
        size_t l;
        char * buf, * p;
        struct rt_msghdr * rt;
        struct sockaddr * sa;
        struct sockaddr * sa_tab[RTAX_MAX];
        int i;
        int r = FAILED;
        if (sysctl(mib, sizeof(mib)/sizeof(int), 0, &l, 0, 0) < 0)
        {
            return FAILED;
        }
        if (l>0)
        {
            buf = (char*)malloc(l);
            if (sysctl(mib, sizeof(mib)/sizeof(int), buf, &l, 0, 0) < 0)
            {
                free(buf);
                return FAILED;
            }
            for(p=buf; p<buf+l; p+=rt->rtm_msglen)
            {
                rt = (struct rt_msghdr *)p;
                sa = (struct sockaddr *)(rt + 1);
                for(i=0; i<RTAX_MAX; i++)
                {
                    if (rt->rtm_addrs & (1 << i))
                    {
                        sa_tab[i] = sa;
                        sa = (struct sockaddr *)((char *)sa + ROUNDUP(sa->sa_len));
                    } else
                    {
                        sa_tab[i] = NULL;
                    }
                }
                if ( ((rt->rtm_addrs & (RTA_DST|RTA_GATEWAY)) == (RTA_DST|RTA_GATEWAY))
                    && sa_tab[RTAX_DST]->sa_family == AF_INET6)
                {
                    //               && sa_tab[RTAX_GATEWAY]->sa_family == AF_INET6) {
                    if (IN6_IS_ADDR_UNSPECIFIED(&((struct sockaddr_in6 *)sa_tab[RTAX_DST])->sin6_addr))
                    {
                        *addr = ((struct sockaddr_in6 *)(sa_tab[RTAX_GATEWAY]))->sin6_addr;
                        r = SUCCESS;
                        break;
                    }
                }
            }
            free(buf);
        }
        return r;
    }

    static int _test_connect(int pf, struct sockaddr *addr, size_t addrlen)
    {
        int s = socket(pf, SOCK_DGRAM, IPPROTO_UDP);
        if (s < 0)
            return 0;
        int ret;
        do
        {
            ret = connect(s, addr, addrlen);
        }
        while (ret < 0 && errno == EINTR);
        int success = (ret == 0);
        do
        {
            ret = close(s);
        }
        while (ret < 0 && errno == EINTR);
        return success;
    }

    static int _have_ipv6()
    {
        static const struct sockaddr_in6 sin6_test =
        {
            .sin6_len = sizeof(sockaddr_in6),
            .sin6_family = AF_INET6,
            .sin6_port = htons(0xFFFF),
            .sin6_addr.s6_addr = {
                0x20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        };
        sockaddr_union addr = { .in6 = sin6_test };
        return _test_connect(PF_INET6, &addr.generic, sizeof(addr.in6));
    }

    static int _have_ipv4()
    {
        static const struct sockaddr_in sin_test =
        {
            .sin_len = sizeof(sockaddr_in),
            .sin_family = AF_INET,
            .sin_port = htons(0xFFFF),
            .sin_addr.s_addr = htonl(0x08080808L),  // 8.8.8.8
        };
        sockaddr_union addr = { .in = sin_test };
        return _test_connect(PF_INET, &addr.generic, sizeof(addr.in));
    }

    TLocalIPStack local_ipstack_detect()
    {
        in6_addr addr6_gateway = {0};
        if (0 != getdefaultgateway6(&addr6_gateway)){ return ELocalIPStack_IPv4;}
        if (IN6_IS_ADDR_UNSPECIFIED(&addr6_gateway)) { return ELocalIPStack_IPv4;}

        in_addr addr_gateway = {0};
        if (0 != getdefaultgateway(&addr_gateway)) { return ELocalIPStack_IPv6;}
        if (INADDR_NONE == addr_gateway.s_addr || INADDR_ANY == addr_gateway.s_addr ) { return ELocalIPStack_IPv6;}

        int have_ipv4 = _have_ipv4();
        int have_ipv6 = _have_ipv6();
        int local_stack = 0;
        if (have_ipv4) { local_stack |= ELocalIPStack_IPv4; }
        if (have_ipv6) { local_stack |= ELocalIPStack_IPv6; }
        if (ELocalIPStack_Dual != local_stack) { return (TLocalIPStack)local_stack; }

        int dns_ip_stack = 0;
        struct __res_state stat = {0};
        res_ninit(&stat);
        union res_sockaddr_union addrs[MAXNS] = {0};
        int count = res_getservers(const_cast<res_state>(&stat), addrs, MAXNS);
        for (int i = 0; i < count; ++i)
        {
            if (AF_INET == addrs[i].sin.sin_family)
            {
                dns_ip_stack |= ELocalIPStack_IPv4;
            }
            else if (AF_INET6 == addrs[i].sin.sin_family)
            {
                dns_ip_stack |= ELocalIPStack_IPv6;
            }
        }
        res_ndestroy(&stat);

        return (TLocalIPStack)(ELocalIPStack_None == dns_ip_stack ?
            local_stack : dns_ip_stack);
    }
}

// ============================================================================
/** Return 1 if this iOS device has only IPv6 address, in this case we always
 *  create inet6 socket in enet and try to use the NAT64 (provided by iOS)
 *  to connect to IPv4 server. */
int isIPV6Only()
{
    using namespace Mars;
    int ipstack = local_ipstack_detect();
    if (ipstack == ELocalIPStack_IPv6)
        return 1;
    else
        return 0;
}   // isIPV6Only

#endif
