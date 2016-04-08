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

#include "network/protocols/stop_server.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "network/network_config.hpp"
#include "online/request_manager.hpp"

StopServer::StopServer() : Protocol(PROTOCOL_SILENT)
{
}

StopServer::~StopServer()
{
}

bool StopServer::notifyEventAsynchronous(Event* event)
{
    return true;
}

void StopServer::setup()
{
    m_state = NONE;
}

void StopServer::asynchronousUpdate()
{
    if (m_state == NONE)
    {
        const TransportAddress& addr = NetworkConfig::get()->getMyAddress();
        m_request = new Online::XMLRequest();
        PlayerManager::setUserDetails(m_request, "stop", Online::API::SERVER_PATH);

        m_request->addParameter("address", addr.getIP());
        m_request->addParameter("port", addr.getPort());

        Log::info("StopServer", "address %s", addr.toString().c_str());

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
                Log::info("StopServer", "Server is now offline.");
            }
            else
            {
                Log::error("StopServer", "Fail to stop server.");
            }
        }
        else
        {
            Log::error("StopServer", "Fail to stop server.");
        }
        m_state = DONE;
    }
    else if (m_state == DONE)
    {
        m_state = EXITING;
        delete m_request;
        m_request = NULL;
        requestTerminate();
    }
}   // asynchronousUpdate
