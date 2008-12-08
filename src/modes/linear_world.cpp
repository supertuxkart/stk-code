//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 SuperTuxKart-Team
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

#include "modes/linear_world.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

#include "gui/race_gui.hpp"
 
#include "track.hpp"
#include "gui/menu_manager.hpp"
#include "translation.hpp"
#include "audio/sound_manager.hpp"
#include "network/network_manager.hpp"

//-----------------------------------------------------------------------------
LinearWorld::LinearWorld() : World()
{
}
void LinearWorld::init()
{
    World::init();
    const unsigned int kart_amount = m_kart.size();
    
    m_kart_display_info = new KartIconDisplayInfo[kart_amount];
    
    for(unsigned int n=0; n<kart_amount; n++)
    {
        KartInfo info;
        info.m_track_sector         = Track::UNKNOWN_SECTOR;
        info.m_last_valid_sector    = Track::UNKNOWN_SECTOR;
        info.m_lap_start_time       = -1.0f;
        RaceManager::getTrack()->findRoadSector(m_kart[n]->getXYZ(), &info.m_track_sector);
        
        //If m_track_sector == UNKNOWN_SECTOR, then the kart is not on top of
        //the road, so we have to use another function to find the sector.
        if (info.m_track_sector == Track::UNKNOWN_SECTOR )
        {
            info.m_on_road = false;
            info.m_track_sector =
                RaceManager::getTrack()->findOutOfRoadSector(m_kart[n]->getXYZ(),
                                                             Track::RS_DONT_KNOW,
                                                             Track::UNKNOWN_SECTOR );
        }
        else
        {
            info.m_on_road = true;
        }
        
        RaceManager::getTrack()->spatialToTrack(info.m_curr_track_coords,
                                                m_kart[n]->getXYZ(),
                                                info.m_track_sector );
        
        info.m_race_lap             = -1;
        info.m_lap_start_time       = -1.0f;
        info.m_time_at_last_lap     = 99999.9f;
        
        m_kart_info.push_back(info);
    }// next kart
}
//-----------------------------------------------------------------------------
LinearWorld::~LinearWorld()
{
    delete[] m_kart_display_info;
}
//-----------------------------------------------------------------------------
void LinearWorld::restartRace()
{
    World::restartRace();
    
    const unsigned int kart_amount = m_kart.size();
    for(unsigned int n=0; n<kart_amount; n++)
    {
        KartInfo& info = m_kart_info[n];
        info.m_track_sector         = Track::UNKNOWN_SECTOR;
        info.m_last_valid_sector    = Track::UNKNOWN_SECTOR;
        info.m_lap_start_time       = -1.0f;
        RaceManager::getTrack()->findRoadSector(m_kart[n]->getXYZ(), &info.m_track_sector);
        
        //If m_track_sector == UNKNOWN_SECTOR, then the kart is not on top of
        //the road, so we have to use another function to find the sector.
        if (info.m_track_sector == Track::UNKNOWN_SECTOR )
        {
            info.m_on_road = false;
            info.m_track_sector =
                RaceManager::getTrack()->findOutOfRoadSector(m_kart[n]->getXYZ(),
                                                             Track::RS_DONT_KNOW,
                                                             Track::UNKNOWN_SECTOR );
        }
        else
        {
            info.m_on_road = true;
        }
        
        RaceManager::getTrack()->spatialToTrack(info.m_curr_track_coords,
                                                m_kart[n]->getXYZ(),
                                                info.m_track_sector );
        
        info.m_race_lap             = -1;
        info.m_lap_start_time       = -1.0f;
        info.m_time_at_last_lap     = 99999.9f;

        updateRacePosition(m_kart[n], info);
    }   // next kart
    
}   // restartRace
//-----------------------------------------------------------------------------
void LinearWorld::update(float delta)
{
    // store previous kart locations
    const unsigned int kart_amount = m_kart_info.size();
    for(unsigned int n=0; n<kart_amount; n++)
    {
        m_kart_info[n].m_last_track_coords = m_kart_info[n].m_curr_track_coords;
    }
    
    // run generic parent stuff that applies to all modes
    World::update(delta);

    // ------------- do stuff specific to this subtype of race -----
    
    for(unsigned int n=0; n<kart_amount; n++)
    {
        // ---------- update rank ------
        if(!m_kart[n]->hasFinishedRace()) updateRacePosition(m_kart[n], m_kart_info[n]);
    }
    for(unsigned int n=0; n<kart_amount; n++)
    {
        KartInfo& kart_info = m_kart_info[n];
        Kart* kart = m_kart[n];
        
        // ---------- deal with sector data ---------
        
        // update sector variables
        int prev_sector = kart_info.m_track_sector;
        
        if(!kart->isRescue())
            RaceManager::getTrack()->findRoadSector( kart->getXYZ(), &kart_info.m_track_sector);
        
        // Check if the kart is taking a shortcut (if it's not already doing one):
        if(!kart->isRescue() && kart_info.m_last_valid_sector != Track::UNKNOWN_SECTOR &&
           RaceManager::getTrack()->isShortcut(kart_info.m_last_valid_sector, kart_info.m_track_sector))
        {
            forceRescue(kart, kart_info, /*is shortcut*/ true);  // bring karts back to where they left the track.     
            if(kart->isPlayerKart())
            {
                RaceGUI* m=(RaceGUI*)menu_manager->getRaceMenu();
                // Can happen if the option menu is called
                if(m)
                    m->addMessage(_("Invalid short-cut!!"), kart, 2.0f, 60);
            }
            return;
        }
        
        if(kart_info.m_track_sector != Track::UNKNOWN_SECTOR && !kart->isRescue()) kart_info.m_last_valid_sector = kart_info.m_track_sector;
        
        // check if kart is on the road - if not, find the closest sector
        if (kart_info.m_track_sector == Track::UNKNOWN_SECTOR && !kart->isRescue())
        {
            kart_info.m_on_road = false;
            if( kart_info.m_curr_track_coords[0] > 0.0 )
                kart_info.m_track_sector =
                    RaceManager::getTrack()->findOutOfRoadSector( kart->getXYZ(),
                                                                  Track::RS_RIGHT,
                                                                  prev_sector );
            else
                kart_info.m_track_sector =
                    RaceManager::getTrack()->findOutOfRoadSector( kart->getXYZ(),
                                                                  Track::RS_LEFT,
                                                                  prev_sector );
        }
        else
        {
            kart_info.m_on_road = true;
        }
        
        // get position (progression) within track
        RaceManager::getTrack()->spatialToTrack( kart_info.m_curr_track_coords /* out */, 
                                                 kart->getXYZ(),
                                                 kart_info.m_track_sector      );
        
        // ------- check the kart isn't going in the wrong way ------
        // only relevant for player karts
        if(m_kart[n]->isPlayerKart())
        {
            RaceGUI* m=menu_manager->getRaceMenu();
            // This can happen if the option menu is called, since the
            // racegui gets deleted
            if(!m) return;
            
            // check if the player is going in the wrong direction
            if(race_manager->getDifficulty()==RaceManager::RD_EASY)
            {
                float angle_diff = RAD_TO_DEGREE(kart->getHPR().getHeading()) -
                                   RaceManager::getTrack()->m_angle[kart_info.m_track_sector];
                if(angle_diff > 180.0f) angle_diff -= 360.0f;
                else if (angle_diff < -180.0f) angle_diff += 360.0f;
                // Display a warning message if the kart is going back way (unless
                // the kart has already finished the race).
                if ((angle_diff > 120.0f || angle_diff < -120.0f)   &&
                    kart->getVelocity().getY() > 0.0f  && !kart->hasFinishedRace() )
                {
                    m->addMessage(_("WRONG WAY!"), kart, -1.0f, 60);
                }  // if angle is too big
            }  // if difficulty easy
        }// end if is player kart
        
        // --------- do lap counting ------
        doLapCounting(kart_info, kart);
    }// next kart
}
//-----------------------------------------------------------------------------
void LinearWorld::doLapCounting ( KartInfo& kart_info, Kart* kart )
{
    bool newLap = kart_info.m_last_track_coords[1]     > 300.0f  &&
                  kart_info.m_curr_track_coords.getY() <  20.0f;
    if ( newLap )
    {
        // Only increase the lap counter and set the new time if the
        // kart hasn't already finished the race (otherwise the race_gui
        // will begin another countdown).
        if(kart_info.m_race_lap+1 <= race_manager->getNumLaps())
        {
            setTimeAtLapForKart( RaceManager::getWorld()->getTime(), kart->getWorldKartId() );
            kart_info.m_race_lap++ ;
        }
        // Race finished
        if(kart_info.m_race_lap >= race_manager->getNumLaps() && 
           RaceManager::getWorld()->raceHasLaps())
        {
            // A client wait does not detect race finished by itself, it will
            // receive a message from the server. So a client does not do
            // anything here.
            if(network_manager->getMode()!=NetworkManager::NW_CLIENT)
                kart->raceFinished(RaceManager::getWorld()->getTime());
        }
        // Only do timings if original time was set properly. Driving backwards
        // over the start line will cause the lap start time to be set to -1.
        if(kart_info.m_lap_start_time>=0.0)
        {
            float time_per_lap;
            if (kart_info.m_race_lap == 1) // just completed first lap
            {
            	time_per_lap=RaceManager::getWorld()->getTime();
            }
            else //completing subsequent laps
            {
            	time_per_lap=RaceManager::getWorld()->getTime() - kart_info.m_lap_start_time;
            }
            
            // if new fastest lap
            if(time_per_lap < RaceManager::getWorld()->getFastestLapTime() &&
               RaceManager::getWorld()->raceHasLaps())
            {
                RaceManager::getWorld()->setFastestLap(kart, time_per_lap);
                RaceGUI* m=(RaceGUI*)menu_manager->getRaceMenu();
                if(m)
                {
                    m->addMessage(_("New fastest lap"), NULL, 
                                  2.0f, 40, 100, 210, 100);
                    char s[20];
                    m->TimeToString(time_per_lap, s);
                    
                    char m_fastest_lap_message[255];
                    snprintf(m_fastest_lap_message, sizeof(m_fastest_lap_message),
                             "%s: %s",s, kart->getName().c_str());
                    m->addMessage(m_fastest_lap_message, NULL, 
                                  2.0f, 40, 100, 210, 100);
                }   // if m
            } // end if new fastest lap
        }
        kart_info.m_lap_start_time = RaceManager::getWorld()->getTime();
    }
    else if ( kart_info.m_curr_track_coords.getY() > 300.0f && kart_info.m_last_track_coords[1] <  20.0f)
    {
        kart_info.m_race_lap-- ;
        // Prevent cheating by setting time to a negative number, indicating
        // that the line wasn't crossed properly.
        kart_info.m_lap_start_time = -1.0f;
    }
}   // doLapCounting
//-----------------------------------------------------------------------------
int LinearWorld::getSectorForKart(const int kart_id) const
{
    return m_kart_info[kart_id].m_track_sector;
}
//-----------------------------------------------------------------------------
float LinearWorld::getDistanceDownTrackForKart(const int kart_id) const
{
    return m_kart_info[kart_id].m_curr_track_coords.getY();
}
//-----------------------------------------------------------------------------
float LinearWorld::getDistanceToCenterForKart(const int kart_id) const
{
    return m_kart_info[kart_id].m_curr_track_coords.getX();
}
//-----------------------------------------------------------------------------
int LinearWorld::getLapForKart(const int kart_id) const
{
    return  m_kart_info[kart_id].m_race_lap;
}
//-----------------------------------------------------------------------------
void LinearWorld::setTimeAtLapForKart(float t, const int kart_id)
{
    m_kart_info[kart_id].m_time_at_last_lap=t;
}
//-----------------------------------------------------------------------------
float LinearWorld::getTimeAtLapForKart(const int kart_id) const
{
    return m_kart_info[kart_id].m_time_at_last_lap;
}
//-----------------------------------------------------------------------------
KartIconDisplayInfo* LinearWorld::getKartsDisplayInfo(const RaceGUI* caller)
{
    int   laps_of_leader       = -1;
    float time_of_leader       = -1;
    // Find the best time for the lap. We can't simply use
    // the time of the kart at position 1, since the kart
    // might have been overtaken by now
    const unsigned int kart_amount = race_manager->getNumKarts();
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        KartIconDisplayInfo& rank_info = m_kart_display_info[i];
        Kart* kart = m_kart[i];
        
        // reset color
        rank_info.r = 1.0;
        rank_info.g = 1.0;
        rank_info.b = 1.0;
        rank_info.lap = -1;
        
        if(kart->isEliminated()) continue;
        const float lap_time = getTimeAtLapForKart(kart->getWorldKartId());
        const int current_lap  = getLapForKart( kart->getWorldKartId() );
        rank_info.lap = current_lap;
        
        if(current_lap > laps_of_leader)
        {
            // more laps than current leader --> new leader and new time computation
            laps_of_leader = current_lap;
            time_of_leader = lap_time;
        } else if(current_lap == laps_of_leader)
        {
            // Same number of laps as leader: use fastest time
            time_of_leader=std::min(time_of_leader,lap_time);
        }
    }
    
    // we now know the best time of the lap. fill the remaining bits of info
    for(unsigned int i = 0; i < kart_amount ; i++)
    {   
        KartIconDisplayInfo& rank_info = m_kart_display_info[i];
        KartInfo& kart_info = m_kart_info[i];
        Kart* kart = m_kart[i];
        
        const int position = kart->getPosition();
        
        if(laps_of_leader>0 &&    // Don't compare times when crossing the start line first
           (getTime() - getTimeAtLapForKart(kart->getWorldKartId())<5.0f || rank_info.lap != laps_of_leader) &&
           RaceManager::getWorld()->raceHasLaps())
        {  // Display for 5 seconds
            char str[256];
            if(position==1)
            {
                str[0]=' '; str[1]=0;
                caller->TimeToString(getTimeAtLapForKart(kart->getWorldKartId()), str+1);
            }
            else
            {
                float timeBehind;
                timeBehind = (kart_info.m_race_lap==laps_of_leader ? getTimeAtLapForKart(kart->getWorldKartId()) : RaceManager::getWorld()->getTime())
                    - time_of_leader;
                str[0]='+'; str[1]=0;
                caller->TimeToString(timeBehind, str+1);
            }
            rank_info.time = str;
        }
        else
        {
            rank_info.time = "";
        }
        
        int numLaps = race_manager->getNumLaps();
        
        if(kart_info.m_race_lap>=numLaps)
        {  // kart is finished, display in green
            rank_info.g = rank_info.b = 0;
        }
        else if(kart_info.m_race_lap>=0 && numLaps>1)
        {
            rank_info.g = rank_info.b = 1.0f-(float)kart_info.m_race_lap/((float)numLaps-1.0f);
        }
    }   // next kart
    
    
    return m_kart_display_info;
}
//-----------------------------------------------------------------------------
void LinearWorld::terminateRace()
{
    World::terminateRace();
    
    // if some karts have not yet finished the race yet, estimate
    // their times and use these values to proceed without waiting
    const unsigned int kart_amount = m_kart.size();
    for ( Karts::size_type i = 0; i < kart_amount; ++i)
    {
        if(!m_kart[i]->hasFinishedRace())
        {
            const float est_finish_time = estimateFinishTimeForKart(m_kart[i], m_kart_info[i]);
            m_kart[i]->raceFinished(est_finish_time);
        }  // if !hasFinishedRace
    }   // for i
}
void LinearWorld::raceResultOrder( int* order )
{
    const unsigned int NUM_KARTS = race_manager->getNumKarts();
    for(unsigned int i=0; i < NUM_KARTS; i++)
    {
        order[RaceManager::getKart(i)->getPosition()-1] = i; // even for eliminated karts
    }
}
//-----------------------------------------------------------------------------
float LinearWorld::estimateFinishTimeForKart  (Kart* kart, KartInfo& kart_info)
{
    // Estimate the arrival time of any karts that haven't arrived
    // yet by using their average speed up to now and the distance
    // still to race. This approach guarantees that the order of 
    // the karts won't change anymore (karts ahead will have a 
    // higher average speed and therefore finish the race earlier 
    // than karts further behind), so the position doesn't have to
    // be updated to get the correct scoring.
    float distance_covered  = kart_info.m_race_lap * RaceManager::getTrack()->getTrackLength()
        + getDistanceDownTrackForKart(kart->getWorldKartId());
    // In case that a kart is rescued behind start line, or ...
    if(distance_covered<0) distance_covered =1.0f;
    
    float average_speed     = distance_covered/RaceManager::getWorld()->getTime();
    
    // Finish time is the time needed for the whole race with 
    // the average speed computed above.
    return race_manager->getNumLaps()*RaceManager::getTrack()->getTrackLength() 
        / average_speed;
    
}   // estimateFinishTime
//-----------------------------------------------------------------------------
// override 'forceRescue' to do some linear-race-specific actions
void LinearWorld::forceRescue(Kart* kart, KartInfo& kart_info, bool shortcut)
{
    // If rescue is triggered while doing a shortcut, reset the kart to the
    // segment where the shortcut started!! And then reset the shortcut
    // flag, so that this shortcut is not counted!
    if(shortcut)
    {
        kart_info.m_track_sector   = kart_info.m_last_valid_sector;
    } 
    
    kart->forceRescue();
}
//-----------------------------------------------------------------------------
/** Decide where to drop a rescued kart
  */
