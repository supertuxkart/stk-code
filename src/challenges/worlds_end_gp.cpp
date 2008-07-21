//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#include "translation.hpp"
#include "worlds_end_gp.hpp"
#include "race_manager.hpp"
#include "world.hpp"

WorldsEndGP::WorldsEndGP() : Challenge("worldsendgp",_("Win the At World's End\nGrand Prix"))
{
    setChallengeDescription(_("Come first in the At World's End\nGrand Prix with 3 'Racer'\nLevel AI karts."));
    
    addUnlockDifficultyReward("skidding","Skidding Preview");
    addUnlockGPReward("All tracks");
    
    addDependency("islandfollow");
    addDependency("racetracktime");
    addDependency("tollwaytime");
    addDependency("junglefollow");
    addDependency("citytime");
    addDependency("tollwayhead");
}

//-----------------------------------------------------------------------------
void WorldsEndGP::setRace() const {
    race_manager->setMajorMode(RaceManager::RM_GRAND_PRIX);
    race_manager->setMinorMode(RaceManager::RM_QUICK_RACE);
    CupData cup("gp4.cup");
    race_manager->setGrandPrix(cup);
    race_manager->setDifficulty(RaceManager::RD_HARD);
    race_manager->setNumKarts(4);
    race_manager->setNumPlayers(1);
}   // setRace

//-----------------------------------------------------------------------------
bool WorldsEndGP::grandPrixFinished()
{
    if (race_manager->getMajorMode()  != RaceManager::RM_GRAND_PRIX    ||
        race_manager->getMinorMode()  != RaceManager::RM_QUICK_RACE    ||
        race_manager->getGrandPrix()->getName() != _("At world's end") ||
        race_manager->getDifficulty()!= RaceManager::RD_HARD           ||
        race_manager->getNumKarts()   < 4                              ||
        race_manager->getNumPlayers() > 1) return false;
    // Check if the player was first:
    for(int i=0; i<(int)race_manager->getNumKarts(); i++)
    {
        const Kart* k=world->getKart(i);
        if(k->isPlayerKart() && !k->isEliminated()) return  k->getPosition()==1;
    }
    return false;
    
}   // grandPrixFinished
//-----------------------------------------------------------------------------
