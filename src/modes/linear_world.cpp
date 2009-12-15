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
#include "states_screens/race_gui.hpp"

#include <sstream>

#include "audio/sound_manager.hpp"
#include "network/network_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

//-----------------------------------------------------------------------------
LinearWorld::LinearWorld() : World()
{
    m_kart_display_info = NULL;
}   // LinearWorld

// ----------------------------------------------------------------------------
void LinearWorld::init()
{
    World::init();
    const unsigned int kart_amount = m_kart.size();

    m_kart_display_info = new RaceGUI::KartIconDisplayInfo[kart_amount];

    for(unsigned int n=0; n<kart_amount; n++)
    {
        KartInfo info;
        info.m_track_sector         = QuadGraph::UNKNOWN_SECTOR;
        info.m_last_valid_sector    = QuadGraph::UNKNOWN_SECTOR;
        info.m_last_valid_race_lap  = -1;
        info.m_lap_start_time       = 0;
        m_track->getQuadGraph().findRoadSector(m_kart[n]->getXYZ(),
                                               &info.m_track_sector);

        //If m_track_sector == UNKNOWN_SECTOR, then the kart is not on top of
        //the road, so we have to use another function to find the sector.
        info.m_on_road = info.m_track_sector != QuadGraph::UNKNOWN_SECTOR;
        if (!info.m_on_road)
        {
            info.m_track_sector =
                m_track->getQuadGraph().findOutOfRoadSector(m_kart[n]->getXYZ(),
                                                            QuadGraph::UNKNOWN_SECTOR );
        }

        m_track->getQuadGraph().spatialToTrack(&info.m_curr_track_coords,
                                               m_kart[n]->getXYZ(),
                                               info.m_track_sector );

        info.m_race_lap             = -1;
        info.m_lap_start_time       = 0;
        info.m_time_at_last_lap     = 99999.9f;

        m_kart_info.push_back(info);
    }   // next kart

}   // init

//-----------------------------------------------------------------------------
LinearWorld::~LinearWorld()
{
    // In case that a track is not found, m_kart_display info was never
    // initialised.
    if(m_kart_display_info)
        delete[] m_kart_display_info;
}   // ~LinearWorld

//-----------------------------------------------------------------------------
void LinearWorld::restartRace()
{
    World::restartRace();

    const unsigned int kart_amount = m_kart.size();
    for(unsigned int n=0; n<kart_amount; n++)
    {
        KartInfo& info = m_kart_info[n];
        info.m_track_sector         = QuadGraph::UNKNOWN_SECTOR;
        info.m_last_valid_sector    = QuadGraph::UNKNOWN_SECTOR;
        info.m_lap_start_time       = 0;
        m_track->getQuadGraph().findRoadSector(m_kart[n]->getXYZ(),
                                               &info.m_track_sector);

        //If m_track_sector == UNKNOWN_SECTOR, then the kart is not on top of
        //the road, so we have to use another function to find the sector.
        info.m_on_road = info.m_track_sector != QuadGraph::UNKNOWN_SECTOR;
        if (!info.m_on_road)
        {
            info.m_track_sector =
                m_track->getQuadGraph().findOutOfRoadSector(m_kart[n]->getXYZ(),
                                              QuadGraph::UNKNOWN_SECTOR );
        }

        m_track->getQuadGraph().spatialToTrack(&info.m_curr_track_coords,
                                               m_kart[n]->getXYZ(),
                                               info.m_track_sector );
        info.m_race_lap             = -1;
        info.m_lap_start_time       = -0;
        info.m_time_at_last_lap     = 99999.9f;

        updateRacePosition(m_kart[n], info);
    }   // next kart
}   // restartRace