void LinearWorld::moveKartAfterRescue(Kart* kart, btRigidBody* body)
{
    KartInfo& info = m_kart_info[kart->getWorldKartId()];
    
    if ( info.m_track_sector > 0 ) info.m_track_sector-- ;
    info.m_last_valid_sector = info.m_track_sector;
    if ( info.m_last_valid_sector > 0 ) info.m_last_valid_sector --;
        
    kart->setXYZ( RaceManager::getTrack()->trackToSpatial(info.m_track_sector) );
    
    btQuaternion heading(btVector3(0.0f, 0.0f, 1.0f), 
                         RaceManager::getTrack()->m_angle[info.m_track_sector] );
    kart->setRotation(heading);
    
    // A certain epsilon is added here to the Z coordinate (0.1), in case
    // that the drivelines are somewhat under the track. Otherwise, the
    // kart will be placed a little bit under the track, triggering
    // a rescue, ...
    btTransform pos;
    pos.setOrigin(kart->getXYZ()+btVector3(0, 0, 0.5f*kart->getKartHeight()+0.1f));
    pos.setRotation(btQuaternion(btVector3(0.0f, 0.0f, 1.0f),
                    RaceManager::getTrack()->m_angle[info.m_track_sector]));

    body->setCenterOfMassTransform(pos);
    
}   // moveKartAfterRescue

