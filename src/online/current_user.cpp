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


#include "online/current_user.hpp"

#include "addons/addons_manager.hpp"
#include "config/user_config.hpp"
#include "online/servers_manager.hpp"
#include "online/profile_manager.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"
#include "addons/addon.hpp"
#include "guiengine/dialog_queue.hpp"
#include "states_screens/dialogs/user_info_dialog.hpp"
#include "states_screens/dialogs/notification_dialog.hpp"
#include "states_screens/online_profile_friends.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>

using namespace Online;

namespace Online{
    static CurrentUser* current_user_singleton(NULL);

    CurrentUser* CurrentUser::get()
    {
        if (current_user_singleton == NULL)
            current_user_singleton = new CurrentUser();
        return current_user_singleton;
    }

    void CurrentUser::deallocate()
    {
        delete current_user_singleton;
        current_user_singleton = NULL;
    }   // deallocate

    // ============================================================================
    CurrentUser::CurrentUser()
    {
        m_state = US_SIGNED_OUT;
        m_token = "";
        m_save_session = false;
        m_profile = NULL;
    }

    // ============================================================================
    const XMLRequest * CurrentUser::requestRecovery(    const irr::core::stringw &username,
                                                        const irr::core::stringw &email)
    {
        assert(getUserState() == US_SIGNED_OUT || getUserState() == US_GUEST);
        XMLRequest * request = new XMLRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("recovery"));
        request->setParameter("username", username);
        request->setParameter("email", email);
        HTTPManager::get()->addRequest(request);
        return request;
    }

    // ============================================================================
    const XMLRequest * CurrentUser::requestSignUp(  const irr::core::stringw &username,
                                                    const irr::core::stringw &password,
                                                    const irr::core::stringw &password_confirm,
                                                    const irr::core::stringw &email,
                                                    bool terms)
    {
        assert(getUserState() == US_SIGNED_OUT || getUserState() == US_GUEST);
        XMLRequest * request = new XMLRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("register"));
        request->setParameter("username", username);
        request->setParameter("password", password);
        request->setParameter("password_confirm", password_confirm);
        request->setParameter("email", email);
        request->setParameter("terms", std::string("on"));
        HTTPManager::get()->addRequest(request);
        return request;
    }

    // ============================================================================
    void CurrentUser::requestSavedSession()
    {
        SignInRequest * request = NULL;
        if(getUserState() != US_SIGNED_IN  && UserConfigParams::m_saved_session)
        {
            request = new SignInRequest(true);
            request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
            request->setParameter("action",std::string("saved-session"));
            request->setParameter("userid", UserConfigParams::m_saved_user);
            request->setParameter("token", UserConfigParams::m_saved_token.c_str());
            HTTPManager::get()->addRequest(request);
            setUserState (US_SIGNING_IN);
        }
    }

    CurrentUser::SignInRequest * CurrentUser::requestSignIn(    const irr::core::stringw &username,
                                                                const irr::core::stringw &password,
                                                                bool save_session, bool request_now)
    {
        assert(getUserState() == US_SIGNED_OUT);
        setSaveSession(save_session);
        SignInRequest * request = new SignInRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action",std::string("connect"));
        request->setParameter("username",username);
        request->setParameter("password",password);
        if (request_now)
        {
            HTTPManager::get()->addRequest(request);
            setUserState (US_SIGNING_IN);
        }
        return request;
    }

    void CurrentUser::signIn(bool success, const XMLNode * input)
    {
        if (success)
        {
            std::string token("");
            int token_fetched       = input->get("token", &token);
            setToken(token);
            irr::core::stringw username("");
            int username_fetched    = input->get("username", &username);
            uint32_t userid(0);
            int userid_fetched      = input->get("userid", &userid);
            m_profile = new Profile(userid, username, true);
            assert(token_fetched && username_fetched && userid_fetched);
            m_state = US_SIGNED_IN;
            if(getSaveSession())
            {
                UserConfigParams::m_saved_user = getID();
                UserConfigParams::m_saved_token = getToken();
                UserConfigParams::m_saved_session = true;
            }
            ProfileManager::get()->addPersistent(m_profile);
            m_profile->fetchFriends();
            HTTPManager::get()->startPolling();
        }
        else
            setUserState (US_SIGNED_OUT);
    }

    void CurrentUser::SignInRequest::callback()
    {
        CurrentUser::get()->signIn(m_success, m_result);
    }

    // ============================================================================

    const CurrentUser::ServerCreationRequest * CurrentUser::requestServerCreation(  const irr::core::stringw &name,
                                                                                    int max_players)
    {
        assert(getUserState() == US_SIGNED_IN);
        ServerCreationRequest * request = new ServerCreationRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action",           std::string("create_server"));
        request->setParameter("token",            getToken());
        request->setParameter("userid",           getID());
        request->setParameter("name",             name);
        request->setParameter("max_players",      max_players);
        HTTPManager::get()->addRequest(request);
        return request;
    }

    void CurrentUser::ServerCreationRequest::callback()
    {
        if(m_success)
        {
            Server * server = new Server(*m_result->getNode("server"));
            ServersManager::get()->addServer(server);
            m_created_server_id = server->getServerId();
        }
    }

    // ============================================================================
    const CurrentUser::SignOutRequest * CurrentUser::requestSignOut(){
        assert(getUserState() == US_SIGNED_IN || getUserState() == US_GUEST);
        SignOutRequest * request = new SignOutRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action",std::string("disconnect"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        HTTPManager::get()->addRequest(request);
        setUserState (US_SIGNING_OUT);
        HTTPManager::get()->stopPolling();
        return request;
    }

    void CurrentUser::signOut(bool success, const XMLNode * input)
    {
        if(!success)
        {
            Log::warn("CurrentUser::signOut", "%s", _("There were some connection issues while signing out. Report a bug if this caused issues."));
        }
        setToken("");
        ProfileManager::get()->clearPersistent();
        m_profile = NULL;
        setUserState (US_SIGNED_OUT);
        UserConfigParams::m_saved_user = 0;
        UserConfigParams::m_saved_token = "";
        UserConfigParams::m_saved_session = false;
    }

    void CurrentUser::SignOutRequest::callback()
    {
        CurrentUser::get()->signOut(m_success, m_result);
    }

    // ============================================================================

    CurrentUser::ServerJoinRequest *  CurrentUser::requestServerJoin(uint32_t server_id,
                                                                    bool request_now)
    {
        assert(getUserState() == US_SIGNED_IN || getUserState() == US_GUEST);
        ServerJoinRequest * request = new ServerJoinRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "address-management.php");
        request->setParameter("action",std::string("request-connection"));
        request->setParameter("token", getToken());
        request->setParameter("id", getID());
        request->setParameter("server_id", server_id);
        if (request_now)
            HTTPManager::get()->addRequest(request);
        return request;
    }

    void CurrentUser::ServerJoinRequest::callback()
    {
        if(isSuccess())
        {
            uint32_t server_id;
            m_result->get("serverid", &server_id);
            ServersManager::get()->setJoinedServer(server_id);
        }
        //FIXME needs changes for actual valid joining
    }

    // ============================================================================

    const XMLRequest * CurrentUser::requestGetAddonVote( const std::string & addon_id) const
    {
        assert(isRegisteredUser());
        XMLRequest * request = new XMLRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("get-addon-vote"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        request->setParameter("addonid", addon_id.substr(6));
        HTTPManager::get()->addRequest(request);
        return request;
    }

    // ============================================================================

    const XMLRequest * CurrentUser::requestUserSearch( const irr::core::stringw & search_string) const
    {
        assert(isRegisteredUser());
        XMLRequest * request = new XMLRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("user-search"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        request->setParameter("search-string", search_string);
        HTTPManager::get()->addRequest(request);
        return request;
    }

    // ============================================================================

    const CurrentUser::SetAddonVoteRequest * CurrentUser::requestSetAddonVote( const std::string & addon_id, float rating) const
    {
        assert(isRegisteredUser());
        CurrentUser::SetAddonVoteRequest * request = new CurrentUser::SetAddonVoteRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("set-addon-vote"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        request->setParameter("addonid", addon_id.substr(6));
        request->setParameter("rating", rating);
        HTTPManager::get()->addRequest(request);
        return request;
    }

    void CurrentUser::SetAddonVoteRequest::callback()
    {
        if(m_success)
        {
            std::string addon_id;
            m_result->get("addon-id", &addon_id);
            float average;
            m_result->get("new-average", &average);
            addons_manager->getAddon(Addon::createAddonId(addon_id))->setRating(average);
        }
    }

    // ============================================================================

    void CurrentUser::requestFriendRequest(const uint32_t friend_id) const
    {
        assert(isRegisteredUser());
        CurrentUser::FriendRequest * request = new CurrentUser::FriendRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("friend-request"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        request->setParameter("friendid", friend_id);
        HTTPManager::get()->addRequest(request);
    }

    void CurrentUser::FriendRequest::callback()
    {
        uint32_t id(0);
        m_result->get("friendid", &id);
        irr::core::stringw info_text("");
        if(m_success)
        {
            CurrentUser::get()->getProfile()->addFriend(id);
            ProfileManager::get()->getProfileByID(id)->setRelationInfo(new Profile::RelationInfo(_("Today"), false, true, false));
            OnlineProfileFriends::getInstance()->refreshFriendsList();
            info_text = _("Friend request send!");
        }
        else
            info_text = m_info;
        GUIEngine::DialogQueue::get()->pushDialog( new UserInfoDialog(id, info_text,!m_success, true), true);
    }

    // ============================================================================

    void CurrentUser::requestAcceptFriend(const uint32_t friend_id) const
    {
        assert(isRegisteredUser());
        CurrentUser::AcceptFriendRequest * request = new CurrentUser::AcceptFriendRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("accept-friend-request"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        request->setParameter("friendid", friend_id);
        HTTPManager::get()->addRequest(request);
    }

    void CurrentUser::AcceptFriendRequest::callback()
    {
        uint32_t id(0);
        m_result->get("friendid", &id);
        irr::core::stringw info_text("");
        if(m_success)
        {
            Profile * profile = ProfileManager::get()->getProfileByID(id);
            profile->setFriend();
            profile->setRelationInfo(new Profile::RelationInfo(_("Today"), false, false, true));
            OnlineProfileFriends::getInstance()->refreshFriendsList();
            info_text = _("Friend request accepted!");
        }
        else
            info_text = m_info;
        GUIEngine::DialogQueue::get()->pushDialog( new UserInfoDialog(id, info_text,!m_success, true), true);
    }

    // ============================================================================

    void CurrentUser::requestDeclineFriend(const uint32_t friend_id) const
    {
        assert(isRegisteredUser());
        CurrentUser::DeclineFriendRequest * request = new CurrentUser::DeclineFriendRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("decline-friend-request"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        request->setParameter("friendid", friend_id);
        HTTPManager::get()->addRequest(request);
    }

    void CurrentUser::DeclineFriendRequest::callback()
    {
        uint32_t id(0);
        m_result->get("friendid", &id);
        irr::core::stringw info_text("");
        if(m_success)
        {
            CurrentUser::get()->getProfile()->removeFriend(id);
            ProfileManager::get()->moveToCache(id);
            ProfileManager::get()->getProfileByID(id)->deleteRelationalInfo();
            OnlineProfileFriends::getInstance()->refreshFriendsList();
            info_text = _("Friend request declined!");
        }
        else
            info_text = m_info;
        GUIEngine::DialogQueue::get()->pushDialog( new UserInfoDialog(id, info_text,!m_success, true), true);

    }

    // ============================================================================

    void CurrentUser::requestCancelFriend(const uint32_t friend_id) const
    {
        assert(isRegisteredUser());
        CurrentUser::CancelFriendRequest * request = new CurrentUser::CancelFriendRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("cancel-friend-request"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        request->setParameter("friendid", friend_id);
        HTTPManager::get()->addRequest(request);
    }

    void CurrentUser::CancelFriendRequest::callback()
    {
        uint32_t id(0);
        m_result->get("friendid", &id);
        irr::core::stringw info_text("");
        if(m_success)
        {
            CurrentUser::get()->getProfile()->removeFriend(id);
            ProfileManager::get()->moveToCache(id);
            ProfileManager::get()->getProfileByID(id)->deleteRelationalInfo();
            OnlineProfileFriends::getInstance()->refreshFriendsList();
            info_text = _("Friend request cancelled!");
        }
        else
            info_text = m_info;
        GUIEngine::DialogQueue::get()->pushDialog( new UserInfoDialog(id, info_text,!m_success, true), true);

    }

    // ============================================================================

    void CurrentUser::requestRemoveFriend(const uint32_t friend_id) const
    {
        assert(isRegisteredUser());
        CurrentUser::RemoveFriendRequest * request = new CurrentUser::RemoveFriendRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("remove-friend-request"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        request->setParameter("friendid", friend_id);
        HTTPManager::get()->addRequest(request);
    }

    void CurrentUser::RemoveFriendRequest::callback()
    {
        uint32_t id(0);
        m_result->get("friendid", &id);
        irr::core::stringw info_text("");
        if(m_success)
        {
            CurrentUser::get()->getProfile()->removeFriend(id);
            ProfileManager::get()->moveToCache(id);
            ProfileManager::get()->getProfileByID(id)->deleteRelationalInfo();
            OnlineProfileFriends::getInstance()->refreshFriendsList();
            info_text = _("Friend removed!");
        }
        else
            info_text = m_info;
        GUIEngine::DialogQueue::get()->pushDialog( new UserInfoDialog(id, info_text,!m_success, true), true);

    }
    // ============================================================================
    void CurrentUser::requestPoll()
    {
        assert(isRegisteredUser());
        CurrentUser::PollRequest * request = new CurrentUser::PollRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("poll"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        HTTPManager::get()->addRequest(request);
    }

    void CurrentUser::PollRequest::callback()
    {
        if(m_success)
        {
            std::string online_friends_string("");

            if(m_result->get("online", &online_friends_string) == 1)
            {
                std::vector<std::string> parts = StringUtils::split(online_friends_string, ' ');
                std::vector<uint32_t> online_friends;
                for(unsigned int i = 0; i < parts.size(); ++i)
                {
                    online_friends.push_back(atoi(parts[i].c_str()));
                }
                bool went_offline = false;
                std::vector<uint32_t> friends = CurrentUser::get()->getProfile()->getFriends();
                std::vector<irr::core::stringw> to_notify;
                for(unsigned int i = 0; i < friends.size(); ++i)
                {
                     std::vector<uint32_t>::iterator iter;
                     for (iter = online_friends.begin(); iter != online_friends.end();)
                     {
                        if (*iter == friends[i])
                        {
                            online_friends.erase(iter--);
                            break;
                        }
                        else
                            ++iter;
                     }
                     bool now_online = false;
                     if(iter != online_friends.end())
                     {
                         now_online = true;
                     }

                     Profile * profile = ProfileManager::get()->getProfileByID(friends[i]);
                     Profile::RelationInfo * relation_info = profile->getRelationInfo();
                     if( relation_info->isOnline() )
                     {
                         if (!now_online)
                         {
                             relation_info->setOnline(false);
                             went_offline = true;
                         }
                     }
                     else
                     {
                         if (now_online)
                         {
                             relation_info->setOnline(true);
                             to_notify.push_back(profile->getUserName());
                         }
                     }

                }

                if(to_notify.size() > 0)
                {
                    irr::core::stringw message("");
                    if(to_notify.size() == 1)
                    {
                        message = to_notify[0] + irr::core::stringw(_(" is now online."));
                    }
                    else if(to_notify.size() == 2)
                    {
                        message = to_notify[0] + irr::core::stringw(_(" and ")) + to_notify[1] + irr::core::stringw(_(" are now online."));
                    }
                    else if(to_notify.size() == 3)
                    {
                        message = to_notify[0] + irr::core::stringw(_(", ")) + to_notify[1] + irr::core::stringw(_(" and ")) + to_notify[2] + irr::core::stringw(_(" are now online."));
                    }
                    else if(to_notify.size() > 3)
                    {
                        message = StringUtils::toWString(to_notify.size()) + irr::core::stringw(_(" friends are now online."));
                    }
                    GUIEngine::DialogQueue::get()->pushDialog( new NotificationDialog(NotificationDialog::T_Friends, message), false);
                    OnlineProfileFriends::getInstance()->refreshFriendsList();
                }
                else if(went_offline)
                {
                    OnlineProfileFriends::getInstance()->refreshFriendsList();
                }
            }
            int friend_request_count = 0;
            for(unsigned int i = 0; i < m_result->getNumNodes(); i++)
            {
                const XMLNode * node = m_result->getNode(i);
                if(node->getName() == "new_friend_request")
                {
                    Profile::RelationInfo * ri = new Profile::RelationInfo("New", false, true, true);
                    Profile * p = new Profile(node);
                    p->setRelationInfo(ri);
                    ProfileManager::get()->addPersistent(p);
                    friend_request_count++;
                }
            }
            if(friend_request_count > 0)
            {
                irr::core::stringw message("");
                if(friend_request_count > 1)
                {
                    message = irr::core::stringw(_("You have ")) + StringUtils::toWString(friend_request_count) + irr::core::stringw(_(" new friend requests!."));
                }
                else
                {
                    message = _("You have a new friend request!.");
                }
                GUIEngine::DialogQueue::get()->pushDialog( new NotificationDialog(NotificationDialog::T_Friends, message), false);
                OnlineProfileFriends::getInstance()->refreshFriendsList();
            }
        }
        // FIXME show connection error??
        // after 2 misses I'll show something

    }

    // ============================================================================
    irr::core::stringw CurrentUser::getUserName() const
    {
        if((getUserState() == US_SIGNED_IN ) || (getUserState() == US_GUEST))
        {
            assert(m_profile != NULL);
            return m_profile->getUserName();
        }
        return _("Currently not signed in");
    }

    // ============================================================================
    uint32_t CurrentUser::getID() const
    {
        if((getUserState() == US_SIGNED_IN ))
        {
            assert(m_profile != NULL);
            return m_profile->getID();
        }
        return 0;
    }
} // namespace Online
