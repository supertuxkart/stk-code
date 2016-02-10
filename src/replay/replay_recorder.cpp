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
#include "physics/btKart.hpp"
#include "race/race_manager.hpp"
#include "tracks/terrain_info.hpp"
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
    m_physic_info.clear();
    m_kart_replay_event.clear();
}   // ~Replay

//-----------------------------------------------------------------------------
/** Initialise the replay recorder. It especially allocates memory
 *  to store the replay data.
 */
void ReplayRecorder::init()
{
    m_complete_replay = false;
    m_incorrect_replay = false;
    m_transform_events.clear();
    m_physic_info.clear();
    m_kart_replay_event.clear();
    m_transform_events.resize(race_manager->getNumberOfKarts());
    m_physic_info.resize(race_manager->getNumberOfKarts());
    m_kart_replay_event.resize(race_manager->getNumberOfKarts());
    unsigned int max_frames = (unsigned int)(  stk_config->m_replay_max_time
                                             / stk_config->m_replay_dt);
    for(unsigned int i=0; i<race_manager->getNumberOfKarts(); i++)
    {
        m_transform_events[i].resize(max_frames);
        m_physic_info[i].resize(max_frames);
        m_kart_replay_event[i].resize(max_frames);
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
/** Saves the current replay data.
 *  \param dt Time step size.
 */
void ReplayRecorder::update(float dt)
{
    if (m_incorrect_replay || m_complete_replay) return;

    const World *world = World::getWorld();
    unsigned int num_karts = world->getNumKarts();

    float time = world->getTime();
    // Once we use interpolate results, we don't have to increase
    // m_next by num_karts, so count how often to increase

    for(unsigned int i=0; i<num_karts; i++)
    {
        const AbstractKart *kart = world->getKart(i);
        // Don't record give-up race
        if (kart->isEliminated()) return;

        if (kart->isGhostKart()) continue;
#ifdef DEBUG
        m_count++;
#endif
        if (time - m_last_saved_time[i] < stk_config->m_replay_dt)
        {
#ifdef DEBUG
            m_count_skipped_time++;
#endif
            continue;
        }
        m_last_saved_time[i] = time;
        m_count_transforms[i]++;
        if (m_count_transforms[i] >= m_transform_events[i].size())
        {
            // Only print this message once.
            if (m_count_transforms[i] == m_transform_events[i].size())
            {
                char buffer[100];
                sprintf(buffer, "Can't store more events for kart %s.",
                        kart->getIdent().c_str());
                Log::warn("ReplayRecorder", buffer);
                m_incorrect_replay = true;
            }
            continue;
        }
        TransformEvent *p      = &(m_transform_events[i][m_count_transforms[i]-1]);
        PhysicInfo *q          = &(m_physic_info[i][m_count_transforms[i]-1]);
        KartReplayEvent *r     = &(m_kart_replay_event[i][m_count_transforms[i]-1]);

        p->m_time              = World::getWorld()->getTime();
        p->m_transform.setOrigin(kart->getXYZ());
        p->m_transform.setRotation(kart->getVisualRotation());

        q->m_speed             = kart->getSpeed();
        q->m_steer             = kart->getSteerPercent();
        const int num_wheels = kart->getVehicle()->getNumWheels();
        for (int j = 0; j < 4; j++)
        {
            if (j > num_wheels || num_wheels == 0)
                q->m_suspension_length[j] = 0.0f;
            else
            {
                q->m_suspension_length[j] = kart->getVehicle()
                    ->getWheelInfo(j).m_raycastInfo.m_suspensionLength;
            }
        }

        bool nitro = false;
        bool zipper = false;
        const KartControl kc = kart->getControls();
        const Material* m = kart->getTerrainInfo()->getMaterial();
        if (kc.m_nitro && kart->isOnGround() &&
            kart->isOnMinNitroTime() > 0.0f && kart->getEnergy() > 0.0f)
        {
            nitro = true;
        }
        if (m)
        {
            if (m->isZipper() && kart->isOnGround())
                zipper = true;
        }
        if (kc.m_fire &&
            kart->getPowerup()->getType() == PowerupManager::POWERUP_ZIPPER)
        {
            zipper = true;
        }
        r->m_on_nitro = nitro;
        r->m_on_zipper = zipper;
    }   // for i

    if (world->getPhase() == World::RESULT_DISPLAY_PHASE && !m_complete_replay)
    {
        m_complete_replay = true;
        save();
    }
}   // update

//-----------------------------------------------------------------------------
/** Saves the replay data stored in the internal data structures.
 */
void ReplayRecorder::save()
{
    if (m_incorrect_replay || !m_complete_replay)
    {
        Log::warn("ReplayRecorder", "Incomplete replay file will not be saved.");
        return;
    }

#ifdef DEBUG
    printf("%d frames, %d removed because of frequency compression\n",
           m_count, m_count_skipped_time);
#endif
    FILE *fd = openReplayFile(/*writeable*/true);
    if (!fd)
    {
        Log::error("ReplayRecorder", "Can't open '%s' for writing - can't save replay data.",
                   getReplayFilename().c_str());
        return;
    }

    Log::info("ReplayRecorder", "Replay saved in '%s'.\n", getReplayFilename().c_str());

    fprintf(fd, "reverse: %d\n", (int)race_manager->getReverseTrack());
    World *world   = World::getWorld();
    unsigned int num_karts = world->getNumKarts();
    for (unsigned int real_karts = 0; real_karts < num_karts; real_karts++)
    {
        if (world->getKart(real_karts)->isGhostKart()) continue;
        fprintf(fd, "kart: %s\n",
            world->getKart(real_karts)->getIdent().c_str());
    }

    fprintf(fd, "kart_list_end\n");
    fprintf(fd, "version: %d\n",    getReplayVersion());
    fprintf(fd, "difficulty: %d\n", race_manager->getDifficulty());
    fprintf(fd, "track: %s\n",      world->getTrack()->getIdent().c_str());
    fprintf(fd, "laps: %d\n",       race_manager->getNumLaps());

    unsigned int max_frames = (unsigned int)(  stk_config->m_replay_max_time 
                                             / stk_config->m_replay_dt      );
    for (unsigned int k = 0; k < num_karts; k++)
    {
        if (world->getKart(k)->isGhostKart()) continue;
        fprintf(fd, "size:     %d\n", m_count_transforms[k]);

        unsigned int num_transforms = std::min(max_frames,
                                               m_count_transforms[k]);
        for (unsigned int i = 0; i < num_transforms; i++)
        {
            const TransformEvent *p  = &(m_transform_events[k][i]);
            const PhysicInfo *q      = &(m_physic_info[k][i]);
            const KartReplayEvent *r = &(m_kart_replay_event[k][i]);
            fprintf(fd, "%f  %f %f %f  %f %f %f %f  %f  %f  %f %f %f %f  %d  %d\n",
                    p->m_time,
                    p->m_transform.getOrigin().getX(),
                    p->m_transform.getOrigin().getY(),
                    p->m_transform.getOrigin().getZ(),
                    p->m_transform.getRotation().getX(),
                    p->m_transform.getRotation().getY(),
                    p->m_transform.getRotation().getZ(),
                    p->m_transform.getRotation().getW(),
                    q->m_speed,
                    q->m_steer,
                    q->m_suspension_length[0],
                    q->m_suspension_length[1],
                    q->m_suspension_length[2],
                    q->m_suspension_length[3],
                    (int)r->m_on_nitro,
                    (int)r->m_on_zipper
                );
        }   // for i
    }
    fclose(fd);
}   // save
