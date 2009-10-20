//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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

#include "states_screens/main_menu_screen.hpp"

#include "guiengine/widget.hpp"
#include "main_loop.hpp"
#include "states_screens/challenges.hpp"
#include "states_screens/credits.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/help_screen_1.hpp"
#include "states_screens/options_screen_av.hpp"
#include "states_screens/state_manager.hpp"

// FIXME : remove, temporary test
#include "states_screens/feature_unlocked.hpp"

using namespace GUIEngine;

MainMenuScreen::MainMenuScreen() : Screen("main.stkgui")
{
}


void MainMenuScreen::init()
{
}

void MainMenuScreen::tearDown()
{
}

void MainMenuScreen::eventCallback(Widget* widget, const std::string& name)
{
    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
    if (ribbon == NULL) return; // only interesting stuff in main menu is the ribbons
    std::string selection = ribbon->getSelectionIDString(GUI_PLAYER_ID);
    
    
    if (selection == "network")
    {
        printf("+++++ FeatureUnlockedCutScene::show() +++++\n");
        // FIXME : remove, temporary test
        StateManager::get()->pushScreen(FeatureUnlockedCutScene::getInstance());
    }
    
    if (selection == "new")
    {
        StateManager::get()->pushScreen( KartSelectionScreen::getInstance() );
    }
    else if (selection == "options")
    {
        StateManager::get()->pushScreen( OptionsScreenAV::getInstance() );
    }
    else if (selection == "quit")
    {
        main_loop->abort();
        return;
    }
    else if (selection == "about")
    {
        StateManager::get()->pushScreen(CreditsScreen::getInstance());
    }
    else if (selection == "help")
    {
        StateManager::get()->pushScreen(HelpScreen1::getInstance());
    }
    else if (selection == "challenges")
    {
        StateManager::get()->pushScreen(ChallengesScreen::getInstance());
    }
    
}

