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

#include "translation.hpp"
#include "challenges/penguin_playground_gp.hpp"
#include "race_manager.hpp"
#include "world.hpp"

PenguinPlaygroundGP::PenguinPlaygroundGP() : Challenge("penguinplaygroundgp", _("Win Penguin Playground Grand\nPrix"))
{
    setChallengeDescription(_("Win Penguin Playground Grand\nPrix with 3 'Racer' Level AI karts."));
    addUnlockModeReward("followleader", _("Follow the Leader"));
    // The energymathclass challenge must be done, otherwise GP can't be selected
}

//-----------------------------------------------------------------------------
void PenguinPlaygroundGP::setRace() const {
    race_manager->setRaceMode(RaceManager::RM_GRAND_PRIX);
    CupData cup("gp1.cup");
    race_manager->setGrandPrix(cup);
    race_manager->setDifficulty(RaceManager::RD_HARD);
    race_manager->setNumKarts(4);
    race_manager->setNumPlayers(1);
}   // setRace

//-----------------------------------------------------------------------------
bool PenguinPlaygroundGP::grandPrixFinished()
{
    if (race_manager->getRaceMode()  != RaceManager::RM_GRAND_PRIX  ||
        race_manager->getGrandPrix()->getName() != "Penguin Playground" ||
        race_manager->getDifficulty()!= RaceManager::RD_HARD        ||
        race_manager->getNumKarts()   < 4                           ||
        race_manager->getNumPlayers() > 1) return false;
    // Check if the player was in top 3:
    for(int i=0; i<(int)race_manager->getNumKarts(); i++)
    {
        const Kart* k=world->getKart(i);
        if(k->isPlayerKart() && !k->isEliminated()) return  k->getPosition()==1;
    }
    return false;
    
}   // grandPrixFinished
//-----------------------------------------------------------------------------
