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
#include "states_screens/race_gui_base.hpp"

#include <sstream>

#include "audio/music_manager.hpp"
#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "network/network_manager.hpp" 
#include "race/history.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

//-----------------------------------------------------------------------------
/** Constructs the linear world. Note that here no functions can be called
 *  that use World::getWorld(), since it is not yet defined.
 */
LinearWorld::LinearWorld() : WorldWithRank()
{
    m_kart_display_info   = NULL;
    m_last_lap_sfx        = sfx_manager->createSoundSource("lastlap");
    m_last_lap_sfx_played = false;
    m_last_lap_sfx_playing = false;
}   // LinearWorld

// ----------------------------------------------------------------------------
/** Actually initialises the world, i.e. creates all data structures to
 *  for all karts etc. In init functions can be called that use
 *  World::getWorld().
 */
void LinearWorld::init()
{
    WorldWithRank::init();
    m_last_lap_sfx_played           = false;
    m_last_lap_sfx_playing          = false;
    const unsigned int kart_amount  = m_karts.size();
    m_kart_display_info = new RaceGUIBase::KartIconDisplayInfo[kart_amount];

    for(unsigned int n=0; n<kart_amount; n++)
    {
        KartInfo info;
        info.m_track_sector         = QuadGraph::UNKNOWN_SECTOR;
        info.m_last_valid_sector    = 0;
        info.m_last_valid_race_lap  = -1;
        info.m_lap_start_time       = 0;
        m_track->getQuadGraph().findRoadSector(m_karts[n]->getXYZ(),
                                               &info.m_track_sector);

        //If m_track_sector == UNKNOWN_SECTOR, then the kart is not on top of
        //the road, so we have to use another function to find the sector.
        info.m_on_road = info.m_track_sector != QuadGraph::UNKNOWN_SECTOR;
        if (!info.m_on_road)
        {
            info.m_track_sector =
                m_track->getQuadGraph().findOutOfRoadSector(m_karts[n]->getXYZ(),
                                                            QuadGraph::UNKNOWN_SECTOR );
        }

        m_track->getQuadGraph().spatialToTrack(&info.m_curr_track_coords,
                                               m_karts[n]->getXYZ(),
                                               info.m_track_sector );

        info.m_race_lap             = -1;
        info.m_lap_start_time       = 0;
        info.m_time_at_last_lap     = 99999.9f;
        info.m_estimated_finish     = -1.0f;

        m_kart_info.push_back(info);
    }   // next kart

}   // init

//-----------------------------------------------------------------------------
LinearWorld::~LinearWorld()
{
    sfx_manager->deleteSFX(m_last_lap_sfx);

    // In case that a track is not found, m_kart_display info was never
    // initialised.
    if(m_kart_display_info)
        delete[] m_kart_display_info;
}   // ~LinearWorld

//-----------------------------------------------------------------------------
void LinearWorld::restartRace()
{
    WorldWithRank::restartRace();
    //if(m_last_lap_sfx->getStatus()== SFXManager::SFX_PLAYING)
    //    m_last_lap_sfx->stop();
    m_last_lap_sfx_played = false;
    m_last_lap_sfx_playing = false;

    const unsigned int kart_amount = m_karts.size();
    for(unsigned int i=0; i<kart_amount; i++)
    {
        KartInfo& info              = m_kart_info[i];
        info.m_track_sector         = QuadGraph::UNKNOWN_SECTOR;
        info.m_last_valid_sector    = 0;
        info.m_lap_start_time       = 0;
        m_track->getQuadGraph().findRoadSector(m_karts[i]->getXYZ(),
                                               &info.m_track_sector);

        //If m_track_sector == UNKNOWN_SECTOR, then the kart is not on top of
        //the road, so we have to use another function to find the sector.
        info.m_on_road = info.m_track_sector != QuadGraph::UNKNOWN_SECTOR;
        if (!info.m_on_road)
        {
            info.m_track_sector =
                m_track->getQuadGraph().findOutOfRoadSector(m_karts[i]->getXYZ(),
                                                            QuadGraph::UNKNOWN_SECTOR );
        }

        m_track->getQuadGraph().spatialToTrack(&info.m_curr_track_coords,
                                               m_karts[i]->getXYZ(),
                                               info.m_track_sector );
        info.m_race_lap             = -1;
        info.m_lap_start_time       = -0;
        info.m_time_at_last_lap     = 99999.9f;
    }   // next kart

    // First all kart infos must be updated before  the kart position can be 
    // recomputed, since otherwise 'new' (initialised) valued will be compared
    // with old values.
    updateRacePosition();
    
#ifdef DEBUG
    //FIXME: this could be defined somewhere in a central header so it can be used everywhere
#define assertExpr( ARG1, OP, ARG2 ) if (!(ARG1 OP ARG2)) \
        { \
            std::cerr << "Failed assert " << #ARG1 << #OP << #ARG2 << " @ " << __FILE__ << ":" << __LINE__ \
                      << "; values are (" << ARG1 << #OP << ARG2 << ")\n"; \
            assert(false); \
        }
    
    for (unsigned int i=0; i<kart_amount; i++)
    {
        for (unsigned int j=i+1; j<kart_amount; j++)
        {
            assertExpr( m_karts[i]->getPosition(), !=, m_karts[j]->getPosition() );
        }
    }
#endif
    
}   // restartRace

