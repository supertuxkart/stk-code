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

#include "states_screens/dialogs/notification_dialog.hpp"

#include <IGUIEnvironment.h>

#include "config/player_manager.hpp"
#include "guiengine/engine.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/online_profile_achievements.hpp"
#include "states_screens/online_profile_friends.hpp"
#include "utils/translation.hpp"


using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;
using namespace Online;

// -----------------------------------------------------------------------------

NotificationDialog::NotificationDialog(Type type, const core::stringw &info)
        : ModalDialog(0.8f,0.5f)
{
    m_info = info;
    m_type = type;
}

void NotificationDialog::load()
{
    loadFromFile("online/notification_dialog.stkgui");
}

void NotificationDialog::beforeAddingWidgets()
{
    m_self_destroy = false;
    m_view = false;
    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget != NULL);
    m_info_widget->setText(m_info, false);
    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_view_widget = getWidget<IconButtonWidget>("view");
    assert(m_view_widget != NULL);
    if(m_type == T_Friends)
    {
        m_view_widget->setText(_("View Friends"));
    }
    else if (m_type == T_Achievements)
    {
        m_view_widget->setText(_("View Achievements"));
    }
    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);
    m_options_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
}



// -----------------------------------------------------------------------------
NotificationDialog::~NotificationDialog()
{
}

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation NotificationDialog::processEvent(const std::string& eventSource)
{

    if (eventSource == m_options_widget->m_properties[PROP_ID])
    {
        const std::string& selection = m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == m_cancel_widget->m_properties[PROP_ID])
        {
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == m_view_widget->m_properties[PROP_ID])
        {
            m_view = true;
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}

// -----------------------------------------------------------------------------

void NotificationDialog::onEnterPressedInternal()
{

    //If enter was pressed while none of the buttons was focused interpret as close
    const int playerID = PLAYER_ID_GAME_MASTER;
    if (GUIEngine::isFocusedForPlayer(m_options_widget, playerID))
        return;
    m_self_destroy = true;
}

// -----------------------------------------------------------------------------

bool NotificationDialog::onEscapePressed()
{
    if (m_cancel_widget->isActivated())
        m_self_destroy = true;
    return false;
}

// -----------------------------------------------------------------------------

void NotificationDialog::onUpdate(float dt)
{
    //If we want to open the registration dialog, we need to close this one first
    if (m_view) m_self_destroy = true;

    // It's unsafe to delete from inside the event handler so we do it here
    if (m_self_destroy)
    {
        // Since dismiss deletes this object, store the instance values which
        // we still need
        bool view = m_view;
        NotificationDialog::Type type = m_type;
        ModalDialog::dismiss();
        if (view)
        {
            if(type == T_Friends)
            {
                ProfileManager::get()->setVisiting(PlayerManager::getCurrentOnlineId());
                StateManager::get()->pushScreen(OnlineProfileFriends::getInstance());
            }
            else if (type == T_Achievements)
            {
                ProfileManager::get()->setVisiting(PlayerManager::getCurrentOnlineId());
                StateManager::get()->pushScreen(OnlineProfileAchievements::getInstance());
            }
        }
        return;
    }
}
