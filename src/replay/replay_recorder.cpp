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
#include "items/attachment.hpp"
#include "guiengine/message_queue.hpp"
#include "karts/ghost_kart.hpp"
#include "karts/skidding.hpp"
#include "karts/kart_gfx.hpp"
#include "modes/linear_world.hpp"
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

    // Give margin to store extra events due to higher precision
    // when a kart's tracked charateristic has suddenly changed
    // in a non-interpolable way.
    m_max_frames = (unsigned int)( FRAME_MARGIN_FOR_FORCED_UPDATES
                                   * stk_config->m_replay_max_time
                                   / stk_config->m_replay_dt);
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
    m_bonus_info.clear();
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
    m_bonus_info.resize(race_manager->getNumberOfKarts());
    m_kart_replay_event.resize(race_manager->getNumberOfKarts());

    for(unsigned int i=0; i<race_manager->getNumberOfKarts(); i++)
    {
        m_transform_events[i].resize(m_max_frames);
        m_physic_info[i].resize(m_max_frames);
        m_bonus_info[i].resize(m_max_frames);
        m_kart_replay_event[i].resize(m_max_frames);
    }

    m_count_transforms.resize(race_manager->getNumberOfKarts(), 0);
    m_last_saved_time.resize(race_manager->getNumberOfKarts(), -1.0f);

}   // init

//-----------------------------------------------------------------------------
/** Saves the current replay data.
 *  \param ticks Number of physics time steps - should be 1.
 */
void ReplayRecorder::update(int ticks)
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
        // If one of the tracked kart data has significantly changed
        // for the kart, update sooner than the usual dt
        bool force_update = false;

        int attachment = -1;
        if (kart->getAttachment()->getType() == Attachment::ATTACH_NOTHING)
            attachment = 0;
        else if (kart->getAttachment()->getType() == Attachment::ATTACH_PARACHUTE)
            attachment = 1;
        else if (kart->getAttachment()->getType() == Attachment::ATTACH_ANVIL)
            attachment = 2;
        else if (kart->getAttachment()->getType() == Attachment::ATTACH_BOMB)
            attachment = 3;
        else if (kart->getAttachment()->getType() == Attachment::ATTACH_SWATTER)
            attachment = 4;
        else if (kart->getAttachment()->getType() == Attachment::ATTACH_BUBBLEGUM_SHIELD)
            attachment = 5;

        if (attachment == -1)
        {
            Log::error("ReplayRecorder", "Unknown attachment type");
            return;
        }

        if (m_count_transforms[i] >= 2)
        {
            BonusInfo *b_prev       = &(m_bonus_info[i][m_count_transforms[i]-1]);
            BonusInfo *b_prev2      = &(m_bonus_info[i][m_count_transforms[i]-2]);
            PhysicInfo *q_prev      = &(m_physic_info[i][m_count_transforms[i]-1]);

            // If the kart starts or stops skidding
            if (kart->getSkidding()->getSkidState() != q_prev->m_skidding_state)
                force_update = true;

            // If the kart changes speed brutally
            // (booster, crash...)
            if (fabsf(kart->getSpeed() - q_prev->m_speed) > 1.0f )
                force_update = true;

            // If the attachment has changed
            if (attachment != b_prev->m_attachment)
                force_update = true;
    
            // If the item amount has changed
            if (kart->getNumPowerup() != b_prev->m_item_amount)
                force_update = true;

            // If nitro starts being used or is collected
            if (kart->getEnergy() != b_prev->m_nitro_amount &&
                b_prev->m_nitro_amount == b_prev2->m_nitro_amount)
                force_update = true;

            // If nitro stops being used
            // (also generate an extra transform on collection,
            //  should be negligble and better than heavier checks)
            if (kart->getEnergy() == b_prev->m_nitro_amount &&
                b_prev->m_nitro_amount != b_prev2->m_nitro_amount)
                force_update = true;

            // If close to the end of the race, reduce the time step
            // for extra precision for the whole race time
            if (race_manager->isLinearRaceMode())
            {
                float full_distance = race_manager->getNumLaps()
                        * Track::getCurrentTrack()->getTrackLength();

                const LinearWorld *linearworld = dynamic_cast<LinearWorld*>(World::getWorld());
                if (full_distance + DISTANCE_MAX_UPDATES >= linearworld->getOverallDistance(i) &&
                    full_distance <= linearworld->getOverallDistance(i) + DISTANCE_FAST_UPDATES)
                {
                    if (fabsf(full_distance - linearworld->getOverallDistance(i)) < DISTANCE_MAX_UPDATES)
                        force_update = true;
                    else if (time - m_last_saved_time[i] < (stk_config->m_replay_dt/2.0f))
                        force_update = true;
                }
            }
        }


        if (time - m_last_saved_time[i] < stk_config->m_replay_dt &&
            !force_update)
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
        BonusInfo *b           = &(m_bonus_info[i][m_count_transforms[i]-1]);
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
        q->m_skidding_state    = kart->getSkidding()->getSkidState();

        b->m_attachment        = attachment;
        b->m_nitro_amount      = kart->getEnergy();
        b->m_item_amount       = kart->getNumPowerup();

        //Only saves distance if recording a linear race
        if (race_manager->isLinearRaceMode())
        {
            const LinearWorld *linearworld = dynamic_cast<LinearWorld*>(World::getWorld());
            r->m_distance = linearworld->getOverallDistance(kart->getWorldKartId());
        }
        else
            r->m_distance = 0.0f;

        kart->getKartGFX()->getGFXStatus(&(r->m_nitro_usage),
            &(r->m_zipper_usage), &(r->m_skidding_effect), &(r->m_red_skidding));
        r->m_jumping = kart->isJumping();
    }   // for i

    if (world->getPhase() == World::RESULT_DISPLAY_PHASE && !m_complete_replay)
    {
        m_complete_replay = true;
        save();
    }
}   // update

