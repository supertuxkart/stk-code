//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Joerg Henrichs
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

#include "modes/profile_world.hpp"

#include "main_loop.hpp"
#include "graphics/camera.hpp"
#include "graphics/irr_driver.hpp"
#include "karts/kart_with_stats.hpp"
#include "karts/controller/controller.hpp"
#include "tracks/track.hpp"

#include <ISceneManager.h>

#include <iomanip>
#include <iostream>

ProfileWorld::ProfileType ProfileWorld::m_profile_mode=PROFILE_NONE;
int   ProfileWorld::m_num_laps    = 0;
float ProfileWorld::m_time        = 0.0f;
bool  ProfileWorld::m_no_graphics = false;

//-----------------------------------------------------------------------------
/** The constructor sets the number of (local) players to 0, since only AI
 *  karts are used.
 */
ProfileWorld::ProfileWorld()
{
    race_manager->setNumLocalPlayers(0);
    // Set number of laps so that the end of the race can be detected by
    // quering the number of finished karts from the race manager (in laps
    // based profiling) - in case of time based profiling, the number of
    // laps is set to 99999.
    race_manager->setNumLaps(m_num_laps);
    setPhase(RACE_PHASE);
    m_frame_count      = 0;
    m_start_time       = irr_driver->getRealTime();
    m_num_triangles    = 0;
    m_num_culls        = 0;
    m_num_solid        = 0;
    m_num_transparent  = 0;
    m_num_trans_effect = 0;
    m_num_calls        = 0;
}   // ProfileWorld

//-----------------------------------------------------------------------------
/** Sets profile mode off again.
 *  Needed because demo mode's closing allows the player to continue playing
 *  STK. If we didn't set it off, profile mode would stay activated.
 */
ProfileWorld::~ProfileWorld()
{
    m_profile_mode = PROFILE_NONE;
}

//-----------------------------------------------------------------------------
/** Enables profiling for a certain amount of time. It also sets the
 *  number of laps to a high number (so that the lap count will not finish
 *  a race before the time is over).
 *  \param time Time to profile a race for.
 */
void ProfileWorld::setProfileModeTime(float time)
{
    m_profile_mode = PROFILE_TIME;
    m_num_laps     = 99999;
    m_time         = time;
}   // setProfileModeTime

//-----------------------------------------------------------------------------
/** Enables profiling for a certain number of laps. The race will end when all
 *  karts have done (at least) this number of laps.
 *  \param laps The number of laps.
 */
void ProfileWorld::setProfileModeLaps(int laps)
{
    m_profile_mode = PROFILE_LAPS;
    m_num_laps     = laps;
}   // setProfileModeLaps

//-----------------------------------------------------------------------------
/** Creates a kart, having a certain position, starting location, and local
 *  and global player id (if applicable).
 *  \param kart_ident Identifier of the kart to create.
 *  \param index Index of the kart.
 *  \param local_player_id If the kart is a player kart this is the index of
 *         this player on the local machine.
 *  \param global_player_id If the akrt is a player kart this is the index of
 *         this player globally (i.e. including network players).
 *  \param init_pos The start XYZ coordinates.
 */
AbstractKart *ProfileWorld::createKart(const std::string &kart_ident, int index,
                                       int local_player_id, int global_player_id,
                                       RaceManager::KartType type)
{
    btTransform init_pos   = m_track->getStartTransform(index);

    Kart *new_kart         = new KartWithStats(kart_ident,
                                               /*world kart id*/ index,
                                               /*position*/ index+1,
                                               init_pos);
    new_kart->init(RaceManager::KT_AI);
    Controller *controller = loadAIController(new_kart);
    new_kart->setController(controller);

    // Create a camera for the last kart (since this way more of the
    // karts can be seen.
    if (index == (int)race_manager->getNumberOfKarts()-1)
    {
        // The camera keeps track of all cameras and will free them
        Camera::createCamera(new_kart);
    }
    return new_kart;
}   // createKart

//-----------------------------------------------------------------------------
/** The race is over if either the requested number of laps have been done
 *  or the requested time is over.
 */
bool ProfileWorld::isRaceOver()
{
    if(m_profile_mode==PROFILE_TIME)
        return getTime()>m_time;

    if(m_profile_mode == PROFILE_LAPS )
    {
        // Now it must be laps based profiling:
        return race_manager->getFinishedKarts()==getNumKarts();
    }
    // Unknown profile mode
    assert(false);
    return false;  // keep compiler happy
}   // isRaceOver

//-----------------------------------------------------------------------------
/** Counts the number of frames.
 */
