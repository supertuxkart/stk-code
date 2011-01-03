//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Alejandro Santiago
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

#include "states_screens/tutorial_screen.hpp"

#include "guiengine/widgets/list_widget.hpp"
#include "utils/string_utils.hpp"
#include "graphics/irr_driver.hpp"

#include "tutorial/tutorial_manager.hpp"
#include "config/user_config.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "network/network_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

#include <fstream>

#include "irrString.h"

using irr::core::stringw;
using irr::core::stringc;

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( TutorialScreen );

// ------------------------------------------------------------------------------------------------------

TutorialScreen::TutorialScreen() : Screen("tutorial.stkgui")
{
}

// ------------------------------------------------------------------------------------------------------

void TutorialScreen::loadedFromFile()
{
}

// ------------------------------------------------------------------------------------------------------

void TutorialScreen::onUpdate(float elapsed_time, irr::video::IVideoDriver*)
{
}

// ------------------------------------------------------------------------------------------------------

void TutorialScreen::init()
{
    Screen::init();

    ListWidget* tutorials_list = this->getWidget<ListWidget>("tutorials");
    assert( tutorials_list != NULL );
    
    // FIXME different icons for each tutorial option & add then to 
    video::ITexture* tutorial_icon = irr_driver->getTexture( file_manager->getGUIDir() + "/keyboard.png" );

    m_icon_bank = new irr::gui::STKModifiedSpriteBank( GUIEngine::getGUIEnv() );
    m_icon_bank->addTextureAsSprite(tutorial_icon);    

    // scale icons depending on screen resolution. the numbers below are a bit arbitrary
    const int screen_width = irr_driver->getFrameSize().Width;
    const float scale = 0.3f + 0.2f*std::max(0, screen_width - 640)/564.0f;
    m_icon_bank->setScale(scale);

    // Add the icons to our list
    tutorials_list->setIcons(m_icon_bank);
    // Re-build track list everytime (accounts for locking changes, etc.)
    //tutorials_list->clearItems();

/*    const std::vector<const Challenge*>& activeTutorials = tutorial_manager->getActiveChallenges();
    const std::vector<const Challenge*>& solvedTutorials = tutorial_manager->getUnlockedFeatures();
    const std::vector<const Challenge*>& lockedTutorials = tutorial_manager->getLockedChallenges();

    const int activeTutorialAmount = activeTutorials.size();
    const int solvedTutorialAmount = solvedTutorials.size();
    const int lockedTutorialAmount = lockedTutorials.size();
*/

    // FIXME Hard coded tutorial options
    tutorials_list->addItem(BASIC_DRIVING, "  Basic Driving" , 0);
    tutorials_list->addItem(SHARP_TURN, "  Sharp Turn" , 0);
    tutorials_list->addItem(NITRO, "  Nitro" , 0 );
    tutorials_list->addItem(COLLECTIBLE_WEAPONS, "  Collectible Weapons" , 0);
    tutorials_list->addItem(SHOOTING_BACKWARDS, "  Shooting Backwards" , 0);

    /*    for (int i=0;i<5;i++)
    {
         //KeyboardConfig *config = input_manager->getDeviceList()->getKeyboardConfig(i);
        
        std::ostringstream kbname;
        kbname << "keyboard" << i;
        const std::string internal_name = kbname.str();
        
        //FIXME: I18N: since irrLicht's list widget has the nasty tendency to put the 
        //             icons very close to the text, I'm adding spaces to compensate...
        
    }
  //  tutorials_list->updateItemDisplay();

   /* if (tutorials_list->getItems().empty())
    {
        fprintf(stderr, "Error, no tutorial!\n");
        return;
    }*/
    //tutorials_list->setSelection(0 /* whatever is first */, PLAYER_ID_GAME_MASTER, true /* focus it */);
}

// ------------------------------------------------------------------------------------------------------

