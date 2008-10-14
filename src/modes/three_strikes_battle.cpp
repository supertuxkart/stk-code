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

#include "modes/three_strikes_battle.hpp"
#include "gui/race_gui.hpp"

#include <string>

ThreeStrikesBattle::ThreeStrikesBattle() : World()
{
    TimedRace::setClockMode(CHRONO);
    
    // FIXME - disable AI karts in the GUI
    if(race_manager->getNumKarts() > race_manager->getNumPlayers())
    {
        fprintf(stderr, "No AI exists for this game mode");
        exit(1);
    }
 
    const unsigned int kart_amount = m_kart.size();
    m_kart_display_info = new KartIconDisplayInfo[kart_amount];
    
    for(unsigned int n=0; n<kart_amount; n++)
    {
        // create the struct that ill hold each player's lives
        BattleInfo info;
        info.m_lives         = 3;
        m_kart_info.push_back(info);
        
        // no positions in this mode
        m_kart[n]->setPosition(-1);
    }// next kart
    
    // TODO - implement
}
ThreeStrikesBattle::~ThreeStrikesBattle()
{
}


void ThreeStrikesBattle::onGo()
{
    // Reset the brakes now that the prestart 
    // phase is over (braking prevents the karts 
    // from sliding downhill)
    for(unsigned int i=0; i<m_kart.size(); i++) 
    {
        m_kart[i]->resetBrakes();
    }
}
void ThreeStrikesBattle::terminateRace()
{
    // TODO - implement
}

std::string ThreeStrikesBattle::getInternalCode() const
{
    return "BATTLE_3_STRIKES";
}
void ThreeStrikesBattle::update(float delta)
{
    World::update(delta);
}
void ThreeStrikesBattle::restartRace()
{
}
//void ThreeStrikesBattle::getDefaultCollectibles(int& collectible_type, int& amount)

KartIconDisplayInfo* ThreeStrikesBattle::getKartsDisplayInfo(const RaceGUI* caller)
{
    const unsigned int kart_amount = race_manager->getNumKarts();
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        KartIconDisplayInfo& rank_info = m_kart_display_info[i];
        
        // reset color
        rank_info.lap = -1;
        
        switch(m_kart_info[i].m_lives)
        {
            case 3:
                rank_info.r = 0.0;
                rank_info.g = 1.0;
                rank_info.b = 0.0;
                break;
            case 2:
                rank_info.r = 1.0;
                rank_info.g = 0.9;
                rank_info.b = 0.0;
                break;
            case 1:
                rank_info.r = 1.0;
                rank_info.g = 0.0;
                rank_info.b = 0.0;
                break;
            case 0:
                rank_info.r = 0.5;
                rank_info.g = 0.5;
                rank_info.b = 0.5;
                break;
        }
        
        char lives[4];
        sprintf(lives, "%i", m_kart_info[i].m_lives);
        
        rank_info.time = lives; // FIXME - rename 'time' to something more generic
    }
    
    return m_kart_display_info;
}

void ThreeStrikesBattle::moveKartAfterRescue(Kart* kart, btRigidBody* body)
{
    // find closest point to drop kart on
    const int start_spots_amount = RaceManager::getTrack()->m_start_positions.size();
    assert(start_spots_amount > 0);
    
    int smallest_distance_found = -1, closest_id_found = -1;
    
    const int kart_x = kart->getXYZ()[0];
    const int kart_y = kart->getXYZ()[1];
    
    for(int n=0; n<start_spots_amount; n++)
    {
        // no need for the overhead to compute exact distance with sqrt(), so using the
        // 'manhattan' heuristic which will do fine enough.
        const int dist_n = abs(kart_x - RaceManager::getTrack()->m_start_positions[n][0]) +
                           abs(kart_y - RaceManager::getTrack()->m_start_positions[n][1]);
        if(dist_n < smallest_distance_found || closest_id_found == -1)
        {
            closest_id_found = n;
            smallest_distance_found = dist_n;
        }
    }
    
    assert(closest_id_found != -1);
    
    kart->setXYZ( Vec3(RaceManager::getTrack()->m_start_positions[closest_id_found]) );
    
    // FIXME - implement heading
   // btQuaternion heading(btVector3(0.0f, 0.0f, 1.0f), 
   //                      DEGREE_TO_RAD(RaceManager::getTrack()->m_angle[info.m_track_sector]) );
   // kart->setRotation(heading);
    
    // A certain epsilon is added here to the Z coordinate (0.1), in case
    // that the points are somewhat under the track. Otherwise, the
    // kart will be placed a little bit under the track, triggering
    // a rescue, ...
    btTransform pos;
    pos.setOrigin(kart->getXYZ()+btVector3(0, 0, 0.5f*kart->getKartHeight()+0.1f));
    //pos.setRotation(btQuaternion(btVector3(0.0f, 0.0f, 1.0f),
    //                             DEGREE_TO_RAD(RaceManager::getTrack()->m_angle[info.m_track_sector])));
    
    body->setCenterOfMassTransform(pos);
    
}