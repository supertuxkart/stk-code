//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 Joerg Henrichs
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

// done: be able to squash karts
// TODO: use a proportional corrector for avoiding brutal movements
// TODO: make the swatter (and other items) appear and disappear progressively
// done: remove the maximum number of squashes
// TODO: add a swatter music
// TODO: be able to squash items
// TODO: move some constants to KartProperties, use all constants from KartProperties

#include "items/swatter.hpp"

#include "achievements/achievements_status.hpp"
#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "config/player_manager.hpp"
#include "graphics/explosion.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "items/attachment.hpp"
#include "items/attachment_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/explosion_animation.hpp"
#include "karts/kart_properties.hpp"
#include "modes/capture_the_flag.hpp"
#include "network/network_string.hpp"
#include "network/rewind_manager.hpp"

#include <IAnimatedMeshSceneNode.h>

#define SWAT_POS_OFFSET        core::vector3df(0.0, 0.2f, -0.4f)
#define SWAT_ANGLE_PER_SECOND 150.0f
#define SWAT_ANGLE_OFFSET (90.0f + 15.0f)
#define SWATTER_ANIMATION_SPEED 100.0f

// ----------------------------------------------------------------------------
/** Constructor: creates a swatter at a given attachment for a kart. If there
 *  was a bomb attached, it triggers the replace bomb animations.
 *  \param attachment The attachment instance where the swatter is attached to.
 *  \param kart The kart to which the swatter is attached.
 *  \param bomb_ticks Remaining bomb time in ticks, -1 if none.
 *  \param ticks Swatter duration.
 *  \param attachment class attachment from karts.
 */
Swatter::Swatter(Kart *kart, int16_t bomb_ticks, int ticks,
                  Attachment* attachment)
       : AttachmentPlugin(kart, attachment)
{
    m_animation_phase  = SWATTER_AIMING;
    m_discard_now      = false;
    m_target_kart     = NULL;
    m_discard_ticks    = World::getWorld()->getTicksSinceStart() + ticks;
    m_bomb_remaining   = bomb_ticks;
    m_scene_node       = NULL;
    m_bomb_scene_node  = NULL;
    m_current_rotation_angle = 0.0f;
    m_target_dist2     = FLT_MAX;
    m_swatter_duration = stk_config->time2Ticks(
        kart->getKartProperties()->getSwatterDuration());
    if (m_bomb_remaining != -1)
    {
        // There are 40 frames in blender for swatter_anim.blender
        // so 40 / 25 * 120
        m_discard_ticks =
            World::getWorld()->getTicksSinceStart() +
            stk_config->time2Ticks(40.0f / 25.0f);
    }
    m_swat_sound = NULL;
    m_swatter_animation_ticks = 0;
    m_played_swatter_animation = false;
}   // Swatter

// ----------------------------------------------------------------------------
/** Destructor, stops any playing sfx.
 */
Swatter::~Swatter()
{
    if(m_bomb_scene_node)
    {
        irr_driver->removeNode(m_bomb_scene_node);
        m_bomb_scene_node = NULL;
    }
#ifndef SERVER_ONLY
    if (m_swat_sound)
    {
        m_swat_sound->deleteSFX();
    }
#endif
}   // ~Swatter

