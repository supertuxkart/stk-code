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
#include "online/http_connector.hpp"
#include "config/user_config.hpp"
#include "utils/translation.hpp"
#include "utils/log.hpp"

namespace Online{
    static Synchronised<CurrentUser*> user_singleton(NULL);

    CurrentUser* CurrentUser::get()
    {
        CurrentUser* user = user_singleton.getData();
        if (user == NULL)
            user = new CurrentUser();
        return user;
    }   // get

    CurrentUser* CurrentUser::acquire()
    {
        user_singleton.lock();
        CurrentUser * user = user_singleton.getData();
        if (user == NULL)
        {
            user_singleton.unlock();
            user = new CurrentUser();
            user_singleton.setAtomic(user);
            user_singleton.lock();
        }
        return user;
    }

    void CurrentUser::release()
    {
        user_singleton.unlock();
    }

    void CurrentUser::deallocate()
    {
        user_singleton.lock();
        CurrentUser* user = user_singleton.getData();
        delete user;
        user = NULL;
        user_singleton.unlock();
    }   // deallocate

    // ============================================================================
    CurrentUser::CurrentUser(){
        m_state = SIGNED_OUT;
        m_id = 0;
        m_name = "";
        m_token = "";
        m_save_session = false;
    }

    // ============================================================================
    XMLRequest * CurrentUser::requestSignUp(    const irr::core::stringw &username,
                                                const irr::core::stringw &password,
                                                const irr::core::stringw &password_confirm,
                                                const irr::core::stringw &email,
                                                bool terms)
    {
        assert(m_state == SIGNED_OUT || m_state == GUEST);
        XMLRequest * request = new XMLRequest(Request::RT_SIGN_UP);
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action",std::string("register"));
        request->setParameter("username",username);
        request->setParameter("password",password);
        request->setParameter("password_confirm",password_confirm);
        HTTPManager::get()->addRequest(request);
        return request;
    }

    // ============================================================================
    SignInRequest * CurrentUser::requestSavedSession()
    {
        SignInRequest * request = NULL;
        if(m_state != SIGNED_IN  && UserConfigParams::m_saved_session)
        {
            request = new SignInRequest();
            request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
            request->setParameter("action",std::string("saved-session"));
            request->setParameter("userid", UserConfigParams::m_saved_user);
            request->setParameter("token", UserConfigParams::m_saved_token.c_str());
            HTTPManager::get()->addRequest(request);
            m_state = SIGNING_IN;
        }
        return request;
    }

    SignInRequest * CurrentUser::requestSignIn(    const irr::core::stringw &username,
                                                                const irr::core::stringw &password,
                                                                bool save_session)
    {
        assert(m_state == SIGNED_OUT);
        m_save_session = save_session;
        SignInRequest * request = new SignInRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action",std::string("connect"));
        request->setParameter("username",username);
        request->setParameter("password",password);
        HTTPManager::get()->addRequest(request);
        m_state = SIGNING_IN;
        return request;
    }

    void CurrentUser::signIn(const SignInRequest * input)
    {
        if (input->isSuccess())
        {
            int token_fetched       = input->getResult()->get("token", &m_token);
            int username_fetched    = input->getResult()->get("username", &m_name);
            int userid_fetched      = input->getResult()->get("userid", &m_id);
            assert(token_fetched && username_fetched && userid_fetched);
            m_state = SIGNED_IN;
            if(m_save_session)
            {
                UserConfigParams::m_saved_user = m_id;
                UserConfigParams::m_saved_token = m_token;
                UserConfigParams::m_saved_session = true;
                m_save_session = false;
            }
        }
        else
            m_state = SIGNED_OUT;
    }

    void SignInRequest::callback()
    {
        CurrentUser::acquire()->signIn(this);
        CurrentUser::release();
    }

    // ============================================================================

    ServerCreationRequest * CurrentUser::requestServerCreation(const irr::core::stringw &name, int max_players)
    {
        assert(m_state == SIGNED_IN);
        ServerCreationRequest * request = new ServerCreationRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action",           std::string("create_server"));
        request->setParameter("token",            m_token);
        request->setParameter("userid",           m_id);
        request->setParameter("name",             name);
        request->setParameter("max_players",      max_players);
        HTTPManager::get()->addRequest(request);
        return request;
    }

    void ServerCreationRequest::callback()
    {
        //FIXME
    }

    // ============================================================================
    SignOutRequest * CurrentUser::requestSignOut(){
        assert(m_state == SIGNED_IN || m_state == GUEST);
        SignOutRequest * request = new SignOutRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action",std::string("disconnect"));
        request->setParameter("token",m_token);
        request->setParameter("userid",m_id);
        HTTPManager::get()->addRequest(request);
        m_state = SIGNING_OUT;
        return request;
    }

    void CurrentUser::signOut(const SignOutRequest * input)
    {
        if(!input->isSuccess())
        {
            Log::warn("CurrentUser::signOut", "%s", _("There were some connection issues while signing out. Report a bug if this caused issues."));
        }
        m_token = "";
        m_name = "";
        m_id = 0;
        m_state = SIGNED_OUT;
        UserConfigParams::m_saved_user = 0;
        UserConfigParams::m_saved_token = "";
        UserConfigParams::m_saved_session = false;
    }

    void SignOutRequest::callback()
    {
        CurrentUser::acquire()->signOut(this);
        CurrentUser::release();
    }

    // ============================================================================

    ServerJoinRequest *  CurrentUser::requestServerJoin(uint32_t server_id){
        assert(m_state == SIGNED_IN || m_state == GUEST);
        ServerJoinRequest * request = new ServerJoinRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "address-management.php");
        request->setParameter("action",std::string("request-connection"));
        request->setParameter("token", m_token);
        request->setParameter("id", m_id);
        request->setParameter("server_id", server_id);
        return request;
    }

    void ServerJoinRequest::callback()
    {
        //FIXME
    }

    // ============================================================================

    irr::core::stringw CurrentUser::getUserName() const
    {
        if((m_state == SIGNED_IN ) || (m_state == GUEST))
            return m_name;
        else
            return _("Currently not signed in");
    }

} // namespace Online
