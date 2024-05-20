//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 C. Michael Murphey
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

// Manages includes common to all help screens
#include "states_screens/help/help_common.hpp"

using namespace GUIEngine;

// -----------------------------------------------------------------------------

HelpScreen5::HelpScreen5() : Screen("help/help5.stkgui")
{
}   // HelpScreen5

// -----------------------------------------------------------------------------

void HelpScreen5::loadedFromFile()
{
}   // loadedFromFile

// -----------------------------------------------------------------------------

void HelpScreen5::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "category")
    {
        
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection != "page5")
            HelpCommon::switchTab(selection);
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}   // eventCallback

// -----------------------------------------------------------------------------

void HelpScreen5::init()
{
    Screen::init();
    RibbonWidget* w = this->getWidget<RibbonWidget>("category");

    if (w != NULL)
    {
        w->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
        w->select( "page5", PLAYER_ID_GAME_MASTER );
    }
}   // init

// -----------------------------------------------------------------------------