//-----------------------------------------------------------------------------
void LinearWorld::update(float delta)
{
    // run generic parent stuff that applies to all modes. It
    // especially updates the kart positions.
    World::update(delta);

    const unsigned int kart_amount = race_manager->getNumKarts();

    // Do stuff specific to this subtype of race.
    // ------------------------------------------
    for(unsigned int n=0; n<kart_amount; n++)
    {
        KartInfo& kart_info = m_kart_info[n];
        Kart* kart = m_kart[n];

        // Nothing to do for karts that are currently being rescued.
        if(kart->isRescue()) continue;

        // ---------- deal with sector data ---------

        // update sector variables
        int prev_sector = kart_info.m_track_sector;
        m_track->getQuadGraph().findRoadSector(kart->getXYZ(),
                                               &kart_info.m_track_sector);

        kart_info.m_on_road = kart_info.m_track_sector != QuadGraph::UNKNOWN_SECTOR;
        if(kart_info.m_on_road)
        {
            kart_info.m_last_valid_sector   = kart_info.m_track_sector;
            kart_info.m_last_valid_race_lap = kart_info.m_race_lap;
        }
        else
        {
            // Kart off road. Find the closest sector instead.
            kart_info.m_track_sector =
                m_track->getQuadGraph().findOutOfRoadSector(kart->getXYZ(), prev_sector );
        }

        // Update track coords (=progression)
        m_track->getQuadGraph().spatialToTrack(&kart_info.m_curr_track_coords,
                                               kart->getXYZ(),
                                               kart_info.m_track_sector    );

    }   // for n

    // Update all positions. This must be done after _all_ karts have
    // updated their position and laps etc, otherwise inconsistencies
    // (like two karts at same position) can occur.
    // ---------------------------------------------------------------
    for(unsigned int i=0; i<kart_amount; i++)
    {
        // ---------- update rank ------
        if(!m_kart[i]->hasFinishedRace() && !m_kart[i]->isEliminated())
        {
            updateRacePosition(m_kart[i], m_kart_info[i]);
            // During the last lap update the estimated finish time.
            // This is used to play the faster music, and by the AI
            if(m_kart_info[i].m_race_lap == race_manager->getNumLaps()-1)
                m_kart_info[i].m_estimated_finish = estimateFinishTimeForKart(m_kart[i]);
            checkForWrongDirection(i);
        }
    }
#ifdef DEBUG
    // FIXME: Debug output in case that the double position error
    // occurs again. It can most likely be removed.
    int pos_used[10];
    for(int i=0; i<10; i++) pos_used[i]=-99;
    for(unsigned int i=0; i<kart_amount; i++)
    {
        if(pos_used[m_kart[i]->getPosition()]!=-99)
        {
            for(unsigned int j =0; j<kart_amount; j++)
            {
                printf("j %d pos %d finished %d laps %d distance %f\n",
                    j, m_kart[j]->getPosition(),
                    m_kart[j]->hasFinishedRace(),
                    m_kart_info[j].m_race_lap,
                    getDistanceDownTrackForKart(m_kart[j]->getWorldKartId()));
            }
        }
        pos_used[m_kart[i]->getPosition()]=i;
    }
#endif

}   // update

//-----------------------------------------------------------------------------
/** Is called by check structures if a kart starts a new lap.
 *  \param kart_index Index of the kart.
 */
void LinearWorld::newLap(unsigned int kart_index)
{
    KartInfo &kart_info = m_kart_info[kart_index];
    Kart    *kart       = m_kart[kart_index];
    // Only increase the lap counter and set the new time if the
    // kart hasn't already finished the race (otherwise the race_gui
    // will begin another countdown).
    if(kart_info.m_race_lap+1 <= race_manager->getNumLaps())
    {
        assert(kart->getWorldKartId()==kart_index);
        setTimeAtLapForKart(getTime(), kart_index );
        kart_info.m_race_lap++ ;
    }
    // Race finished
    if(kart_info.m_race_lap >= race_manager->getNumLaps() && raceHasLaps())
    {
        // A client wait does not detect race finished by itself, it will
        // receive a message from the server. So a client does not do
        // anything here.
        if(network_manager->getMode()!=NetworkManager::NW_CLIENT)
            kart->raceFinished(getTime());
    }
    {
        float time_per_lap;
        if (kart_info.m_race_lap == 1) // just completed first lap
        {
            time_per_lap=getTime();
        }
        else //completing subsequent laps
        {
            time_per_lap=getTime() - kart_info.m_lap_start_time;
        }

        // if new fastest lap
        if(time_per_lap < getFastestLapTime() && raceHasLaps() &&
            kart_info.m_race_lap>0)
        {
            setFastestLap(kart, time_per_lap);
            m_race_gui->addMessage(_("New fastest lap"), NULL,
                                   2.0f, 40, video::SColor(255, 100, 210, 100));
            std::string s = StringUtils::timeToString(time_per_lap);

            irr::core::stringw m_fastest_lap_message;
            m_fastest_lap_message += (s + ": ").c_str();
            m_fastest_lap_message += kart->getName();
                        
            m_race_gui->addMessage(m_fastest_lap_message, NULL,
                                   2.0f, 40, video::SColor(255, 100, 210, 100));
        } // end if new fastest lap
    }
    kart_info.m_lap_start_time = getTime();
}   // newLap

