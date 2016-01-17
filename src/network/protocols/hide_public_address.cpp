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

#include "network/protocols/hide_public_address.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "network/protocol_manager.hpp"
#include "online/request_manager.hpp"
#include "utils/log.hpp"

HidePublicAddress::HidePublicAddress() : Protocol(PROTOCOL_SILENT)
{
}

HidePublicAddress::~HidePublicAddress()
{
}

void HidePublicAddress::setup()
{
    m_state = NONE;
}

void HidePublicAddress::asynchronousUpdate()
{
    if (m_state == NONE)
    {
        m_request = new Online::XMLRequest();
        PlayerManager::setUserDetails(m_request, "unset", Online::API::SERVER_PATH);

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
                Log::debug("HidePublicAddress", "Address hidden successfully.");
            }
            else
            {
                Log::error("HidePublicAddress", "Fail to hide address.");
            }
        }
        else
        {
            Log::error("HidePublicAddress", "Fail to hide address.");
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
}