//-----------------------------------------------------------------------------
/** Compute the replay's UID ; partly based on race data ; partly randomly
 */
unsigned long long int ReplayRecorder::computeUID(float min_time)
{
    unsigned long long int unique_identifier = 0;

    // First store some basic replay data
    int min_time_uid = (int) (min_time*1000);
    min_time_uid = min_time_uid%60000;

    int day, month, year;
    StkTime::getDate(&day, &month, &year);
    unsigned long long int date_uid = year%10;
    date_uid = date_uid*12 + (month-1);;
    date_uid = date_uid*31 + (day-1);

    int reverse = race_manager->getReverseTrack() ? 1 : 0;
    unique_identifier += reverse;
    unique_identifier += race_manager->getDifficulty()*2;
    unique_identifier += (race_manager->getNumLaps()-1)*8;
    unique_identifier += min_time_uid*160;
    unique_identifier += date_uid*9600000;

    // Add a random value to make sure the identifier is unique
    // and use it to make the non-random part non-obvious
    // using magic and arbitrary constants

    int random = rand()%9998 + 2; //avoid 0 and 1
    unique_identifier += random*47;
    unique_identifier *= random*10000;
    unique_identifier += (10000-random);

    return unique_identifier;
}

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

    fprintf(fd, "version: %d\n", getCurrentReplayVersion());
    fprintf(fd, "stk_version: %s\n", STK_VERSION);
    for (unsigned int real_karts = 0; real_karts < num_karts; real_karts++)
    {
        const AbstractKart *kart = world->getKart(real_karts);
        if (kart->isGhostKart()) continue;

        // XML encode the username to handle Unicode
        fprintf(fd, "kart: %s %s\n", kart->getIdent().c_str(),
                StringUtils::xmlEncode(kart->getController()->getName()).c_str());
    }

    m_last_uid = computeUID(min_time);

    fprintf(fd, "kart_list_end\n");
    fprintf(fd, "reverse: %d\n",    (int)race_manager->getReverseTrack());
    fprintf(fd, "difficulty: %d\n", race_manager->getDifficulty());
    fprintf(fd, "track: %s\n",      Track::getCurrentTrack()->getIdent().c_str());
    fprintf(fd, "laps: %d\n",       race_manager->getNumLaps());
    fprintf(fd, "min_time: %f\n",   min_time);
    #ifdef _WIN32
        /* format string for Windows */
        #define ULONGLONG "%I64u"
    #else
        #define ULONGLONG "%Lu"
    #endif
    fprintf(fd, "replay_uid: " ULONGLONG "\n", m_last_uid);

    for (unsigned int k = 0; k < num_karts; k++)
    {
        if (world->getKart(k)->isGhostKart()) continue;
        fprintf(fd, "size:     %d\n", m_count_transforms[k]);

        unsigned int num_transforms = std::min(m_max_frames,
                                               m_count_transforms[k]);
        for (unsigned int i = 0; i < num_transforms; i++)
        {
            const TransformEvent *p  = &(m_transform_events[k][i]);
            const PhysicInfo *q      = &(m_physic_info[k][i]);
            const BonusInfo *b      = &(m_bonus_info[k][i]);
            const KartReplayEvent *r = &(m_kart_replay_event[k][i]);
            fprintf(fd, "%f  %f %f %f  %f %f %f %f  %f  %f  %f %f %f %f %d  %d %f %d  %f %d %d %d %d %d\n",
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
                    q->m_skidding_state,
                    b->m_attachment,
                    b->m_nitro_amount,
                    b->m_item_amount,
                    r->m_distance,
                    r->m_nitro_usage,
                    (int)r->m_zipper_usage,
                    r->m_skidding_effect,
                    (int)r->m_red_skidding,
                    (int)r->m_jumping
                );
        }   // for i
    }
    fclose(fd);
}   // save

