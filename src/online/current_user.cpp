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

#include "achievements/achievements_manager.hpp"
#include "addons/addons_manager.hpp"
#include "config/user_config.hpp"
#include "online/servers_manager.hpp"
#include "online/profile_manager.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"
#include "addons/addon.hpp"
#include "guiengine/dialog_queue.hpp"
#include "states_screens/dialogs/change_password_dialog.hpp"
#include "states_screens/dialogs/login_dialog.hpp"
#include "states_screens/dialogs/user_info_dialog.hpp"
#include "states_screens/dialogs/notification_dialog.hpp"
#include "states_screens/online_profile_friends.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>

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
        assert(m_state == US_SIGNED_OUT || m_state == US_GUEST);
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
        assert(m_state == US_SIGNED_OUT || m_state == US_GUEST);
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
        if(m_state == US_SIGNED_OUT  && UserConfigParams::m_saved_session)
        {
            request = new SignInRequest(true);
            request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
            request->setParameter("action",std::string("saved-session"));
            request->setParameter("userid", UserConfigParams::m_saved_user);
            request->setParameter("token", UserConfigParams::m_saved_token.c_str());
            HTTPManager::get()->addRequest(request);
            m_state = US_SIGNING_IN;
        }
    }

    CurrentUser::SignInRequest * CurrentUser::requestSignIn(    const irr::core::stringw &username,
                                                                const irr::core::stringw &password,
                                                                bool save_session, bool request_now)
    {
        assert(m_state == US_SIGNED_OUT);
        m_save_session = save_session;
        SignInRequest * request = new SignInRequest(request_now);
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action",std::string("connect"));
        request->setParameter("username",username);
        request->setParameter("password",password);
        request->setParameter("save-session", StringUtils::boolstr(save_session));
        if (request_now)
        {
            HTTPManager::get()->addRequest(request);
            m_state = US_SIGNING_IN;
        }
        return request;
    }

    void CurrentUser::signIn(bool success, const XMLNode * input)
    {
        if (success)
        {
            int token_fetched       = input->get("token", &m_token);
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
            AchievementsManager::get()->updateCurrentPlayer();
            std::string achieved_string("");
            if(input->get("achieved", &achieved_string) == 1)
            {
                std::vector<uint32_t> achieved_ids = StringUtils::splitToUInt(achieved_string, ' ');
                AchievementsManager::get()->getActive()->sync(achieved_ids);
            }
            m_profile->fetchFriends();
        }
        else
        {
            m_state = US_SIGNED_OUT;
        }
    }

    void CurrentUser::SignInRequest::callback()
    {
        CurrentUser::get()->signIn(m_success, m_result);
        if(GUIEngine::ModalDialog::isADialogActive())
        {
            LoginDialog * dialog  = dynamic_cast<LoginDialog*>(GUIEngine::ModalDialog::getCurrent());
            if(dialog != NULL)
            {
                if(m_success)
                    dialog->success();
                else
                    dialog->error(m_info);
            }
        }
    }

    // ============================================================================

    const CurrentUser::ServerCreationRequest * CurrentUser::requestServerCreation(  const irr::core::stringw &name,
                                                                                    int max_players)
    {
        assert(m_state == US_SIGNED_IN);
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
    void CurrentUser::requestSignOut(){
        assert(m_state == US_SIGNED_IN || m_state == US_GUEST);
        SignOutRequest * request = new SignOutRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action",std::string("disconnect"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        HTTPManager::get()->addRequest(request);
        m_state = US_SIGNING_OUT;
    }

    void CurrentUser::signOut(bool success, const XMLNode * input)
    {
        if(!success)
        {
            Log::warn("CurrentUser::signOut", "%s", "There were some connection issues while signing out. Report a bug if this caused issues.");
        }
        m_token = "";
        ProfileManager::get()->clearPersistent();
        m_profile = NULL;
        m_state = US_SIGNED_OUT;
        UserConfigParams::m_saved_user = 0;
        UserConfigParams::m_saved_token = "";
        UserConfigParams::m_saved_session = false;
        AchievementsManager::get()->updateCurrentPlayer();
    }

    void CurrentUser::SignOutRequest::callback()
    {
        CurrentUser::get()->signOut(m_success, m_result);
    }

    // ============================================================================

    CurrentUser::ServerJoinRequest *  CurrentUser::requestServerJoin(uint32_t server_id,
                                                                    bool request_now)
    {
        assert(m_state == US_SIGNED_IN || m_state == US_GUEST);
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
        if(m_success)
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
        assert(m_state == US_SIGNED_IN);
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
    /**
     * A request to the server, to fetch matching results for the supplied search term.
     * \param search_string the string to search for.
     */
    const XMLRequest * CurrentUser::requestUserSearch( const irr::core::stringw & search_string) const
    {
        assert(m_state == US_SIGNED_IN);
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
    /**
     * A request to the server, to perform a vote on an addon.
     * \param addon_id the id of the addon to vote for.
     * \param rating the voted rating.
     */
    const CurrentUser::SetAddonVoteRequest * CurrentUser::requestSetAddonVote( const std::string & addon_id, float rating) const
    {
        assert(m_state == US_SIGNED_IN);
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

    /**
     * Callback for the request to vote for an addon. Updates the local average rating.
     */
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
    /**
     * A request to the server, to invite a user to be friends.
     * \param friend_id The id of the user which has to be friended.
     */
    void CurrentUser::requestFriendRequest(const uint32_t friend_id) const
    {
        assert(m_state == US_SIGNED_IN);
        CurrentUser::FriendRequest * request = new CurrentUser::FriendRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("friend-request"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        request->setParameter("friendid", friend_id);
        HTTPManager::get()->addRequest(request);
    }

    /**
     * Callback for the request to send a friend invitation. Shows a confirmation message and takes care of updating all the cached information.
     */
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
    /**
     * A request to the server, to accept a friend request.
     * \param friend_id The id of the user of which the request has to be accepted.
     */
    void CurrentUser::requestAcceptFriend(const uint32_t friend_id) const
    {
        assert(m_state == US_SIGNED_IN);
        CurrentUser::AcceptFriendRequest * request = new CurrentUser::AcceptFriendRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("accept-friend-request"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        request->setParameter("friendid", friend_id);
        HTTPManager::get()->addRequest(request);
    }

    /**
     * Callback for the request to accept a friend invitation. Shows a confirmation message and takes care of updating all the cached information.
     */
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
    /**
     * A request to the server, to decline a friend request.
     * \param friend_id The id of the user of which the request has to be declined.
     */
    void CurrentUser::requestDeclineFriend(const uint32_t friend_id) const
    {
        assert(m_state == US_SIGNED_IN);
        CurrentUser::DeclineFriendRequest * request = new CurrentUser::DeclineFriendRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("decline-friend-request"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        request->setParameter("friendid", friend_id);
        HTTPManager::get()->addRequest(request);
    }

    /**
     * Callback for the request to decline a friend invitation. Shows a confirmation message and takes care of updating all the cached information.
     */
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
    /**
     * A request to the server, to cancel a pending friend request.
     * \param friend_id The id of the user of which the request has to be canceled.
     */
    void CurrentUser::requestCancelFriend(const uint32_t friend_id) const
    {
        assert(m_state == US_SIGNED_IN);
        CurrentUser::CancelFriendRequest * request = new CurrentUser::CancelFriendRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("cancel-friend-request"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        request->setParameter("friendid", friend_id);
        HTTPManager::get()->addRequest(request);
    }

    /**
     * Callback for the request to cancel a friend invitation. Shows a confirmation message and takes care of updating all the cached information.
     */
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
    /**
     * A request to the server, to remove a friend relation.
     * \param friend_id The id of the friend to be removed.
     */
    void CurrentUser::requestRemoveFriend(const uint32_t friend_id) const
    {
        assert(m_state == US_SIGNED_IN);
        CurrentUser::RemoveFriendRequest * request = new CurrentUser::RemoveFriendRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("remove-friend"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        request->setParameter("friendid", friend_id);
        HTTPManager::get()->addRequest(request);
    }

    /**
     * Callback for the request to remove a friend. Shows a confirmation message and takes care of updating all the cached information.
     */
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
    /**
     * A request to the server, to change the password of the signed in user.
     * \param current_password The active password of the currently signed in user.
     * \param new_password     The password the user wants to change to.
     * \param new_password_ver Confirmation of that password. Has to be the exact same.
     */
    void CurrentUser::requestPasswordChange(const irr::core::stringw &current_password,
                                            const irr::core::stringw &new_password,
                                            const irr::core::stringw &new_password_ver) const
    {
        assert(m_state == US_SIGNED_IN);
        ChangePasswordRequest * request = new ChangePasswordRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("change_password"));
        request->setParameter("userid", getID());
        request->setParameter("current", current_password);
        request->setParameter("new1", new_password);
        request->setParameter("new2", new_password_ver);
        HTTPManager::get()->addRequest(request);
    }

    /**
     * Callback for the change password request. If the matching dialog is still open, show a confirmation message.
     */
    void CurrentUser::ChangePasswordRequest::callback()
    {
        if(GUIEngine::ModalDialog::isADialogActive())
        {
            ChangePasswordDialog * dialog  = dynamic_cast<ChangePasswordDialog*>(GUIEngine::ModalDialog::getCurrent());
            if(dialog != NULL)
            {
                if(m_success)
                    dialog->success();
                else
                    dialog->error(m_info);
            }
        }
    }
    // ============================================================================
    /**
     * Sends a request to the server to see if any new information is available. (online friends, notifications, etc.).
     */
    void CurrentUser::requestPoll() const
    {
        assert(m_state == US_SIGNED_IN);
        CurrentUser::PollRequest * request = new CurrentUser::PollRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
        request->setParameter("action", std::string("poll"));
        request->setParameter("token", getToken());
        request->setParameter("userid", getID());
        HTTPManager::get()->addRequest(request);
    }

    /**
     * Callback for the poll request. Parses the information and spawns notifications accordingly.
     */
    void CurrentUser::PollRequest::callback()
    {
        if(m_success)
        {
            if(!CurrentUser::get()->isRegisteredUser())
                return;
            if(CurrentUser::get()->getProfile()->hasFetchedFriends())
            {
                std::string online_friends_string("");
                if(m_result->get("online", &online_friends_string) == 1)
                {
                    std::vector<uint32_t> online_friends = StringUtils::splitToUInt(online_friends_string, ' ');
                    bool went_offline = false;
                    std::vector<uint32_t> friends = CurrentUser::get()->getProfile()->getFriends();
                    std::vector<irr::core::stringw> to_notify;
                    for(unsigned int i = 0; i < friends.size(); ++i)
                    {
                         bool now_online = false;
                         std::vector<uint32_t>::iterator iter =
                             std::find(online_friends.begin(),online_friends.end(), friends[i]);
                         if (iter != online_friends.end())
                         {
                             now_online = true;
                             online_friends.erase(iter);
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
                                 //User came online
                                 relation_info->setOnline(true);
                                 profile->setFriend(); //Do this because a user might have accepted a pending friend request.
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
            }
            else
            {
                CurrentUser::get()->getProfile()->fetchFriends();
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
                    message = _("You have a new friend request!");
                }
                GUIEngine::DialogQueue::get()->pushDialog( new NotificationDialog(NotificationDialog::T_Friends, message), false);
                OnlineProfileFriends::getInstance()->refreshFriendsList();
            }
        }
        // FIXME show connection error??
        // Perhaps show something after 2 misses.

    }
    // ============================================================================
    /**
     * Sends a message to the server that the client has been closed, if a user is signed in.
     */
    void CurrentUser::onSTKQuit() const
    {
        if(isRegisteredUser())
        {
            HTTPRequest * request = new HTTPRequest(true, HTTPManager::MAX_PRIORITY);
            request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
            request->setParameter("action", std::string("client-quit"));
            request->setParameter("token", getToken());
            request->setParameter("userid", getID());
            HTTPManager::get()->addRequest(request);
        }
    }

    // ============================================================================
    /**
     * Sends a confirmation to the server that an achievement has been completed, if a user is signed in.
     * \param achievement_id the id of the achievement that got completed
     */
    void CurrentUser::onAchieving(uint32_t achievement_id) const
    {
        if(isRegisteredUser())
        {
            HTTPRequest * request = new HTTPRequest(true);
            request->setURL((std::string)UserConfigParams::m_server_multiplayer + "client-user.php");
            request->setParameter("action", std::string("achieving"));
            request->setParameter("token", getToken());
            request->setParameter("userid", getID());
            request->setParameter("achievementid", achievement_id);
            HTTPManager::get()->addRequest(request);
        }
    }

    // ============================================================================
    /** \return the username if signed in. */
    irr::core::stringw CurrentUser::getUserName() const
    {
        if((m_state == US_SIGNED_IN ) || (m_state == US_GUEST))
        {
            assert(m_profile != NULL);
            return m_profile->getUserName();
        }
        return _("Currently not signed in");
    }

    // ============================================================================
    /** \return the online id. */
    uint32_t CurrentUser::getID() const
    {
        if((m_state == US_SIGNED_IN ))
        {
            assert(m_profile != NULL);
            return m_profile->getID();
        }
        return 0;
    }
} // namespace Online