//-----------------------------------------------------------------------------
int LinearWorld::getSectorForKart(const int kart_id) const
{
    return m_kart_info[kart_id].m_track_sector;
}   // getSectorForKart

//-----------------------------------------------------------------------------
float LinearWorld::getDistanceDownTrackForKart(const int kart_id) const
{
    return m_kart_info[kart_id].m_curr_track_coords.getY();
}   // getDistanceDownTrackForKart

//-----------------------------------------------------------------------------
float LinearWorld::getDistanceToCenterForKart(const int kart_id) const
{
    return m_kart_info[kart_id].m_curr_track_coords.getX();
}   // getDistanceToCenterForKart

//-----------------------------------------------------------------------------
int LinearWorld::getLapForKart(const int kart_id) const
{
    return  m_kart_info[kart_id].m_race_lap;
}   // getLapForKart

//-----------------------------------------------------------------------------
void LinearWorld::setTimeAtLapForKart(float t, const int kart_id)
{
    m_kart_info[kart_id].m_time_at_last_lap=t;
}   // setTimeAtLapForKart

//-----------------------------------------------------------------------------
/** Returns the estimated finishing time. Only valid during the last lap!
 *  \param kart_id Id of the kart.
 */
float LinearWorld::getEstimatedFinishTime(const int kart_id) const
{
    assert(m_kart_info[kart_id].m_race_lap == race_manager->getNumLaps()-1);
    return m_kart_info[kart_id].m_estimated_finish;
}   // getEstimatedFinishTime

//-----------------------------------------------------------------------------
float LinearWorld::getTimeAtLapForKart(const int kart_id) const
{
    return m_kart_info[kart_id].m_time_at_last_lap;
}   // getTimeAtLapForKart