void TutorialScreen::eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "tutorials")
    {
        ListWidget* tutorials = this->getWidget<ListWidget>("tutorials");
        const std::string& selection = tutorials->getSelectionInternalName();
        if (selection == BASIC_DRIVING)
        {
            // Verify the kart in the config exists
            if (kart_properties_manager->getKart(UserConfigParams::m_default_kart) == NULL)
                UserConfigParams::m_default_kart.revertToDefaults();
            
            // Use latest used device
            InputDevice* device = input_manager->getDeviceList()->getLatestUsedDevice();

            // Create player and associate player with device (FIXME: ask for player ident)
            StateManager::get()->createActivePlayer( UserConfigParams::m_all_players.get(0), device );

            // Set up race manager appropriately
            race_manager->setNumLocalPlayers(1);
            race_manager->setLocalKartInfo(0, UserConfigParams::m_default_kart);

            // ASSIGN should make sure that only input from assigned devices is read.
            input_manager->getDeviceList()->setAssignMode(ASSIGN);

            // Go straight to the race
            StateManager::get()->enterGameState();

            // Initialise global data - necessary even in local games to avoid
            // many if tests in other places (e.g. if network_game call
            // network_manager else call race_manager).
            network_manager->initCharacterDataStructures();

            // Launch tutorial
            //m_tutorial_manager->getTutorial(selection)->setRace();

            
            // FIXME this code have to be in Tutorial class (and loaded from file xD)
            RaceManager::MajorRaceModeType m_major;
            RaceManager::MinorRaceModeType m_minor;
            RaceManager::Difficulty        m_difficulty;

            m_minor = RaceManager::MINOR_MODE_NORMAL_RACE;
            m_major = RaceManager::MAJOR_MODE_SINGLE;
            m_difficulty = RaceManager::RD_EASY;

            race_manager->setMinorMode(m_minor);
            race_manager->setMajorMode(m_major);
            race_manager->setTrack((std::string)"canyon");
            
            race_manager->setDifficulty(m_difficulty);
            race_manager->setNumLaps(1);
            race_manager->setNumKarts(1);
            race_manager->setNumLocalPlayers(1);              
            //race_manager->setCoinTarget(m_energy);*/

            // Sets up kart info, including random list of kart for AI
            network_manager->setupPlayerKartInfo();
            race_manager->startNew();
        }
        else if (name == SHARP_TURN)
        {
        }
        else if (name == NITRO)
        {
        }
        else if (name == COLLECTIBLE_WEAPONS)
        {
        }
        else if (name == SHOOTING_BACKWARDS)
        {
        }
    }
}
            /*int i = -1, read = 0;
            read = sscanf( selection.c_str(), "gamepad%i", &i );
            if (read == 1 && i != -1)
            {
                OptionsScreenInput2::getInstance()->setDevice( input_manager->getDeviceList()->getGamepadConfig(i) );
                StateManager::get()->replaceTopMostScreen(OptionsScreenInput2::getInstance());
                //updateInputButtons( input_manager->getDeviceList()->getGamepadConfig(i) );
            }
            else
            {
                std::cerr << "Cannot read internal gamepad input device ID : " << selection.c_str() << std::endl;
            }*/


     /*   DynamicRibbonWidget* w = this->getWidget<DynamicRibbonWidget>("tutorial");
        assert( w != NULL );

        // only player 0 can start a challenge (i.e. we have no multiplayer tutorial)
        std::string selection = w->getSelectionIDString( PLAYER_ID_GAME_MASTER );

        /*if (selection == "locked")
        {
            unlock_manager->playLockSound();
        }
        else if (!selection.empty())
        {
            //FIXME: simplify and centralize race start sequence!!

            // Verify the kart in the config exists
            if (kart_properties_manager->getKart(UserConfigParams::m_default_kart) == NULL)
            {
                UserConfigParams::m_default_kart.revertToDefaults();
            }

            // Use latest used device
            InputDevice* device = input_manager->getDeviceList()->getLatestUsedDevice();

            // Create player and associate player with device (FIXME: ask for player ident)
            StateManager::get()->createActivePlayer( UserConfigParams::m_all_players.get(0), device );

            // Set up race manager appropriately
            race_manager->setNumLocalPlayers(1);
            race_manager->setLocalKartInfo(0, UserConfigParams::m_default_kart);

            // ASSIGN should make sure that only input from assigned devices is read.
            input_manager->getDeviceList()->setAssignMode(ASSIGN);

            // Go straight to the race
            StateManager::get()->enterGameState();

            // Initialise global data - necessary even in local games to avoid
            // many if tests in other places (e.g. if network_game call
            // network_manager else call race_manager).
            network_manager->initCharacterDataStructures();

            // Launch challenge
            unlock_manager->getChallenge(selection)->setRace();

            // Sets up kart info, including random list of kart for AI
            network_manager->setupPlayerKartInfo();
            race_manager->startNew();
        //}
    }
}*/

// ------------------------------------------------------------------------------------------------------