// ----------------------------------------------------------------------------
void Swatter::updateGraphics(float dt)
{
#ifndef SERVER_ONLY
    if (m_bomb_remaining != -1)
    {
        if (!m_scene_node)
        {
            m_scene_node = m_kart->getAttachment()->getNode();
            m_scene_node->setPosition(SWAT_POS_OFFSET);
            m_scene_node->setMesh(attachment_manager
                ->getMesh(Attachment::ATTACH_SWATTER_ANIM, m_kart->getIdent()));
            m_scene_node->setRotation(core::vector3df(0.0, -180.0, 0.0));
            m_scene_node->setAnimationSpeed(0.9f);
            m_scene_node->setCurrentFrame(0.0f);
            m_scene_node->setLoopMode(false);
        }
        if (!m_bomb_scene_node)
        {
            m_bomb_scene_node = irr_driver->addAnimatedMesh(
                attachment_manager->getMesh(Attachment::ATTACH_BOMB, m_kart->getIdent()), "bomb");
#ifdef DEBUG
            std::string debug_name = m_kart->getIdent() + " (attachment)";
            m_bomb_scene_node->setName(debug_name.c_str());
#endif
            m_bomb_scene_node->setParent(m_kart->getNode());
            AttachmentUtils::setBombClock(m_bomb_scene_node, m_bomb_remaining);
            m_bomb_scene_node->setAnimationSpeed(0.0f);
        }

        float swat_bomb_frame = stk_config->ticks2Time(
            World::getWorld()->getTicksSinceStart() -
            (m_discard_ticks - stk_config->time2Ticks(40.0f / 25.0f)))
            * 25.0f;

        if (swat_bomb_frame >= (float)m_scene_node->getEndFrame())
            swat_bomb_frame = (float)m_scene_node->getEndFrame();

        m_scene_node->setRotation(core::vector3df(0.0, -180.0, 0.0));
        m_scene_node->setCurrentFrame(swat_bomb_frame);

        if (swat_bomb_frame >= 32.5f && m_bomb_scene_node != NULL)
        {
            m_bomb_scene_node->setPosition(
                core::vector3df(-(swat_bomb_frame - 32.5f), 0.0f, 0.0f));
            m_bomb_scene_node->setRotation(
                core::vector3df(-(swat_bomb_frame - 32.5f), 0.0f, 0.0f));
        }

        if (swat_bomb_frame >= 35)
        {
            m_bomb_scene_node->setVisible(false);
        }   // bom_frame > 35
    }   // if removing bomb
    else
    {
        if (!m_scene_node)
        {
            m_scene_node = m_kart->getAttachment()->getNode();
            m_scene_node->setMesh(attachment_manager
                ->getMesh(Attachment::ATTACH_SWATTER, m_kart->getIdent()));
            m_scene_node->setPosition(SWAT_POS_OFFSET);
            m_scene_node->setLoopMode(false);
            m_scene_node->setAnimationSpeed(0.0f);
        }
        if (!m_swat_sound)
        {
            if (m_kart->getIdent() == "nolok")
                m_swat_sound = SFXManager::get()->createSoundSource("hammer");
            else
                m_swat_sound = SFXManager::get()->createSoundSource("swatter");
        }
        if (!m_discard_now)
        {
            switch (m_animation_phase)
            {
            case SWATTER_AIMING:
                {
                    pointToTarget(dt);
                    m_played_swatter_animation = false;
                }
                break;
            case SWATTER_TO_TARGET:
                {
                    if (!m_played_swatter_animation)
                    {
                        m_played_swatter_animation = true;
                        // Setup the animation
                        m_scene_node->setCurrentFrame(0.0f);
                        m_scene_node->setAnimationSpeed(SWATTER_ANIMATION_SPEED);
                        Vec3 swatter_pos = m_kart->getTrans()(Vec3(SWAT_POS_OFFSET));
                        m_swat_sound->setPosition(swatter_pos);
                        m_swat_sound->play();
                    }
                    pointToTarget(dt);
                }
                break;
            case SWATTER_FROM_TARGET:
                break;
            }
        }
    }
#endif
}   // updateGraphics

// ----------------------------------------------------------------------------
/** Updates an armed swatter: it checks for any karts that are close enough
 *  and not invulnerable, it swats the kart.
 *  \param ticks Time step size.
 *  \return True if the attachment should be discarded.
 */
