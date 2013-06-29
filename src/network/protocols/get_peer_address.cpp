//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 SuperTuxKart-Team
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

#include "network/protocols/get_peer_address.hpp"

#include "network/time.hpp"
#include "network/http_functions.hpp"

#include <stdio.h>
#include <stdlib.h>

GetPeerAddress::GetPeerAddress(CallbackObject* callback_object) : Protocol(callback_object, PROTOCOL_SILENT)
{
}

GetPeerAddress::~GetPeerAddress()
{
}

void GetPeerAddress::notifyEvent(Event* event)
{
}

void GetPeerAddress::setup()
{
    m_state = NONE;
}

void GetPeerAddress::update()
{
    if (m_state == NONE)
    {
        static double target = 0;
        double current_time = Time::getSeconds();
        while (current_time < target-1800) // sometimes the getSeconds method forgets 3600 seconds.
            current_time += 3600;
        if (current_time > target)
        {
            char url[512];
            sprintf(url, "http://stkconnect.freeserver.me/log.php?get&nick=%s", m_peer_name.c_str());
            std::string result = HTTP::getPage(url);
            if (result == "")
            {
                printf("__GetPeerAddress> The host you try to reach does not exist. Change the host name please.\n");
                pause();
                return;
            }
            std::string ip_addr = result;
            ip_addr.erase(ip_addr.find_first_of(':'));
            std::string port_nb = result;
            port_nb.erase(0, port_nb.find_first_of(':')+1);
            uint32_t dst_ip = (uint32_t)(atoi(ip_addr.c_str()));
            uint16_t dst_port = (uint32_t)(atoi(port_nb.c_str()));
            if (dst_ip == 0 || dst_port == 0)
            {
                printf("__GetPeerAddress> The host you try to reach is not online. There will be a new try in 10 seconds.\n");
                target = current_time+10;
            }
            else
            {
                printf("__GetPeerAddress> Public ip of target is %i.%i.%i.%i:%i\n", (dst_ip>>24)&0xff, (dst_ip>>16)&0xff, (dst_ip>>8)&0xff, dst_ip&0xff, dst_port);
                uint32_t server_ip =   ((dst_ip&0x000000ff)<<24) // change the server IP to have a network-byte order
                                    + ((dst_ip&0x0000ff00)<<8)
                                    + ((dst_ip&0x00ff0000)>>8)
                                    + ((dst_ip&0xff000000)>>24); 
                uint16_t server_port = dst_port;
                TransportAddress* addr = static_cast<TransportAddress*>(m_callback_object);
                addr->ip = server_ip;
                addr->port = server_port;
                m_state = DONE;
            }
        }
    }
    else if (m_state == DONE)
    {
        m_listener->requestTerminate(this);
    }
}

void GetPeerAddress::setPeerName(std::string peer_name)
{
    m_peer_name = peer_name;
}
