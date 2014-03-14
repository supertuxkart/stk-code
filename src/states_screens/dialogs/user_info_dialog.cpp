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

#include "states_screens/dialogs/user_info_dialog.hpp"

#include "audio/sfx_manager.hpp"
#include "guiengine/dialog_queue.hpp"
#include "guiengine/engine.hpp"
#include "online/online_profile.hpp"
#include "online/messages.hpp"
#include "states_screens/online_profile_friends.hpp"
#include "states_screens/online_profile_overview.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;
using namespace Online;

// -----------------------------------------------------------------------------

UserInfoDialog::UserInfoDialog(uint32_t showing_id, const core::stringw info, bool error, bool from_queue)
        : ModalDialog(0.8f,0.8f), m_showing_id(showing_id)
{
    m_error = error;
    m_info = info;
    if(!from_queue) load();
}

void UserInfoDialog::load()
{
    loadFromFile("online/user_info_dialog.stkgui");
    if(m_error)
        m_info_widget->setErrorColor();
    m_name_widget->setText(m_profile->getUserName(),false);
    m_info_widget->setText(m_info, false);
    if(m_remove_widget->isVisible() && !m_profile->isFriend())
        m_remove_widget->setLabel("Cancel Request");
}

void UserInfoDialog::beforeAddingWidgets()
{
    m_profile = ProfileManager::get()->getProfileByID(m_showing_id);
    m_self_destroy = false;
    m_enter_profile = false;
    m_processing = false;
    m_name_widget = getWidget<LabelWidget>("name");
    assert(m_name_widget != NULL);
    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget != NULL);

    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_remove_widget = getWidget<IconButtonWidget>("remove");
    assert(m_remove_widget != NULL);
    m_friend_widget = getWidget<IconButtonWidget>("friend");
    assert(m_friend_widget != NULL);
    m_accept_widget = getWidget<IconButtonWidget>("accept");
    assert(m_accept_widget != NULL);
    m_decline_widget = getWidget<IconButtonWidget>("decline");
    assert(m_decline_widget != NULL);
    m_enter_widget = getWidget<IconButtonWidget>("enter");
    assert(m_enter_widget != NULL);
    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);
    m_options_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    m_accept_widget->setVisible(false);
    m_decline_widget->setVisible(false);
    m_remove_widget->setVisible(false);
    if(m_profile->isCurrentUser())
    {
        m_friend_widget->setVisible(false);
    }
    if(m_profile->isFriend())
    {
        m_friend_widget->setVisible(false);
        m_remove_widget->setVisible(true);
    }

    OnlineProfile::RelationInfo * relation_info = m_profile->getRelationInfo();
    if(relation_info != NULL)
    {
        if(relation_info->isPending())
        {
            m_friend_widget->setVisible(false);
            if(relation_info->isAsker())
            {
                m_accept_widget->setVisible(true);
                m_decline_widget->setVisible(true);
            }
            else
            {
                m_remove_widget->setVisible(true);
            }
        }
    }

}



// -----------------------------------------------------------------------------
UserInfoDialog::~UserInfoDialog()
{
}

// -----------------------------------------------------------------------------
void UserInfoDialog::sendFriendRequest()
{
    class FriendRequest : public XMLRequest
    {
        // ------------------------------------------------------------------------
        /** Callback for the request to send a friend invitation. Shows a
         *  confirmation message and takes care of updating all the cached
         *  information.
         */
        virtual void callback()
        {
            uint32_t id(0);
            getXMLData()->get("friendid", &id);
            core::stringw info_text("");
            if (isSuccess())
            {
                CurrentUser::get()->getProfile()->addFriend(id);
                OnlineProfile::RelationInfo *info =
                             new OnlineProfile::RelationInfo(_("Today"), false,
                                                             true, false);
                ProfileManager::get()->getProfileByID(id)->setRelationInfo(info);
                OnlineProfileFriends::getInstance()->refreshFriendsList();
                info_text = _("Friend request send!");
            }
            else
                info_text = getInfo();
            UserInfoDialog *dialog = new UserInfoDialog(id, info_text,
                                                       !isSuccess(), true);
            GUIEngine::DialogQueue::get()->pushDialog(dialog, true);

        }   // callback
    public:
        FriendRequest() : XMLRequest(true) {}
    };   // FriendRequest

    // ------------------------------------------------------------------------

    FriendRequest *request = new FriendRequest();
    CurrentUser::setUserDetails(request);
    request->addParameter("action", "friend-request");
    request->addParameter("friendid", m_profile->getID());
    request->queue();

    m_processing = true;
    m_options_widget->setDeactivated();

}   // sendFriendRequest

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation UserInfoDialog::processEvent(const std::string& eventSource)
{

    if (eventSource == m_options_widget->m_properties[PROP_ID])
    {
        const std::string& selection = m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == m_cancel_widget->m_properties[PROP_ID])
        {
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == m_enter_widget->m_properties[PROP_ID])
        {
            ProfileManager::get()->setVisiting(m_profile->getID());
            m_enter_profile = true;
            m_options_widget->setDeactivated();
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == m_friend_widget->m_properties[PROP_ID])
        {
            sendFriendRequest();
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == m_remove_widget->m_properties[PROP_ID])
        {
            if(m_profile->getRelationInfo()->isPending())
                CurrentUser::get()->requestCancelFriend(m_profile->getID());
            else
                CurrentUser::get()->requestRemoveFriend(m_profile->getID());
            m_processing = true;
            m_options_widget->setDeactivated();
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == m_accept_widget->m_properties[PROP_ID])
        {
            CurrentUser::get()->requestAcceptFriend(m_profile->getID());
            m_processing = true;
            m_options_widget->setDeactivated();
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == m_decline_widget->m_properties[PROP_ID])
        {
            CurrentUser::get()->requestDeclineFriend(m_profile->getID());
            m_processing = true;
            m_options_widget->setDeactivated();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}

// -----------------------------------------------------------------------------
void UserInfoDialog::deactivate()
{
    m_options_widget->setDeactivated();
}

// -----------------------------------------------------------------------------
void UserInfoDialog::activate()
{

}


// -----------------------------------------------------------------------------

void UserInfoDialog::onEnterPressedInternal()
{

    //If enter was pressed while none of the buttons was focused interpret as join event
    const int playerID = PLAYER_ID_GAME_MASTER;
    if (GUIEngine::isFocusedForPlayer(m_options_widget, playerID))
        return;
    m_self_destroy = true;
}

// -----------------------------------------------------------------------------

bool UserInfoDialog::onEscapePressed()
{
    if (m_cancel_widget->isActivated())
        m_self_destroy = true;
    return false;
}

// -----------------------------------------------------------------------------


void UserInfoDialog::onUpdate(float dt)
{
    if(m_processing) m_info_widget->setText(Messages::processing(), false);

    //If we want to open the registration dialog, we need to close this one first
    if (m_enter_profile) m_self_destroy = true;

    // It's unsafe to delete from inside the event handler so we do it here
    if (m_self_destroy)
    {
        ModalDialog::dismiss();
        if (m_enter_profile)
            StateManager::get()->replaceTopMostScreen(OnlineProfileOverview::getInstance());
        return;
    }
}
