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

#include "states_screens/help_screen_1.hpp"

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "input/keyboard_device.hpp"
#include "karts/kart_properties_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/help_screen_2.hpp"
#include "states_screens/help_screen_3.hpp"
#include "states_screens/help_screen_4.hpp"
#include "states_screens/help_screen_5.hpp"
#include "states_screens/help_screen_6.hpp"
#include "states_screens/help_screen_7.hpp"
#include "states_screens/state_manager.hpp"

using namespace GUIEngine;

// FIXME : it's hugely repetitive to have one class per help screen when
//         THEY ALL DO THE SAME THING
//         (the specialized test of this first screen is a tiny exception)
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
        RaceManager::get()->setNumPlayers(1);
        RaceManager::get()->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
        RaceManager::get()->setMinorMode (RaceManager::MINOR_MODE_TUTORIAL);
        RaceManager::get()->setNumKarts( 1 );
        RaceManager::get()->setTrack( "tutorial" );
        RaceManager::get()->setDifficulty(RaceManager::DIFFICULTY_EASY);
        RaceManager::get()->setReverseTrack(false);

        // Use the last used device
        InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();

        // Create player and associate player with keyboard
        StateManager::get()->createActivePlayer(PlayerManager::getCurrentPlayer(),
                                                device);

        if (kart_properties_manager->getKart(UserConfigParams::m_default_kart) == NULL)
        {
            Log::warn("HelpScreen1", "Cannot find kart '%s', will revert to default",
                      UserConfigParams::m_default_kart.c_str());
            UserConfigParams::m_default_kart.revertToDefaults();
        }
        RaceManager::get()->setPlayerKart(0, UserConfigParams::m_default_kart);

        // ASSIGN should make sure that only input from assigned devices
        // is read.
        input_manager->getDeviceManager()->setAssignMode(ASSIGN);
        input_manager->getDeviceManager()
            ->setSinglePlayer( StateManager::get()->getActivePlayer(0) );

        StateManager::get()->enterGameState();
        RaceManager::get()->setupPlayerKartInfo();
        RaceManager::get()->startNew(false);
    }
    else if (name == "category")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        Screen *screen = NULL;
        //if (selection == "page1")
        //    screen = HelpScreen1::getInstance();
        if (selection == "page2")
            screen = HelpScreen2::getInstance();
        else if (selection == "page3")
            screen = HelpScreen3::getInstance();
        else if (selection == "page4")
            screen = HelpScreen4::getInstance();
        else if (selection == "page5")
            screen = HelpScreen5::getInstance();
        else if (selection == "page6")
            screen = HelpScreen6::getInstance();
        else if (selection == "page7")
            screen = HelpScreen7::getInstance();
        if(screen)
            StateManager::get()->replaceTopMostScreen(screen);
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
