//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015  Joerg Henrichs
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

#include "items/attachment.hpp"
#include "items/powerup.hpp"
#include "karts/ghost_kart.hpp"
#include "karts/controller/ghost_controller.hpp"
#include "karts/kart_gfx.hpp"
#include "karts/kart_model.hpp"
#include "modes/easter_egg_hunt.hpp"
#include "modes/linear_world.hpp"
#include "modes/world.hpp"
#include "replay/replay_recorder.hpp"
#include "tracks/track.hpp"

#include "LinearMath/btQuaternion.h"

#include <ge_render_info.hpp>

GhostKart::GhostKart(const std::string& ident, unsigned int world_kart_id,
                     int position, float color_hue,
                     const ReplayPlay::ReplayData& rd)
          : Kart(ident, world_kart_id,
                 position, btTransform(btQuaternion(0, 0, 0, 1)),
                 HANDICAP_NONE,
                 std::make_shared<GE::GERenderInfo>(color_hue, true/*transparent*/)),
                 m_replay_data(rd), m_last_egg_idx(0)
{
    m_graphical_y_offset = 0;
    m_finish_computed = false;
}   // GhostKart

// ----------------------------------------------------------------------------
void GhostKart::reset()
{
    m_node->setVisible(true);
    Kart::reset();
    // This will set the correct start position
    update(0);
    updateGraphics(0);
    m_last_egg_idx = 0;
}   // reset

// ----------------------------------------------------------------------------
void GhostKart::addReplayEvent(float time,
                               const btTransform &trans,
                               const ReplayBase::PhysicInfo &pi,
                               const ReplayBase::BonusInfo &bi,
                               const ReplayBase::KartReplayEvent &kre)
{
    GhostController* gc = dynamic_cast<GhostController*>(getController());
    gc->addReplayTime(time);

    m_all_transform.push_back(trans);
    m_all_physic_info.push_back(pi);
    m_all_bonus_info.push_back(bi);
    m_all_replay_events.push_back(kre);

    // Use first frame of replay to calculate default suspension
    if (m_all_physic_info.size() == 1)
    {
        float f = 0;
        for (int i = 0; i < 4; i++)
            f += m_all_physic_info[0].m_suspension_length[i];
        m_graphical_y_offset = -f / 4 + getKartModel()->getLowestPoint();
        m_kart_model->setDefaultSuspension();
    }

}   // addReplayEvent

// ----------------------------------------------------------------------------
/** Called once per rendered frame. It is used to only update any graphical
 *  effects.
 *  \param dt Time step size (since last call).
 */
void GhostKart::updateGraphics(float dt)
{
    Vec3 center_shift(0, m_graphical_y_offset, 0);
    center_shift = getTrans().getBasis() * center_shift;

    // Don't call Kart's updateGraphics, since it assumes physics. Instead
    // immediately call Moveable's updateGraphics.
    Moveable::updateSmoothedGraphics(dt);
    Moveable::updateGraphics(center_shift, btQuaternion(0, 0, 0, 1));
    // Also update attachment's graphics
    m_attachment->updateGraphics(dt);
}   // updateGraphics

// ----------------------------------------------------------------------------
/** Updates the current event of the ghost kart using interpolation
 *  \param dt Time step size.
 */
