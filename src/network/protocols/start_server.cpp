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

#include "network/protocols/start_server.hpp"

#include "network/network_manager.hpp"
#include "online/current_user.hpp"
#include "online/http_manager.hpp"
#include "config/user_config.hpp"

StartServer::StartServer() : Protocol(NULL, PROTOCOL_SILENT)
{
}

StartServer::~StartServer()
{
}

void StartServer::setup()
{
    m_state = NONE;
}

void StartServer::asynchronousUpdate()
{
    if (m_state == NONE)
    {
        TransportAddress addr = NetworkManager::getInstance()->getPublicAddress();
        m_request = new Online::XMLRequest();
        m_request->setURL((std::string)UserConfigParams::m_server_multiplayer + "address-management.php");
        m_request->setParameter("id",Online::CurrentUser::get()->getUserID());
        m_request->setParameter("token",Online::CurrentUser::get()->getToken());
        m_request->setParameter("address",addr.ip);
        m_request->setParameter("port",addr.port);
        m_request->setParameter("max_players",UserConfigParams::m_server_max_players);
        m_request->setParameter("action","start-server");

        Online::HTTPManager::get()->addRequest(m_request);
        m_state = REQUEST_PENDING;
    }
    else if (m_state == REQUEST_PENDING && m_request->isDone())
    {
        const XMLNode * result = m_request->getResult();
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
}
