//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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

#include "replay/replay_recorder.hpp"

#include "config/stk_config.hpp"
#include "io/file_manager.hpp"
#include "karts/ghost_kart.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"

#include <algorithm>
#include <stdio.h>
#include <string>

ReplayRecorder *ReplayRecorder::m_replay_recorder = NULL;

//-----------------------------------------------------------------------------
/** Initialises the Replay engine
 */
ReplayRecorder::ReplayRecorder()
{
}   // ReplayRecorder

//-----------------------------------------------------------------------------
/** Frees all stored data. */
ReplayRecorder::~ReplayRecorder()
{
    m_transform_events.clear();
}   // ~Replay

//-----------------------------------------------------------------------------
/** Initialise the replay recorder. It especially allocates memory
 *  to store the replay data.
 */
void ReplayRecorder::init()
{
    m_transform_events.clear();
    m_transform_events.resize(race_manager->getNumberOfKarts());
    m_skid_control.resize(race_manager->getNumberOfKarts());
    m_kart_replay_event.resize(race_manager->getNumberOfKarts());
    unsigned int max_frames = (unsigned int)(  stk_config->m_replay_max_time
                                             / stk_config->m_replay_dt);
    for(unsigned int i=0; i<race_manager->getNumberOfKarts(); i++)
    {
        m_transform_events[i].resize(max_frames);
        // Rather arbitraritly sized, it will be added with push_back
        m_kart_replay_event[i].reserve(500);
    }
    m_count_transforms.clear();
    m_count_transforms.resize(race_manager->getNumberOfKarts(), 0);
    m_last_saved_time.clear();
    m_last_saved_time.resize(race_manager->getNumberOfKarts(), -1.0f);

#ifdef DEBUG
    m_count                       = 0;
    m_count_skipped_time          = 0;
    m_count_skipped_interpolation = 0;
#endif
}   // init

//-----------------------------------------------------------------------------
/** Resets all ghost karts back to start position.
 */
void ReplayRecorder::reset()
{
}   // reset

//-----------------------------------------------------------------------------
/** Saves the current replay data.
 *  \param dt Time step size.
 */
void ReplayRecorder::update(float dt)
{
    const World *world = World::getWorld();
    unsigned int num_karts = world->getNumKarts();

    float time = world->getTime();
    // Once we use interpolate results, we don't have to increase
    // m_next by num_karts, so count how often to increase

    for(unsigned int i=0; i<num_karts; i++)
    {
        const AbstractKart *kart = world->getKart(i);

        // Check if skidding state has changed. If so, store this
        if(kart->getControls().m_skid != m_skid_control[i])
        {
            KartReplayEvent kre;
            kre.m_time = World::getWorld()->getTime();
            if(kart->getControls().m_skid==KartControl::SC_LEFT)
                kre.m_type = KartReplayEvent::KRE_SKID_LEFT;
            else if(kart->getControls().m_skid==KartControl::SC_RIGHT)
                kre.m_type = KartReplayEvent::KRE_SKID_RIGHT;
            else
                kre.m_type = KartReplayEvent::KRE_NONE;
            m_kart_replay_event[i].push_back(kre);
            if(m_skid_control[i]!=KartControl::SC_NONE)
                m_skid_control[i] = KartControl::SC_NONE;
            else
                m_skid_control[i] = kart->getControls().m_skid;
        }
#ifdef DEBUG
        m_count ++;
#endif
        if(time - m_last_saved_time[i]<stk_config->m_replay_dt)
        {
#ifdef DEBUG
            m_count_skipped_time ++;
#endif
            continue;
        }
        m_last_saved_time[i] = time;
        m_count_transforms[i]++;
        if(m_count_transforms[i]>=m_transform_events[i].size())
        {
            // Only print this message once.
            if(m_count_transforms[i]==m_transform_events[i].size())
            {
                char buffer[100];
                sprintf(buffer, "Can't store more events for kart %s.",
                        kart->getIdent().c_str());
                Log::warn("ReplayRecorder", buffer);
            }
            continue;
        }
        TransformEvent *p = &(m_transform_events[i][m_count_transforms[i]-1]);
        p->m_time      = World::getWorld()->getTime();
        p->m_transform.setOrigin(kart->getXYZ());
        p->m_transform.setRotation(kart->getVisualRotation());
    }   // for i
}   // update

//-----------------------------------------------------------------------------
/** Saves the replay data stored in the internal data structures.
 */
void ReplayRecorder::Save()
{
#ifdef DEBUG
    printf("%d frames, %d removed because of frequency compression\n",
           m_count, m_count_skipped_time);
#endif
    FILE *fd = openReplayFile(/*writeable*/true);
    if(!fd)
    {
        Log::error("ReplayRecorder", "Can't open '%s' for writing - can't save replay data.",
                   getReplayFilename().c_str());
        return;
    }

    Log::info("ReplayRecorder", "Replay saved in '%s'.\n", getReplayFilename().c_str());

    World *world   = World::getWorld();
    unsigned int num_karts = world->getNumKarts();
    fprintf(fd, "Version:  %d\n",   getReplayVersion());
    fprintf(fd, "difficulty: %d\n", race_manager->getDifficulty());
    fprintf(fd, "track: %s\n",      world->getTrack()->getIdent().c_str());
    fprintf(fd, "Laps: %d\n",       race_manager->getNumLaps());

    unsigned int max_frames = (unsigned int)(  stk_config->m_replay_max_time
                                             / stk_config->m_replay_dt      );
    for(unsigned int k=0; k<num_karts; k++)
    {
        fprintf(fd, "model: %s\n", world->getKart(k)->getIdent().c_str());
        fprintf(fd, "size:     %d\n", m_count_transforms[k]);

        unsigned int num_transforms = std::min(max_frames,
                                               m_count_transforms[k]);
        for(unsigned int i=0; i<num_transforms; i++)
        {
            const TransformEvent *p=&(m_transform_events[k][i]);
            fprintf(fd, "%f  %f %f %f  %f %f %f %f\n",
                    p->m_time,
                    p->m_transform.getOrigin().getX(),
                    p->m_transform.getOrigin().getY(),
                    p->m_transform.getOrigin().getZ(),
                    p->m_transform.getRotation().getX(),
                    p->m_transform.getRotation().getY(),
                    p->m_transform.getRotation().getZ(),
                    p->m_transform.getRotation().getW()
                );
        }   // for i
        fprintf(fd, "events: %d\n", (int)m_kart_replay_event[k].size());
        for(unsigned int i=0; i<m_kart_replay_event[k].size(); i++)
        {
            const KartReplayEvent *p=&(m_kart_replay_event[k][i]);
            fprintf(fd, "%f %d\n", p->m_time, p->m_type);
        }
    }
    fclose(fd);
}   // Save

