//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#include "modes/game_tutorial.hpp"

//#include "challenges/unlock_manager.hpp"
//#include "config/user_config.hpp"

//-----------------------------------------------------------------------------
GameTutorial::GameTutorial() : LinearWorld()
{
    WorldStatus::setClockMode(CLOCK_CHRONO);
}   // GameTutorial

//-----------------------------------------------------------------------------
/** Returns true if the race is finished, i.e. all player karts are finished.
 */
bool GameTutorial::isRaceOver()
{
    // The race is over if all players have finished the race. Remaining 
    // times for AI opponents will be estimated in enterRaceOverState
    return race_manager->allPlayerFinished();
}   // isRaceOver
/*
//-----------------------------------------------------------------------------
void GameTutorial::getDefaultCollectibles(int& collectible_type, int& amount)
{
    // in time trial mode, give zippers
    if(race_manager->getMinorMode() == RaceManager::MINOR_MODE_TIME_TRIAL)
    {
        collectible_type = PowerupManager::POWERUP_ZIPPER;
        amount = race_manager->getNumLaps();
    }
    else World::getDefaultCollectibles(collectible_type, amount);
}   // getDefaultCollectibles
*/
//-----------------------------------------------------------------------------
/** Returns if this mode supports bonus boxes or not.
 */
/*
bool GameTutorial::haveBonusBoxes()
{
    // in time trial mode, don't use bonus boxes
    return race_manager->getMinorMode() != RaceManager::MINOR_MODE_TIME_TRIAL;
}   // haveBonusBoxes
*/
//-----------------------------------------------------------------------------
/** Returns an identifier for this race. 
 */
/*
std::string GameTutorial::getIdent() const
{
    if(race_manager->getMinorMode() == RaceManager::MINOR_MODE_TIME_TRIAL)
        return IDENT_TTRIAL;
    else
        return IDENT_STD;    
}   // getIdent
*/
