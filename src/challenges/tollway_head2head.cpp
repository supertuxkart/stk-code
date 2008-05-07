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
#include "tollway_head2head.hpp"
#include "world.hpp"
#include "race_manager.hpp"

TollwayHead2Head::TollwayHead2Head() : Challenge("tollwayhead", _("Win a Head to Head on\nTux Tollway"))
{
    setChallengeDescription(_("Win a 1 lap Head to Head\non Tux Tollway against 1 'Driver'\nlevel AI kart."));
    setFeatureDescription(_("New track: Fort Magma\nnow available"));
    setFeature("fortmagma");
}   // TollwayTime

//-----------------------------------------------------------------------------
void TollwayHead2Head::setRace() const {
    race_manager->setRaceMode(RaceManager::RM_QUICK_RACE);
    race_manager->setTrack("tuxtrack");
    race_manager->setDifficulty(RaceManager::RD_HARD);
    race_manager->setNumLaps(1);
    race_manager->setNumKarts(2);
    race_manager->setNumPlayers(1);
}   // setRace

//-----------------------------------------------------------------------------
bool TollwayHead2Head::raceFinished()
{
    std::string track_name = world->getTrack()->getIdent();
    if(track_name!="tuxtrack"      ) return false;    // wrong track
    Kart* kart=world->getPlayerKart(0);
    if(kart->getLap()!=1       ) return false;    // wrong number of laps
    if(race_manager->getNumKarts()!=2 ) return false; //wrong number of AI karts
    return true;
}   // raceFinished
