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
#include "utils/translation.hpp"

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
    m_is_guest = false;
    m_id = 0;
    m_name = "";
    m_token = "";
}

// ============================================================================
// Register
bool CurrentOnlineUser::signUp( const irr::core::stringw &username,
                                const irr::core::stringw &password,
                                const irr::core::stringw &password_ver,
                                const irr::core::stringw &email,
                                bool terms,
                                irr::core::stringw &info)
{
    assert(m_is_signed_in == false);
    HTTPConnector * connector = new HTTPConnector((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
    connector->setParameter("action",std::string("register"));
    connector->setParameter("username",username);
    connector->setParameter("password",password);

    const XMLNode * result = connector->getXMLFromPage();
    std::string rec_success;

    bool success = false;
    if(result->get("success", &rec_success))
    {
        success = (rec_success == "yes");
        assert(result->get("info", &info));
    }
    else
    {
            info = _("Unable to connect to the server. Check your internet connection or try again later.");
    }
    return success;
}


// ============================================================================

bool CurrentOnlineUser::signIn( const irr::core::stringw &username,
                                const irr::core::stringw &password,
                                irr::core::stringw &info)
{
    assert(m_is_signed_in == false);
    HTTPConnector * connector = new HTTPConnector((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
    connector->setParameter("action",std::string("connect"));
    connector->setParameter("username",username);
    connector->setParameter("password",password);
    const XMLNode * result = connector->getXMLFromPage();
    std::string rec_success = "";
    if(result->get("success", &rec_success))
    {
        if (rec_success =="yes")
        {
            int token_fetched = result->get("token", &m_token);
            int username_fetched = result->get("username", &m_name);
            int userid_fetched = result->get("userid", &m_id);
            assert(token_fetched && username_fetched && userid_fetched);
            m_is_signed_in = true;
            m_is_guest = false;
        }
        result->get("info", &info);
    }
    else
    {
        info = _("Unable to connect to the server. Check your internet connection or try again later.");
    }

    return m_is_signed_in;
}
// ============================================================================
bool CurrentOnlineUser::signOut(){
    assert(m_is_signed_in == true);
    HTTPConnector * connector = new HTTPConnector((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
    connector->setParameter("action",std::string("disconnect"));
    connector->setParameter("token",m_token);
    connector->setParameter("userid",m_id);

    const XMLNode * result = connector->getXMLFromPage();
    std::string rec_success = "";
    if(result->get("success", &rec_success))
    {
        if (rec_success =="yes")
        {
            m_token = "";
            m_name = "";
            m_id = 0;
            m_is_signed_in = false;
            m_is_guest = false;
        }
    }
    return !m_is_signed_in;
}

// ============================================================================

irr::core::stringw CurrentOnlineUser::getUserName() const
{
    if(m_is_signed_in)
        return m_name;
    else
        return _("Currently not signed in");
}
