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


#include "online/current_user.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>
#include "config/user_config.hpp"
#include "utils/translation.hpp"
#include "utils/log.hpp"
#include "online/servers_manager.hpp"

using namespace Online;

namespace Online{
    static CurrentUser* current_user_singleton(NULL);

    CurrentUser* CurrentUser::get()
    {
        if (current_user_singleton == NULL)
            current_user_singleton = new CurrentUser();
        return current_user_singleton;
    }

    void CurrentUser::deallocate()
    {
        delete current_user_singleton;
        current_user_singleton = NULL;
    }   // deallocate

    // ============================================================================
    CurrentUser::CurrentUser()
        : User("",0)
    {
        setState(US_SIGNED_OUT);
        setToken("");
        setSaveSession(false);
    }

    // ============================================================================
    const XMLRequest * CurrentUser::requestSignUp(  const irr::core::stringw &username,
                                                    const irr::core::stringw &password,
                                                    const irr::core::stringw &password_confirm,
                                                    const irr::core::stringw &email,
                                                    bool terms)
    {
        assert(getState() == US_SIGNED_OUT || getState() == US_GUEST);
        XMLRequest * request = new XMLRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("register"));
        request->setParameter("username", username);
        request->setParameter("password", password);
        request->setParameter("password_confirm", password_confirm);
        request->setParameter("email", email);
        request->setParameter("terms", std::string("on"));
        HTTPManager::get()->addRequest(request);
        return request;
    }

    // ============================================================================
    const CurrentUser::SignInRequest * CurrentUser::requestSavedSession()
    {
        SignInRequest * request = NULL;
        if(getState() != US_SIGNED_IN  && UserConfigParams::m_saved_session)
        {
            request = new SignInRequest();
            request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
            request->setParameter("action",std::string("saved-session"));
            request->setParameter("userid", UserConfigParams::m_saved_user);
            request->setParameter("token", UserConfigParams::m_saved_token.c_str());
            HTTPManager::get()->addRequest(request);
            setState(US_SIGNING_IN);
        }
        return request;
    }

    const CurrentUser::SignInRequest * CurrentUser::requestSignIn(  const irr::core::stringw &username,
                                                                    const irr::core::stringw &password,
                                                                    bool save_session)
    {
        assert(getState() == US_SIGNED_OUT);
        setSaveSession(save_session);
        SignInRequest * request = new SignInRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action",std::string("connect"));
        request->setParameter("username",username);
        request->setParameter("password",password);
        HTTPManager::get()->addRequest(request);
        setState(US_SIGNING_IN);
        return request;
    }

    void CurrentUser::signIn(const SignInRequest * input)
    {
        if (input->isSuccess())
        {
            std::string token("");
            int token_fetched       = input->getResult()->get("token", &token);
            setToken(token);
            irr::core::stringw username("");
            int username_fetched    = input->getResult()->get("username", &username);
            setUserName(username);
            uint32_t userid(0);
            int userid_fetched      = input->getResult()->get("userid", &userid);
            setUserID(userid);
            assert(token_fetched && username_fetched && userid_fetched);
            setState(US_SIGNED_IN);
            if(getSaveSession())
            {
                UserConfigParams::m_saved_user = getUserID();
                UserConfigParams::m_saved_token = getToken();
                UserConfigParams::m_saved_session = true;
            }
        }
        else
            setState(US_SIGNED_OUT);
    }

    void CurrentUser::SignInRequest::callback()
    {
        CurrentUser::get()->signIn(this);
    }

    // ============================================================================

    const CurrentUser::ServerCreationRequest * CurrentUser::requestServerCreation(  const irr::core::stringw &name,
                                                                                    int max_players)
    {
        assert(getState() == US_SIGNED_IN);
        ServerCreationRequest * request = new ServerCreationRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action",           std::string("create_server"));
        request->setParameter("token",            getToken());
        request->setParameter("userid",           getUserID());
        request->setParameter("name",             name);
        request->setParameter("max_players",      max_players);
        HTTPManager::get()->addRequest(request);
        return request;
    }

    void CurrentUser::ServerCreationRequest::callback()
    {
        if(isSuccess())
        {
            Server * server = new Server(*getResult()->getNode("server"));
            ServersManager::get()->addServer(server);
            m_created_server_id.setAtomic(server->getServerId());
        }
    }

    // ============================================================================
    const CurrentUser::SignOutRequest * CurrentUser::requestSignOut(){
        assert(getState() == US_SIGNED_IN || getState() == US_GUEST);
        SignOutRequest * request = new SignOutRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action",std::string("disconnect"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getUserID());
        HTTPManager::get()->addRequest(request);
        setState(US_SIGNING_OUT);
        return request;
    }

    void CurrentUser::signOut(const SignOutRequest * input)
    {
        if(!input->isSuccess())
        {
            Log::warn("CurrentUser::signOut", "%s", _("There were some connection issues while signing out. Report a bug if this caused issues."));
        }
        setToken("");
        setUserName("");
        setUserID(0);
        setState(US_SIGNED_OUT);
        UserConfigParams::m_saved_user = 0;
        UserConfigParams::m_saved_token = "";
        UserConfigParams::m_saved_session = false;
    }

    void CurrentUser::SignOutRequest::callback()
    {
        CurrentUser::get()->signOut(this);
    }

    // ============================================================================

    const CurrentUser::ServerJoinRequest *  CurrentUser::requestServerJoin(uint32_t server_id){
        assert(getState() == US_SIGNED_IN || getState() == US_GUEST);
        ServerJoinRequest * request = new ServerJoinRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "address-management.php");
        request->setParameter("action",std::string("request-connection"));
        request->setParameter("token", getToken());
        request->setParameter("id", getUserID());
        request->setParameter("server_id", server_id);
        HTTPManager::get()->addRequest(request);
        return request;
    }

    void CurrentUser::ServerJoinRequest::callback()
    {
        if(isSuccess())
        {
            uint32_t server_id;
            getResult()->get("serverid", &server_id);
            ServersManager::get()->setJoinedServer(server_id);
        }
        //FIXME needs changes for actual valid joining
    }

    // ============================================================================

    const irr::core::stringw CurrentUser::getUserName() const
    {
        if((getState() == US_SIGNED_IN ) || (getState() == US_GUEST))
            return getUserName();
        else
            return _("Currently not signed in");
    }
} // namespace Online
