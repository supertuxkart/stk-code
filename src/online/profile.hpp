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

#ifndef HEADER_ONLINE_PROFILE_HPP
#define HEADER_ONLINE_PROFILE_HPP

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
      * \brief Class that represents an online profile
      * \ingroup online
      */
    class Profile
    {
        public :
            enum ConstructorType
            {
                C_DEFAULT = 1,
                C_RELATION_INFO
            };
            class RelationInfo
            {
            private:
                bool m_is_online;
                bool m_is_pending;
                bool m_is_asker;
                irr::core::stringw m_date;
            public:
                RelationInfo(const irr::core::stringw & date, bool is_online, bool is_pending, bool is_asker = false);
                bool isPending(){return m_is_pending;}
                bool isAsker(){return m_is_asker;}
                const irr::core::stringw & getDate() { return m_date; }
                bool                            isOnline() const                 { return m_is_online; }
                void                            setOnline(bool online)           { m_is_online = online; }
            };
            class FriendsListRequest : public XMLRequest
            {
                virtual void callback ();
            public:
                FriendsListRequest() : XMLRequest(0, true) {}
            };

            typedef std::vector<uint32_t> IDList;
        private:

            enum State
            {
                S_FETCHING = 1,
                S_READY
            };

            Synchronised<State>             m_state;
            bool                            m_is_current_user;
            uint32_t                        m_id;
            irr::core::stringw              m_username;
            RelationInfo *                  m_relation_info;

            bool                            m_has_fetched_friends;
            std::vector<uint32_t>           m_friends;

            bool                            m_cache_bit;

            void                            setState(State state)       { m_state.setAtomic(state); }
            const State                     getState() const            { return m_state.getAtomic(); }

            void requestFriendsList();
            void friendsListCallback(const XMLNode * input);

        public:
                                            Profile(    const uint32_t           & userid,
                                                        const irr::core::stringw & username);
                                            Profile(    const XMLNode * xml,
                                                        ConstructorType type = C_DEFAULT);
                                            ~Profile();
            void                            fetchFriends();
            const std::vector<uint32_t> &   getFriends();

            bool                            isFetching() const               { return getState() == S_FETCHING; }
            bool                            isReady() const                  { return getState() == S_READY; }

            bool                            isCurrentUser() const            { return m_is_current_user; }
            const RelationInfo *            getRelationInfo()                { return m_relation_info; }

            void                            setCacheBit()                    { m_cache_bit = true; }
            void                            unsetCacheBit()                  { m_cache_bit = false; }
            bool                            getCacheBit() const              { return m_cache_bit; }

            uint32_t                        getID() const                    { return m_id; }
            const irr::core::stringw &      getUserName() const              { return m_username; }


    };   // class CurrentUser

} // namespace Online

#endif

/*EOF*/
