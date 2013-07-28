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

#include "http_manager.hpp"
#include "online/server.hpp"
#include "online/user.hpp"
#include "utils/types.hpp"

#include <irrString.h>

#include <string>

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
                uint32_t getCreatedServerID() {return m_created_server_id.getAtomic();}
            };

            class ServerJoinRequest : public XMLRequest {
                virtual void callback ();
            public:
                ServerJoinRequest() : XMLRequest(RT_SERVER_JOIN) {}
            };


        private:
            std::string                 m_token;
            bool                        m_save_session;
            UserState                   m_state;

            CurrentUser();

            void signIn                 (const SignInRequest            * input);
            void signOut                (const SignOutRequest           * input);
            void createServer           (const ServerCreationRequest    * input);

        public:
            //Singleton
            static CurrentUser*         acquire();
            static void                 release();
            static void                 deallocate();

            SignInRequest *             requestSavedSession();
            SignInRequest *             requestSignIn(          const irr::core::stringw &username,
                                                                const irr::core::stringw &password,
                                                                bool save_session,
                                                                bool request_now = true);
            SignOutRequest *            requestSignOut();
            ServerCreationRequest *     requestServerCreation(  const irr::core::stringw &name, int max_players);
            ServerJoinRequest *         requestServerJoin(      uint32_t server_id, bool request_now = true);


            // Register
            XMLRequest *                 requestSignUp(         const irr::core::stringw &username,
                                                                const irr::core::stringw &password,
                                                                const irr::core::stringw &password_ver,
                                                                const irr::core::stringw &email,
                                                                bool terms);

            /** Returns the username if signed in. */
            irr::core::stringw          getUserName()   const;
            bool                        isSignedIn()    const { return m_state == US_SIGNED_IN; }
            bool                        isGuest()       const { return m_state == US_GUEST; }
            bool                        isSigningIn()   const { return m_state == US_SIGNING_IN; }
            UserState                   getUserState()        { return m_state; }
            std::string                 getToken()      const { return m_token; }

    };   // class CurrentUser

} // namespace Online

#endif

/*EOF*/
