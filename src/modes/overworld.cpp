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
#include "karts/kart_properties_manager.hpp"
#include "modes/overworld.hpp"
#include "network/network_manager.hpp"
#include "states_screens/dialogs/select_challenge.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/race_gui_overworld.hpp"
#include "tracks/track.hpp"

//-----------------------------------------------------------------------------
/** Function to simplify the start process */
void OverWorld::enterOverWorld()
{
    
    race_manager->setNumLocalPlayers(1);
    race_manager->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
    race_manager->setMinorMode (RaceManager::MINOR_MODE_OVERWORLD);
    race_manager->setNumKarts( 1 );
    race_manager->setTrack( "overworld" );
    race_manager->setDifficulty(RaceManager::RD_HARD);
    
    // Use keyboard 0 by default (FIXME: let player choose?)
    InputDevice* device = input_manager->getDeviceList()->getKeyboard(0);
    
    // Create player and associate player with keyboard
    StateManager::get()->createActivePlayer( 
                                            UserConfigParams::m_all_players.get(0), device );
    
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
    network_manager->setupPlayerKartInfo();
    race_manager->startNew();
    
}

//-----------------------------------------------------------------------------
OverWorld::OverWorld() : LinearWorld()
{
    m_return_to_garage = false;
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
    
    if (m_return_to_garage)
    {
        m_return_to_garage = false;
        race_manager->exitRace();
        KartSelectionScreen* s = KartSelectionScreen::getInstance();
        s->setMultiplayer(false);
        s->setFromOverworld(true);
        StateManager::get()->resetAndGoToScreen(s);
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
        if (challenges[n].m_force_field.m_is_locked) continue;
        
        if ((kart_xyz - Vec3(challenges[n].m_position)).length2_2d() < CHALLENGE_DISTANCE_SQUARED)
        {
            new SelectChallengeDialog(0.8f, 0.8f, challenges[n].m_challenge_id);
        } // end if
    } // end for
}

