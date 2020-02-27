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

#include <enet/enet.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif
int isIPv6Socket();
void setIPv6Socket(int val);
int getaddrinfo_compat(const char* hostname, const char* servname,
                       const struct addrinfo* hints, struct addrinfo** res);
int64_t upperIPv6(const char* ipv6);
int insideIPv6CIDR(const char* ipv6_cidr, const char* ipv6_in);
#ifdef __cplusplus
}
#endif
std::string getIPV6ReadableFromIn6(const struct sockaddr_in6* in);
bool sameIPV6(const struct sockaddr_in6* in_1,
              const struct sockaddr_in6* in_2);
bool isIPv4MappedAddress(const struct sockaddr_in6* in6);
