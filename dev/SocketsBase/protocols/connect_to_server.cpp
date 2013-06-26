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

#include "protocols/connect_to_server.hpp"

#include "client_network_manager.hpp"
#include "time.hpp"

#include <stdio.h>
#include <stdlib.h>

// ----------------------------------------------------------------------------

ConnectToServer::ConnectToServer(CallbackObject* callback_object) : 
    Protocol(callback_object, PROTOCOL_CONNECTION)
{
    m_server_ip = 0;
    m_server_port = 0;
    m_state = NONE;
}

// ----------------------------------------------------------------------------

ConnectToServer::~ConnectToServer()
{
}

// ----------------------------------------------------------------------------

void ConnectToServer::notifyEvent(Event* event)
{
    if (event->type == EVENT_TYPE_CONNECTED && 
        event->peer->getAddress() == m_server_ip && 
        event->peer->getPort() == m_server_port)
    {
        printf("The Connect To Server protocol has received an event notifying \
                that he's connected to the peer. The peer sent \"%s\"\n", 
                event->data.c_str());
        m_state = DONE; // we received a message, we are connected
    }
}

// ----------------------------------------------------------------------------

void ConnectToServer::setup()
{
    m_state = NONE;
    if (m_server_ip == 0 || m_server_port == 0 )
    {
        printf("You have to set the server's public ip:port of the server.\n");
        m_listener->requestTerminate(this);
    }
}

// ----------------------------------------------------------------------------

void ConnectToServer::update()
{
    if (m_state == NONE)
    {
        static double target = 0;
        double currentTime = Time::getSeconds();
         // sometimes the getSeconds method forgets 3600 seconds.
        while (currentTime < target-1800)
            currentTime += 3600;
        if (currentTime > target)
        {
            NetworkManager::getInstance()->connect(
                                TransportAddress(m_server_ip, m_server_port));
            if (NetworkManager::getInstance()->isConnectedTo(
                                TransportAddress(m_server_ip, m_server_port)))
            {   
                m_state = DONE;
                return;
            }
            target = currentTime+5;
            printf("Retrying to connect in 5 seconds.\n");
        }
    }
    else if (m_state == DONE)
    {
        m_listener->requestTerminate(this);
    }
}

// ----------------------------------------------------------------------------

void ConnectToServer::setServerAddress(uint32_t ip, uint16_t port)
{
    m_server_ip = ip;
    m_server_port = port;
}

// ----------------------------------------------------------------------------

