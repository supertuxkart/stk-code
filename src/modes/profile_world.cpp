//  $Id: profile_world.cpp 3882 2009-08-18 11:05:40Z davemk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Joerg Henrichs
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

#include "graphics/camera.hpp"
#include "graphics/irr_driver.hpp"
#include "tracks/track.hpp"

ProfileWorld::ProfileType ProfileWorld::m_profile_mode=PROFILE_NONE;
int   ProfileWorld::m_num_laps = 0;
float ProfileWorld::m_time   = 0.0f;

//-----------------------------------------------------------------------------
/** The constructor sets the number of (local) players to 0, since only AI
 *  karts are used.
 */
ProfileWorld::ProfileWorld()
{
    race_manager->setNumPlayers(0);
    race_manager->setNumLocalPlayers(0);
    // Set number of laps so that the end of the race can be detected by 
    // quering the number of finished karts from the race manager (in laps
    // based profiling) - otherwise just a high number.
    race_manager->setNumLaps(m_profile_mode==PROFILE_LAPS ? m_num_laps : 99999);
    m_phase            = RACE_PHASE;
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
/** Enables profiling for a certain amount of time.
 *  \param time Time to profile a race for.
 */
void ProfileWorld::setProfileModeTime(float time)
{
    m_profile_mode = PROFILE_TIME;
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
Kart *ProfileWorld::createKart(const std::string &kart_ident, int index, 
                               int local_player_id, int global_player_id)
{
    // Ignore the kart identifier specified for this kart, instead load
    // _only_ the kart specified for the player. This allows to measure
    // the impact different karts have on performance.
    const std::string prof_kart_id = race_manager->getKartIdent(
                                           race_manager->getNumberOfKarts()-1);
    btTransform init_pos   = m_track->getStartTransform(index);

    Kart *new_kart         = new Kart(prof_kart_id, index+1, init_pos);

    Controller *controller = loadAIController(new_kart);
    new_kart->setController(controller);

    // Create a camera for the last kart (since this way more of the
    // karts can be seen.
    if (index == (int)race_manager->getNumberOfKarts()-1)
    {
        new_kart->setCamera(new Camera(index, new_kart));
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

    // Now it must be laps based profiling:
    return race_manager->getFinishedKarts()==getNumKarts();
}   // isRaceOver

//-----------------------------------------------------------------------------
/** Counts the number of framces.
 */
void ProfileWorld::update(float dt)
{
    StandardRace::update(dt);
    m_frame_count++;
    video::IVideoDriver *driver = irr_driver->getVideoDriver();
    io::IAttributes   *attr = irr_driver->getSceneManager()->getParameters();
    m_num_triangles    += (int)(driver->getPrimitiveCountDrawn( 0 ) * ( 1.f / 1000.f ));
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
        if (m_karts[i]->hasFinishedRace() || m_karts[i]->isEliminated()) continue;
        m_karts[i]->finishedRace(estimateFinishTimeForKart(m_karts[i]));
    }

    float runtime = (irr_driver->getRealTime()-m_start_time)*0.001f;
    printf("Number of frames: %d time %f, Average FPS: %f\n",
           m_frame_count, runtime, (float)m_frame_count/runtime);
    printf("Average # drawn nodes           %f k\n",
            (float)m_num_triangles/m_frame_count);
    printf("Average # culled nodes:         %f k\n",
            (float)m_num_culls/m_frame_count);
    printf("Average # solid nodes:          %f k\n",
            (float)m_num_solid/m_frame_count);
    printf("Average # transparent nodes:    %f\n",
            (float)m_num_transparent/m_frame_count);
    printf("Average # transp. effect nodes: %f\n",
            (float)m_num_trans_effect/m_frame_count);

    float min_t=999999.9f, max_t=0.0, av_t=0.0;
    for ( KartList::size_type i = 0; i < m_karts.size(); ++i)
    {
        max_t = std::max(max_t, m_karts[i]->getFinishTime());
        min_t = std::min(min_t, m_karts[i]->getFinishTime());
        if(m_profile_mode==PROFILE_TIME)
            av_t += getTime();
        else
            av_t += m_karts[i]->getFinishTime();
        printf("%ls  start %d  end %d time %f\n",
            m_karts[i]->getName().c_str(),1+(int)i,
            m_karts[i]->getPosition(),
            m_karts[i]->getFinishTime());
    }
    printf("min %f  max %f  av %f\n",min_t, max_t, av_t/m_karts.size());

    std::exit(-2);
}   // enterRaceOverState
