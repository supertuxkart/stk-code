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
#include "states_screens/dialogs/select_challenge.hpp"
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
        if (challenges[n].m_force_field.m_is_locked) continue;
        
        if ((kart_xyz - Vec3(challenges[n].m_position)).length2_2d() < CHALLENGE_DISTANCE_SQUARED)
        {
            new SelectChallengeDialog(0.8f, 0.8f, challenges[n].m_challenge_id);
        } // end if
    } // end for
}