bool Swatter::updateAndTestFinished()
{
    const int ticks_start = World::getWorld()->getTicksSinceStart();
    if (World::getWorld()->getTicksSinceStart() > m_discard_ticks)
        return true;

    // If removing bomb animation playing, it will only ends when
    // m_discard_ticks > world ticks
    if (m_bomb_remaining != -1)
        return false;

    if (!m_discard_now)
    {
        switch (m_animation_phase)
        {
        case SWATTER_AIMING:
            {
                // Avoid swatter near the start and the end lifetime of swatter
                // to make sure all clients know the existence of swatter each other
                if (m_swatter_duration - m_attachment->getTicksLeft() < stk_config->time2Ticks(0.5f) ||
                    m_attachment->getTicksLeft() < stk_config->time2Ticks(0.75f) ) // ~0.167f and ~0.5f below
                    return false;

                chooseTarget();
                if (!m_target_kart)
                    break;

                // Get the node corresponding to the joint at the center of the
                // swatter (by swatter, I mean the thing hold in the hand, not
                // the whole thing)
                // The joint node doesn't update in server without graphics,
                // so an approximate position is used
                //scene::ISceneNode* swatter_node =
                //    m_scene_node->getJointNode("Swatter");
                //assert(swatter_node);
                //Vec3 swatter_pos = swatter_node->getAbsolutePosition();
                Vec3 swatter_pos = m_kart->getTrans()(Vec3(SWAT_POS_OFFSET));

                float dist2 = (m_target_kart->getXYZ()-swatter_pos).length2();
                float min_dist2
                     = m_kart->getKartProperties()->getSwatterDistance();

                if (dist2 < min_dist2 && !m_kart->isGhostKart())
                {
                    // Start squashing
                    m_animation_phase = SWATTER_TO_TARGET;
                    // TODO - Tie it explicitly to animation speed
                    // TODO - Make it less likely that the swatter squashes thin air
                    m_swatter_animation_ticks =
                        m_attachment->getTicksLeft() - stk_config->time2Ticks(0.166666672f);
                }
            }
            break;
        case SWATTER_TO_TARGET:
            {
                // Did we just finish the first part of the movement?
                if (m_attachment->getTicksLeft() < m_swatter_animation_ticks &&
                    m_attachment->getTicksLeft() > stk_config->time2Ticks(0.5f))
                {
                    // Squash the karts and items around and
                    // change the current phase
                    squashThingsAround();
                    m_animation_phase = SWATTER_FROM_TARGET;
                    const int end_ticks = ticks_start + stk_config->time2Ticks(0.5f);
                    // In Battle mode, the swatter gives a point / takes a life / resets
                    // another kart, so we remove it after one successful hit
                    if (RaceManager::get()->isBattleMode())
                    {
                        m_discard_now = true;
                        m_discard_ticks = end_ticks;
                    }
                    m_swatter_animation_ticks =
                        m_attachment->getTicksLeft() - stk_config->time2Ticks(0.5f);
                }
            }
            break;
        case SWATTER_FROM_TARGET:
            {
                if (m_attachment->getTicksLeft() < m_swatter_animation_ticks &&
                    m_attachment->getTicksLeft() > 0)
                    m_animation_phase = SWATTER_AIMING;
            break;
            }
        }
    }
    return false;
}   // updateAndTestFinished

// ----------------------------------------------------------------------------
/** Determine the nearest kart or item and update the current target
 *  accordingly.
 */
void Swatter::chooseTarget()
{
    // TODO: for the moment, only handle karts...
    const World*  world         = World::getWorld();
    Kart* closest_kart  = NULL;
    m_target_dist2 = FLT_MAX;

    for(unsigned int i=0; i<world->getNumKarts(); i++)
    {
        Kart *kart = world->getKart(i);
        if(kart->isEliminated() || kart==m_kart || kart->getKartAnimation())
            continue;
        // don't squash an already hurt kart
        if (kart->isInvulnerable() || kart->isSquashed())
            continue;

        // Don't hit teammates in team world
        if (world->hasTeam() &&
            world->getKartTeam(kart->getWorldKartId()) ==
            world->getKartTeam(m_kart->getWorldKartId()))
            continue;

        float dist2 = (kart->getXYZ()-m_kart->getXYZ()).length2();
        if(dist2 < m_target_dist2)
        {
            m_target_dist2 = dist2;
            closest_kart = kart;
        }
    }
    // Not larger than 2^5 - 1 for kart id for optimizing state saving
    if (closest_kart && closest_kart->getWorldKartId() < 31)
        m_target_kart = closest_kart;
    else
        m_target_kart = NULL;

    // If no kart is close enough, there is no target
    if (m_target_dist2 > 2500.0f) // 50 * 50
        m_target_kart = NULL;
}