void GhostKart::update(int ticks)
{
    GhostController* gc = dynamic_cast<GhostController*>(getController());
    if (gc == NULL) return;

    gc->update(ticks);
    if (gc->isReplayEnd())
    {
        m_node->setVisible(false);
        getKartGFX()->setGFXInvisible();
        return;
    }

    const unsigned int idx = gc->getCurrentReplayIndex();
    if (!RaceManager::get()->isWatchingReplay())
    {
        if (idx == 0)
        {
            m_node->setVisible(false);
        }
        if (idx == 1)
        {
            // Start showing the ghost when it start racing
            m_node->setVisible(true);
        }
    }

    const float rd         = gc->getReplayDelta();
    assert(idx < m_all_transform.size());

    if (idx >= m_all_transform.size() - 1)
    {
        setXYZ(m_all_transform.back().getOrigin());
        setRotation(m_all_transform.back().getRotation());
    }
    else
    {
        setXYZ((1- rd)*m_all_transform[idx    ].getOrigin()
               +  rd  *m_all_transform[idx + 1].getOrigin() );
        const btQuaternion q = m_all_transform[idx].getRotation()
            .slerp(m_all_transform[idx + 1].getRotation(), rd);
        setRotation(q);
    }

    Moveable::updatePosition();
    float dt = stk_config->ticks2Time(ticks);
    getKartModel()->update(dt, dt*(m_all_physic_info[idx].m_speed),
        m_all_physic_info[idx].m_steer, m_all_physic_info[idx].m_speed,
        /*lean*/0.0f, idx);

    // Attachment management
    // Note that this doesn't get ticks value from replay file,
    // and can't get the exact ticks values when it depends from kart
    // properties. It also doesn't manage the case where a shield is
    // renewed. These inaccuracies are minor as it is used for
    // graphical effect only.

    Attachment::AttachmentType attach_type =
        ReplayRecorder::codeToEnumAttach(m_all_bonus_info[idx].m_attachment);
    int16_t attach_ticks = 0;
    if (attach_type == Attachment::ATTACH_BUBBLEGUM_SHIELD)
        attach_ticks = (int16_t)stk_config->time2Ticks(10);
    else if (attach_type == Attachment::ATTACH_BOMB)
        attach_ticks = (int16_t)stk_config->time2Ticks(30);
    // The replay history will take care of clearing,
    // just make sure it won't expire by itself
    else
        attach_ticks = 32767;

    if (attach_type == Attachment::ATTACH_NOTHING)
        m_attachment->clear();
    // Setting again reinitialize the graphical size of the attachment,
    // so do so only if the type change
    else if (attach_type != m_attachment->getType())
    {
        m_attachment->set(attach_type, attach_ticks, NULL,
            /*set_by_rewind_parachute*/true);
    }

    // So that the attachment's model size is updated
    m_attachment->update(ticks);

    // Nitro and zipper amount (shown in the GUI in watch-only mode)
    m_powerup->reset();

    // Update item amount and type
    PowerupManager::PowerupType item_type =
        ReplayRecorder::codeToEnumItem(m_all_bonus_info[idx].m_item_type); 
    m_powerup->set(item_type, m_all_bonus_info[idx].m_item_amount);

    // Update special values in easter egg and battle modes
    if (RaceManager::get()->isEggHuntMode())
    {
        if (idx > m_last_egg_idx &&
            m_all_bonus_info[idx].m_special_value >
            m_all_bonus_info[m_last_egg_idx].m_special_value)
        {
            EasterEggHunt *world = dynamic_cast<EasterEggHunt*>(World::getWorld());
            assert(world);
            world->collectedEasterEggGhost(getWorldKartId());
            m_last_egg_idx = idx;
        }
    }

    m_collected_energy = (1- rd)*m_all_bonus_info[idx    ].m_nitro_amount
                         +  rd  *m_all_bonus_info[idx + 1].m_nitro_amount;

    // Graphical effects for nitro, zipper and skidding
    getKartGFX()->setGFXFromReplay(m_all_replay_events[idx].m_nitro_usage,
                                   m_all_replay_events[idx].m_zipper_usage,
                                   m_all_replay_events[idx].m_skidding_effect,
                                   m_all_replay_events[idx].m_red_skidding);
    getKartGFX()->update(dt);

    Vec3 front(0, 0, getKartLength()*0.5f);
    m_xyz_front = getTrans()(front);

    if (m_all_replay_events[idx].m_jumping && !m_is_jumping)
    {
        m_is_jumping = true;
        getKartModel()->setAnimation(KartModel::AF_JUMP_START);
    }
    else if (!m_all_replay_events[idx].m_jumping && m_is_jumping)
    {
        m_is_jumping = false;
        getKartModel()->setAnimation(KartModel::AF_DEFAULT);
    }

}   // update

// ----------------------------------------------------------------------------
/** Returns the speed of the kart in meters/second. */
float GhostKart::getSpeed() const
{
    const GhostController* gc =
        dynamic_cast<const GhostController*>(getController());

    unsigned int current_index = gc->getCurrentReplayIndex();
    const float rd             = gc->getReplayDelta();

    assert(gc->getCurrentReplayIndex() < m_all_physic_info.size());

    if (current_index >= m_all_physic_info.size() - 1)
        return m_all_physic_info.back().m_speed;

    return (1-rd)*m_all_physic_info[current_index    ].m_speed
           +  rd *m_all_physic_info[current_index + 1].m_speed;
}   // getSpeed

// ----------------------------------------------------------------------------
/** Compute the time at which the ghost finished the race */
void GhostKart::computeFinishTime()
{
    // In egg hunts, the finish time is the moment at which all egs are collected
    if (RaceManager::get()->isEggHuntMode())
    {
        EasterEggHunt *world = dynamic_cast<EasterEggHunt*>(World::getWorld());
        assert(world);
        int max_eggs = world->numberOfEggsToFind(); 
        m_finish_time = getTimeForEggs(max_eggs);
    }
    else // linear races
    {
        float full_distance = RaceManager::get()->getNumLaps()
                            * Track::getCurrentTrack()->getTrackLength();
        m_finish_time = getTimeForDistance(full_distance);
    }
    m_finish_computed = true;
}  // computeFinishTime

