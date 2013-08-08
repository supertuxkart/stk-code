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

#ifndef HEADER_ONLINE_PROFILE_MANAGER_HPP
#define HEADER_ONLINE_PROFILE_MANAGER_HPP

#include "http_manager.hpp"
#include "online/request.hpp"
#include "online/user.hpp"
#include "utils/types.hpp"
#include "utils/ptr_vector.hpp"


#include <irrString.h>

#include <string>

namespace Online{

    // ============================================================================

    /**
      * \brief Class that takes care of online profiles
      * \ingroup online
      */
    class ProfileManager
    {
        public :
            class FriendsListRequest : public XMLRequest
            {
                virtual void callback ();
            public:
                FriendsListRequest() : XMLRequest() {}
            };
        private:
            ProfileManager      ();

            enum State
            {
                S_FETCHING = 1,
                S_READY
            };

            Synchronised<State>         m_state;
            bool                        m_is_current_user;
            uint32_t                    m_visiting_id;
            irr::core::stringw          m_visiting_username;

            bool                        m_has_fetched_friends;
            PtrVector<Online::User>     m_friends;
            FriendsListRequest *        m_friends_list_request;



            void                            setState(State state)       { m_state.setAtomic(state); }
            const State                     getState() const            { return m_state.getAtomic(); }

            void friendsListCallback(const XMLNode * input);

        public:
            /**Singleton */
            static ProfileManager *         get();
            static void                     deallocate();

            void set(Online::User * = NULL);
            void fetchFriends();
            const PtrVector<Online::User> & getFriends();

            bool isFetching() { return getState() == S_FETCHING; }
            bool isReady() { return getState() == S_READY; }


    };   // class CurrentUser

} // namespace Online

#endif

/*EOF*/
