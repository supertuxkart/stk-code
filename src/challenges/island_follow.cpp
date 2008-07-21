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
#include "challenges/island_follow.hpp"
#include "world.hpp"
#include "race_manager.hpp"

IslandFollow::IslandFollow() : Challenge("islandfollow", _("Follow the Leader on a\nDesert Island"))
{
    setChallengeDescription(_("Win a Follow the Leader race\nwith 3 AI karts\non a Desert Island."));
    addUnlockGPReward("At world's end");
    addDependency("moonandbackgp");
    addDependency("tollwaytime");
    addDependency("citytime");
}   // IslandFollow

//-----------------------------------------------------------------------------
void IslandFollow::setRace() const {
    race_manager->setMajorMode(RaceManager::RM_SINGLE);
    race_manager->setMinorMode(RaceManager::RM_FOLLOW_LEADER);
    race_manager->setTrack("islandtrack");
    race_manager->setDifficulty(RaceManager::RD_EASY);
    race_manager->setNumLaps(3);
    race_manager->setNumKarts(4);
    race_manager->setNumPlayers(1);
}   // setRace

//-----------------------------------------------------------------------------
bool IslandFollow::raceFinished()
{
    std::string track_name = world->getTrack()->getIdent();
    if(track_name!="islandtrack"      ) return false;    // wrong track
    if(race_manager->getNumKarts()<4) return false; //not enough AI karts
    //Check if player came first
    for(int i=0; i<(int)race_manager->getNumKarts(); i++)
    {
        const Kart* k=world->getKart(i);
        if(k->isPlayerKart()) return  k->getPosition()==2;
    }
    return false;

}   // raceFinished
//-----------------------------------------------------------------------------