// ------------------------------------------------------------------------
/** Returns the finish time for a ghost kart. */
float GhostKart::getGhostFinishTime()
{
    if (!m_finish_computed)
        computeFinishTime();

    return m_finish_time;
}  // getGhostFinishTime

// ----------------------------------------------------------------------------
/** Returns the time at which the kart was at a given distance.
  * Returns -1.0f if none */
float GhostKart::getTimeForDistance(float distance)
{
    const GhostController* gc =
        dynamic_cast<const GhostController*>(getController());
    
    int current_index = gc->getCurrentReplayIndex();

    // Second, get the current distance
    float current_distance = m_all_replay_events[current_index].m_distance;

    // This determines in which direction we will search a matching frame
    bool search_forward = (current_distance < distance);

    // These are used to compute the time
    // upper_frame is set to current index so that the timer still runs
    // after the ghost has finished the race
    int lower_frame_index = current_index-1;
    unsigned int upper_frame_index = current_index;
    float upper_ratio     = 0.0f;

    // Third, search frame by frame in the good direction
    // Binary search is not used because the kart might go backwards
    // at some point (accident, rescue, wrong interpolation later corrected)
    // The distances are stored in the replay file rather than being
    // dynamically computed. Version 3 replay files thus don't support
    // the live time difference, but this reduces tremendously the overhead
    while (1)
    {
        // If we have reached the end of the replay file without finding the
        // searched distance, break
        if (upper_frame_index >= m_all_replay_events.size() ||
            lower_frame_index < 0 )
            break;

        // The target distance was reached between those two frames
        if (m_all_replay_events[lower_frame_index].m_distance <= distance &&
            m_all_replay_events[upper_frame_index].m_distance >= distance )
        {
            float lower_diff =
                distance - m_all_replay_events[lower_frame_index].m_distance;
            float upper_diff =
                m_all_replay_events[upper_frame_index].m_distance - distance;

            if ((lower_diff + upper_diff) == 0)
                upper_ratio = 0.0f;
            else
                upper_ratio = lower_diff/(lower_diff+upper_diff);
                
            break;
        }

        if (search_forward)
        {
            lower_frame_index++;
            upper_frame_index++;
        }
        else
        {
            lower_frame_index--;
            upper_frame_index--;
        }
    }

    float ghost_time;

    if (upper_frame_index >= m_all_replay_events.size() ||
        lower_frame_index < 0 )
        ghost_time = -1.0f;
    else
        ghost_time = gc->getTimeAtIndex(lower_frame_index)*(1.0f-upper_ratio)
                    +gc->getTimeAtIndex(upper_frame_index)*(upper_ratio);

    return ghost_time;
} // getTimeForDIstance

// ----------------------------------------------------------------------------
/** Returns the smallest time at which the kart had the required number of eggs
  * Returns -1.0f if none */
float GhostKart::getTimeForEggs(int egg_number)
{
    const GhostController* gc =
        dynamic_cast<const GhostController*>(getController());
    
    int current_index = gc->getCurrentReplayIndex();

    // Second, get the current egg number
    int current_eggs = m_all_bonus_info[current_index].m_special_value;

    // This determines in which direction we will search a matching frame
    bool search_forward = (current_eggs < egg_number);

    // This used to compute the time
    int lower_frame_index = current_index-1;
    unsigned int upper_frame_index = current_index;

    // Third, search frame by frame in the good direction
    // A modified binary search would be an optimization
    while (1)
    {
        // If we have reached the end of the replay file without finding the
        // searched distance, break
        if (upper_frame_index >= m_all_bonus_info.size() ||
            lower_frame_index < 0 )
            break;

        // The target distance was reached between those two frames
        if (m_all_bonus_info[lower_frame_index].m_special_value <  egg_number &&
            m_all_bonus_info[upper_frame_index].m_special_value == egg_number)
        {
            break;
        }

        if (search_forward)
        {
            lower_frame_index++;
            upper_frame_index++;
        }
        else
        {
            lower_frame_index--;
            upper_frame_index--;
        }
    }

    float ghost_time;

    if (upper_frame_index >= m_all_bonus_info.size() ||
        lower_frame_index < 0 )
        ghost_time = -1.0f;
    else
        ghost_time = gc->getTimeAtIndex(upper_frame_index);

    return ghost_time;
} // getTimeForEggs
