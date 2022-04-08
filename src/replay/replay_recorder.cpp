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
#include "items/powerup.hpp"
#include "items/powerup_manager.hpp"
#include "guiengine/message_queue.hpp"
#include "karts/controller/player_controller.hpp"
#include "karts/ghost_kart.hpp"
#include "karts/skidding.hpp"
#include "karts/kart_gfx.hpp"
#include "modes/easter_egg_hunt.hpp"
#include "modes/linear_world.hpp"
#include "modes/world.hpp"
#include "physics/btKart.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <algorithm>
#include <stdio.h>
#include <string>
#include <cinttypes>

ReplayRecorder *ReplayRecorder::m_replay_recorder = NULL;

//-----------------------------------------------------------------------------
/** Initialises the Replay engine
 */
ReplayRecorder::ReplayRecorder()
{
    m_complete_replay = false;
    m_incorrect_replay = false;
    m_previous_steer   = 0.0f;

    assert(stk_config->m_replay_max_frames >= 0);
    m_max_frames = stk_config->m_replay_max_frames;
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
    m_transform_events.resize(RaceManager::get()->getNumberOfKarts());
    m_physic_info.resize(RaceManager::get()->getNumberOfKarts());
    m_bonus_info.resize(RaceManager::get()->getNumberOfKarts());
    m_kart_replay_event.resize(RaceManager::get()->getNumberOfKarts());

    for(unsigned int i=0; i<RaceManager::get()->getNumberOfKarts(); i++)
    {
        m_transform_events[i].resize(m_max_frames);
        m_physic_info[i].resize(m_max_frames);
        m_bonus_info[i].resize(m_max_frames);
        m_kart_replay_event[i].resize(m_max_frames);
    }

    m_count_transforms.resize(RaceManager::get()->getNumberOfKarts(), 0);
    m_last_saved_time.resize(RaceManager::get()->getNumberOfKarts(), -1.0f);

}   // init

//-----------------------------------------------------------------------------
/** Saves the current replay data.
 *  \param ticks Number of physics time steps - should be 1.
 */
