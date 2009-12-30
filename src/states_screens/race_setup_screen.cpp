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

#include <irrlicht.h>

#include "challenges/unlock_manager.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/arenas_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/tracks_screen.hpp"
#include "utils/translation.hpp"

#include "states_screens/race_setup_screen.hpp"


using namespace GUIEngine;

RaceSetupScreen::RaceSetupScreen() : Screen("racesetup.stkgui")
{
}


void RaceSetupScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "difficulty")
    {
        RibbonWidget* w = dynamic_cast<RibbonWidget*>(widget);
        assert(w != NULL);
        const std::string& selection = w->getSelectionIDString(GUI_PLAYER_ID);
        
        if (selection == "novice")
        {
            UserConfigParams::m_difficulty = RaceManager::RD_EASY;
            race_manager->setDifficulty(RaceManager::RD_EASY);
        }
        else if (selection == "intermediate")
        {
            UserConfigParams::m_difficulty = RaceManager::RD_MEDIUM;
            race_manager->setDifficulty(RaceManager::RD_MEDIUM);
        }
        else if (selection == "expert")
        {
            UserConfigParams::m_difficulty = RaceManager::RD_HARD;
            race_manager->setDifficulty(RaceManager::RD_HARD);
        }
    }
    else if (name == "gamemode")
    {
        DynamicRibbonWidget* w = dynamic_cast<DynamicRibbonWidget*>(widget);
        const std::string selectedMode = w->getSelectionIDString(GUI_PLAYER_ID);
        
        if (selectedMode == "normal")
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_NORMAL_RACE);
            StateManager::get()->pushScreen( TracksScreen::getInstance() );
        }
        else if (selectedMode == "timetrial")
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_TIME_TRIAL);
            StateManager::get()->pushScreen( TracksScreen::getInstance() );
        }
        else if (selectedMode == "ftl")
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_FOLLOW_LEADER);
            StateManager::get()->pushScreen( TracksScreen::getInstance() );
        }
        else if (selectedMode == "3strikes")
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_3_STRIKES);
            StateManager::get()->pushScreen( ArenasScreen::getInstance() );
        }
        else if (selectedMode == "locked")
        {
            std::cout << "Requesting sound to be played\n";
            unlock_manager->playLockSound();
        }
    }
    else if (name == "aikartamount")
    {
        SpinnerWidget* w = dynamic_cast<SpinnerWidget*>(widget);
        
        race_manager->setNumKarts( race_manager->getNumPlayers() + w->getValue() );
    }
    /*
     289         race_manager->setDifficulty((RaceManager::Difficulty)m_difficulty);
     290         UserConfigParams::setDefaultNumDifficulty(m_difficulty);
     
     // if there is no AI, there's no point asking the player for the amount of karts.
     299     // It will always be the same as the number of human players
     300     if(RaceManager::isBattleMode( race_manager->getMinorMode() ))
     301     {
     302         race_manager->setNumKarts(race_manager->getNumLocalPlayers());
     303         // Don't change the default number of karts in user_config
     304     }
     305     else
     306     {
     307         race_manager->setNumKarts(m_num_karts);
     308         UserConfigParams::setDefaultNumKarts(race_manager->getNumKarts());
     309     }
     
     311     if( race_manager->getMajorMode() != RaceManager::MAJOR_MODE_GRAND_PRIX    &&
     312         RaceManager::modeHasLaps( race_manager->getMinorMode() )    )
     313     {
     314         race_manager->setNumLaps( m_num_laps );
     315         UserConfigParams::setDefaultNumLaps(m_num_laps);
     316     }
     317     // Might still be set from a previous challenge
     318     race_manager->setCoinTarget(0);
     
     input_manager->setMode(InputManager::INGAME);
     
     race_manager->setLocalKartInfo(0, argv[i+1]);
     
     race_manager->setDifficulty(RaceManager::RD_EASY);
     race_manager->setDifficulty(RaceManager::RD_HARD);
     race_manager->setDifficulty(RaceManager::RD_HARD);
     
     race_manager->setTrack(argv[i+1]);
     
     UserConfigParams::setDefaultNumKarts(stk_config->m_max_karts);
     race_manager->setNumKarts(UserConfigParams::getDefaultNumKarts() );
     
     UserConfigParams::getDefaultNumKarts()
     
     StateManager::enterGameState();
     race_manager->startNew();
     */
    
    
}

void RaceSetupScreen::init()
{
    RibbonWidget* w = getWidget<RibbonWidget>("difficulty");
    assert( w != NULL );
    w->setSelection( race_manager->getDifficulty(), GUI_PLAYER_ID );
    
    SpinnerWidget* kartamount = getWidget<SpinnerWidget>("aikartamount");
    kartamount->setValue( race_manager->getNumKarts() - race_manager->getNumPlayers() );
    
    DynamicRibbonWidget* w2 = getWidget<DynamicRibbonWidget>("gamemode");
    assert( w2 != NULL );
    w2->clearItems();

    // ---- Add game modes
    irr::core::stringw name1 = irr::core::stringw(RaceManager::getNameOf(RaceManager::MINOR_MODE_NORMAL_RACE)) +
                                L"\n";
    name1 +=  _("All blows allowed, so catch weapons and make clever use of them!");
    
    w2->addItem( name1, "normal", "/gui/mode_normal.png");
    
    irr::core::stringw name2 = irr::core::stringw(RaceManager::getNameOf(RaceManager::MINOR_MODE_TIME_TRIAL)) +
                                L"\n";
    name2 += _("Contains no powerups, so only your driving skills matter!");
    w2->addItem( name2, "timetrial", "/gui/mode_tt.png");
    
    if (unlock_manager->isLocked("followtheleader"))
    {
        w2->addItem( _("Locked : solve active challenges to gain access to more!"),
                    "locked", "/gui/mode_ftl.png", true);
    }
    else
    {
        irr::core::stringw name3 = irr::core::stringw(RaceManager::getNameOf(RaceManager::MINOR_MODE_FOLLOW_LEADER)) +
                        L"\n";
        name3 += _("Run for second place, as the last kart will be disqualified every time the counter hits zero. Beware : going in front of the leader will get you eliminated too!");
        w2->addItem(name3, "ftl", "/gui/mode_ftl.png", false);
    }
    
    if (race_manager->getNumPlayers() > 1)
    {
        irr::core::stringw name4 = irr::core::stringw(RaceManager::getNameOf(RaceManager::MINOR_MODE_3_STRIKES)) +
                        L"\n";
        name4 += _("Hit others with weapons until they lose all their lives. (Only in multiplayer games)");
        w2->addItem( name4, "3strikes", "/gui/mode_3strikes.png");
    }
    

    w2->updateItemDisplay();    
}

void RaceSetupScreen::tearDown()
{
}
