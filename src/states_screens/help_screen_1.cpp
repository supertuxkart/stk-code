//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Marianne Gagnon
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

#include "states_screens/help_screen_1.hpp"

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/help_screen_2.hpp"
#include "states_screens/help_screen_3.hpp"
#include "states_screens/help_screen_4.hpp"
#include "states_screens/state_manager.hpp"

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( HelpScreen1 );

// -----------------------------------------------------------------------------

HelpScreen1::HelpScreen1() : Screen("help1.stkgui")
{
}   // HelpScreen1

// -----------------------------------------------------------------------------

void HelpScreen1::loadedFromFile()
{
}   // loadedFromFile

// -----------------------------------------------------------------------------

void HelpScreen1::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "startTutorial")
    {
        race_manager->setNumLocalPlayers(1);
        race_manager->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
        race_manager->setMinorMode (RaceManager::MINOR_MODE_TUTORIAL);
        race_manager->setNumKarts( 1 );
        race_manager->setTrack( "tutorial" );
        race_manager->setDifficulty(RaceManager::DIFFICULTY_EASY);
        race_manager->setReverseTrack(false);

        // Use keyboard 0 by default (FIXME: let player choose?)
        InputDevice* device = input_manager->getDeviceList()->getKeyboard(0);

        // Create player and associate player with keyboard
        StateManager::get()->createActivePlayer(PlayerManager::getCurrentPlayer(),
                                                device);

        if (kart_properties_manager->getKart(UserConfigParams::m_default_kart) == NULL)
        {
            fprintf(stderr, "[MainMenuScreen] WARNING: cannot find kart '%s', will revert to default\n",
                    UserConfigParams::m_default_kart.c_str());
            UserConfigParams::m_default_kart.revertToDefaults();
        }
        race_manager->setLocalKartInfo(0, UserConfigParams::m_default_kart);

        // ASSIGN should make sure that only input from assigned devices
        // is read.
        input_manager->getDeviceList()->setAssignMode(ASSIGN);
        input_manager->getDeviceList()
            ->setSinglePlayer( StateManager::get()->getActivePlayer(0) );

        StateManager::get()->enterGameState();
        race_manager->setupPlayerKartInfo();
        race_manager->startNew(false);
    }
    else if (name == "category")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str();

        //if (selection == "page1") StateManager::get()->replaceTopMostScreen(Help1Screen::getInstance());
        //else
        if (selection == "page2") StateManager::get()->replaceTopMostScreen(HelpScreen2::getInstance());
        else if (selection == "page3") StateManager::get()->replaceTopMostScreen(HelpScreen3::getInstance());
        else if (selection == "page4") StateManager::get()->replaceTopMostScreen(HelpScreen4::getInstance());
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

    if (StateManager::get()->getGameState() == GUIEngine::INGAME_MENU)
    {
        tutorial->setDeactivated();
    }
    else
    {
        tutorial->setActivated();
    }

    if (w != NULL)  w->select( "page1", PLAYER_ID_GAME_MASTER );
}   //init

// -----------------------------------------------------------------------------
