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

#include "network/http_functions.hpp"
#include "online/http_connector.hpp"
#include "online/current_online_user.hpp"
#include "config/user_config.hpp"
#include "utils/log.hpp"

GetPeerAddress::GetPeerAddress(uint32_t peer_id, CallbackObject* callback_object) : Protocol(callback_object, PROTOCOL_SILENT)
{
    m_peer_id = peer_id;
}

GetPeerAddress::~GetPeerAddress()
{
}

void GetPeerAddress::notifyEvent(Event* event)
{
    // nothing there. If we receive events, they must be ignored
}

void GetPeerAddress::setup()
{
    m_state = NONE;
}

void GetPeerAddress::update()
{
    if (m_state == NONE)
    {
        
        HTTPConnector * connector = new HTTPConnector((std::string)UserConfigParams::m_server_multiplayer + "address-management.php");
        connector->setParameter("id",CurrentOnlineUser::get()->getUserID());
        connector->setParameter("token",CurrentOnlineUser::get()->getToken());
        connector->setParameter("peer_id",m_peer_id);
        connector->setParameter("action","get");

        const XMLNode * result = connector->getXMLFromPage();
        std::string rec_success;

        if(result->get("success", &rec_success))
        {
            if (rec_success == "yes")
            {
                TransportAddress* addr = static_cast<TransportAddress*>(m_callback_object);
                result->get("ip", &addr->ip);
                result->get("port", &addr->port);
                Log::debug("GetPeerAddress", "Address gotten successfully.");
            }
            else
            {
                Log::error("GetPeerAddress", "Fail to get address.");
            }
        }
        else
        {
            Log::error("GetPeerAddress", "Fail to get address.");
        }
        m_state = DONE;
    }
    else if (m_state == DONE)
    {
        m_listener->requestTerminate(this);
    }
}

void GetPeerAddress::setPeerID(uint32_t peer_id)
{
    m_peer_id = peer_id;
}
