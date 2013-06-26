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

#include "show_public_address.hpp"

#include "../http_functions.hpp"

#include <stdio.h>

ShowPublicAddress::ShowPublicAddress(CallbackObject* callback_object) : Protocol(callback_object, PROTOCOL_SILENT)
{
}

ShowPublicAddress::~ShowPublicAddress()
{
}

void ShowPublicAddress::notifyEvent(Event* event)
{
}

void ShowPublicAddress::setup()
{
    m_state = NONE;
    if (m_public_ip == 0 || m_public_port == 0 || m_username == "" || m_password == "")
    {
        printf("__ShowPublicAddress> You have to set the public ip:port, username:password and the host nickname before starting this protocol.\n");
        m_listener->requestTerminate(this);
    }
}

void ShowPublicAddress::update()
{
    if (m_state == NONE)
    {
       char url[512];
        sprintf(url, "http://stkconnect.freeserver.me/log.php?set&nick=%s&ip=%u&port=%u&pwd=%s", m_username.c_str(), m_public_ip, m_public_port, m_password.c_str());
        std::string result = HTTP::getPage(url);
        if (result[0] == 's' && result[1] == 'u' && result[2] == 'c' && result[3] == 'c' && result[4] == 'e' && result[5] == 's' && result[6] == 's')
        {
            printf("__ShowPublicAddress> Address set.\n");
            m_state = DONE;
        }
        if (result[0] == 'f' && result[1] == 'a' && result[2] == 'i' && result[3] == 'l')
        {
            printf("__ShowPublicAddress> Login fail. Please re-set username:password and unpause the protocol.\n");
            m_state = NONE;
            pause();
        }
    }
    else if (m_state == DONE)
    {
        m_listener->requestTerminate(this);
    }
}

void ShowPublicAddress::setUsername(std::string username)
{
    m_username = username;
}
void ShowPublicAddress::setPassword(std::string password)
{
    m_password = password;
}
void ShowPublicAddress::setPublicAddress(uint32_t ip, uint16_t port)
{
    m_public_ip = ip;
    m_public_port = port;
}
