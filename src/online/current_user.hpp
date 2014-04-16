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

#include "online/http_request.hpp"
#include "online/online_profile.hpp"
#include "online/request_manager.hpp"
#include "online/server.hpp"
#include "online/xml_request.hpp"
#include "utils/leak_check.hpp"
#include "utils/synchronised.hpp"
#include "utils/types.hpp"

#include <irrString.h>

#include <string>
#include <assert.h>

class PlayerManager;

namespace Online
{

    class OnlineProfile;

    // ============================================================================

    /**
      * \brief Class that represents an online registered user
      * \ingroup online
      */
    class CurrentUser
    {
        private:
            LEAK_CHECK()
        public:
            // ----------------------------------------------------------------
            class SignInRequest : public XMLRequest
            {
                virtual void callback ();
            public:
                SignInRequest(bool manage_memory = false)
                    : XMLRequest(manage_memory, /*priority*/10) {}
            };   // SignInRequest

            // ----------------------------------------------------------------
            class SignOutRequest : public XMLRequest
            {
                virtual void callback ();
            public:
                SignOutRequest() : XMLRequest(true,/*priority*/10) {}
            };   // SignOutRequest

            // ----------------------------------------------------------------
            class PollRequest : public XMLRequest {
                virtual void callback ();
            public:
                PollRequest() : XMLRequest(true) {}
            };   // PollRequest

        private:
            std::string                 m_token;
            bool                        m_save_session;
            OnlineProfile              *m_profile;

            bool saveSession()  const   { return m_save_session;      }


            void signIn                 (bool success, const XMLNode * input);
            void signOut                (bool success, const XMLNode * input);

            // For now declare functions that will become part of PlayerManager
            // or Playerprofile to be private, and give only PlayerManager
            // access to them. FIXME

            // FIXME: This apparently does not compile on linux :(
            // So for now (while this is needed) I'll only add this on
            // windows only (where it works).
#ifdef WIN32
             friend class PlayerManager;
    private:
#else
    public:
#endif
            uint32_t getID() const;
            void setUserDetails(HTTPRequest *request,
                                const std::string &action,
                                const std::string &php_script = "");
            bool isRegisteredUser() const { return m_state == US_SIGNED_IN; }
            /** Returns the user state. */
            enum UserState
            {
                US_SIGNED_OUT = 0,
                US_SIGNED_IN,
                US_GUEST,
                US_SIGNING_IN,
                US_SIGNING_OUT
            };
            UserState                   m_state;
            const UserState getUserState() const { return m_state; }
            // ----------------------------------------------------------------
            /** Returns a pointer to the profile associated with the current
            *  user. */
            OnlineProfile* getProfile() const { return m_profile; }
            // ----------------------------------------------------------------
            /** Returns the session token of the signed in user. */
            const std::string& getToken() const { return m_token; }
            void requestPoll() const;
            void requestSavedSession();
            void onSTKQuit() const;
            void requestSignOut();
            SignInRequest *requestSignIn(const irr::core::stringw &username,
                                         const irr::core::stringw &password,
                                         bool save_session,
                                         bool request_now = true);

        public:
            CurrentUser();


            // ----------------------------------------------------------------
            // ----------------------------------------------------------------

    };   // class CurrentUser

} // namespace Online

#endif

/*EOF*/
