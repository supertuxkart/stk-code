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

#ifndef HEADER_CURRENT_ONLINE_USER_HPP
#define HEADER_CURRENT_ONLINE_USER_HPP

#include "online/user.hpp"
#include <string>
#include <irrString.h>
#include "utils/types.hpp"
#include "online/server.hpp"
#include "http_manager.hpp"
#include "utils/synchronised.hpp"


namespace Online{

    // ============================================================================

    /**
      * \brief Class that represents an online registered user
      * \ingroup online
      */
    class CurrentUser : public User
    {
        public:
            enum UserState
            {
                US_SIGNED_OUT,
                US_SIGNED_IN,
                US_GUEST,
                US_SIGNING_IN,
                US_SIGNING_OUT
            };

            enum RequestType
            {
                RT_SIGN_IN = 1,
                RT_SIGN_OUT,
                RT_SIGN_UP,
                RT_SERVER_JOIN,
                RT_SERVER_CREATION
            };

            class SignInRequest : public XMLRequest
            {
                virtual void callback ();
            public:
                SignInRequest() : XMLRequest(RT_SIGN_IN) {}
            };

            class SignOutRequest : public XMLRequest
            {
                virtual void callback ();
            public:
                SignOutRequest() : XMLRequest(RT_SIGN_OUT) {}
            };

            class ServerCreationRequest : public XMLRequest {
                virtual void callback ();
                Synchronised<uint32_t> m_created_server_id;
            public:
                ServerCreationRequest() : XMLRequest(RT_SERVER_CREATION) {}
                const uint32_t getCreatedServerID() const {return m_created_server_id.getAtomic();}
            };

            class ServerJoinRequest : public XMLRequest {
                virtual void callback ();
            public:
                ServerJoinRequest() : XMLRequest(RT_SERVER_JOIN) {}
            };


        private:
            Synchronised<std::string>   m_token;
            Synchronised<bool>          m_save_session;
            Synchronised<UserState>     m_state;

            UserState                   getState()              const   { return m_state.getAtomic();             }
            bool                        getSaveSession()        const   { return m_save_session.getAtomic();      }
            const std::string           getToken()              const   { return m_token.getAtomic();      }

            void setState               (UserState user_state)          { m_state.setAtomic(user_state);          }
            void setSaveSession         (bool save_session)             { m_save_session.setAtomic(save_session); }
            void setToken               (const std::string & token)     { m_token.setAtomic(token);               }

            CurrentUser();

            void signIn                 (const SignInRequest            * input);
            void signOut                (const SignOutRequest           * input);
            void createServer           (const ServerCreationRequest    * input);

        public:
            //Singleton
            static CurrentUser *            get();
            static void                     deallocate();

            const SignInRequest *           requestSavedSession();
            const SignInRequest *           requestSignIn(          const irr::core::stringw &username,
                                                                    const irr::core::stringw &password,
                                                                    bool save_session);
            const SignOutRequest *          requestSignOut();
            const ServerCreationRequest *   requestServerCreation(  const irr::core::stringw &name, int max_players);
            const ServerJoinRequest *       requestServerJoin(      uint32_t server_id);


            // Register
            const XMLRequest *              requestSignUp(          const irr::core::stringw &username,
                                                                    const irr::core::stringw &password,
                                                                    const irr::core::stringw &password_ver,
                                                                    const irr::core::stringw &email,
                                                                    bool terms);

            /** Returns the username if signed in. */
            const irr::core::stringw    getUserName()   const;
            const UserState             getUserState()  const { return m_state.getAtomic(); }

    };   // class CurrentUser

} // namespace Online

#endif

/*EOF*/
