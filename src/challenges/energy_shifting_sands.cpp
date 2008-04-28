//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be ruseful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "challenges/energy_shifting_sands.hpp"
#include "race_manager.hpp"
#include "world.hpp"

EnergyShiftingSands::EnergyShiftingSands() : Challenge("energyshiftingsands","Collect the Pharaohs Treasure")
{
    setChallengeDescription("Collect at least 9 coins\non 3 laps of Shifting Sands\nin under 2:20 minutes.");
    setFeatureDescription("New Grand Prix: To the Moon and Back\nnow available");
    setFeature("To the Moon and Back");
    // The energymathclass challenge must be done, otherwise GP can't be selected
    addDependency("energymathclass");
    addDependency("racetracktime");
}

//-----------------------------------------------------------------------------
void EnergyShiftingSands::setRace() const {
    race_manager->setRaceMode(RaceManager::RM_QUICK_RACE);
    race_manager->setTrack("sandtrack");
    race_manager->setDifficulty(RaceManager::RD_EASY);
    race_manager->setNumLaps(3);
    race_manager->setNumKarts(1);
    race_manager->setNumPlayers(1);
}   // setRace

//-----------------------------------------------------------------------------
bool EnergyShiftingSands::raceFinished()
{
    std::string track_name = world->getTrack()->getIdent();
    if(track_name!="sandtrack") return false;    // wrong track
    Kart* kart=world->getPlayerKart(0);
    if(kart->getFinishTime()>140) return false;    // too slow
    if(kart->getLap()!=3       ) return false;    // wrong number of laps
    if(kart->getNumHerring()<9 ) return false;    // not enough herrings
    return true;
    
}   // raceFinished
//-----------------------------------------------------------------------------
