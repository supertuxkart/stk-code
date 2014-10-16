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

#include "states_screens/online_profile_base.hpp"

#include "config/player_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "online/online_profile.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"
#include "states_screens/online_profile_friends.hpp"
#include "states_screens/online_profile_achievements.hpp"
#include "states_screens/online_profile_settings.hpp"

#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace Online;


OnlineProfileBase::OnlineProfileBase(const char* filename) : Screen(filename)
{
}   // OnlineProfileBase

// -----------------------------------------------------------------------------
/** Callback when the xml file was loaded.
 */
void OnlineProfileBase::loadedFromFile()
{
    m_profile_tabs = getWidget<RibbonWidget>("profile_tabs");
    assert(m_profile_tabs != NULL);
    m_header = getWidget<LabelWidget>("title");
    assert(m_header != NULL);

    m_friends_tab =
        (IconButtonWidget *) m_profile_tabs->findWidgetNamed("tab_friends");
    assert(m_friends_tab != NULL);
    m_achievements_tab =
        (IconButtonWidget*)m_profile_tabs->findWidgetNamed("tab_achievements");
    assert(m_achievements_tab != NULL);
    m_settings_tab =
        (IconButtonWidget *) m_profile_tabs->findWidgetNamed("tab_settings");
    assert(m_settings_tab != NULL);
}   // loadedFromFile

// -----------------------------------------------------------------------------
/** Callback before widgets are added. Clears all widgets and saves the
 *  current profile.
 */
void OnlineProfileBase::beforeAddingWidget()
{
    m_visiting_profile = ProfileManager::get()->getVisitingProfile();
    if (!m_visiting_profile || !m_visiting_profile->isCurrentUser())
        m_settings_tab->setVisible(false);
    else
        m_settings_tab->setVisible(true);

    // If not logged in, don't show profile or friends
    if (!m_visiting_profile)
    {
        m_friends_tab->setVisible(false);
        m_profile_tabs->setVisible(false);
    }
}   // beforeAddingWidget

// -----------------------------------------------------------------------------
/** Called when entering this menu (before widgets are added).
 */
void OnlineProfileBase::init()
{
    Screen::init();

    m_friends_tab->setTooltip( _("Friends") );
    m_achievements_tab->setTooltip( _("Achievements") );
    m_settings_tab->setTooltip( _("Account Settings") );

    // If no visiting_profile is defined, use the data of the current player.
    if (!m_visiting_profile || m_visiting_profile->isCurrentUser())
        m_header->setText(_("Your profile"), false);
    else if (m_visiting_profile)
    {
        m_header->setText(m_visiting_profile->getUserName() + _("'s profile"), false);
    }
    else
        Log::error("OnlineProfileBase", "No visting profile");

}   // init

// -----------------------------------------------------------------------------
bool OnlineProfileBase::onEscapePressed()
{
    //return to main menu if it's your profile
    if (!m_visiting_profile || m_visiting_profile->isCurrentUser())
        return true;

    //return to your profile if it's another profile
    ProfileManager::get()->setVisiting(PlayerManager::getCurrentOnlineId());
    StateManager::get()->replaceTopMostScreen(OnlineProfileAchievements::getInstance());
    return false;
}   // onEscapePressed

// -----------------------------------------------------------------------------
/** Called when an event occurs (i.e. user clicks on something).
*/
void OnlineProfileBase::eventCallback(Widget* widget, const std::string& name,
                                      const int playerID)
{
    if (name == m_profile_tabs->m_properties[PROP_ID])
    {
        std::string selection =
            ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        StateManager *sm = StateManager::get();
        if (selection == m_friends_tab->m_properties[PROP_ID])
            sm->replaceTopMostScreen(OnlineProfileFriends::getInstance());
        else if (selection == m_achievements_tab->m_properties[PROP_ID])
            sm->replaceTopMostScreen(OnlineProfileAchievements::getInstance());
        else if (selection == m_settings_tab->m_properties[PROP_ID])
            sm->replaceTopMostScreen(OnlineProfileSettings::getInstance());
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}   // eventCallback