//-----------------------------------------------------------------------------
/** Find the position (rank) of 'kart' and update it accordingly
  */
void LinearWorld::updateRacePosition ( Kart* kart, KartInfo& kart_info )
{
    int p = 1 ;
    
    const unsigned int kart_amount = m_kart.size();
    for ( unsigned int j = 0 ; j < kart_amount ; j++ )
    {
        if(j == kart->getWorldKartId()) continue; // don't compare a kart with itself
        if(m_kart[j]->isEliminated()) continue;   // dismiss eliminated karts   
        
        // Count karts ahead of the current kart, i.e. kart that are already
        // finished (the current kart k has not yet finished!!), have done more
        // laps, or the same number of laps, but a greater distance.
        if (
            /* has already finished */
            m_kart[j]->hasFinishedRace()                                                         ||
            /* has done more lapses */
            getLapForKart(m_kart[j]->getWorldKartId()) >  getLapForKart(kart->getWorldKartId())  ||
            /* is at the same lap but further in it */
            (getLapForKart(m_kart[j]->getWorldKartId()) == getLapForKart(kart->getWorldKartId()) && 
             getDistanceDownTrackForKart(m_kart[j]->getWorldKartId()) > getDistanceDownTrackForKart(kart->getWorldKartId()) )
            )
            p++ ;
    }//next kart
    
    kart->setPosition(p);
    // Switch on faster music if not already done so, if the
    // first kart is doing its last lap, and if the estimated
    // remaining time is less than 30 seconds.
    if(!m_faster_music_active                           && 
       kart_info.m_race_lap == race_manager->getNumLaps()-1    && 
       p==1                                             &&
       useFastMusicNearEnd()                            &&
       estimateFinishTimeForKart( kart, m_kart_info[kart->getWorldKartId()] )-getTime()<30.0f   ) 
    {
        sound_manager->switchToFastMusic();
        m_faster_music_active=true;
    }
}   // updateRacePosition