// ----------------------------------------------------------------------------
/** If there is a current target, point in its direction, otherwise adopt the
 *  default position. */
void Swatter::pointToTarget(float dt)
{
#ifndef SERVER_ONLY
    if (m_kart->isGhostKart() || !m_scene_node || dt == 0.0f)
        return;

    float target_angle = 0.0f;

    if (m_target_kart != NULL)
    {
        Vec3 swatter_to_target =
            m_kart->getTrans().inverse()(m_target_kart->getXYZ());
        float dy = -swatter_to_target.getZ();
        float dx = swatter_to_target.getX();
        target_angle = SWAT_ANGLE_OFFSET + atan2f(dy, dx) * 180 / M_PI;
        // Move into the 0-360 range for easier handling
        // Values that need more than one 360° correction shouldn't happen
        if (target_angle < 0.0f)
            target_angle += 360.0f;
        if (target_angle > 360.0f)
            target_angle -= 360.0f;
    }

    float angle_diff = target_angle - m_current_rotation_angle;
    // If we have an angle of over 180°, then it's shorter the other way around
    // We have to take the difference with 360, but also flip the sign
    if (angle_diff > 180.0f)
        angle_diff = -360.0f + angle_diff;
    else if (angle_diff < -180.0f)
        angle_diff = 360.0f + angle_diff;

    // We can only change the angle so much in the given dt
    float max_angle_change = dt * SWAT_ANGLE_PER_SECOND;

    // Since the squash being successful is independent of these graphics, we
    // want to avoid the swatter not turning quickly enough when rapidly
    // approaching a target.
    if (m_target_kart != NULL)
    {
        float time_to_turn = std::fabsf(angle_diff) * (1 / SWAT_ANGLE_PER_SECOND);
        if (time_to_turn > 0.004f)
        {
            // TODO: the time to cover the swatter distance is fixed so we should be able to avoid
            //       running the computationally inefficient sqrtf calls
            float distance_to_squash = sqrtf(m_target_dist2)
                                       - sqrtf(m_kart->getKartProperties()->getSwatterDistance());
            distance_to_squash = std::max(0.0f, distance_to_squash);
            float time_to_reach_at_high_speed = std::max(0.004f, distance_to_squash * 0.02f);
            if (time_to_reach_at_high_speed < time_to_turn)
            {
                float extra_change_factor = std::min(3.0f, time_to_turn / time_to_reach_at_high_speed);
                max_angle_change *= extra_change_factor;
            }
        }
    }

    // If we are close enough, just snap to the target angle directly
    if ((angle_diff > 0.0f && angle_diff < max_angle_change) ||
        (angle_diff < 0.0f && angle_diff > -max_angle_change))
        m_current_rotation_angle = target_angle;
    // Otherwise move by the max amount possible
    else if (angle_diff > 0.0f)
        m_current_rotation_angle += max_angle_change;
    else if (angle_diff < 0.0f)
        m_current_rotation_angle -= max_angle_change;

    // Keep the value in the 0-360 range
    if (m_current_rotation_angle < 0.0f)
        m_current_rotation_angle += 360.0f;

    m_scene_node->setRotation(core::vector3df(0.0, m_current_rotation_angle, 0.0));
#endif
}   // pointToTarget

// ----------------------------------------------------------------------------
/** Squash karts or items that are around the end position (determined using
 *  a joint) of the swatter.
 */
