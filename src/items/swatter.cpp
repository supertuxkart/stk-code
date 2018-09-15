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
#include "achievements/achievement_info.hpp"
#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "config/player_manager.hpp"
#include "graphics/explosion.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "items/attachment_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/explosion_animation.hpp"
#include "karts/kart_properties.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/rewind_info.hpp"
#include "network/rewind_manager.hpp"

#define SWAT_POS_OFFSET        core::vector3df(0.0, 0.2f, -0.4f)
#define SWAT_ANGLE_MIN  45
#define SWAT_ANGLE_MAX  135
#define SWAT_ANGLE_OFFSET (90.0f + 15.0f)
#define SWATTER_ANIMATION_SPEED 100.0f

/** Constructor: creates a swatter at a given attachment for a kart. If there
 *  was a bomb attached, it triggers the replace bomb animations.
 *  \param attachment The attachment instance where the swatter is attached to.
 *  \param kart The kart to which the swatter is attached.
 *  \param was_bomb True if the kart had a bomb as attachment.
 *  \param bomb_scene_node The scene node of the bomb (i.e. the previous
 *         attachment scene node).
 */
Swatter::Swatter(AbstractKart *kart, bool was_bomb,
                 scene::ISceneNode* bomb_scene_node, int ticks)
       : AttachmentPlugin(kart),
         m_swatter_start_ticks(World::getWorld()->getTicksSinceStart()),
         m_swatter_end_ticks(World::getWorld()->getTicksSinceStart() + ticks)
{
    m_animation_phase  = SWATTER_AIMING;
    m_discard_now      = false;
    m_target           = NULL;
    m_closest_kart     = NULL;
    m_bomb_scene_node  = bomb_scene_node;
    m_swat_bomb_frame  = 0.0f;

    // Setup the node
    m_scene_node = kart->getAttachment()->getNode();
    m_scene_node->setPosition(SWAT_POS_OFFSET);

    if (was_bomb)
    {
        m_scene_node->setMesh(attachment_manager
            ->getMesh(Attachment::ATTACH_SWATTER_ANIM));
        m_scene_node->setRotation(core::vector3df(0.0, -180.0, 0.0));
        m_scene_node->setAnimationSpeed(0.9f);
        m_scene_node->setCurrentFrame(0.0f);
        m_scene_node->setLoopMode(false);
        // There are 40 frames in blender for swatter_anim.blender
        // so 40 / 25 * 120
        m_removed_bomb_ticks =
            World::getWorld()->getTicksSinceStart() + 192;
    }
    else
    {
        m_removed_bomb_ticks = std::numeric_limits<int>::max();
        m_scene_node->setAnimationSpeed(0);
    }

    m_swat_sound = NULL;
    m_start_swat_ticks = std::numeric_limits<int>::max();
    m_end_swat_ticks = std::numeric_limits<int>::max();
#ifndef SERVER_ONLY
    if (kart->getIdent() == "nolok")
        m_swat_sound = SFXManager::get()->createSoundSource("hammer");
    else
        m_swat_sound = SFXManager::get()->createSoundSource("swatter");
#endif
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
void Swatter::updateGrahpics(float dt)
{
#ifndef SERVER_ONLY
    if (m_removed_bomb_ticks != std::numeric_limits<int>::max())
    {
        m_swat_bomb_frame += dt*25.0f;
        m_scene_node->setRotation(core::vector3df(0.0, -180.0, 0.0));

        m_scene_node->setCurrentFrame(m_swat_bomb_frame);

        if (m_swat_bomb_frame >= 32.5f && m_bomb_scene_node != NULL)
        {
            m_bomb_scene_node->setPosition(m_bomb_scene_node
                ->getPosition() + core::vector3df(-dt*15.0f, 0.0f, 0.0f) );
            m_bomb_scene_node->setRotation(m_bomb_scene_node
                ->getRotation() + core::vector3df(-dt*15.0f, 0.0f, 0.0f) );
        }

        if (m_swat_bomb_frame >= m_scene_node->getEndFrame())
        {
            return;
        }
        else if (m_swat_bomb_frame >= 35)
        {
            if (m_bomb_scene_node != NULL)
            {
                irr_driver->removeNode(m_bomb_scene_node);
                m_bomb_scene_node = NULL;
            }
        }   // bom_frame > 35
    }   // if removing bomb
#endif
}   // updateGrahpics

// ----------------------------------------------------------------------------
/** Updates an armed swatter: it checks for any karts that are close enough
 *  and not invulnerable, it swats the kart.
 *  \param dt Time step size.
 *  \return True if the attachment should be discarded.
 */
bool Swatter::updateAndTestFinished(int ticks)
{
    const int ticks_start = World::getWorld()->getTicksSinceStart();
    if (m_removed_bomb_ticks != std::numeric_limits<int>::max())
    {
        if (ticks_start >= m_removed_bomb_ticks)
            return true;
        return false;
    }   // if removing bomb

    if (RewindManager::get()->isRewinding())
        return false;

    if (!m_discard_now)
    {
        switch (m_animation_phase)
        {
        case SWATTER_AIMING:
            {
                // Avoid swatter near the start and the end lifetime of swatter
                // to make sure all clients know the existence of swatter each other
                if (ticks_start - m_swatter_start_ticks < 60 ||
                    m_swatter_end_ticks - ticks_start < 60)
                    return false;

                chooseTarget();
                pointToTarget();
                if(!m_target || !m_closest_kart) break;

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

                float dist2 = (m_closest_kart->getXYZ()-swatter_pos).length2();
                float min_dist2
                     = m_kart->getKartProperties()->getSwatterDistance();

                if (dist2 < min_dist2 && !m_kart->isGhostKart())
                {
                    // Start squashing
                    m_animation_phase = SWATTER_TO_TARGET;
                    m_start_swat_ticks = ticks_start + 20;
                    // Setup the animation
                    m_scene_node->setCurrentFrame(0.0f);
                    m_scene_node->setLoopMode(false);
                    m_scene_node->setAnimationSpeed(SWATTER_ANIMATION_SPEED);

#ifndef SERVER_ONLY
                    // Play swat sound
                    m_swat_sound->setPosition(swatter_pos);
                    m_swat_sound->play();
#endif
                }
            }
            break;
        case SWATTER_TO_TARGET:
            {
                pointToTarget();
                // Did we just finish the first part of the movement?
                if (ticks_start > m_start_swat_ticks)
                {
                    m_start_swat_ticks = std::numeric_limits<int>::max();
                    // Squash the karts and items around and
                    // change the current phase
                    squashThingsAround();
                    m_animation_phase = SWATTER_FROM_TARGET;
                    const int end_ticks = ticks_start + 60;
                    if (race_manager
                        ->getMinorMode()==RaceManager::MINOR_MODE_BATTLE ||
                        race_manager
                        ->getMinorMode()==RaceManager::MINOR_MODE_SOCCER)
                    {
                        // Remove swatter from kart in arena gameplay
                        // after one successful hit
                        m_discard_now = true;
                    }
                    m_end_swat_ticks = end_ticks;
                }
            }
            break;

        case SWATTER_FROM_TARGET:
            break;
        }
    }

    if (m_discard_now)
    {
        return ticks_start > m_end_swat_ticks;
    }
    else if (ticks_start > m_end_swat_ticks)
    {
        m_animation_phase = SWATTER_AIMING;
        m_end_swat_ticks = std::numeric_limits<int>::max();
        return false;
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
    AbstractKart* closest_kart  = NULL;
    float         min_dist2     = FLT_MAX;

    for(unsigned int i=0; i<world->getNumKarts(); i++)
    {
        AbstractKart *kart = world->getKart(i);
        // TODO: isSwatterReady(), isSquashable()?
        if(kart->isEliminated() || kart==m_kart)
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
        if(dist2<min_dist2)
        {
            min_dist2 = dist2;
            closest_kart = kart;
        }
    }
    m_target = closest_kart;    // may be NULL
    m_closest_kart = closest_kart;
}

// ----------------------------------------------------------------------------
/** If there is a current target, point in its direction, otherwise adopt the
 *  default position. */
void Swatter::pointToTarget()
{
#ifndef SERVER_ONLY
    if (m_kart->isGhostKart()) return;

    if(!m_target)
    {
        m_scene_node->setRotation(core::vector3df());
    }
    else
    {
        Vec3 swatter_to_target =
            m_kart->getTrans().inverse()(m_target->getXYZ());
        float dy = -swatter_to_target.getZ();
        float dx = swatter_to_target.getX();
        float angle = SWAT_ANGLE_OFFSET + atan2f(dy, dx) * 180 / M_PI;
        m_scene_node->setRotation(core::vector3df(0.0, angle, 0.0));
    }
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

    AbstractKart* closest_kart = m_closest_kart;
    float duration = kp->getSwatterSquashDuration();
    float slowdown =  kp->getSwatterSquashSlowdown();
    closest_kart->setSquash(duration, slowdown);

    // Locally add a event to replay the squash during rewind
    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient())
    {
        RewindManager::get()->addRewindInfoEventFunction(new
            RewindInfoEventFunction(World::getWorld()->getTicksSinceStart(),
            /*undo_function*/[](){},
            /*replay_function*/[closest_kart, duration, slowdown]()
            {
                closest_kart->setSquash(duration, slowdown);
            }));
    }

    // Handle achievement if the swatter is used by the current player
    if (m_kart->getController()->canGetAchievements())
    {
        PlayerManager::increaseAchievement(AchievementInfo::ACHIEVE_MOSQUITO,
                                           "swatter", 1);
    }

    if (m_closest_kart->getAttachment()->getType()==Attachment::ATTACH_BOMB)
    {   // make bomb explode
        m_closest_kart->getAttachment()->update(10000);
        HitEffect *he = new Explosion(m_kart->getXYZ(),  "explosion", "explosion.xml");
        if(m_kart->getController()->isLocalPlayerController())
            he->setLocalPlayerKartHit();
        projectile_manager->addHitEffect(he);
        ExplosionAnimation::create(m_closest_kart);
    }   // if kart has bomb attached
    if (m_closest_kart->isSquashed())
    {
        // The kart may not be squashed if it was protected by a bubblegum shield
        World::getWorld()->kartHit(m_closest_kart->getWorldKartId(),
            m_kart->getWorldKartId());
    }

    // TODO: squash items
}   // squashThingsAround
