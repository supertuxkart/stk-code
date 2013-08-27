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

#include "online/http_manager.hpp"
#include "online/server.hpp"
#include "online/profile.hpp"
#include "utils/types.hpp"
#include "utils/synchronised.hpp"

#include <irrString.h>

#include <string>
#include <assert.h>

namespace Online{

    // ============================================================================

    /**
      * \brief Class that represents an online registered user
      * \ingroup online
      */
    class CurrentUser
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

            class SignInRequest : public XMLRequest
            {
                virtual void callback ();
            public:
                SignInRequest(bool manage_memory = false) : XMLRequest(manage_memory) {}
            };

            class SignOutRequest : public XMLRequest
            {
                virtual void callback ();
            public:
                SignOutRequest() : XMLRequest() {}
            };

            class ServerCreationRequest : public XMLRequest {
                virtual void callback ();
                uint32_t m_created_server_id;
            public:
                ServerCreationRequest() : XMLRequest() {}
                const uint32_t getCreatedServerID() const { assert(isDone()); return m_created_server_id;}
            };

            class ServerJoinRequest : public XMLRequest {
                virtual void callback ();
            public:
                ServerJoinRequest() : XMLRequest() {}
            };

            class SetAddonVoteRequest : public XMLRequest {
                virtual void callback ();
            public:
                SetAddonVoteRequest() : XMLRequest() {}
            };

            class FriendRequest : public XMLRequest {
                virtual void callback ();
            public:
                FriendRequest() : XMLRequest(true) {}
            };

            class AcceptFriendRequest : public XMLRequest {
                virtual void callback ();
            public:
                AcceptFriendRequest() : XMLRequest(true) {}
            };

            class DeclineFriendRequest : public XMLRequest {
                virtual void callback ();
            public:
                DeclineFriendRequest() : XMLRequest(true) {}
            };

            class RemoveFriendRequest : public XMLRequest {
                virtual void callback ();
            public:
                RemoveFriendRequest() : XMLRequest(true) {}
            };

            class CancelFriendRequest : public XMLRequest {
                virtual void callback ();
            public:
                CancelFriendRequest() : XMLRequest(true) {}
            };

            class PollRequest : public XMLRequest {
                virtual void callback ();
            public:
                PollRequest() : XMLRequest(true) {}
            };


        private:
            std::string                 m_token;
            bool                        m_save_session;
            UserState                   m_state;
            Profile *                   m_profile;

            bool                        getSaveSession()        const   { return m_save_session;      }

            void setUserState           (UserState user_state)          { m_state = user_state;          }
            void setSaveSession         (bool save_session)             { m_save_session = save_session; }
            void setToken               (const std::string & token)     { m_token= token;               }

            CurrentUser();

            void signIn                 (bool success, const XMLNode * input);
            void signOut                (bool success, const XMLNode * input);

        public:
            /**Singleton */
            static CurrentUser *            get();
            static void                     deallocate();

            void                            requestSavedSession();
            SignInRequest *                 requestSignIn(  const irr::core::stringw &username,
                                                            const irr::core::stringw &password,
                                                            bool save_session,
                                                            bool request_now = true);
            const SignOutRequest *          requestSignOut();
            const ServerCreationRequest *   requestServerCreation(const irr::core::stringw &name, int max_players);
            ServerJoinRequest *             requestServerJoin(uint32_t server_id, bool request_now = true);


            /** Register */
            const XMLRequest *              requestSignUp(  const irr::core::stringw &username,
                                                            const irr::core::stringw &password,
                                                            const irr::core::stringw &password_ver,
                                                            const irr::core::stringw &email,
                                                            bool terms);

            const XMLRequest *              requestRecovery(const irr::core::stringw &username,
                                                            const irr::core::stringw &email);

            const XMLRequest *              requestGetAddonVote(const std::string & addon_id) const;
            const SetAddonVoteRequest *     requestSetAddonVote(const std::string & addon_id, float rating) const;
            void                            requestFriendRequest(const uint32_t friend_id) const;
            void                            requestAcceptFriend(const uint32_t friend_id) const;
            void                            requestDeclineFriend(const uint32_t friend_id) const;
            void                            requestRemoveFriend(const uint32_t friend_id) const;
            void                            requestCancelFriend(const uint32_t friend_id) const;

            const XMLRequest *              requestUserSearch(const irr::core::stringw & search_string) const;



            /** Returns the username if signed in. */
            irr::core::stringw              getUserName()           const;
            uint32_t                        getID()                 const;
            const UserState                 getUserState()          const { return m_state; }
            bool                            isRegisteredUser()      const { return m_state == US_SIGNED_IN; }
            const std::string &             getToken()              const { return m_token; }
            Profile *                       getProfile()            const { return m_profile; }

            void                            requestPoll();

    };   // class CurrentUser

} // namespace Online

#endif

/*EOF*/
