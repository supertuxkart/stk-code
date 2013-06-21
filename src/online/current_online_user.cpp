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


#include "online/current_online_user.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>
#include "online/http_connector.hpp"
#include "config/user_config.hpp"

static CurrentOnlineUser* user_singleton = NULL;

CurrentOnlineUser* CurrentOnlineUser::get()
{
    if (user_singleton == NULL)
        user_singleton = new CurrentOnlineUser();
    return user_singleton;
}   // get

void CurrentOnlineUser::deallocate()
{
    delete user_singleton;
    user_singleton = NULL;
}   // deallocate

// ============================================================================


CurrentOnlineUser::CurrentOnlineUser(){
    m_is_signed_in = false;
}

// ============================================================================

bool CurrentOnlineUser::signIn(const std::string &username, const std::string &password)
{
    assert(m_is_signed_in == false);
    HTTPConnector * connector = new HTTPConnector((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
    HTTPConnector::Parameters parameters;
    parameters["action"] = "connect";
    parameters["user"] = username;
    parameters["password"] = password;
    const XMLNode * result = connector->getXMLFromPage(parameters);
    std::string token;
    bool succes = result->get("token", &token);
    if(succes)
    {
        m_user = new OnlineUser(username);
        m_token = token;
        m_is_signed_in = true;
    }
    else
    {
        //I don't know if something should happen here yet
    }
    return m_is_signed_in;
}

// ============================================================================

std::string CurrentOnlineUser::getUserName() const
{
    if(m_is_signed_in){
        assert(m_user != NULL);
        return m_user->getUserName();
    }else{
        return "Guest";
    }

}
