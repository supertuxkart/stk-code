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

#include "network/protocols/get_peer_address.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "network/protocol_manager.hpp"
#include "network/stk_host.hpp"
#include "online/request_manager.hpp"
#include "utils/log.hpp"

GetPeerAddress::GetPeerAddress(uint32_t peer_id, CallbackObject* callback_object)
              : Protocol(PROTOCOL_SILENT, callback_object)
{
    m_peer_id = peer_id;
}   // GetPeerAddress

// ----------------------------------------------------------------------------
GetPeerAddress::~GetPeerAddress()
{
}   // ~GetPeerAddress

// ----------------------------------------------------------------------------
void GetPeerAddress::setup()
{
    m_address.clear();

    m_request = new Online::XMLRequest();
    PlayerManager::setUserDetails(m_request, "get",
                                  Online::API::SERVER_PATH);
    m_request->addParameter("peer_id", m_peer_id);

    Online::RequestManager::get()->addRequest(m_request);
}   // setup

// ----------------------------------------------------------------------------
void GetPeerAddress::asynchronousUpdate()
{
    if (m_request->isDone())
    {
        const XMLNode * result = m_request->getXMLData();
        delete m_request;
        m_request = NULL;

        std::string success;
        if(result->get("success", &success) && success == "yes")
        {
            uint32_t ip;
            result->get("ip", &ip);
            m_address.setIP(ip);

            uint16_t port;
            if (m_address.getIP() == STKHost::get()->getPublicAddress().getIP())
                result->get("private_port", &port);
            else
                result->get("port", &port);
            m_address.setPort(port);

            Log::debug("GetPeerAddress", "Peer address retrieved.");
        }
        else
        {
            Log::error("GetPeerAddress", "Failed to get peer address.");
        }
        requestTerminate();
    }
}   // asynchronousUpdate

// ----------------------------------------------------------------------------
void GetPeerAddress::setPeerID(uint32_t peer_id)
{
    m_peer_id = peer_id;
}   // setPeerID
