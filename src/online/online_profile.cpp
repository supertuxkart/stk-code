//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#include "online/online_profile.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "online/profile_manager.hpp"
#include "online/request_manager.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>

using namespace Online;

namespace Online
{

OnlineProfile::RelationInfo::RelationInfo(const irr::core::stringw & date,
                                    bool is_online, bool is_pending,
                                    bool is_asker)
{
    m_date          = date;
    m_is_online     = is_online;
    m_is_pending    = is_pending;
    m_is_asker      = is_asker;
}   // RelationInfo::RelationInfo

// ----------------------------------------------------------------------------
void OnlineProfile::RelationInfo::setOnline(bool online)
{
    m_is_online = online;
    if (m_is_online)
        m_is_pending = false;
}

// ============================================================================
/** Constructor for a new profile.  It does only store the ID, a name, and
 *  if it is the current user.
 */
OnlineProfile::OnlineProfile(const uint32_t  & userid,
                             const irr::core::stringw & username,
                             bool is_current_user)
{
    m_state                    = (State)0;
    m_cache_bit                = true;
    m_id                       = userid;
    m_is_current_user          = is_current_user;
    m_username                 = username;
    m_has_fetched_friends      = false;
    m_has_fetched_achievements = false;
    m_relation_info            = NULL;
    m_is_friend                = false;
}   // Profile

// ----------------------------------------------------------------------------
/** Creates a new profile from an XML Node. Two different profiles can be
 *  created: either a simple one with no relation, or a profile with
 *  relation information, i.e. it contains how this profile is related
 *  to the current profile.
 *  \param xml The XML node with the data to use.
 *  \param type Either C_DEFAULT (no relation), or C_RELATION_INFO
 *         if the XML node contains relation information.
 */
OnlineProfile::OnlineProfile(const XMLNode * xml, ConstructorType type)
{
    m_relation_info            = NULL;
    m_is_friend                = false;
    m_cache_bit                = true;
    m_has_fetched_friends      = false;
    m_has_fetched_achievements = false;
    if (type == C_RELATION_INFO)
    {
        irr::core::stringw date("");
        bool is_pending = false, is_asker = false, is_online = false;

        xml->get("date", &date);
        xml->get("is_pending", &is_pending);

        if (is_pending)
        {
            xml->get("is_asker", &is_asker);
        }
        else
        {
            xml->get("online", &is_online);
            m_is_friend = true;
        }
        m_relation_info = new RelationInfo(date, is_online, is_pending,
                                           is_asker);
        xml = xml->getNode("user");
    }

    xml->get("id",        &m_id      );
    xml->get("user_name", &m_username);
    m_is_current_user = (m_id == PlayerManager::getCurrentOnlineId());
    m_state = (State)0;
}   // OnlineProfile(XMLNode)

// ----------------------------------------------------------------------------
OnlineProfile::~OnlineProfile()
{
    delete m_relation_info;
}   // ~OnlineProfile

// ----------------------------------------------------------------------------
/** Triggers an asynchronous request to get the achievements for this user
 *  from the server. The state of this profile is changed to be fetching,
 *  and will be reset to ready when the server request returns.
 */
void OnlineProfile::fetchAchievements()
{
    assert(PlayerManager::isCurrentLoggedIn());
    if (m_has_fetched_achievements || m_is_current_user)
        return;

    m_state = State(m_state | S_FETCHING_ACHIEVEMENTS);

    // ------------------------------------------------------------------------
    /** A simple class that receives the achievements, and calls the right
     *  Profile instance to store them. */
    class AchievementsRequest : public XMLRequest
    {
    public:
        AchievementsRequest() : XMLRequest(true, true) {}
        virtual void callback()
        {
            uint32_t user_id = 0;
            getXMLData()->get("visitingid", &user_id);
            OnlineProfile *profile = ProfileManager::get()->getProfileByID(user_id);
            if (profile)
                profile->storeAchievements(getXMLData());
        }   // AchievementsRequest::callback
    };   // class AchievementsRequest
    // ------------------------------------------------------------------------

    AchievementsRequest * request = new AchievementsRequest();
    PlayerManager::setUserDetails(request, "get-achievements");
    request->addParameter("visitingid", m_id);
    RequestManager::get()->addRequest(request);
}   // fetchAchievements

// ----------------------------------------------------------------------------
/** Stores the achievement ids from an XML node into this profile. It also
 *  sets that achievements have been fetched, and changes the state to be
 *  READY again.
 *  \param input XML node with the achievements data.
 */
void OnlineProfile::storeAchievements(const XMLNode * input)
{
    m_achievements.clear();
    std::string achieved_string("");
    if (input->get("achieved", &achieved_string) == 1)
    {
        m_achievements = StringUtils::splitToUInt(achieved_string, ' ');
    }
    m_has_fetched_achievements = true;
    m_state = State(m_state & ~S_FETCHING_ACHIEVEMENTS);
}   // storeAchievements

// ----------------------------------------------------------------------------
/** Triggers an asynchronous request to download the friends for this user.
 *  The state of this profile is changed to be fetching,
 *  and will be reset to ready when the server request returns.
 */
void OnlineProfile::fetchFriends()
{
    assert(PlayerManager::isCurrentLoggedIn());
    if (m_has_fetched_friends)
        return;

    m_state = State(m_state | S_FETCHING_FRIENDS);

    // ------------------------------------------------------------------------
    class FriendsListRequest : public XMLRequest
    {
    public:
        FriendsListRequest() : XMLRequest(true, true) {}
        virtual void callback()
        {
            uint32_t user_id = 0;
            getXMLData()->get("visitingid", &user_id);
            OnlineProfile *profile = ProfileManager::get()->getProfileByID(user_id);
            if (profile)
                profile->storeFriends(getXMLData());
        }   // callback
    };   // class FriendsListRequest
    // ------------------------------------------------------------------------

    FriendsListRequest * request = new FriendsListRequest();
    PlayerManager::setUserDetails(request, "get-friends-list");
    request->addParameter("visitingid", m_id);
    RequestManager::get()->addRequest(request);
}   // fetchFriends

// ----------------------------------------------------------------------------
/** Stores the friends from an XML node into this profile. It also
 *  sets that friends have been fetched, and changes the state of the profile
 *  to be READY again.
 *  \param input XML node with the friends data.
 */
void OnlineProfile::storeFriends(const XMLNode * input)
{
    const XMLNode * friends_xml = input->getNode("friends");
    m_friends.clear();
    for (unsigned int i = 0; i < friends_xml->getNumNodes(); i++)
    {
        OnlineProfile * profile;
        if (m_is_current_user)
        {
            profile = new OnlineProfile(friends_xml->getNode(i), C_RELATION_INFO);
            m_friends.push_back(profile->getID());
            ProfileManager::get()->addPersistent(profile);
        }
        else
        {
            profile = new OnlineProfile(friends_xml->getNode(i)->getNode("user"),
                                        C_DEFAULT);
            m_friends.push_back(profile->getID());
            ProfileManager::get()->addToCache(profile);
        }
    }   // for i in nodes
    m_has_fetched_friends = true;
    m_state = State(m_state & ~S_FETCHING_FRIENDS);
}   // storeFriends

// ----------------------------------------------------------------------------
/** Removed a friend with a given id.
 *  \param id Friend id to remove.
 */
void OnlineProfile::removeFriend(const uint32_t id)
{
    assert(m_has_fetched_friends);
    IDList::iterator iter;
    for (iter = m_friends.begin(); iter != m_friends.end();)
    {
        if (*iter == id)
        {
            m_friends.erase(iter++);
            break;
        }
        else
        {
            ++iter;
        }
    } // for friend in friends
} // removeFriend

// ----------------------------------------------------------------------------
/** Adds a friend to the friend list.
 *  \param id The id of the profile to add.
 */
void OnlineProfile::addFriend(const uint32_t id)
{
    assert(m_has_fetched_friends);

    // find if friend id is is already in the user list
    for (unsigned int i = 0; i < m_friends.size(); i++)
    {
        if (m_friends[i] == id)
            return;
    }

    m_friends.push_back(id);
}   // addFriend

// ----------------------------------------------------------------------------
/** Deletes the relational info for this profile.
 */
void OnlineProfile::deleteRelationalInfo()
{
    delete m_relation_info;
    m_relation_info = NULL;
}   // deleteRelationalInfo

// ----------------------------------------------------------------------------
/** Returns the list of all friend ids.
 */
const OnlineProfile::IDList& OnlineProfile::getFriends()
{
    assert(m_has_fetched_friends            && 
           (m_state & S_FETCHING_FRIENDS) == 0);
    return m_friends;
}    // getFriends

// ----------------------------------------------------------------------------
/** Returns the list of all achievement ids.
 */
const OnlineProfile::IDList& OnlineProfile::getAchievements()
{
    assert(m_has_fetched_achievements               && 
           (m_state & S_FETCHING_ACHIEVEMENTS) == 0 &&
           !m_is_current_user);
    return m_achievements;
}   // getAchievements

// ----------------------------------------------------------------------------
/** Merges the information from a given profile with this profile. Any data
 *  that is in the given profile that's not available in this profile will
 *  be copied over, then the given profile will be deleted.
 */
void OnlineProfile::merge(OnlineProfile *profile)
{
    assert(profile != NULL);

    // profile has fetched friends, use that instead
    if (!m_has_fetched_friends && profile->m_has_fetched_friends)
        m_friends = profile->m_friends;

    // profile has fetched achievements, use that instead
    if (!m_has_fetched_achievements && profile->m_has_fetched_achievements)
        m_achievements = profile->m_achievements;

    // current relation is not set, use the profile one
    if (m_relation_info == NULL && profile->m_relation_info != NULL)
    {
        m_relation_info = profile->m_relation_info;

        // We don't want the destructor of the profile instance to destroy
        // the relation info
        profile->m_relation_info = NULL;
    }

    delete profile;
} // merge
} // namespace Online