void ProfileWorld::update(float dt)
{
    StandardRace::update(dt);

    m_frame_count++;
    video::IVideoDriver *driver = irr_driver->getVideoDriver();
    io::IAttributes   *attr = irr_driver->getSceneManager()->getParameters();
    m_num_triangles    += (int)(driver->getPrimitiveCountDrawn( 0 )
                        * ( 1.f / 1000.f ));
    m_num_calls        += attr->getAttributeAsInt("calls");
    m_num_culls        += attr->getAttributeAsInt("culled" );
    m_num_solid        += attr->getAttributeAsInt("drawn_solid" );
    m_num_transparent  += attr->getAttributeAsInt("drawn_transparent" );
    m_num_trans_effect += attr->getAttributeAsInt("drawn_transparent_effect" );

}   // update

//-----------------------------------------------------------------------------
/** This function is called when the race is finished, but end-of-race
 *  animations have still to be played. In the case of profiling,
 *  we can just abort here without waiting for the animations.
 */
void ProfileWorld::enterRaceOverState()
{
    // If in timing mode, the number of laps is way too high (which avoids
    // aborting too early). So in this case determine the maximum number
    // of laps and set this +1 as the number of laps to get more meaningful
    // time estimations.
    if(m_profile_mode==PROFILE_TIME)
    {
        int max_laps = -2;
        for(unsigned int i=0; i<race_manager->getNumberOfKarts(); i++)
        {
            if(m_kart_info[i].m_race_lap>max_laps)
                max_laps = m_kart_info[i].m_race_lap;
        }   // for i<getNumberOfKarts
        race_manager->setNumLaps(max_laps+1);
    }

    StandardRace::enterRaceOverState();
    // Estimate finish time and set all karts to be finished.
    for (unsigned int i=0; i<race_manager->getNumberOfKarts(); i++)
    {
        // ---------- update rank ------
        if (m_karts[i]->hasFinishedRace() || m_karts[i]->isEliminated())
            continue;
        m_karts[i]->finishedRace(estimateFinishTimeForKart(m_karts[i]));
    }

    // Print framerate statistics
    float runtime = (irr_driver->getRealTime()-m_start_time)*0.001f;
    Log::verbose("profile", "Number of frames: %d time %f, Average FPS: %f",
                 m_frame_count, runtime, (float)m_frame_count/runtime);

    // Print geometry statistics if we're not in no-graphics mode
    if(!m_no_graphics)
    {
        Log::verbose("profile", "Average # drawn nodes           %f k",
                     (float)m_num_triangles/m_frame_count);
        Log::verbose("profile", "Average # culled nodes:         %f k",
                     (float)m_num_culls/m_frame_count);
        Log::verbose("profile", "Average # solid nodes:          %f k",
                     (float)m_num_solid/m_frame_count);
        Log::verbose("profile", "Average # transparent nodes:    %f",
                     (float)m_num_transparent/m_frame_count);
        Log::verbose("profile", "Average # transp. effect nodes: %f",
                     (float)m_num_trans_effect/m_frame_count);
    }

    // Print race statistics for each individual kart
    float min_t=999999.9f, max_t=0.0, av_t=0.0;
    Log::verbose("profile", "name start_position end_position time average_speed top_speed "
           "skid_time rescue_time rescue_count brake_count "
           "explosion_time explosion_count bonus_count banana_count "
           "small_nitro_count large_nitro_count bubblegum_count");

    std::set<std::string> all_groups;

    for ( KartList::size_type i = 0; i < m_karts.size(); ++i)
    {
        KartWithStats* kart = dynamic_cast<KartWithStats*>(m_karts[i]);

        max_t = std::max(max_t, kart->getFinishTime());
        min_t = std::min(min_t, kart->getFinishTime());
        av_t += kart->getFinishTime();
        std::ostringstream ss;
        ss << kart->getIdent() << " "
           << kart->getController()->getControllerName() << " ";
        ss << 1+(int)i << " " << kart->getPosition() << " "
           << kart->getFinishTime() << " ";

        all_groups.insert(kart->getController()->getControllerName());
        float distance = (float)(m_profile_mode==PROFILE_LAPS
                                 ? race_manager->getNumLaps() : 1);
        distance *= m_track->getTrackLength();
        ss << distance/kart->getFinishTime() << " " << kart->getTopSpeed() << " ";
        ss << kart->getSkiddingTime() << " " << kart->getRescueTime() << " ";
        ss << kart->getRescueCount() << " " << kart->getBrakeCount() << " ";
        ss << kart->getExplosionTime() << " " << kart->getExplosionCount() << " ";
        ss << kart->getBonusCount() << " " << kart->getBananaCount() << " ";
        ss << kart->getSmallNitroCount() << " " << kart->getLargeNitroCount() << " ";
        ss << kart->getBubblegumCount() << " " << kart->getOffTrackCount() << " ";
        Log::verbose("profile", ss.str().c_str());
    }

    // Print group statistics of all karts
    Log::verbose("profile", "min %f  max %f  av %f\n",
                  min_t, max_t, av_t/m_karts.size());

    // Determine maximum length of group name
    unsigned int max_len=4;   // for 'name' heading
    for(std::set<std::string>::iterator it = all_groups.begin();
        it !=all_groups.end(); it++)
    {
        if(it->size()>max_len) max_len = (unsigned int) it->size();
    }
    max_len++;  // increase by 1 for one additional space after the name

    std::ostringstream ss;
    Log::verbose("profile", "");
    ss << "name" << std::setw(max_len-4) << " "
       << "Strt End  Time    AvSp  Top   Skid  Resc Rsc Brake Expl Exp Itm Ban SNitLNit Bub  Off";
    Log::verbose("profile", ss.str().c_str());
    for(std::set<std::string>::iterator it = all_groups.begin();
        it !=all_groups.end(); it++)
    {
        int   count         = 0,    position_gain   = 0,    rescue_count = 0;
        int   brake_count   = 0,    bonus_count     = 0,    banana_count = 0;
        int   l_nitro_count = 0,    s_nitro_count   = 0,    bubble_count = 0;
        int   expl_count    = 0,    off_track_count = 0;
        float skidding_time = 0.0f, rescue_time     = 0.0f, expl_time    = 0.0f;
        float av_time       = 0.0f;
        for ( unsigned int i = 0; i < (unsigned int)m_karts.size(); ++i)
        {
            KartWithStats* kart = dynamic_cast<KartWithStats*>(m_karts[i]);
            const std::string &name=kart->getController()->getControllerName();
            if(name!=*it)
                continue;
            count ++;
            std::ostringstream ss;
            ss.setf(std::ios::fixed, std::ios::floatfield);
            ss.precision(2);
            ss << name << std::setw(max_len-name.size()) << " ";
            ss << std::setw(4) <<  1 + i
               << std::setw(4) << kart->getPosition()
               << std::setw(7) << kart->getFinishTime();
            position_gain += 1+i - kart->getPosition();

            float distance = (float)(m_profile_mode==PROFILE_LAPS
                                     ? race_manager->getNumLaps() : 1);
            distance *= m_track->getTrackLength();

            Log::verbose("profile",
                   "%s %4.2f %3.2f %6.2f %4.2f %3d %5d %4.2f %3d %3d %3d %3d %3d %3d %5d",
                   ss.str().c_str(), distance/kart->getFinishTime(),
                   kart->getTopSpeed(),
                   kart->getSkiddingTime(),        kart->getRescueTime(),
                   kart->getRescueCount(),         kart->getBrakeCount(),
                   kart->getExplosionTime(),       kart->getExplosionCount(),
                   kart->getBonusCount(),          kart->getBananaCount(),
                   kart->getSmallNitroCount(),     kart->getLargeNitroCount(),
                   kart->getBubblegumCount(),      kart->getOffTrackCount()
                   );
            Log::verbose("profile", "nitro %f\n", kart->getEnergy());
            av_time += kart->getFinishTime();
            skidding_time   += kart->getSkiddingTime();
            rescue_time     += kart->getRescueTime();
            rescue_count    += kart->getRescueCount();
            brake_count     += kart->getBrakeCount();
            bonus_count     += kart->getBonusCount();
            banana_count    += kart->getBananaCount();
            l_nitro_count   += kart->getLargeNitroCount();
            s_nitro_count   += kart->getSmallNitroCount();
            bubble_count    += kart->getBubblegumCount();
            expl_time       += kart->getExplosionTime();
            expl_count      += kart->getExplosionCount();
            off_track_count += kart->getOffTrackCount();
        }    // for i < m_karts.size
        
        Log::verbose("profile", std::string(max_len+85, '-').c_str());
        ss.clear();
        ss.str("");
        ss << *it << std::string(max_len-it->size(),' ');
        ss << std::showpos << std::setw(4) << position_gain
           << std::noshowpos << std::setw(13) << av_time/count
           << std::string(11,' ');

        Log::verbose("profile", "%s%6.2f %4.2f %3d %5d %4.2f %3d %3d %3d %3d %3d %3d %5d",
               ss.str().c_str(), skidding_time/count, rescue_time/count,
               rescue_count,brake_count, expl_time, expl_count, bonus_count,
               banana_count, s_nitro_count, l_nitro_count, bubble_count,
               off_track_count);
        Log::verbose("profile", "");
    }   // for it !=all_groups.end
    delete this;
    main_loop->abort();
}   // enterRaceOverState
