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

#include "network/protocols/connect_to_server.hpp"

#include "network/client_network_manager.hpp"
#include "utils/time.hpp"
#include "utils/log.hpp"

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
        Log::info("ConnectToServer", "The Connect To Server protocol has \
                received an event notifying that he's connected to the peer.");
        m_state = DONE; // we received a message, we are connected
    }
}

// ----------------------------------------------------------------------------

void ConnectToServer::setup()
{
    m_state = NONE;
    if (m_server_ip == 0 || m_server_port == 0 )
    {
        Log::error("ConnectToServer", "You have to set the server's public \
                ip:port of the server.\n");
        m_listener->requestTerminate(this);
    }
}

// ----------------------------------------------------------------------------

void ConnectToServer::update()
{
    if (m_state == NONE)
    {
        static double target = 0;
        double currentTime = Time::getRealTime();
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
            Log::info("ConnectToServer", "Retrying to connect in 5 seconds.\n");
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

