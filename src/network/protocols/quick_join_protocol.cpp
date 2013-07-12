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

#include "quick_join_protocol.hpp"

#include "network/network_manager.hpp"
#include "online/current_online_user.hpp"
#include "online/http_connector.hpp"
#include "config/user_config.hpp"
#include "utils/log.hpp"

QuickJoinProtocol::QuickJoinProtocol(CallbackObject* callback_object, uint32_t* server_id) : Protocol(callback_object, PROTOCOL_SILENT)
{
    m_server_id = server_id;
}

QuickJoinProtocol::~QuickJoinProtocol()
{
}

void QuickJoinProtocol::notifyEvent(Event* event)
{
}

void QuickJoinProtocol::setup()
{
    m_state = NONE;
}

void QuickJoinProtocol::update()
{
    if (m_state == NONE)
    {
        TransportAddress addr = NetworkManager::getInstance()->getPublicAddress();
        HTTPConnector connector((std::string)UserConfigParams::m_server_multiplayer + "address-management.php");
        connector.setParameter("id",CurrentOnlineUser::get()->getUserID());
        connector.setParameter("token",CurrentOnlineUser::get()->getToken());
        connector.setParameter("action","quick-join");

        const XMLNode * result = connector.getXMLFromPage();
        std::string rec_success;
        TransportAddress* res = static_cast<TransportAddress*>(m_callback_object);

        if(result->get("success", &rec_success))
        {
            if(rec_success == "yes")
            {
                result->get("ip", &res->ip);
                result->get("port", &res->port);
                result->get("hostid", m_server_id);
                Log::info("QuickJoinProtocol", "Quick joining %d:%d (server#%d).", res->ip, res->port, *m_server_id);
            }
            else
            {
                Log::error("QuickJoinProtocol", "Fail to quick join.");
            }
        }
        else
        {
            Log::error("QuickJoinProtocol", "Fail to quick join.");
        }
        m_state = DONE;
    }
    else if (m_state == DONE)
    {
        m_listener->requestTerminate(this);
    }
}
