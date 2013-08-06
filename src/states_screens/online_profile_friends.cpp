//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Glenn De Jonghe
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
#include "utils/translation.hpp"

#include <IGUIButton.h>

#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace Online;

DEFINE_SCREEN_SINGLETON( OnlineProfileFriends );

// -----------------------------------------------------------------------------

OnlineProfileFriends::OnlineProfileFriends() : Screen("online/profile_overview.stkgui")
{
}   // OnlineProfileFriends




// -----------------------------------------------------------------------------

void OnlineProfileFriends::loadedFromFile()
{
    m_profile_tabs = this->getWidget<RibbonWidget>("profile_tabs");
    assert(m_profile_tabs != NULL);
    LabelWidget * header = this->getWidget<LabelWidget>("title");
    assert(header != NULL);
    header->setText(_("Your profile"), false);

}   // loadedFromFile

// -----------------------------------------------------------------------------

void OnlineProfileFriends::init()
{
    Screen::init();
    m_profile_tabs->select( "tab_players", PLAYER_ID_GAME_MASTER );

    /*
    tabBar->getRibbonChildren()[0].setTooltip( _("Graphics") );
    tabBar->getRibbonChildren()[1].setTooltip( _("Audio") );
    tabBar->getRibbonChildren()[2].setTooltip( _("User Interface") );
    tabBar->getRibbonChildren()[4].setTooltip( _("Controls") );*/
}   // init

// -----------------------------------------------------------------------------

void OnlineProfileFriends::tearDown()
{
    Screen::tearDown();
}   // tearDown

// -----------------------------------------------------------------------------

void OnlineProfileFriends::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == m_profile_tabs->m_properties[PROP_ID])
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str();

        //if (selection == "tab_audio") StateManager::get()->replaceTopMostScreen(OptionsScreenAudio::getInstance());
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}   // eventCallback