void ReplayRecorder::update(int ticks)
{
    if (m_incorrect_replay || m_complete_replay) return;

    World *world = World::getWorld();
    const bool single_player = RaceManager::get()->getNumPlayers() == 1;
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

        // Don't save directly the enum value, because any change
        // to it would break the reading of old replays
        int attachment = enumToCode(kart->getAttachment()->getType());
        int powerup_type = enumToCode(kart->getPowerup()->getType());
        int special_value = 0;

        // In egg hunt mode, use store the number of eggs found so far
        // This assumes that egg hunt mode is only available in single-player
        if (RaceManager::get()->isEggHuntMode())
        {
            EasterEggHunt *easterworld = dynamic_cast<EasterEggHunt*>(World::getWorld());
            special_value = easterworld->numberOfEggsFound();
        }

        if (attachment == -1)
        {
            Log::error("ReplayRecorder", "Unknown attachment type");
            return;
        }
        if (powerup_type == -1)
        {
            Log::error("ReplayRecorder", "Unknown powerup type");
            return;
        }

        if (m_count_transforms[i] >= 2)
        {
            BonusInfo *b_prev       = &(m_bonus_info[i][m_count_transforms[i]-1]);
            BonusInfo *b_prev2      = &(m_bonus_info[i][m_count_transforms[i]-2]);
            PhysicInfo *q_prev      = &(m_physic_info[i][m_count_transforms[i]-1]);

            // If the kart changes its steering
            if (fabsf(kart->getControls().getSteer() - m_previous_steer) >
                                            stk_config->m_replay_delta_steering)
                force_update = true;

            // If the kart starts or stops skidding
            if (kart->getSkidding()->getSkidState() != q_prev->m_skidding_state)
                force_update = true;
            // If the kart changes speed significantly
            float speed_change = fabsf(kart->getSpeed() - q_prev->m_speed);
            if ( speed_change > stk_config->m_replay_delta_speed )
            {
                if (speed_change > 4*stk_config->m_replay_delta_speed)
                    force_update = true;
                else if (speed_change > 2*stk_config->m_replay_delta_speed &&
                         time - m_last_saved_time[i] > (stk_config->m_replay_dt/8.0f))
                    force_update = true;
                else if (time - m_last_saved_time[i] > (stk_config->m_replay_dt/3.0f))
                    force_update = true;
            }

            // If the attachment has changed
            if (attachment != b_prev->m_attachment)
                  force_update = true;
    
            // If the item amount has changed
            if (kart->getNumPowerup() != b_prev->m_item_amount)
                force_update = true;

            // If the item type has changed
            if (powerup_type != b_prev->m_item_type)
                force_update = true;

            // In egg-hunt mode, if an egg has been collected
            // In battle mode, if a live has been lost/gained
            if (special_value != b_prev->m_special_value)
                force_update = true;

            // If nitro starts being used or is collected
            if (kart->getEnergy() != b_prev->m_nitro_amount &&
                b_prev->m_nitro_amount == b_prev2->m_nitro_amount)
                force_update = true;

            // If nitro stops being used
            // (also generates an extra transform on collection,
            //  should be negligible and better than heavier checks)
            if (kart->getEnergy() == b_prev->m_nitro_amount &&
                b_prev->m_nitro_amount != b_prev2->m_nitro_amount)
                force_update = true;

            // If close to the end of the race, reduce the time step
            // for extra precision
            // TODO : fast updates when close to the last egg in egg hunt
            if (RaceManager::get()->isLinearRaceMode())
            {
                float full_distance = RaceManager::get()->getNumLaps()
                        * Track::getCurrentTrack()->getTrackLength();

                const LinearWorld *linearworld = dynamic_cast<LinearWorld*>(World::getWorld());
                if (full_distance + DISTANCE_MAX_UPDATES >= linearworld->getOverallDistance(i) &&
                    full_distance <= linearworld->getOverallDistance(i) + DISTANCE_FAST_UPDATES)
                {
                    if (fabsf(full_distance - linearworld->getOverallDistance(i)) < DISTANCE_MAX_UPDATES)
                        force_update = true;
                    else if (time - m_last_saved_time[i] > (stk_config->m_replay_dt/5.0f))
                        force_update = true;
                }
            }
        }


        if ( time - m_last_saved_time[i] < (stk_config->m_replay_dt - stk_config->ticks2Time(1)) &&
            !force_update)
        {
#ifdef DEBUG
            m_count_skipped_time++;
#endif
            continue;
        }

        m_previous_steer = kart->getControls().getSteer();
        m_last_saved_time[i] = time;
        m_count_transforms[i]++;
        if (m_count_transforms[i] >= m_transform_events[i].size())
        {
            // Only print this message once.
            if (m_count_transforms[i] == m_transform_events[i].size())
            {
                Log::warn("ReplayRecorder", "Can't store more events for kart %s.",
                    kart->getIdent().c_str());
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
        b->m_item_type         = powerup_type;
        b->m_special_value     = special_value;

        //Only saves distance if recording a linear race
        if (RaceManager::get()->isLinearRaceMode())
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
uint64_t ReplayRecorder::computeUID(float min_time)
{
    uint64_t unique_identifier = 0;

    // First store some basic replay data
    int min_time_uid = (int) (min_time*1000);
    min_time_uid = min_time_uid%60000;

    int day, month, year;
    StkTime::getDate(&day, &month, &year);
    uint64_t date_uid = year%10;
    date_uid = date_uid*12 + (month-1);;
    date_uid = date_uid*31 + (day-1);

    int reverse = RaceManager::get()->getReverseTrack() ? 1 : 0;
    unique_identifier += reverse;
    unique_identifier += RaceManager::get()->getDifficulty()*2;
    unique_identifier += (RaceManager::get()->getNumLaps()-1)*8;
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
        StringUtils::utf8ToWide(file_manager->getReplayDir() + getReplayFilename()));
    MessageQueue::add(MessageQueue::MT_GENERIC, msg);

    fprintf(fd, "version: %d\n", getCurrentReplayVersion());
    fprintf(fd, "stk_version: %s\n", STK_VERSION);

    unsigned int player_count = 0;
    for (unsigned int real_karts = 0; real_karts < num_karts; real_karts++)
    {
        const AbstractKart *kart = world->getKart(real_karts);
        if (kart->isGhostKart()) continue;

        // XML encode the username to handle Unicode
        fprintf(fd, "kart: %s %s\n", kart->getIdent().c_str(),
                StringUtils::xmlEncode(kart->getController()->getName()).c_str());

        if (kart->getController()->isPlayerController())
        {
            fprintf(fd, "kart_color: %f\n", StateManager::get()->getActivePlayer(player_count)->getConstProfile()->getDefaultKartColor());
            player_count++;
        }
        else
            fprintf(fd, "kart_color: 0\n");
    }

    m_last_uid = computeUID(min_time);

    int num_laps = RaceManager::get()->getNumLaps();
    if (num_laps == 9999) num_laps = 0; // no lap in that race mode

    fprintf(fd, "kart_list_end\n");
    fprintf(fd, "reverse: %d\n",    (int)RaceManager::get()->getReverseTrack());
    fprintf(fd, "difficulty: %d\n", RaceManager::get()->getDifficulty());
    fprintf(fd, "mode: %s\n",       RaceManager::get()->getMinorModeName().c_str());
    fprintf(fd, "track: %s\n",      Track::getCurrentTrack()->getIdent().c_str());
    fprintf(fd, "laps: %d\n",       num_laps);
    fprintf(fd, "min_time: %f\n",   min_time);
    fprintf(fd, "replay_uid: %" PRIu64 "\n", m_last_uid);

    for (unsigned int k = 0; k < num_karts; k++)
    {
        if (world->getKart(k)->isGhostKart()) continue;

        const unsigned int num_transforms = std::min(m_max_frames,
                                                     m_count_transforms[k]);

        fprintf(fd, "size:     %d\n", num_transforms);

        for (unsigned int i = 0; i < num_transforms; i++)
        {
            const TransformEvent *p  = &(m_transform_events[k][i]);
            const PhysicInfo *q      = &(m_physic_info[k][i]);
            const BonusInfo *b      = &(m_bonus_info[k][i]);
            const KartReplayEvent *r = &(m_kart_replay_event[k][i]);
            fprintf(fd, "%f  %f %f %f  %f %f %f %f  %f  %f  %f %f %f %f %d  %d %f %d %d %d  %f %d %d %d %d %d\n",
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
                    b->m_item_type,
                    b->m_special_value,
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

/* Returns an encoding value for a given attachment type.
 * The internal values of the enum for attachments may change if attachments
 * are introduced, removed or even reordered. To avoid compatibility issues
 * with previous replay files, we add a layer to encode an enum semantical
 * value the same way, independently of its internal value.
 * \param type : the type of attachment to encode */

int ReplayRecorder::enumToCode (Attachment::AttachmentType type)
{
    int code =
        (type == Attachment::ATTACH_NOTHING)          ? 0 :
        (type == Attachment::ATTACH_PARACHUTE)        ? 1 :
        (type == Attachment::ATTACH_ANVIL)            ? 2 :
        (type == Attachment::ATTACH_BOMB)             ? 3 :
        (type == Attachment::ATTACH_SWATTER)          ? 4 :
        (type == Attachment::ATTACH_BUBBLEGUM_SHIELD) ? 5 :
                                                       -1 ;

    return code;
} // enumToCode

/* Returns an encoding value for a given item type
 * \param type : the type of item to encode */

int ReplayRecorder::enumToCode (PowerupManager::PowerupType type)
{
    int code =
        (type == PowerupManager::POWERUP_NOTHING)    ? 0 :
        (type == PowerupManager::POWERUP_BUBBLEGUM)  ? 1 :
        (type == PowerupManager::POWERUP_CAKE)       ? 2 :
        (type == PowerupManager::POWERUP_BOWLING)    ? 3 :
        (type == PowerupManager::POWERUP_ZIPPER)     ? 4 :
        (type == PowerupManager::POWERUP_PLUNGER)    ? 5 :
        (type == PowerupManager::POWERUP_SWITCH)     ? 6 :
        (type == PowerupManager::POWERUP_SWATTER)    ? 7 :
        (type == PowerupManager::POWERUP_RUBBERBALL) ? 8 :
        (type == PowerupManager::POWERUP_PARACHUTE)  ? 9 :
                                                      -1 ;

    return code;
} // enumToCode

/* Returns the attachment enum value for a given replay code */
Attachment::AttachmentType ReplayRecorder::codeToEnumAttach (int code)
{
    Attachment::AttachmentType type =
        (code == 0) ? Attachment::ATTACH_NOTHING          :
        (code == 1) ? Attachment::ATTACH_PARACHUTE        :
        (code == 2) ? Attachment::ATTACH_ANVIL            :
        (code == 3) ? Attachment::ATTACH_BOMB             :
        (code == 4) ? Attachment::ATTACH_SWATTER          :
        (code == 5) ? Attachment::ATTACH_BUBBLEGUM_SHIELD :
                      Attachment::ATTACH_NOTHING ;

    return type;
} // codeToEnumAttach

/* Returns the item enum value for a given replay code */
PowerupManager::PowerupType ReplayRecorder::codeToEnumItem (int code)
{
    PowerupManager::PowerupType type =
        (code == 0) ? PowerupManager::POWERUP_NOTHING    :
        (code == 1) ? PowerupManager::POWERUP_BUBBLEGUM  :
        (code == 2) ? PowerupManager::POWERUP_CAKE       :
        (code == 3) ? PowerupManager::POWERUP_BOWLING    :
        (code == 4) ? PowerupManager::POWERUP_ZIPPER     :
        (code == 5) ? PowerupManager::POWERUP_PLUNGER    :
        (code == 6) ? PowerupManager::POWERUP_SWITCH     :
        (code == 7) ? PowerupManager::POWERUP_SWATTER    :
        (code == 8) ? PowerupManager::POWERUP_RUBBERBALL :
        (code == 9) ? PowerupManager::POWERUP_PARACHUTE  :
                      PowerupManager::POWERUP_NOTHING ;

    return type;
} // codeToEnumItem
