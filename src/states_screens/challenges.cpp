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


#include "states_screens/challenges.hpp"


#include "challenges/unlock_manager.hpp"
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

#include <iostream>
#include <fstream>

#include "irrString.h"
using irr::core::stringw;
using irr::core::stringc;

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( ChallengesScreen );

// ------------------------------------------------------------------------------------------------------

ChallengesScreen::ChallengesScreen() : Screen("challenges.stkgui")
{
           
}

// ------------------------------------------------------------------------------------------------------

void ChallengesScreen::loadedFromFile()
{
}

// ------------------------------------------------------------------------------------------------------

void ChallengesScreen::onUpdate(float elapsed_time, irr::video::IVideoDriver*)
{
}

// ------------------------------------------------------------------------------------------------------

void ChallengesScreen::beforeAddingWidget()
{
    DynamicRibbonWidget* w = this->getWidget<DynamicRibbonWidget>("challenges");
    assert( w != NULL );
    
    GameSlot* slot = unlock_manager->getCurrentSlot();
    
    const std::vector<const ChallengeData*>& activeChallenges = slot->getActiveChallenges();
    const std::vector<const ChallengeData*>& solvedChallenges = slot->getUnlockedFeatures();
    const std::vector<const ChallengeData*>& lockedChallenges = slot->getLockedChallenges();
    
    const int activeChallengeAmount = activeChallenges.size();
    const int solvedChallengeAmount = solvedChallenges.size();
    const int lockedChallengeAmount = lockedChallenges.size();
    
    
    w->setItemCountHint(activeChallengeAmount + solvedChallengeAmount + lockedChallengeAmount);
}

// ------------------------------------------------------------------------------------------------------

void ChallengesScreen::init()
{
    Screen::init();
    DynamicRibbonWidget* w = this->getWidget<DynamicRibbonWidget>("challenges");
    assert( w != NULL );
    
    // Re-build track list everytime (accounts for locking changes, etc.)
    w->clearItems();
    
    GameSlot* slot = unlock_manager->getCurrentSlot();

    const std::vector<const ChallengeData*>& activeChallenges = slot->getActiveChallenges();
    const std::vector<const ChallengeData*>& solvedChallenges = slot->getUnlockedFeatures();
    const std::vector<const ChallengeData*>& lockedChallenges = slot->getLockedChallenges();
    
    const int activeChallengeAmount = activeChallenges.size();
    const int solvedChallengeAmount = solvedChallenges.size();
    const int lockedChallengeAmount = lockedChallenges.size();
    
    for (int n=0; n<activeChallengeAmount; n++)
    {
        w->addItem(activeChallenges[n]->getName() + L"\n" + activeChallenges[n]->getChallengeDescription(),
                   activeChallenges[n]->getId(), "/gui/challenge.png");
    }
    for (int n=0; n<solvedChallengeAmount; n++)
    {
        // TODO : add bronze/silver/gold difficulties to challenges
        w->addItem(solvedChallenges[n]->getName() + L"\n" + solvedChallenges[n]->getChallengeDescription(),
                   solvedChallenges[n]->getId(), "/textures/cup_gold.png");
    }
    for (int n=0; n<lockedChallengeAmount; n++)
    {
        w->addItem( _("Locked : solve active challenges to gain access to more!"), "locked",
                   "/gui/challenge.png", LOCKED_BADGE);
    }
    
    
    w->updateItemDisplay();
    
    if (w->getItems().empty())
    {
        fprintf(stderr, "Error, no challenge!\n");
        return;
    }
    w->setSelection(0 /* whatever is first */, PLAYER_ID_GAME_MASTER, true /* focus it */);
}

// ------------------------------------------------------------------------------------------------------

void ChallengesScreen::eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "challenges")
    {
        DynamicRibbonWidget* w = this->getWidget<DynamicRibbonWidget>("challenges");
        assert( w != NULL );
        
        // only player 0 can start a challenge (i.e. we have no multiplayer challenges)
        std::string selection = w->getSelectionIDString( PLAYER_ID_GAME_MASTER );
        
        if (selection == "locked")
        {
            unlock_manager->playLockSound();
        }
        else if (!selection.empty() && selection != RibbonWidget::NO_ITEM_ID)
        {
            //FIXME: simplify and centralize race start sequence!!

            // Verify the kart in the config exists
            if (kart_properties_manager->getKart(UserConfigParams::m_default_kart) == NULL)
            {
                UserConfigParams::m_default_kart.revertToDefaults();
            }
            
            const ChallengeData* c = unlock_manager->getChallenge(selection);
            if (c == NULL)
            {
                std::cerr << "[ChallengesScreen] ERROR: cannot find challenge '" << selection.c_str() << "'!\n";
                return;
            }
            
            // Use latest used device
            InputDevice* device = input_manager->getDeviceList()->getLatestUsedDevice();
            
            int id = StateManager::get()->createActivePlayer( unlock_manager->getCurrentPlayer(), device );
            input_manager->getDeviceList()->setSinglePlayer( StateManager::get()->getActivePlayer(id) );

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
            c->setRace();

            // Sets up kart info, including random list of kart for AI
            network_manager->setupPlayerKartInfo();
            race_manager->startNew();
        }
    }
}

// ------------------------------------------------------------------------------------------------------

