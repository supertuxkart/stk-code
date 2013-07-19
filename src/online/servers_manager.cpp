//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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


#include "online/servers_manager.hpp"

#include <string>
#include <irrString.h>
#include <assert.h>
#include "online/http_connector.hpp"
#include "config/user_config.hpp"
#include "utils/translation.hpp"

namespace Online{

    static ServersManager* user_singleton = NULL;

    ServersManager* ServersManager::get()
    {
        if (user_singleton == NULL)
            user_singleton = new ServersManager();
        return user_singleton;
    }   // get

    void ServersManager::deallocate()
    {
        delete user_singleton;
        user_singleton = NULL;
    }   // deallocate

    // ============================================================================
    ServersManager::ServersManager(){
        m_servers = new PtrVector<Server>;
        refresh();
    }

    // ============================================================================
    void ServersManager::refresh()
    {
        HTTPConnector * connector = new HTTPConnector((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        connector->setParameter("action",std::string("get_server_list"));

        const XMLNode * result = connector->getXMLFromPage();
        std::string rec_success = "";
        if(result->get("success", &rec_success))
        {
            if (rec_success =="yes")
            {
                const XMLNode * servers_xml = result->getNode("servers");
                m_servers->clearAndDeleteAll();
                for (unsigned int i = 0; i < servers_xml->getNumNodes(); i++)
                {
                    m_servers->push_back(new Server(*servers_xml->getNode(i)));
                }
            }
        }
        //FIXME error message
    }

    // ============================================================================
    Server * ServersManager::getQuickPlay()
    {
        if(m_servers->size() > 0)
            return m_servers->get(0);
        return NULL;
    }
} // namespace Online
