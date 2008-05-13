//  $Id: energy_math_class.cpp 1259 2007-09-24 12:28:19Z hiker $
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
#include "challenges/energy_math_class.hpp"
#include "race_manager.hpp"
#include "world.hpp"

EnergyMathClass::EnergyMathClass() : Challenge("energymathclass", _("Collect Coins in Math Class"))
{
    setChallengeDescription(_("Collect at least 6 coins\non three laps of\nOliver's Math Class\nin under 1 minute."));
    addUnlockModeReward("grandprix", _("Grand Prix"));
}   // EnergyMathClass

//-----------------------------------------------------------------------------
void EnergyMathClass::setRace() const {
    race_manager->setRaceMode(RaceManager::RM_QUICK_RACE);
    race_manager->setTrack("olivermath");
    race_manager->setDifficulty(RaceManager::RD_EASY);
    race_manager->setNumLaps(3);
    race_manager->setNumKarts(1);
    race_manager->setNumPlayers(1);
    race_manager->setCoinTarget(6);
}   // setRace
\
//-----------------------------------------------------------------------------
bool EnergyMathClass::raceFinished()
{
    std::string track_name = world->getTrack()->getIdent();
    if(track_name!="olivermath") return false;    // wrong track
    Kart* kart=world->getPlayerKart(0);
    if(kart->getFinishTime()>60) return false;    // too slow
    if(kart->getLap()!=3       ) return false;    // wrong number of laps
    if(kart->getNumHerring()<6 ) return false;    // not enough herrings
    return true;
}   // raceFinished
//-----------------------------------------------------------------------------
