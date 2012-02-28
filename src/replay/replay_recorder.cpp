//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012 Joerg Henrichs
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
    for(unsigned int i=0; i<race_manager->getNumberOfKarts(); i++)
    {
        m_transform_events[i].resize(stk_config->m_max_history);
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
    World *world = World::getWorld();
    unsigned int num_karts = world->getNumKarts();

    float time = world->getTime();
    // Once we use interpolate results, we don't have to increase
    // m_next by num_karts, so count how often to increase
    unsigned int count = 0;
    for(unsigned int i=0; i<num_karts; i++)
    {
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

        m_count_transforms[i]++;
        const Kart *kart = world->getKart(i);
        if(m_count_transforms[i]>=m_transform_events[i].size())
        {
            // Only print this message once.
            if(m_count_transforms[i]==m_transform_events[i].size())
                printf("Can't store more events for kart %s.\n",
                        kart->getIdent().c_str());
            continue;
        }
        TransformEvent *p = &(m_transform_events[i][m_count_transforms[i]-1]);
        p->m_time      = World::getWorld()->getTime();
        p->m_transform = kart->getTrans();
    }   // for i
}   // updateRecording

//-----------------------------------------------------------------------------
/** Saves the replay data stored in the internal data structures.
 */
void ReplayRecorder::Save()
{
    FILE *fd = openReplayFile(/*writeable*/true);
    if(!fd)
    {
        printf("Can't open '%s' for writing - can't save replay data.\n",
            getReplayFilename().c_str());
        return;
    }

    printf("Replay saved in '%s'.\n", getReplayFilename().c_str());

    World *world   = World::getWorld();
    unsigned int num_karts = world->getNumKarts();
    fprintf(fd, "Version:  %d\n",   getReplayVersion());
    fprintf(fd, "difficulty: %d\n", race_manager->getDifficulty());
    fprintf(fd, "track: %s\n",      world->getTrack()->getIdent().c_str());
    fprintf(fd, "Laps: %d\n",       race_manager->getNumLaps());
    fprintf(fd, "numkarts: %d\n",   num_karts);

    for(unsigned int k=0; k<num_karts; k++)
    {
        fprintf(fd, "model %d: %s\n",k, world->getKart(k)->getIdent().c_str());
        fprintf(fd, "size:     %d\n", m_count_transforms[k]);

        for(unsigned int i=0; i<m_count_transforms[k]; i++)
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
        }   // for k
    }
    fprintf(fd, "Replay file end.\n");
    fclose(fd);
}   // Save

