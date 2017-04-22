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

/** Returns if this IP address belongs to a LAN, i.e. is in 192.168* or
 *  10*, 172.16-31.*, or is on the same host, i.e. 127*.
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
}   // isLAN

// ----------------------------------------------------------------------------
/** Unit testing. Test various LAN patterns to verify that isLAN() works as
 *  expected.
 */
void TransportAddress::unitTesting()
{
    TransportAddress t1("192.168.0.0");
    assert(t1.getIP() == (192 << 24) + (168 << 16));
    assert(t1.isLAN());

    TransportAddress t2("192.168.255.255");
    assert(t2.getIP() == (192 << 24) + (168 << 16) + (255 << 8) + 255);
    assert(t2.isLAN());

    TransportAddress t3("193.168.0.1");
    assert(t3.getIP() == (193 << 24) + (168 << 16) + 1);
    assert(!t3.isLAN());

    TransportAddress t4("192.167.255.255");
    assert(t4.getIP() == (192 << 24) + (167 << 16) + (255 << 8) + 255);
    assert(!t4.isLAN());

    TransportAddress t5("192.169.0.0");
    assert(t5.getIP() == (192 << 24) + (169 << 16));
    assert(!t5.isLAN());

    TransportAddress t6("172.16.0.0");
    assert(t6.getIP() == (172 << 24) + (16 << 16));
    assert(t6.isLAN());

    TransportAddress t7("172.31.255.255");
    assert(t7.getIP() == (172 << 24) + (31 << 16) + (255 << 8) + 255);
    assert(t7.isLAN());

    TransportAddress t8("172.15.255.255");
    assert(t8.getIP() == (172 << 24) + (15 << 16) + (255 << 8) + 255);
    assert(!t8.isLAN());

    TransportAddress t9("172.32.0.0");
    assert(t9.getIP() == (172 << 24) + (32 << 16));
    assert(!t9.isLAN());

    TransportAddress t10("10.0.0.0");
    assert(t10.getIP() == (10 << 24));
    assert(t10.isLAN());

    TransportAddress t11("10.255.255.255");
    assert(t11.getIP() == (10 << 24) + (255 << 16) + (255 << 8) + 255);
    assert(t11.isLAN());

    TransportAddress t12("9.255.255.255");
    assert(t12.getIP() == (9 << 24) + (255 << 16) + (255 << 8) + 255);
    assert(!t12.isLAN());

    TransportAddress t13("11.0.0.0");
    assert(t13.getIP() == (11 << 24) );
    assert(!t13.isLAN());

    TransportAddress t14("127.0.0.0");
    assert(t14.getIP() == (127 << 24));
    assert(t14.isLAN());

    TransportAddress t15("127.255.255.255");
    assert(t15.getIP() == (127 << 24) + (255 << 16) + (255 << 8) + 255);
    assert(t15.isLAN());

    TransportAddress t16("126.255.255.255");
    assert(t16.getIP() == (126 << 24) + (255 << 16) + (255 << 8) + 255);
    assert(!t16.isLAN());

    TransportAddress t17("128.0.0.0");
    assert(t17.getIP() == (128 << 24));
    assert(t17.isLAN());

}   // unitTesting