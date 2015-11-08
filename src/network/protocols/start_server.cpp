//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

#include "network/protocols/start_server.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "network/network_manager.hpp"
#include "online/request_manager.hpp"

StartServer::StartServer() : Protocol(NULL, PROTOCOL_SILENT)
{
}   // StartServer

// ----------------------------------------------------------------------------
StartServer::~StartServer()
{
}   // ~StartServer

// ----------------------------------------------------------------------------
void StartServer::setup()
{
    m_state = NONE;
}   // setup

// ----------------------------------------------------------------------------
void StartServer::asynchronousUpdate()
{
    if (m_state == NONE)
    {
        const TransportAddress& addr = NetworkManager::getInstance()->getPublicAddress();
        m_request = new Online::XMLRequest();
        PlayerManager::setUserDetails(m_request, "start", Online::API::SERVER_PATH);

        m_request->addParameter("address", addr.getIP());
        m_request->addParameter("port", addr.getPort());
        m_request->addParameter("private_port", NetworkManager::getInstance()->getHost()->getPort());
        m_request->addParameter("max_players", UserConfigParams::m_server_max_players);

        Log::info("ShowPublicAddress", "Showing addr %s", addr.toString().c_str());

        Online::RequestManager::get()->addRequest(m_request);
        m_state = REQUEST_PENDING;
    }
    else if (m_state == REQUEST_PENDING && m_request->isDone())
    {
        const XMLNode * result = m_request->getXMLData();
        std::string rec_success;

        if(result->get("success", &rec_success))
        {
            if(rec_success == "yes")
            {
                Log::info("StartServer", "Server is now online.");
            }
            else
            {
                Log::error("StartServer", "Fail to start server.");
            }
        }
        else
        {
            Log::error("StartServer", "Fail to start server.");
        }
        m_state = DONE;
    }
    else if (m_state == DONE)
    {
        m_state = EXITING;
        delete m_request;
        m_request = NULL;
        m_listener->requestTerminate(this);
    }
}   // asynchronousUpdate

// ----------------------------------------------------------------------------
