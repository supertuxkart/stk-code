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
    bool CurrentUser::trySavedSession()
    {
        if (m_state == SIGNED_IN) return true;
        bool success = false;
        if(UserConfigParams::m_saved_session)
        {
            HTTPConnector * connector = new HTTPConnector((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
            connector->setParameter("action",std::string("saved-session"));
            connector->setParameter("userid", UserConfigParams::m_saved_user);
            connector->setParameter("token", UserConfigParams::m_saved_token.c_str());
            const XMLNode * result = connector->getXMLFromPage();
            std::string rec_success = "";
            std::string info;
            if(result->get("success", &rec_success))
            {
                if (rec_success =="yes")
                {
                    int token_fetched = result->get("token", &m_token);
                    int username_fetched = result->get("username", &m_name);
                    int userid_fetched = result->get("userid", &m_id);
                    assert(token_fetched && username_fetched && userid_fetched);
                    UserConfigParams::m_saved_token = m_token;
                    m_state = SIGNED_IN;
                    success = true;
                }
                result->get("info", &info);
            }
            else
            {
                Log::error("trySavedSession","%s",
                    _("Unable to connect to the server. Check your internet connection or try again later."));
            }
        }
        return success;
    }

    // ============================================================================
    // Register
    bool CurrentUser::signUp(   const irr::core::stringw &username,
                                const irr::core::stringw &password,
                                const irr::core::stringw &password_ver,
                                const irr::core::stringw &email,
                                bool terms,
                                irr::core::stringw &info)
    {
        assert(m_state == SIGNED_OUT || m_state == GUEST);
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

    void CurrentUser::requestSignIn(    const irr::core::stringw &username,
                                        const irr::core::stringw &password,
                                        bool save_session)
    {
        assert(m_state == SIGNED_OUT);
        m_save_session = save_session;
        XMLRequest * request = new XMLRequest((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setListener(this, CurrentUser::SIGN_IN_REQUEST);
        request->setParameter("action",std::string("connect"));
        request->setParameter("username",username);
        request->setParameter("password",password);
        if(!HTTPManager::get()->addRequest(request))
        {
            assert(false);
        }
        m_state = SIGNING_IN;
    }

    bool CurrentUser::signIn(XMLRequest * input)
    {
        const XMLNode * xml = input->getResult();
        bool success = false;
        irr::core::stringw info;
        std::string rec_success;
        if(xml->get("success", &rec_success))
        {
            if (rec_success =="yes")
            {
                int token_fetched       = xml->get("token", &m_token);
                int username_fetched    = xml->get("username", &m_name);
                int userid_fetched      = xml->get("userid", &m_id);
                assert(token_fetched && username_fetched && userid_fetched);
                m_state = SIGNED_IN;
                success = true;
                if(m_save_session)
                {
                    UserConfigParams::m_saved_user = m_id;
                    UserConfigParams::m_saved_token = m_token;
                    UserConfigParams::m_saved_session = true;
                }
            }
            xml->get("info", &info);
        }
        else
        {
            info = _("Unable to connect to the server. Check your internet connection or try again later.");
        }
        input->setInfo(info);
        if(!success) m_state = SIGNED_OUT;
        return success;
    }

    // ============================================================================

    XMLRequest * CurrentUser::createServerRequest ( const irr::core::stringw &name, int max_players)
    {
        assert(m_state == SIGNED_IN);
        XMLRequest * request = new XMLRequest((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action",           std::string("create_server"));
        request->setParameter("token",            m_token);
        request->setParameter("userid",           m_id);
        request->setParameter("name",             name);
        request->setParameter("max_players",      max_players);
        if(!HTTPManager::get()->addRequest(request))
            assert(false);
        return request;
    }

    bool CurrentUser::createServer( const XMLNode * input , irr::core::stringw &info)
    {
        bool success = false;
        std::string rec_success = "";
        if(input->get("success", &rec_success))
        {
           if (rec_success =="yes")
           {
               // FIXME
               success = true;
           }
           input->get("info", &info);
        }
        else
        {
           info = _("Unable to connect to the server. Check your internet connection or try again later.");
        }

        return success;
    }


    // ============================================================================
    bool CurrentUser::signOut(irr::core::stringw &info){
        assert(m_state == SIGNED_IN || m_state == GUEST);
        HTTPConnector * connector = new HTTPConnector((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        connector->setParameter("action",std::string("disconnect"));
        connector->setParameter("token",m_token);
        connector->setParameter("userid",m_id);

        bool success = false;
        const XMLNode * result = connector->getXMLFromPage();
        std::string rec_success = "";
        if(result->get("success", &rec_success))
        {
            if (rec_success =="yes")
            {
                m_token = "";
                m_name = "";
                m_id = 0;
                m_state = SIGNED_OUT;
                UserConfigParams::m_saved_user = 0;
                UserConfigParams::m_saved_token = "";
                UserConfigParams::m_saved_session = false;
                success = true;
            }
            result->get("info", &info);
        }
        else
        {
            info = _("Unable to connect to the server. Check your internet connection or try again later.");
        }
        return success;
    }

    // ============================================================================

    bool CurrentUser::requestJoin(uint32_t server_id, irr::core::stringw &info){
        assert(m_state == SIGNED_IN || m_state == GUEST);
        HTTPConnector * connector = new HTTPConnector((std::string)UserConfigParams::m_server_multiplayer + "address-management.php");
        connector->setParameter("action",std::string("request-connection"));
        connector->setParameter("token", m_token);
        connector->setParameter("id", m_id);
        connector->setParameter("server_id", server_id);
        bool success = false;
        const XMLNode * result = connector->getXMLFromPage();
        std::string rec_success = "";
        if(result->get("success", &rec_success))
        {
            if (rec_success =="yes")
            {
                success = true;
            }
            else
            {
                success = false;
            }
            result->get("info", &info);
        }
        else
        {
            info = _("Unable to connect to the server. Check your internet connection or try again later.");
        }
        return success;
    }

    // ============================================================================

    irr::core::stringw CurrentUser::getUserName() const
    {
        if((m_state == SIGNED_IN ) || (m_state == GUEST))
            return m_name;
        else
            return _("Currently not signed in");
    }


    // ============================================================================


    void CurrentUser::onHTTPCallback(HTTPRequest * finished_request)
    {
        XMLRequest * input = (XMLRequest *) finished_request;

        switch(finished_request->getListenerTarget())
        {
            case CurrentUser::SIGN_IN_REQUEST:
                 signIn(input);
                 break;
            default:
                 break;
        }
    }
} // namespace Online