//-----------------------------------------------------------------------------
/** General update function called once per frame. This updates the kart
 *  sectors, which are then used to determine the kart positions.
 *  \param dt Time step size.
 */
void LinearWorld::update(float dt)
{
    // run generic parent stuff that applies to all modes. It
    // especially updates the kart positions.
    WorldWithRank::update(dt);
    
    if (m_last_lap_sfx_playing && m_last_lap_sfx->getStatus() != SFXManager::SFX_PLAYING)
    {
        music_manager->getCurrentMusic()->resetTemporaryVolume();
        m_last_lap_sfx_playing = false;
    }
    
    const unsigned int kart_amount = getNumKarts();

    // Do stuff specific to this subtype of race.
    // ------------------------------------------
    for(unsigned int n=0; n<kart_amount; n++)
    {
        KartInfo& kart_info = m_kart_info[n];
        Kart* kart = m_karts[n];

        // Nothing to do for karts that are currently being rescued or eliminated
        if(kart->playingEmergencyAnimation()) continue;

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
    
    WorldWithRank::updateTrack(dt);
    updateRacePosition();
    
    for (unsigned int i=0; i<kart_amount; i++)
    {
        // ---------- update rank ------
        if (m_karts[i]->hasFinishedRace() || m_karts[i]->isEliminated()) continue;

        // During the last lap update the estimated finish time.
        // This is used to play the faster music, and by the AI
        if (m_kart_info[i].m_race_lap == race_manager->getNumLaps()-1)
        {
            m_kart_info[i].m_estimated_finish = estimateFinishTimeForKart(m_karts[i]);
        }
        checkForWrongDirection(i);
    }
    
#ifdef DEBUG
    // FIXME: Debug output in case that the double position error occurs again.
    std::vector<int> pos_used;
    pos_used.resize(kart_amount+1, -99);
    for(unsigned int i=0; i<kart_amount; i++)
    {
        if(pos_used[m_karts[i]->getPosition()]!=-99)
        {
            for(unsigned int j =0; j<kart_amount; j++)
            {
                printf("kart id=%d, position=%d, finished=%d, laps=%d, distanceDownTrack=%f %s\n",
                    j, m_karts[j]->getPosition(),
                    m_karts[j]->hasFinishedRace(),
                    m_kart_info[j].m_race_lap,
                    getDistanceDownTrackForKart(m_karts[j]->getWorldKartId()),
                    (m_karts[j]->getPosition() == m_karts[i]->getPosition() ? "<--- !!!" : ""));
            }
        }
        pos_used[m_karts[i]->getPosition()]=i;
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
    Kart    *kart       = m_karts[kart_index];

    // Don't do anything if a kart that has already finished the race
    // crosses the start line again. This avoids 'fastest lap' messages
    // if the end controller does a fastest lap.
    if(kart->hasFinishedRace()) return;

    const int lap_count = race_manager->getNumLaps();
    
    // Only increase the lap counter and set the new time if the
    // kart hasn't already finished the race (otherwise the race_gui
    // will begin another countdown).
    if(kart_info.m_race_lap+1 <= lap_count)
    {
        assert(kart->getWorldKartId()==kart_index);
        setTimeAtLapForKart(getTime(), kart_index );
        kart_info.m_race_lap++ ;
    }
    // Last lap message (kart_index's assert in previous block already)
    if(kart_info.m_race_lap+1 == lap_count)
    {
        m_race_gui->addMessage(_("Final lap!"), m_karts[kart_index],
                               3.0f, 40, video::SColor(255, 210, 100, 50), true);
        if(!m_last_lap_sfx_played && lap_count > 1)
        {
            m_last_lap_sfx->play();
            m_last_lap_sfx_played = true;
            m_last_lap_sfx_playing = true;
            
            // In case that no music is defined
            if(music_manager->getCurrentMusic() && music_manager->getMasterMusicVolume() > 0.2f)
            {
                music_manager->getCurrentMusic()->setTemporaryVolume(0.2f);
            }
        }
    }
    else if (kart_info.m_race_lap > 0 && kart_info.m_race_lap+1 < lap_count)
    {
        m_race_gui->addMessage(StringUtils::insertValues(_("Lap %i"), kart_info.m_race_lap+1),
                               m_karts[kart_index], 3.0f, 40, video::SColor(255, 210, 100, 50), true);
    }

    // The race positions must be updated here: consider the situation where 
    // the first kart does not cross the finish line in its last lap, instead 
    // it passes it, the kart reverses and crosses the finishing line 
    // backwards. Just before crossing the finishing line the kart will be on
    // the last lap, but with a distance along the track close to zero. 
    // Therefore its position will be wrong. If the race position gets updated
    // after increasing the number of laps (but before tagging the kart to have
    // finished the race) the position will be correct (since the kart now
    // has one additional lap it will be ahead of the other karts).
    // Without this call the incorrect position for this kart would remain
    // (since a kart that has finished the race does not get its position
    // changed anymore), potentially resulting in a duplicated race position
    // (since the first kart does not have position 1, no other kart can get
    // position 1, so one rank will be duplicated).
    // Similarly the situation can happen if the distance along track should
    // go back to zero before actually crossing the finishing line. While this
    // should not happen, it could potentially be caused by floating point
    // errors. In this case the call to updateRacePosition will avoid 
    // duplicated race positions as well.
    updateRacePosition();

    // Race finished
    if(kart_info.m_race_lap >= race_manager->getNumLaps() && raceHasLaps())
    {
        // A client wait does not detect race finished by itself, it will
        // receive a message from the server. So a client does not do
        // anything here.
        if(network_manager->getMode()!=NetworkManager::NW_CLIENT)
        {
            kart->finishedRace(getTime());
        }
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
                                   2.0f, 40, video::SColor(255, 100, 210, 100), true);
            std::string s = StringUtils::timeToString(time_per_lap);

            irr::core::stringw m_fastest_lap_message;
            //I18N: as in "fastest lap: 60 seconds by Wilber"
            m_fastest_lap_message += StringUtils::insertValues(_("%s by %s"), s.c_str(), kart->getName().c_str()).c_str();

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
/** Returns the distance the kart has travelled along the track since 
 *  crossing the start line..
 *  \param kart_id Index of the kart.
 */
float LinearWorld::getDistanceDownTrackForKart(const int kart_id) const
{
    return m_kart_info[kart_id].m_curr_track_coords.getZ();
}   // getDistanceDownTrackForKart

//-----------------------------------------------------------------------------
/** Gets the distance of the kart from the center of the driveline. Positive
 *  is to the right of the center, negative values to the left.
 *  \param kart_id Index of kart.
 */
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
RaceGUIBase::KartIconDisplayInfo* LinearWorld::getKartsDisplayInfo()
{
    int   laps_of_leader       = -1;
    float time_of_leader       = -1;
    // Find the best time for the lap. We can't simply use
    // the time of the kart at position 1, since the kart
    // might have been overtaken by now
    const unsigned int kart_amount = getNumKarts();
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        RaceGUIBase::KartIconDisplayInfo& rank_info = m_kart_display_info[i];
        Kart* kart = m_karts[i];

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
        RaceGUIBase::KartIconDisplayInfo& rank_info = m_kart_display_info[i];
        KartInfo& kart_info = m_kart_info[i];
        Kart* kart = m_karts[i];

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
            rank_info.m_text = irr::core::stringw(str.c_str());
        }
        else
        {
            rank_info.m_text = "";
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
/** Estimate the arrival time of any karts that haven't arrived yet by using 
 *  their average speed up to now and the distance still to race. This 
 *  approach guarantees that the order of the karts won't change anymore 
 *  (karts ahead will have a higher average speed and therefore finish the 
 *  race earlier than karts further behind), so the position doesn't have to
 *  be updated to get the correct scoring.
 *  \param kart The kart for which to estimate the finishing times.
 */
float LinearWorld::estimateFinishTimeForKart(Kart* kart)
{
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

}   // estimateFinishTimeForKart

//-----------------------------------------------------------------------------
/** Decide where to drop a rescued kart
  */
void LinearWorld::moveKartAfterRescue(Kart* kart)
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

    btQuaternion heading(btVector3(0.0f, 1.0f, 0.0f),
                         m_track->getAngle(info.m_track_sector) );
    kart->setRotation(heading);

    // A certain epsilon is added here to the Z coordinate, in case
    // that the drivelines are somewhat under the track. Otherwise, the
    // kart might be placed a little bit under the track, triggering
    // a rescue, ... (experimentally found value)
    float epsilon = 0.5f * kart->getKartHeight();

    btTransform pos;
    pos.setOrigin(kart->getXYZ()+btVector3(0, kart->getKartHeight() + epsilon, 0));
    pos.setRotation(btQuaternion(btVector3(0.0f, 1.0f, 0.0f),
                    m_track->getAngle(info.m_track_sector)));

    kart->getBody()->setCenterOfMassTransform(pos);

    //project kart to surface of track
    bool kart_over_ground = m_physics->projectKartDownwards(kart);

    if (kart_over_ground)
    {
        //add vertical offset so that the kart starts off above the track
        float vertical_offset = kart->getKartProperties()->getVertRescueOffset() *
                                kart->getKartHeight();
        kart->getBody()->translate(btVector3(0, vertical_offset, 0));
    }
    else
    {
        fprintf(stderr, "WARNING: invalid position after rescue for kart %s on track %s.\n",
                (kart->getIdent().c_str()), m_track->getIdent().c_str());
    }


}   // moveKartAfterRescue

//-----------------------------------------------------------------------------
/** Find the position (rank) of every kart
  */
void LinearWorld::updateRacePosition()
{
    // Mostly for debugging:
    beginSetKartPositions();
    const unsigned int kart_amount = m_karts.size();

#ifdef DEBUG
    bool rank_changed = false;
#endif
    
    // NOTE: if you do any changes to this loop, the next loop (see
    // DEBUG_KART_RANK below) needs to have the same changes applied
    // so that debug output is still correct!!!!!!!!!!!
    for (unsigned int i=0; i<kart_amount; i++)
    {
        Kart* kart          = m_karts[i];
        // Karts that are either eliminated or have finished the
        // race already have their (final) position assigned. If
        // these karts would get their rank updated, it could happen
        // that a kart that finished first will be overtaken after
        // crossing the finishing line and become second!
        if(kart->isEliminated() || kart->hasFinishedRace())
        {
            // This is only necessary to support debugging inconsistencies
            // in kart position parameters.
            setKartPosition(i, kart->getPosition());
            continue;
        }
        KartInfo& kart_info = m_kart_info[i];
        
        int p = 1 ;

        const int my_id                = kart->getWorldKartId();
        const int my_laps              = getLapForKart(my_id);
        const float my_progression     = getDistanceDownTrackForKart(my_id);        
        // Count karts ahead of the current kart, i.e. kart that are already finished,
        // have done more laps, or the same number of laps, but a greater distance.
        for (unsigned int j = 0 ; j < kart_amount ; j++)
        {
            if(j == kart->getWorldKartId())  continue; // don't compare a kart with itself
            
            if(m_karts[j]->isEliminated())   continue; // dismiss eliminated karts

            if(!kart->hasFinishedRace() && m_karts[j]->hasFinishedRace())
            {
                p++;
                continue;
            }

            /* has done more or less lapses */
            assert(j==m_karts[j]->getWorldKartId());
            int other_laps = getLapForKart(j);
            if (other_laps !=  my_laps)
            {
                if(other_laps > my_laps)
                {
                    p++; // Other kart has more lapses
                }
                continue;
            }
            // Now both karts have the same number of lapses. Test progression.
            // A kart is ahead if it's driven further, or driven the same
            // distance, but started further to the back.
            float other_progression = getDistanceDownTrackForKart(j);
            if(other_progression > my_progression ||
                (other_progression == my_progression &&
                m_karts[j]->getInitialPosition() > kart->getInitialPosition()) )
            {
                p++;
#if _DEBUG_PRINTS_LIKE_MAD_
                std::cout << "    " << p << " : " << m_karts[j]->getIdent() <<
                        " because he has is further within the track (my progression is " <<
                        my_progression << ", his progression is " << other_progression << ")\n";
#endif
            }
        } //next kart

#ifndef DEBUG
        setKartPosition(i, p);
#else
        rank_changed |= kart->getPosition()!=p;
        if (!setKartPosition(i,p))
        {
            std::cerr << "ERROR, same rank used twice!!\n";

            std::cerr <<  "Info used to decide ranking :\n";
            for (unsigned int d=0; d<kart_amount; d++)
            {
                std::cerr << "   kart " << m_karts[d]->getIdent() << " has finished(" << m_karts[d]->hasFinishedRace()
                          << "), is at lap (" << getLapForKart(d) << "), is at distance("
                          << getDistanceDownTrackForKart(d) << "), is eliminated(" << m_karts[d]->isEliminated() << ")" << std::endl;
            }
                        
            std::cerr <<  "Who has each ranking so far :\n";
            for (unsigned int d=0; d<i; d++)
            {
                std::cerr << "    " << m_karts[d]->getIdent() << " has rank " << m_karts[d]->getPosition() << std::endl;
            }
            
            std::cerr << "    --> And " << kart->getIdent() << " is being set at rank " << p << std::endl;
            history->Save();
            assert(false);
        }
#endif

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
            music_manager->switchToFastMusic();
            m_faster_music_active=true;
        }
    }   // for i<kart_amount

    // Define this to get a detailled analyses each time a race position 
    // changes.
#ifdef DEBUG
#undef DEBUG_KART_RANK
#ifdef DEBUG_KART_RANK
    if(rank_changed)
    {
        std::cout << "Counting laps at "<<getTime()<<" seconds.\n";
        for (unsigned int i=0; i<kart_amount; i++)
        {
            Kart* kart          = m_karts[i];
            if(kart->isEliminated() || kart->hasFinishedRace()) continue;
            KartInfo& kart_info = m_kart_info[i];
            int p = 1 ;
            const int my_id                = kart->getWorldKartId();
            const int my_laps              = getLapForKart(my_id);
            const float my_progression     = getDistanceDownTrackForKart(my_id);
            std::cout << "counting karts ahead of " << kart->getIdent() 
                << " (laps "<<m_kart_info[i].m_race_lap<<", progress "
                << my_progression<<").\n";
            for (unsigned int j = 0 ; j < kart_amount ; j++)
            {
                if(j == kart->getWorldKartId())  continue; // don't compare a kart with itself
                if(!kart->hasFinishedRace() && m_karts[j]->hasFinishedRace())
                {
                    p++;
                    std::cout << "    " << p << " : " << m_karts[j]->getIdent() << " because he has finished.\n";
                    continue;
                }
                int other_laps = getLapForKart(j);
                if (other_laps !=  my_laps)
                {
                    if(other_laps > my_laps)
                    {
                        p++; // Other kart has more lapses
                        std::cout << "    " << p << " : " << m_karts[j]->getIdent() << " because he has more laps than me.\n";
                    }
                    continue;
                }
                float other_progression = getDistanceDownTrackForKart(j);
                if(other_progression > my_progression ||
                    (other_progression == my_progression &&
                    m_karts[j]->getInitialPosition() > kart->getInitialPosition()) )
                {
                    p++;
                    std::cout << "    " << p << " : " << m_karts[j]->getIdent() <<
                        " because he is further within the track (my progression is " <<
                        my_progression << ", his progression is " << other_progression << ")\n";
                }
            } //next kart
        }   // for i<kart_amount
        std::cout << "-------------------------------------------\n";
    }   // if rank_changed
#endif
#endif

    endSetKartPositions();
}   // updateRacePosition

//-----------------------------------------------------------------------------
/** Checks if a kart is going in the wrong direction. This is done only for
 *  player karts to display a message to the player.
 *  \param i Kart id.
 */
void LinearWorld::checkForWrongDirection(unsigned int i)
{
    if(!m_karts[i]->getController()->isPlayerController()) return;
    if(!m_kart_info[i].m_on_road ||
        m_karts[i]->playingEmergencyAnimation()) return;

    const Kart *kart=m_karts[i];
    // If the kart can go in more than one directions from the current track
    // don't do any reverse message handling, since it is likely that there
    // will be one direction in which it isn't going backwards anyway.
    if(m_track->getQuadGraph().getNumberOfSuccessors(m_kart_info[i].m_track_sector)>1)
        return;

    // check if the player is going in the wrong direction
    float angle_diff = kart->getHeading() -
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
