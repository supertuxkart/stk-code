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


#include "online/profile_manager.hpp"

#include "online/current_user.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>

using namespace Online;

namespace Online{
    static ProfileManager* current_user_singleton(NULL);

    ProfileManager* ProfileManager::get()
    {
        if (current_user_singleton == NULL)
            current_user_singleton = new ProfileManager();
        return current_user_singleton;
    }

    void ProfileManager::deallocate()
    {
        delete current_user_singleton;
        current_user_singleton = NULL;
    }   // deallocate

    // ============================================================================
    ProfileManager::ProfileManager()
    {
        setState (S_READY);
        m_is_current_user = false;
        m_has_fetched_friends = false;
    }

    // ============================================================================
    void ProfileManager::set(User * user)
    {
        if (user == NULL)
        {
            assert(CurrentUser::get()->isRegisteredUser());
            m_is_current_user = true;
            m_visiting_id = CurrentUser::get()->getUserID();
            m_visiting_username = CurrentUser::get()->getUserName();
        }
        else
        {
            m_is_current_user = false;
            m_visiting_id = CurrentUser::get()->getUserID();
        }
    }

    // ============================================================================
    void ProfileManager::fetchFriends()
    {
        if(m_has_fetched_friends)
            return;
        //m_friends_list_request
        setState (S_FETCHING);
    }
    // ============================================================================


    void ProfileManager::friendsListCallback(const XMLNode * input)
    {
        uint32_t friendid = 0;
        irr::core::stringw username("");
        const XMLNode * friends_xml = input->getNode("friends");
        m_friends.clearAndDeleteAll();
        for (unsigned int i = 0; i < friends_xml->getNumNodes(); i++)
        {
            friends_xml->getNode(i)->get("friend_id", &friendid);
            m_friends.push_back(new User(username, friendid));
        }
        ProfileManager::setState (ProfileManager::S_READY);
    }


    // ============================================================================

    void ProfileManager::FriendsListRequest::callback()
    {
        ProfileManager::get()->friendsListCallback(m_result);
    }

    // ============================================================================

    const PtrVector<Online::User> & ProfileManager::getFriends()
    {
        assert (m_has_fetched_friends && getState() == S_READY);
        return m_friends;
    }
    // ============================================================================

} // namespace Online
