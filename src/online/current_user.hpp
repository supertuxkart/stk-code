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
#include "online/request_manager.hpp"
#include "online/server.hpp"
#include "online/xml_request.hpp"
#include "utils/types.hpp"
#include "utils/synchronised.hpp"

#include <irrString.h>

#include <string>
#include <assert.h>

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
        public:
            enum UserState
            {
                US_SIGNED_OUT = 0,
                US_SIGNED_IN,
                US_GUEST,
                US_SIGNING_IN,
                US_SIGNING_OUT
            };

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
            class ServerCreationRequest : public XMLRequest {
                virtual void callback ();
                uint32_t m_created_server_id;
            public:
                ServerCreationRequest() : XMLRequest() {}
                const uint32_t getCreatedServerID() const 
                {
                    assert(isDone());
                    return m_created_server_id;
                }   // getCreatedServerID
            };   // ServerCreationRequest

            // ----------------------------------------------------------------

            class ServerJoinRequest : public XMLRequest {
                virtual void callback ();
            public:
                ServerJoinRequest() : XMLRequest() {}
            };   // ServerJoinRequest

            // ----------------------------------------------------------------
            class SetAddonVoteRequest : public XMLRequest {
                virtual void callback ();
            public:
                SetAddonVoteRequest() : XMLRequest() {}
            };   // SetAddonVoteRequest

            // ----------------------------------------------------------------
            class FriendRequest : public XMLRequest {
                virtual void callback ();
            public:
                FriendRequest() : XMLRequest(true) {}
            };   // FriendRequest

            // ----------------------------------------------------------------
            class AcceptFriendRequest : public XMLRequest {
                virtual void callback ();
            public:
                AcceptFriendRequest() : XMLRequest(true) {}
            };   // AcceptFriendRequest

            // ----------------------------------------------------------------
            class DeclineFriendRequest : public XMLRequest {
                virtual void callback ();
            public:
                DeclineFriendRequest() : XMLRequest(true) {}
            };   // DeclineFriendRequest

            // ----------------------------------------------------------------
            class RemoveFriendRequest : public XMLRequest {
                virtual void callback ();
            public:
                RemoveFriendRequest() : XMLRequest(true) {}
            };   // RemoveFriendRequest

            // ----------------------------------------------------------------
            class CancelFriendRequest : public XMLRequest {
                virtual void callback ();
            public:
                CancelFriendRequest() : XMLRequest(true) {}
            };   // CancelFriendRequest

            // ----------------------------------------------------------------
            class PollRequest : public XMLRequest {
                virtual void callback ();
            public:
                PollRequest() : XMLRequest(true) {}
            };   // PollRequest

            // ----------------------------------------------------------------
            class ChangePasswordRequest : public XMLRequest
            {
                virtual void callback ();
            public:
                ChangePasswordRequest() : XMLRequest(true) {}
            };   // ChangePasswordRequest


        private:
            std::string                 m_token;
            bool                        m_save_session;
            UserState                   m_state;
            OnlineProfile              *m_profile;

            bool saveSession()  const   { return m_save_session;      }

            CurrentUser();

            void signIn                 (bool success, const XMLNode * input);
            void signOut                (bool success, const XMLNode * input);

        public:
            /**Singleton */
            static CurrentUser *            get();
            static void                     deallocate();
            template <class C>
            static C* createHTMLRequest()
            {
                CurrentUser *cu = CurrentUser::get();
                assert(cu->m_state == US_SIGNED_IN);
                assert(cu->m_profile);
                C *request = new C();
                request->setServerURL("client-user.php");
                request->addParameter("token", cu->m_token);
                request->addParameter("userid", cu->m_profile->getID());
                return request;
            }   // createRequest

            void                            requestSavedSession();
            SignInRequest *                 requestSignIn(  const irr::core::stringw &username,
                                                            const irr::core::stringw &password,
                                                            bool save_session,
                                                            bool request_now = true);
            void                            requestSignOut();
            const ServerCreationRequest *   requestServerCreation(const irr::core::stringw &name, int max_players);
            ServerJoinRequest *             requestServerJoin(uint32_t server_id, bool request_now = true);


            /** Register */
            const XMLRequest *              requestSignUp(  const irr::core::stringw &username,
                                                            const irr::core::stringw &password,
                                                            const irr::core::stringw &password_ver,
                                                            const irr::core::stringw &email);

            const XMLRequest *              requestRecovery(const irr::core::stringw &username,
                                                            const irr::core::stringw &email);

            const XMLRequest *              requestGetAddonVote(const std::string & addon_id) const;
            const SetAddonVoteRequest *     requestSetAddonVote(const std::string & addon_id, float rating) const;
            void                            requestFriendRequest(const uint32_t friend_id) const;
            void                            requestAcceptFriend(const uint32_t friend_id) const;
            void                            requestDeclineFriend(const uint32_t friend_id) const;
            void                            requestRemoveFriend(const uint32_t friend_id) const;
            void                            requestCancelFriend(const uint32_t friend_id) const;
            void                            requestPasswordChange(  const irr::core::stringw &current_password,
                                                                    const irr::core::stringw &new_password,
                                                                    const irr::core::stringw &new_password_ver) const;

            XMLRequest *                    requestUserSearch(const irr::core::stringw & search_string) const;

            void                            onSTKQuit() const;
            void                            onAchieving(uint32_t achievement_id) const;
            void                            requestPoll() const;

            irr::core::stringw              getUserName()           const;
            uint32_t                        getID()                 const;
            // ----------------------------------------------------------------
            /** Returns the user state. */
            const UserState getUserState() const { return m_state; }
            // ----------------------------------------------------------------
            /** Returns whether a user is signed in or not. */
            bool isRegisteredUser() const { return m_state == US_SIGNED_IN; }
            // ----------------------------------------------------------------
            /** Returns the session token of the signed in user. */
            const std::string& getToken() const { return m_token; }
            // ----------------------------------------------------------------
            /** Returns a pointer to the profile associated with the current
             *  user. */
            OnlineProfile* getProfile() const { return m_profile; }
            // ----------------------------------------------------------------

    };   // class CurrentUser

} // namespace Online

#endif

/*EOF*/
