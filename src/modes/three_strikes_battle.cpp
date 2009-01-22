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

#include <string>

#include "gui/race_gui.hpp"
#include "audio/sound_manager.hpp"
#include "tracks/track.hpp"

//-----------------------------------------------------------------------------
ThreeStrikesBattle::ThreeStrikesBattle() : World()
{
    TimedRace::setClockMode(CHRONO);
    m_use_highscores = false;
    
    World::init();
    
    // check for possible problems if AI karts were incorrectly added
    if(race_manager->getNumKarts() > race_manager->getNumPlayers())
    {
        fprintf(stderr, "No AI exists for this game mode\n");
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
}
//-----------------------------------------------------------------------------
ThreeStrikesBattle::~ThreeStrikesBattle()
{
    delete[] m_kart_display_info;
}
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
void ThreeStrikesBattle::terminateRace()
{
    updateKartRanks();
    
    // if some karts have not yet finished yet
    const unsigned int kart_amount = m_kart.size();
    for ( Karts::size_type i = 0; i < kart_amount; ++i)
    {
        if(!m_kart[i]->hasFinishedRace())
        {
            m_kart[i]->raceFinished(TimedRace::getTime());
        }  // if !hasFinishedRace
    }   // for i
    
    World::terminateRace();
}
//-----------------------------------------------------------------------------
void ThreeStrikesBattle::kartHit(const int kart_id)
{
    assert(kart_id >= 0);
    assert(kart_id < (int)m_kart.size());
    
    // make kart lose a life
    m_kart_info[kart_id].m_lives--;
    
    updateKartRanks();
        
    // check if kart is 'dead'
    if(m_kart_info[kart_id].m_lives < 1)
    {
        m_kart[kart_id]->raceFinished(TimedRace::getTime());
        removeKart(kart_id);
    }
    
    // almost over, use fast music
    if(getCurrentNumKarts()==2 && m_faster_music_active==false)  
    {
        sound_manager->switchToFastMusic();
        m_faster_music_active = true;
    }
}
//-----------------------------------------------------------------------------
std::string ThreeStrikesBattle::getInternalCode() const
{
    return "BATTLE_3_STRIKES";
}
//-----------------------------------------------------------------------------
void ThreeStrikesBattle::updateKartRanks()
{
    // sort karts by their times then give each one its position.
    // in battle-mode, long time = good (meaning he survived longer)
    
    const unsigned int NUM_KARTS = race_manager->getNumKarts();
    
    int *karts_list = new int[NUM_KARTS];
    for( unsigned int n = 0; n < NUM_KARTS; ++n ) karts_list[n] = n;
    
    bool sorted=false;
    do
    {
        sorted = true;
        for( unsigned int n = 0; n < NUM_KARTS-1; ++n )
        {
            const int this_karts_time = m_kart[karts_list[n]]->hasFinishedRace() ?
                (int)m_kart[karts_list[n]]->getFinishTime() : (int)TimedRace::getTime();
            const int next_karts_time = m_kart[karts_list[n+1]]->hasFinishedRace() ?
                (int)m_kart[karts_list[n+1]]->getFinishTime() : (int)TimedRace::getTime();

            bool swap = false;
            
            // if next kart survived longer...
            if( next_karts_time > this_karts_time) swap = true;
            // if next kart has more lives...
            else if(m_kart_info[karts_list[n+1]].m_lives > m_kart_info[karts_list[n]].m_lives) swap = true;

            if(swap)
            {
                int tmp = karts_list[n+1];
                karts_list[n+1] = karts_list[n];
                karts_list[n] = tmp;
                sorted = false;
                break;
            }
        }
    } while(!sorted);
    
    for( unsigned int n = 0; n < NUM_KARTS; ++n )
    {
        m_kart[ karts_list[n] ]->setPosition( n+1 );
    }
    delete [] karts_list;
}
//-----------------------------------------------------------------------------
void ThreeStrikesBattle::update(float delta)
{
    World::update(delta);

    // check if over
    if(getCurrentNumKarts()==1 || getCurrentNumPlayers()==0)
    {
        // Add the results for the remaining kart
        for(int i=0; i<(int)race_manager->getNumKarts(); i++)
            if(!m_kart[i]->isEliminated()) 
                race_manager->RaceFinished(m_kart[i], TimedRace::getTime());
    
        if(!RaceManager::getWorld()->isFinishPhase())
            TimedRace::enterRaceOverState();
        return;
    }
}
//-----------------------------------------------------------------------------
void ThreeStrikesBattle::restartRace()
{
    World::restartRace();
    
    const unsigned int kart_amount = m_kart.size();
    
    for(unsigned int n=0; n<kart_amount; n++)
    {
        m_kart_info[n].m_lives         = 3;
        
        // no positions in this mode
        m_kart[n]->setPosition(-1);
    }// next kart
}
//void ThreeStrikesBattle::getDefaultCollectibles(int& collectible_type, int& amount)
//-----------------------------------------------------------------------------
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
                rank_info.g = 0.9f;
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
//-----------------------------------------------------------------------------
void ThreeStrikesBattle::moveKartAfterRescue(Kart* kart, btRigidBody* body)
{
    // find closest point to drop kart on
    const int start_spots_amount = RaceManager::getTrack()->m_start_positions.size();
    assert(start_spots_amount > 0);
    
    int smallest_distance_found = -1, closest_id_found = -1;
    
    const int kart_x = (int)(kart->getXYZ()[0]);
    const int kart_y = (int)(kart->getXYZ()[1]);
    
    for(int n=0; n<start_spots_amount; n++)
    {
        // no need for the overhead to compute exact distance with sqrt(), so using the
        // 'manhattan' heuristic which will do fine enough.
        const int dist_n = abs((int)(kart_x - RaceManager::getTrack()->m_start_positions[n][0])) +
                           abs((int)(kart_y - RaceManager::getTrack()->m_start_positions[n][1]));
        if(dist_n < smallest_distance_found || closest_id_found == -1)
        {
            closest_id_found = n;
            smallest_distance_found = dist_n;
        }
    }
    
    assert(closest_id_found != -1);
    
    kart->setXYZ( Vec3(RaceManager::getTrack()->m_start_positions[closest_id_found]) );
    
    // FIXME - implement correct heading
    btQuaternion heading(btVector3(0.0f, 0.0f, 1.0f), 0 /* angle */ );
    kart->setRotation(heading);
    
    // A certain epsilon is added here to the Z coordinate (0.1), in case
    // that the points are somewhat under the track. Otherwise, the
    // kart will be placed a little bit under the track, triggering
    // a rescue, ...
    btTransform pos;
    pos.setOrigin(kart->getXYZ()+btVector3(0, 0, 0.5f*kart->getKartHeight()+0.1f));
    pos.setRotation( btQuaternion(btVector3(0.0f, 0.0f, 1.0f), 0 /* angle */) );
    
    body->setCenterOfMassTransform(pos);
    
}
//-----------------------------------------------------------------------------
void ThreeStrikesBattle::raceResultOrder( int* order )
{
    updateKartRanks();
    
    const unsigned int num_karts = race_manager->getNumKarts();
    for( unsigned int kart_id = 0; kart_id < num_karts; ++kart_id )
    {
        const int pos = m_kart[kart_id]->getPosition() - 1;
        assert(pos >= 0);
        assert(pos < (int)num_karts);
        order[pos] = kart_id;
    }
}
//-----------------------------------------------------------------------------
bool ThreeStrikesBattle::acceptPowerup(const int type) const
{
    // these powerups don't make much sense in battle mode
    if(type == POWERUP_PARACHUTE || type == POWERUP_ANVIL || type == POWERUP_ZIPPER) return false;
    
    return true;
}

