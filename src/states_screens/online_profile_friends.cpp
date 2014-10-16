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
#include "states_screens/dialogs/user_info_dialog.hpp"
#include "states_screens/online_user_search.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

#include <IGUIButton.h>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace Online;

DEFINE_SCREEN_SINGLETON( OnlineProfileFriends );

// -----------------------------------------------------------------------------
/** Constructor for a display of all friends.
 */
OnlineProfileFriends::OnlineProfileFriends()
                    : OnlineProfileBase("online/profile_friends.stkgui")
{
}   // OnlineProfileFriends

// -----------------------------------------------------------------------------
/** Callback when the xml file was loaded.
 */
void OnlineProfileFriends::loadedFromFile()
{
    OnlineProfileBase::loadedFromFile();
    m_friends_list_widget = getWidget<ListWidget>("friends_list");
    assert(m_friends_list_widget != NULL);
    m_search_button_widget = getWidget<ButtonWidget>("search_button");
    assert(m_search_button_widget != NULL);
    m_search_box_widget = getWidget<TextBoxWidget>("search_box");
    assert(m_search_box_widget != NULL);
}   // loadedFromFile

// ----------------------------------------------------------------------------
/** Callback before widgets are added. Clears all widgets.
*/
void OnlineProfileFriends::beforeAddingWidget()
{
    OnlineProfileBase::beforeAddingWidget();
    m_friends_list_widget->clearColumns();
    m_friends_list_widget->addColumn( _("Username"), 2 );
    if(m_visiting_profile->isCurrentUser())
    {
        m_friends_list_widget->addColumn( _("Since"), 1 );
        m_friends_list_widget->addColumn( _("Status"), 2 );
    }
}   // beforeAddingWidget

// -----------------------------------------------------------------------------
/** Called when entering this menu (before widgets are added).
 */
void OnlineProfileFriends::init()
{
    OnlineProfileBase::init();
    m_profile_tabs->select( m_friends_tab->m_properties[PROP_ID],
                            PLAYER_ID_GAME_MASTER );
    assert(m_visiting_profile != NULL);
    m_visiting_profile->fetchFriends();
    m_waiting_for_friends = true;
    m_friends_list_widget->clear();
    m_friends_list_widget->addItem("loading",
                              StringUtils::loadingDots(_("Fetching friends")));
}   // init

// -----------------------------------------------------------------------------
/** Called when an event occurs (i.e. user clicks on something).
*/
void OnlineProfileFriends::eventCallback(Widget* widget,
                                         const std::string& name,
                                         const int playerID)
{
    OnlineProfileBase::eventCallback( widget, name, playerID);
    if (name == m_search_button_widget->m_properties[GUIEngine::PROP_ID])
    {
        OnlineUserSearch * instance = OnlineUserSearch::getInstance();
        instance->setSearchString(m_search_box_widget->getText().trim());
        StateManager::get()->replaceTopMostScreen(instance);
    }
    else if (name == m_friends_list_widget->m_properties[GUIEngine::PROP_ID])
    {
        int index = m_friends_list_widget->getSelectionID();
        if (index>-1)
            new UserInfoDialog(m_visiting_profile->getFriends()[index]);
    }
}   // eventCallback

// ----------------------------------------------------------------------------
/** Displays the friends from a given profile.
 */
void OnlineProfileFriends::displayResults()
{
    m_friends_list_widget->clear();
    const OnlineProfile::IDList &friends = m_visiting_profile->getFriends();
    for (unsigned int i = 0; i < friends.size(); i++)
    {
        std::vector<ListWidget::ListCell> row;
        OnlineProfile* friend_profile =
            ProfileManager::get()->getProfileByID(friends[i]);

        // When looking at friends of a friend those profiles are not
        // guaranteed to be persistent, so they might not be found in cache.
        if (!friend_profile)
        {
            Log::warn("OnlineProfileFriends",
                      "Profile for %d not found - ignored.", friends[i]);
            continue;
        }
        row.push_back(ListWidget::ListCell(friend_profile->getUserName(),
                                           -1, 2)                         );
        if (m_visiting_profile->isCurrentUser())
        {
            OnlineProfile::RelationInfo * relation_info =
                                             friend_profile->getRelationInfo();
            row.push_back(ListWidget::ListCell(relation_info->getDate(),
                                               -1, 1, true)             );
            irr::core::stringw status("");
            if (relation_info->isPending())
            {
                status = (relation_info->isAsker() ? _("New Request")
                                                   : _("Pending")     );
            }
            else
                status = (relation_info->isOnline() ? _("Online")
                                                    : _("Offline") );
            row.push_back(ListWidget::ListCell(status, -1, 2, true));
        }
        m_friends_list_widget->addItem("friend", row);
    }
    m_waiting_for_friends = false;

}   // displayResults

// ----------------------------------------------------------------------------
/** Called each frame to check if results have arrived.
 *  \param delta Time step size.
 */
void OnlineProfileFriends::onUpdate(float delta)
{
    if(m_waiting_for_friends)
    {
        if(m_visiting_profile->hasFetchedFriends())
        {
            displayResults();
        }
        else
        {
            m_friends_list_widget->renameItem("loading",
                              StringUtils::loadingDots(_("Fetching friends")));
        }
    }
}   // onUpdate