void Swatter::squashThingsAround()
{
    if (m_kart->isGhostKart()) return;

    const KartProperties *kp = m_kart->getKartProperties();

    float duration = kp->getSwatterSquashDuration();
    float slowdown =  kp->getSwatterSquashSlowdown();
    // The squash attempt may fail because of invulnerability, shield, etc.
    // Making a bomb explode counts as a success
    bool success = m_target_kart->setSquash(duration, slowdown);
    const bool has_created_explosion_animation =
        success && m_target_kart->getKartAnimation() != NULL;

    if (success)
    {
        World::getWorld()->kartHit(m_target_kart->getWorldKartId(),
            m_kart->getWorldKartId());

        CaptureTheFlag* ctf = dynamic_cast<CaptureTheFlag*>(World::getWorld());
        if (ctf)
        {
            int reset_ticks = (ctf->getTicksSinceStart() / 10) * 10 + 80;
            ctf->resetKartForSwatterHit(m_target_kart->getWorldKartId(),
                reset_ticks);
        }
        // Handle achievement if the swatter is used by the current player
        if (m_kart->getController()->canGetAchievements())
        {
            PlayerManager::addKartHit(m_target_kart->getWorldKartId());
            PlayerManager::increaseAchievement(AchievementsStatus::SWATTER_HIT, 1);
            PlayerManager::increaseAchievement(AchievementsStatus::ALL_HITS, 1);
            if (RaceManager::get()->isLinearRaceMode())
            {
                PlayerManager::increaseAchievement(AchievementsStatus::SWATTER_HIT_1RACE, 1);
                PlayerManager::increaseAchievement(AchievementsStatus::ALL_HITS_1RACE, 1);
            }
        }
    }

    if (!GUIEngine::isNoGraphics() && has_created_explosion_animation &&
        !RewindManager::get()->isRewinding())
    {
        HitEffect *he = new Explosion(m_kart->getXYZ(),  "explosion", "explosion.xml");
        if(m_kart->getController()->isLocalPlayerController())
            he->setLocalPlayerKartHit();
        ProjectileManager::get()->addHitEffect(he);
    }   // if kart has bomb attached

    // TODO: squash items
}   // squashThingsAround

// ----------------------------------------------------------------------------
void Swatter::restoreState(BareNetworkString* buffer)
{
    int16_t prev_bomb_remaing = m_bomb_remaining;
    m_bomb_remaining = buffer->getUInt16();
    if (prev_bomb_remaing != m_bomb_remaining)
    {
        // Wrong state, clear mesh and let updateGraphics reset itself
        m_scene_node = NULL;
        if (m_bomb_scene_node)
        {
            irr_driver->removeNode(m_bomb_scene_node);
            m_bomb_scene_node = NULL;
        }
    }
    if (m_bomb_remaining == -1)
    {
        uint8_t combined = buffer->getUInt8();
        int kart_id = combined & 31;
        if (kart_id == 31)
            m_target_kart = NULL;
        else
            m_target_kart = World::getWorld()->getKart(kart_id);
        m_animation_phase = AnimationPhase((combined >> 5) & 3);
        m_discard_now = (combined >> 7) == 1;
        m_discard_ticks = buffer->getUInt32();
        m_swatter_animation_ticks = buffer->getUInt16();
    }
    else
        m_discard_ticks = buffer->getUInt32();
}   // restoreState

// ----------------------------------------------------------------------------
void Swatter::saveState(BareNetworkString* buffer) const
{
    buffer->addUInt16(m_bomb_remaining);
    if (m_bomb_remaining == -1)
    {
        uint8_t combined =
            m_target_kart ? (uint8_t)m_target_kart->getWorldKartId() : 31;
        combined |= m_animation_phase << 5;
        combined |= (m_discard_now ? (1 << 7) : 0);
        buffer->addUInt8(combined).addUInt32(m_discard_ticks)
            .addUInt16(m_swatter_animation_ticks);
    }
    else
        buffer->addUInt32(m_discard_ticks);
}   // saveState
