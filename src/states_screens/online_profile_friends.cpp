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

#include "states_screens/online_profile_friends.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/online_user_search.hpp"
#include "states_screens/dialogs/user_info_dialog.hpp"
#include "utils/translation.hpp"
#include "online/messages.hpp"

#include <IGUIButton.h>

#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace Online;

DEFINE_SCREEN_SINGLETON( OnlineProfileFriends );

// -----------------------------------------------------------------------------

OnlineProfileFriends::OnlineProfileFriends() : OnlineProfileBase("online/profile_friends.stkgui")
{
    m_selected_friend_index = -1;
}   // OnlineProfileFriends

// -----------------------------------------------------------------------------

void OnlineProfileFriends::loadedFromFile()
{
    OnlineProfileBase::loadedFromFile();
    m_friends_list_widget = getWidget<GUIEngine::ListWidget>("friends_list");
    assert(m_friends_list_widget != NULL);
    m_search_button_widget = getWidget<GUIEngine::ButtonWidget>("search_button");
    assert(m_search_button_widget != NULL);
    m_search_box_widget = getWidget<GUIEngine::TextBoxWidget>("search_box");
    assert(m_search_box_widget != NULL);

}   // loadedFromFile

// ----------------------------------------------------------------------------

void OnlineProfileFriends::beforeAddingWidget()
{
    m_friends_list_widget->clearColumns();
    m_friends_list_widget->addColumn( _("Friends"), 3 );
}

// -----------------------------------------------------------------------------

void OnlineProfileFriends::init()
{
    OnlineProfileBase::init();
    m_profile_tabs->select( m_friends_tab->m_properties[PROP_ID], PLAYER_ID_GAME_MASTER );
    assert(m_visiting_profile != NULL);
    m_visiting_profile->fetchFriends();
    m_waiting_for_friends = true;
    m_friends_list_widget->clear();
    m_friends_list_widget->addItem("spacer", L"");
    m_friends_list_widget->addItem("loading", Messages::fetchingFriends());
}   // init
// -----------------------------------------------------------------------------

void OnlineProfileFriends::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    OnlineProfileBase::eventCallback( widget, name, playerID);
    if (name == m_search_button_widget->m_properties[GUIEngine::PROP_ID])
    {
        OnlineUserSearch * instance = OnlineUserSearch::getInstance();
        instance->setSearchString(m_search_box_widget->getText().trim());
        StateManager::get()->pushScreen(instance);
    }
    else if (name == m_friends_list_widget->m_properties[GUIEngine::PROP_ID])
    {
        m_selected_friend_index = m_friends_list_widget->getSelectionID();
        new UserInfoDialog(m_visiting_profile->getFriends()[m_selected_friend_index]);
    }
}   // eventCallback

// ----------------------------------------------------------------------------
void OnlineProfileFriends::onUpdate(float delta,  irr::video::IVideoDriver* driver)
{
    if(m_waiting_for_friends)
    {
        if(m_visiting_profile->isReady())
        {
            m_friends_list_widget->clear();
            for(unsigned int i = 0; i < m_visiting_profile->getFriends().size(); i++)
            {
                PtrVector<GUIEngine::ListWidget::ListCell> * row = new PtrVector<GUIEngine::ListWidget::ListCell>;
                Profile * friend_profile = ProfileManager::get()->getProfileByID(m_visiting_profile->getFriends()[i]);
                row->push_back(new GUIEngine::ListWidget::ListCell(friend_profile->getUserName(),-1,3));
                m_friends_list_widget->addItem("friend", row);
            }
            m_waiting_for_friends = false;
        }
        else
        {
            m_friends_list_widget->renameItem("loading", Messages::fetchingFriends());
        }
    }
}
