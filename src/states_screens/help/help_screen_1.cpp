//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#include "guiengine/widgets/button_widget.hpp"
#include "modes/tutorial_utils.hpp"

#ifdef ANDROID
#include <SDL_system.h>
#endif

using namespace GUIEngine;

// -----------------------------------------------------------------------------

HelpScreen1::HelpScreen1() : Screen("help/help1.stkgui")
{
}   // HelpScreen1

// -----------------------------------------------------------------------------

void HelpScreen1::loadedFromFile()
{
}   // loadedFromFile

// -----------------------------------------------------------------------------

void HelpScreen1::beforeAddingWidget()
{
#ifdef ANDROID
    if (SDL_IsAndroidTV())
    {
        Widget* tutorial = getWidget("startTutorial");
        if (tutorial)
            tutorial->setVisible(false);

        Widget* tutorial_icon = getWidget("tutorialIcon");
        if (tutorial_icon)
            tutorial_icon->setVisible(false);
    }
#endif
}

void HelpScreen1::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "startTutorial")
    {
        TutorialUtils::startTutorial();
    }
    else if (name == "category")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection != "page1")
            HelpCommon::switchTab(selection);
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}   // eventCallback

// -----------------------------------------------------------------------------

void HelpScreen1::init()
{
    Screen::init();
    RibbonWidget* w = this->getWidget<RibbonWidget>("category");
    ButtonWidget* tutorial = getWidget<ButtonWidget>("startTutorial");

    tutorial->setActive(StateManager::get()->getGameState() !=
                                                       GUIEngine::INGAME_MENU);

    if (w != NULL)
    {
        w->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
        w->select( "page1", PLAYER_ID_GAME_MASTER );
    }
}   //init

// -----------------------------------------------------------------------------
