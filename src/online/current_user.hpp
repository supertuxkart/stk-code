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
                SIGNED_OUT,
                SIGNED_IN,
                GUEST,
                SIGNING_IN,
                SIGNING_OUT
            };

            class SignInRequest : public XMLRequest
            {
                virtual void callback ();
            };

            class SignOutRequest : public XMLRequest
            {
                virtual void callback ();
            };

            class ServerCreationRequest : public XMLRequest
            {
                virtual void callback ();
            };

            class ServerJoinRequest : public XMLRequest
            {
                virtual void callback ();
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
            static CurrentUser*         get(); //FIXME To be removed

            //Singleton methods
            static CurrentUser*         acquire();
            static void                 release();
            static void                 deallocate();

            SignInRequest *             requestSavedSession();
            SignInRequest *             requestSignIn(          const irr::core::stringw &username,
                                                                const irr::core::stringw &password,
                                                                bool save_session);
            SignOutRequest *            requestSignOut();
            ServerCreationRequest *     requestServerCreation(  const irr::core::stringw &name, int max_players);
            ServerJoinRequest *         requestServerJoin(      uint32_t server_id);


            // Register
            bool                        signUp(                 const irr::core::stringw &username,
                                                                const irr::core::stringw &password,
                                                                const irr::core::stringw &password_ver,
                                                                const irr::core::stringw &email,
                                                                bool terms,
                                                                irr::core::stringw &info);



            /** Returns the username if signed in. */
            irr::core::stringw          getUserName()   const;
            bool                        isSignedIn()    const   { return m_state == SIGNED_IN; }
            bool                        isGuest()       const   { return m_state == GUEST; }
            bool                        isSigningIn()   const   { return m_state == SIGNING_IN; }
            UserState                   getUserState()          { return m_state; }

    };   // class CurrentUser

} // namespace Online

#endif

/*EOF*/
