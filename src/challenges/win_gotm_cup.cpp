//  $Id: win_gotm_cup.cpp 1259 2007-09-24 12:28:19Z hiker $
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

#include <algorithm>
#include "translation.hpp"
#include "challenges/win_gotm_cup.hpp"
#include "world.hpp"
#include "race_manager.hpp"
#include "cup_data.hpp"

WinGOTMCup::WinGOTMCup() : Challenge("wingotmcup", _("Win GOTM Cup"))
{
    setChallengeDescription(_("Win the GOTM Cup\non level 'racer'\nwith at least\nthree computer opponents."));
    setFeatureDescription(_("New track\n'Amazonian Journey'\navailable"));
    // The energymathclass challenge must be done, otherwise GP can't be selected
    addDependency("energymathclass");
    setFeature("jungle");
}   // WinGOTMCup

//-----------------------------------------------------------------------------
void WinGOTMCup::setRace() const {
    race_manager->setRaceMode(RaceManager::RM_GRAND_PRIX);
    CupData cup("gotm.cup");
    race_manager->setGrandPrix(cup);
    race_manager->setDifficulty(RaceManager::RD_HARD);
    race_manager->setNumKarts(4);
    race_manager->setNumPlayers(1);
}   // setRace

//-----------------------------------------------------------------------------
bool WinGOTMCup::grandPrixFinished()
{
    if (race_manager->getRaceMode()  != RaceManager::RM_GRAND_PRIX  ||
        race_manager->getDifficulty()!= RaceManager::RD_HARD        ||
        race_manager->getNumKarts()   < 4                           ||
        race_manager->getNumPlayers() > 1) return false;
    // Check if the player was number one:
    for(int i=0; i<(int)race_manager->getNumKarts(); i++)
    {
        const Kart* k=world->getKart(i);
        if(k->isPlayerKart() && !k->isEliminated()) return  k->getPosition()==1;
    }
    return false;
}   // raceFinished
//-----------------------------------------------------------------------------
