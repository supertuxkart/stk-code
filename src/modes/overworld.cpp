//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 SuperTuxKart-Team
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

#include "challenges/unlock_manager.hpp"
#include "input/device_manager.hpp"
#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "karts/kart.hpp"
#include "modes/overworld.hpp"
#include "network/network_manager.hpp"
#include "states_screens/race_gui_overworld.hpp"
#include "tracks/track.hpp"

//-----------------------------------------------------------------------------
OverWorld::OverWorld() : LinearWorld()
{
}

// ----------------------------------------------------------------------------
/** Actually initialises the world, i.e. creates all data structures to
 *  for all karts etc. In init functions can be called that use
 *  World::getWorld().
 */
void OverWorld::init()
{
    LinearWorld::init();
}   // init

//-----------------------------------------------------------------------------
OverWorld::~OverWorld()
{
}   // ~OverWorld

//-----------------------------------------------------------------------------
/** General update function called once per frame.
 *  \param dt Time step size.
 */
void OverWorld::update(float dt)
{
    LinearWorld::update(dt);
    
    const unsigned int kart_amount  = m_karts.size();

    // isn't cool, on the overworld nitro is free!
    for(unsigned int n=0; n<kart_amount; n++)
    {
        m_karts[n]->setEnergy(100.0f);
    }
}   // update

//-----------------------------------------------------------------------------
/** Override the base class method to change behavior. We don't want wrong
 *  direction messages in the overworld since there is no direction there.
 *  \param i Kart id.
 */
void OverWorld::checkForWrongDirection(unsigned int i)
{
}   // checkForWrongDirection

//-----------------------------------------------------------------------------

void OverWorld::createRaceGUI()
{
    m_race_gui = new RaceGUIOverworld();
}

//-----------------------------------------------------------------------------

void OverWorld::onFirePressed(Controller* who)
{
    const std::vector<OverworldChallenge>& challenges = m_track->getChallengeList();

    Vec3 kart_xyz = getKart(0)->getXYZ();
    for (unsigned int n=0; n<challenges.size(); n++)
    {
        if ((kart_xyz - Vec3(challenges[n].m_position)).length2_2d() < 20)
        {
            core::rect<s32> pos(15,
                                10, 
                                15 + UserConfigParams::m_width/2,
                                10 + GUIEngine::getTitleFontHeight());
            
            const ChallengeData* challenge = unlock_manager->getChallenge(challenges[n].m_challenge_id);
            
            if (challenge == NULL)
            {
                fprintf(stderr, "[RaceGUIOverworld] ERROR: Cannot find challenge <%s>\n",
                        challenges[n].m_challenge_id.c_str());
                break;
            }
            
            race_manager->exitRace();
            //StateManager::get()->resetActivePlayers();
            
            // Use latest used device
            InputDevice* device = input_manager->getDeviceList()->getLatestUsedDevice();
            assert(device != NULL);
            
            // Set up race manager appropriately
            race_manager->setNumLocalPlayers(1);
            race_manager->setLocalKartInfo(0, UserConfigParams::m_default_kart);
            
            //int id = StateManager::get()->createActivePlayer( unlock_manager->getCurrentPlayer(), device );
            input_manager->getDeviceList()->setSinglePlayer( StateManager::get()->getActivePlayer(0) );
            
            // ASSIGN should make sure that only input from assigned devices is read.
            input_manager->getDeviceList()->setAssignMode(ASSIGN);
            
            // Go straight to the race
            StateManager::get()->enterGameState();                
            
            // Initialise global data - necessary even in local games to avoid
            // many if tests in other places (e.g. if network_game call 
            // network_manager else call race_manager).
            network_manager->initCharacterDataStructures();
            
            // Launch challenge
            challenge->setRace();
            
            // Sets up kart info, including random list of kart for AI
            network_manager->setupPlayerKartInfo();
            race_manager->startNew();
            return;
        } // end if
    } // end for
}

