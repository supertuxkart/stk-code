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


#include "online/profile.hpp"

#include "online/profile_manager.hpp"
#include "online/http_manager.hpp"
#include "config/user_config.hpp"
#include "online/current_user.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>

using namespace Online;

namespace Online{


    // ============================================================================
    Profile::Profile(   const uint32_t           & userid,
                        const irr::core::stringw & username)
    {
        setState (S_READY);
        m_cache_bit = true;
        m_id = userid;
        m_is_current_user = (m_id == CurrentUser::get()->getUserID());
        m_username = username;
        m_has_fetched_friends = false;
    }

    // ============================================================================
    void Profile::fetchFriends()
    {
        assert(CurrentUser::get()->isRegisteredUser());
        if(m_has_fetched_friends)
            return;
        setState (S_FETCHING);
        requestFriendsList();
    }
    // ============================================================================


    void Profile::friendsListCallback(const XMLNode * input)
    {
        const XMLNode * friends_xml = input->getNode("friends");
        m_friends.clear();
        uint32_t friend_id(0);
        irr::core::stringw friend_username("");
        for (unsigned int i = 0; i < friends_xml->getNumNodes(); i++)
        {
            friends_xml->getNode(i)->get("friend_id", &friend_id);
            friends_xml->getNode(i)->get("friend_name", &friend_username);
            ProfileManager::get()->addToCache(
                new Profile(friend_id, friend_username)
            );
            m_friends.push_back(friend_id);

        }
        m_has_fetched_friends = true;
        Profile::setState (Profile::S_READY);
    }


    // ============================================================================

    void Profile::requestFriendsList()
    {
        FriendsListRequest * request = new FriendsListRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action",std::string("get-friends-list"));
        request->setParameter("token", CurrentUser::get()->getToken());
        request->setParameter("userid", CurrentUser::get()->getUserID());
        request->setParameter("visitingid", m_id);
        HTTPManager::get()->addRequest(request);
    }

    void Profile::FriendsListRequest::callback()
    {
        uint32_t user_id(0);
        m_result->get("visitingid", &user_id);
        assert(ProfileManager::get()->getProfileByID(user_id) != NULL);
        ProfileManager::get()->getProfileByID(user_id)->friendsListCallback(m_result);
    }

    // ============================================================================

    const std::vector<uint32_t> & Profile::getFriends()
    {
        assert (m_has_fetched_friends && getState() == S_READY);
        return m_friends;
    }
    // ============================================================================

} // namespace Online
