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
#include "guiengine/message_queue.hpp"
#include "karts/ghost_kart.hpp"
#include "karts/kart_gfx.hpp"
#include "modes/world.hpp"
#include "physics/btKart.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"

#include <algorithm>
#include <stdio.h>
#include <string>
#include <karts/controller/player_controller.hpp>

ReplayRecorder *ReplayRecorder::m_replay_recorder = NULL;

//-----------------------------------------------------------------------------
/** Initialises the Replay engine
 */
ReplayRecorder::ReplayRecorder()
{
    m_complete_replay = false;
    m_incorrect_replay = false;
}   // ReplayRecorder

//-----------------------------------------------------------------------------
/** Frees all stored data. */
ReplayRecorder::~ReplayRecorder()
{
}   // ~Replay

//-----------------------------------------------------------------------------
/** Reset the replay recorder. */
void ReplayRecorder::reset()
{
    m_complete_replay = false;
    m_incorrect_replay = false;
    m_transform_events.clear();
    m_physic_info.clear();
    m_kart_replay_event.clear();
    m_count_transforms.clear();
    m_last_saved_time.clear();

#ifdef DEBUG
    m_count                       = 0;
    m_count_skipped_time          = 0;
    m_count_skipped_interpolation = 0;
#endif
}   // clear

//-----------------------------------------------------------------------------
/** Initialise the replay recorder. It especially allocates memory
 *  to store the replay data.
 */
void ReplayRecorder::init()
{
    reset();
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

    m_count_transforms.resize(race_manager->getNumberOfKarts(), 0);
    m_last_saved_time.resize(race_manager->getNumberOfKarts(), -1.0f);

}   // init

//-----------------------------------------------------------------------------
/** Saves the current replay data.
 *  \param dt Time step size.
 */
void ReplayRecorder::update(float dt)
{
    if (m_incorrect_replay || m_complete_replay) return;

    World *world = World::getWorld();
    const bool single_player = race_manager->getNumPlayers() == 1;
    unsigned int num_karts = world->getNumKarts();

    float time = world->getTime();
    for(unsigned int i=0; i<num_karts; i++)
    {
        AbstractKart *kart = world->getKart(i);
        // If a single player give up in game menu, stop recording
        if (kart->isEliminated() && single_player) return;

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
                m_incorrect_replay = single_player;
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

        kart->getKartGFX()->getGFXStatus(&(r->m_nitro_usage),
            &(r->m_zipper_usage), &(r->m_skidding_state), &(r->m_red_skidding));
        r->m_jumping = kart->isJumping();
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
        MessageQueue::add(MessageQueue::MT_ERROR,
            _("Incomplete replay file will not be saved."));
        return;
    }

#ifdef DEBUG
    Log::debug("ReplayRecorder", "%d frames, %d removed because of"
        "frequency compression", m_count, m_count_skipped_time);
#endif
    const World *world           = World::getWorld();
    const unsigned int num_karts = world->getNumKarts();
    float min_time = 99999.99f;
    for (unsigned int k = 0; k < num_karts; k++)
    {
        if (world->getKart(k)->isGhostKart()) continue;
        float cur_time = world->getKart(k)->getFinishTime();
        if (cur_time < min_time)
            min_time = cur_time;
    }

    int day, month, year;
    StkTime::getDate(&day, &month, &year);
    std::string time = StringUtils::toString(min_time);
    std::replace(time.begin(), time.end(), '.', '_');
    std::ostringstream oss;
    oss << Track::getCurrentTrack()->getIdent() << "_" << year << month << day
        << "_" << num_karts << "_" << time << ".replay";
    m_filename = oss.str();

    FILE *fd = openReplayFile(/*writeable*/true);
    if (!fd)
    {
        Log::error("ReplayRecorder", "Can't open '%s' for writing - "
            "can't save replay data.", getReplayFilename().c_str());
        return;
    }

    core::stringw msg = _("Replay saved in \"%s\".",
        (file_manager->getReplayDir() + getReplayFilename()).c_str());
    MessageQueue::add(MessageQueue::MT_GENERIC, msg);

    fprintf(fd, "version: %d\n",    getReplayVersion());
    for (unsigned int real_karts = 0; real_karts < num_karts; real_karts++)
    {
        const AbstractKart *kart = world->getKart(real_karts);
        if (kart->isGhostKart()) continue;

        // XML encode the username to handle Unicode
        fprintf(fd, "kart: %s %s\n", kart->getIdent().c_str(),
                StringUtils::xmlEncode(kart->getController()->getName()).c_str());
    }

    fprintf(fd, "kart_list_end\n");
    fprintf(fd, "reverse: %d\n",    (int)race_manager->getReverseTrack());
    fprintf(fd, "difficulty: %d\n", race_manager->getDifficulty());
    fprintf(fd, "track: %s\n",      Track::getCurrentTrack()->getIdent().c_str());
    fprintf(fd, "laps: %d\n",       race_manager->getNumLaps());
    fprintf(fd, "min_time: %f\n",   min_time);

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
            fprintf(fd, "%f  %f %f %f  %f %f %f %f  %f  %f  %f %f %f %f  %d %d %d %d %d\n",
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
                    r->m_nitro_usage,
                    (int)r->m_zipper_usage,
                    r->m_skidding_state,
                    (int)r->m_red_skidding,
                    (int)r->m_jumping
                );
        }   // for i
    }
    fclose(fd);
}   // save
