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

#include "modes/standard_race.hpp"
#include "user_config.hpp"
#include "unlock_manager.hpp"
#include "gui/menu_manager.hpp"

//-----------------------------------------------------------------------------
StandardRace::StandardRace() : LinearWorld()
{
    TimedRace::setClockMode(CHRONO);
}

//-----------------------------------------------------------------------------
StandardRace::~StandardRace()
{
}
    
#if 0
#pragma mark -
#pragma mark clock events
#endif

//-----------------------------------------------------------------------------
void StandardRace::onGo()
{
    // Reset the brakes now that the prestart 
    // phase is over (braking prevents the karts 
    // from sliding downhill)
    for(unsigned int i=0; i<m_kart.size(); i++) 
    {
        m_kart[i]->resetBrakes();
    }
}
//-----------------------------------------------------------------------------
void StandardRace::terminateRace()
{
    LinearWorld::terminateRace();
}

#if 0
#pragma mark -
#pragma mark overridden from World
#endif

//-----------------------------------------------------------------------------
void StandardRace::restartRace()
{
    LinearWorld::restartRace();
}
//-----------------------------------------------------------------------------
void StandardRace::update(float delta)
{    
    LinearWorld::update(delta);
    if(!TimedRace::isRacePhase()) return;
    
    // All karts are finished
    if(race_manager->getFinishedKarts() >= race_manager->getNumKarts() )
    {
        TimedRace::enterRaceOverState();
	    if(user_config->m_profile<0) printProfileResultAndExit();
        unlock_manager->raceFinished();
    }   // if all karts are finished
    
    // All player karts are finished, but computer still racing
    // ===========================================================
    else if(race_manager->allPlayerFinished())
    {
        // Set delay mode to have time for camera animation, and
        // to give the AI some time to get non-estimated timings
        TimedRace::enterRaceOverState(true /* delay */);
    }
}

//-----------------------------------------------------------------------------
void StandardRace::getDefaultCollectibles(int& collectible_type, int& amount)
{
    // in time trial mode, give zippers
    if(race_manager->getMinorMode() == RaceManager::MINOR_MODE_TIME_TRIAL)
    {
        collectible_type = POWERUP_ZIPPER;
        amount = race_manager->getNumLaps();
    }
    else World::getDefaultCollectibles(collectible_type, amount);
}
//-----------------------------------------------------------------------------
bool StandardRace::enableBonusBoxes()
{
    // in time trial mode, don't use bonus boxes
    return race_manager->getMinorMode() != RaceManager::MINOR_MODE_TIME_TRIAL;
}
//-----------------------------------------------------------------------------
std::string StandardRace::getInternalCode() const
{
    if(race_manager->getMinorMode() == RaceManager::MINOR_MODE_TIME_TRIAL)
        return "STD_TIMETRIAL";
    else
        return "STANDARD";
}
