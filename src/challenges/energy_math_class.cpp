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
#include "challenges/energy_math_class.hpp"
#include "world.hpp"

EnergyMathClass::EnergyMathClass() : Challenge("energymathclass", "Collect Coins in Race track")
{
    setChallengeDescription("Collect at least 6 coins\non three laps of\nOliver's Math Class\nin under 1 minute.");
    setFeatureDescription("New game mode\n'Grand Prix'\nnow available");
    setFeature("grandprix");
}   // EnergyMathClass

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
