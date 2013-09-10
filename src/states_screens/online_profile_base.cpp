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

#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"
#include "states_screens/online_profile_overview.hpp"
#include "states_screens/online_profile_friends.hpp"
#include "states_screens/online_profile_achievements.hpp"

#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace Online;


OnlineProfileBase::OnlineProfileBase(const char* filename) : Screen(filename)
{
    m_visiting_profile = ProfileManager::get()->getVisitingProfile();
}   // OnlineProfileBase

// -----------------------------------------------------------------------------

void OnlineProfileBase::loadedFromFile()
{
    m_profile_tabs = this->getWidget<RibbonWidget>("profile_tabs");
    assert(m_profile_tabs != NULL);
    m_header = this->getWidget<LabelWidget>("title");
    assert(m_header != NULL);

    m_overview_tab = (IconButtonWidget *) m_profile_tabs->findWidgetNamed("tab_overview");
    assert(m_overview_tab != NULL);
    m_friends_tab = (IconButtonWidget *) m_profile_tabs->findWidgetNamed("tab_friends");
    assert(m_friends_tab != NULL);
    m_achievements_tab = (IconButtonWidget *) m_profile_tabs->findWidgetNamed("tab_achievements");
    assert(m_achievements_tab != NULL);

}   // loadedFromFile

// -----------------------------------------------------------------------------
void OnlineProfileBase::beforeAddingWidget()
{
}

// -----------------------------------------------------------------------------
void OnlineProfileBase::init()
{
    Profile * previous_profile = m_visiting_profile;
    m_visiting_profile = ProfileManager::get()->getVisitingProfile();

    if(previous_profile->isCurrentUser() != m_visiting_profile->isCurrentUser())
    {
        GUIEngine::reshowCurrentScreen();
        return;
    }

    Screen::init();

    m_overview_tab->setTooltip( _("Overview") );
    m_friends_tab->setTooltip( _("Friends") );
    m_achievements_tab->setTooltip( _("Achievements") );


    if (m_visiting_profile->isCurrentUser())
        m_header->setText(_("Your profile"), false);
    else
        m_header->setText( m_visiting_profile->getUserName() +  _("'s profile"), false);

}   // init

// -----------------------------------------------------------------------------

void OnlineProfileBase::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == m_profile_tabs->m_properties[PROP_ID])
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str();

        if (selection == m_overview_tab->m_properties[PROP_ID]) StateManager::get()->replaceTopMostScreen(OnlineProfileOverview::getInstance());
        else if (selection == m_friends_tab->m_properties[PROP_ID]) StateManager::get()->replaceTopMostScreen(OnlineProfileFriends::getInstance());
        else if (selection == m_achievements_tab->m_properties[PROP_ID]) StateManager::get()->replaceTopMostScreen(OnlineProfileAchievements::getInstance());
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}   // eventCallback