//-----------------------------------------------------------------------------
RaceGUI::KartIconDisplayInfo* LinearWorld::getKartsDisplayInfo()
{
    int   laps_of_leader       = -1;
    float time_of_leader       = -1;
    // Find the best time for the lap. We can't simply use
    // the time of the kart at position 1, since the kart
    // might have been overtaken by now
    const unsigned int kart_amount = race_manager->getNumKarts();
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        RaceGUI::KartIconDisplayInfo& rank_info = m_kart_display_info[i];
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
        RaceGUI::KartIconDisplayInfo& rank_info = m_kart_display_info[i];
        KartInfo& kart_info = m_kart_info[i];
        Kart* kart = m_kart[i];

        const int position = kart->getPosition();

        if(laps_of_leader>0 &&    // Don't compare times when crossing the start line first
           (getTime() - getTimeAtLapForKart(kart->getWorldKartId())<5.0f || rank_info.lap != laps_of_leader) &&
           raceHasLaps())
        {  // Display for 5 seconds
            std::string str;
            if(position == 1)
            {
                str = " " + StringUtils::timeToString( getTimeAtLapForKart(kart->getWorldKartId()) );
            }
            else
            {
                float timeBehind;
                timeBehind = (kart_info.m_race_lap==laps_of_leader
                                ? getTimeAtLapForKart(kart->getWorldKartId())
                                : getTime())
                           - time_of_leader;
                str = "+" + StringUtils::timeToString(timeBehind);
            }
            rank_info.time = irr::core::stringw(str.c_str());
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
}   // getKartsDisplayInfo

//-----------------------------------------------------------------------------
void LinearWorld::terminateRace()
{
    World::terminateRace();

    // if some karts have not yet finished the race yet, estimate
    // their times and use these values to proceed without waiting
    const unsigned int kart_amount = m_kart.size();
    for ( Karts::size_type i = 0; i < kart_amount; ++i)
    {
        // Eliminated karts have already called raceFinished.
        if(!m_kart[i]->hasFinishedRace() && !m_kart[i]->isEliminated())
        {
            const float est_finish_time = m_kart_info[i].m_estimated_finish;
            m_kart[i]->raceFinished(est_finish_time);
        }  // if !hasFinishedRace
    }   // for i
}   // terminateRace

// ----------------------------------------------------------------------------
void LinearWorld::raceResultOrder( int* order )
{
    const unsigned int NUM_KARTS = race_manager->getNumKarts();
    for(unsigned int i=0; i < NUM_KARTS; i++)
    {
        order[RaceManager::getKart(i)->getPosition()-1] = i; // even for eliminated karts
    }
}   // raceResultOrder

//-----------------------------------------------------------------------------
float LinearWorld::estimateFinishTimeForKart(Kart* kart)
{
    // Estimate the arrival time of any karts that haven't arrived
    // yet by using their average speed up to now and the distance
    // still to race. This approach guarantees that the order of
    // the karts won't change anymore (karts ahead will have a
    // higher average speed and therefore finish the race earlier
    // than karts further behind), so the position doesn't have to
    // be updated to get the correct scoring.
    const KartInfo &kart_info = m_kart_info[kart->getWorldKartId()];
    float distance_covered  = kart_info.m_race_lap * m_track->getTrackLength()
        + getDistanceDownTrackForKart(kart->getWorldKartId());
    // In case that a kart is rescued behind start line, or ...
    if(distance_covered<0) distance_covered =1.0f;

    const float full_distance = race_manager->getNumLaps()*m_track->getTrackLength();
    const float average_speed = distance_covered/getTime();

    // Finish time is the time needed for the whole race with
    // the average speed computed above.
    return getTime() + (full_distance - distance_covered)  / average_speed;

}   // estimateFinishTime

//-----------------------------------------------------------------------------
/** Decide where to drop a rescued kart
  */
void LinearWorld::moveKartAfterRescue(Kart* kart, btRigidBody* body)
{
    KartInfo& info = m_kart_info[kart->getWorldKartId()];

    // If the kart is off road, rescue it to the last valid track position
    // instead of the current one (since the sector might be determined by
    // being closest to it, which allows shortcuts like drive towards another
    // part of the lap, press rescue, and be rescued to this other part of
    // the track (example: math class, drive towards the left after start,
    // when hitting the books, press rescue --> you are rescued to the
    // end of the track).
    if(!info.m_on_road)
    {
        info.m_track_sector = info.m_last_valid_sector;
    }
    info.m_race_lap =  info.m_last_valid_race_lap;
    // FIXME - removing 1 here makes it less likely to fall in a rescue loop since the kart
    // moves back on each attempt. This is still a weak hack. Also some other code depends
    // on 1 being substracted, like 'forceRescue'
    if ( info.m_track_sector > 0 ) info.m_track_sector-- ;
    info.m_last_valid_sector = info.m_track_sector;
    if ( info.m_last_valid_sector > 0 ) info.m_last_valid_sector --;

    kart->setXYZ( m_track->trackToSpatial(info.m_track_sector) );

    btQuaternion heading(btVector3(0.0f, 0.0f, 1.0f),
                         m_track->getAngle(info.m_track_sector) );
    kart->setRotation(heading);

    // A certain epsilon is added here to the Z coordinate, in case
    // that the drivelines are somewhat under the track. Otherwise, the
    // kart might be placed a little bit under the track, triggering
    // a rescue, ... (experimentally found value)
    float epsilon = 0.5f * kart->getKartHeight();

    btTransform pos;
    pos.setOrigin(kart->getXYZ()+btVector3(0, 0, kart->getKartHeight() + epsilon));
    pos.setRotation(btQuaternion(btVector3(0.0f, 0.0f, 1.0f),
                    m_track->getAngle(info.m_track_sector)));

    body->setCenterOfMassTransform(pos);

    //project kart to surface of track
    bool kart_over_ground = m_physics->projectKartDownwards(kart);

    if (kart_over_ground)
    {
        //add vertical offset so that the kart starts off above the track
        float vertical_offset = kart->getKartProperties()->getZRescueOffset() *
                                kart->getKartHeight();
        body->translate(btVector3(0, 0, vertical_offset));
    }
    else
    {
        fprintf(stderr, "WARNING: invalid position after rescue for kart %s on track %s.\n",
                (kart->getIdent().c_str()), m_track->getIdent().c_str());
    }


}   // moveKartAfterRescue

//-----------------------------------------------------------------------------
/** Find the position (rank) of 'kart' and update it accordingly
  */
void LinearWorld::updateRacePosition ( Kart* kart, KartInfo& kart_info )
{
    int p = 1 ;

    const unsigned int kart_amount = m_kart.size();
    const int my_id                = kart->getWorldKartId();
    const int my_laps              = getLapForKart(my_id);
    const float my_progression     = getDistanceDownTrackForKart(my_id);
    for ( unsigned int j = 0 ; j < kart_amount ; j++ )
    {
        if(j == kart->getWorldKartId()) continue; // don't compare a kart with itself
        if(m_kart[j]->isEliminated())   continue; // dismiss eliminated karts

        // Count karts ahead of the current kart, i.e. kart that are already
        // finished (the current kart k has not yet finished!!), have done more
        // laps, or the same number of laps, but a greater distance.
        if(m_kart[j]->hasFinishedRace()) { p++; continue; }

        /* has done more or less lapses */
        assert(j==m_kart[j]->getWorldKartId());
        int other_laps = getLapForKart(j);
        if (other_laps !=  my_laps)
        {
            if(other_laps > my_laps) p++; // Other kart has more lapses
            continue;
        }
        // Now both karts have the same number of lapses. Test progression.
        // A kart is ahead if it's driven further, or driven the same
        // distance, but started further to the back.
        float other_progression = getDistanceDownTrackForKart(j);
        if(other_progression > my_progression ||
            (other_progression == my_progression &&
            m_kart[j]->getInitialPosition() > kart->getInitialPosition()) )
        {
                        p++;
        }
    } //next kart

    kart->setPosition(p);
    // Switch on faster music if not already done so, if the
    // first kart is doing its last lap, and if the estimated
    // remaining time is less than 30 seconds.
    if(!m_faster_music_active                                  &&
       kart_info.m_race_lap == race_manager->getNumLaps()-1    &&
       p==1                                                    &&
       useFastMusicNearEnd()                                   &&
       kart_info.m_estimated_finish > 0                        &&
       kart_info.m_estimated_finish - getTime() < 30.0f              )
    {
        sound_manager->switchToFastMusic();
        m_faster_music_active=true;
    }
}   // updateRacePosition

//-----------------------------------------------------------------------------
/** Checks if a kart is going in the wrong direction. This is done only for
 *  player karts to display a message to the player.
 *  \param i Kart id.
 */
void LinearWorld::checkForWrongDirection(unsigned int i)
{
    if(!m_kart[i]->isPlayerKart()) return;
    if(!m_kart_info[i].m_on_road) return;

    // FIXME: Don't do this if the in-game option menu is on the screen!
    // if(option_menu) return;

    const Kart *kart=m_kart[i];
    // If the kart can go in more than one directions from the current track
    // don't do any reverse message handling, since it is likely that there
    // will be one direction in which it isn't going backwards anyway.
    if(m_track->getQuadGraph().getNumberOfSuccessors(m_kart_info[i].m_track_sector)>1)
        return;

    // check if the player is going in the wrong direction
    float angle_diff = kart->getHPR().getHeading() -
                       m_track->getAngle(m_kart_info[i].m_track_sector);
    if(angle_diff > M_PI) angle_diff -= 2*M_PI;
    else if (angle_diff < -M_PI) angle_diff += 2*M_PI;
    // Display a warning message if the kart is going back way (unless
    // the kart has already finished the race).
    if (( angle_diff >  DEGREE_TO_RAD* 120.0f ||
          angle_diff < -DEGREE_TO_RAD*120.0f)      &&
        kart->getVelocityLC().getY() > 0.0f        &&
        !kart->hasFinishedRace() )
    {
        m_race_gui->addMessage(_("WRONG WAY!"), kart, -1.0f, 60);
    }  // if angle is too big
}   // checkForWrongDirection

//-----------------------------------------------------------------------------
