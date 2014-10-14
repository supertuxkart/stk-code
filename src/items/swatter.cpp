//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2013 Joerg Henrichs
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

#include "audio/music_manager.hpp"
#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "graphics/explosion.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "items/attachment.hpp"
#include "items/projectile_manager.hpp"
#include "karts/controller/controller.hpp"
#include "karts/explosion_animation.hpp"
#include "karts/kart_properties.hpp"
#include "modes/world.hpp"
#include "karts/abstract_kart.hpp"

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
                 scene::ISceneNode* bomb_scene_node)
       : AttachmentPlugin(kart)
{
    m_animation_phase  = SWATTER_AIMING;
    m_target           = NULL;
    m_removing_bomb    = was_bomb;
    m_bomb_scene_node  = bomb_scene_node;
    m_swat_bomb_frame  = 0.0f;

    // Setup the node
    m_scene_node = kart->getAttachment()->getNode();
    m_scene_node->setPosition(SWAT_POS_OFFSET);

    if (m_removing_bomb)
    {
        m_scene_node->setMesh(irr_driver->getAnimatedMesh(
                        file_manager->getAsset(FileManager::MODEL,"swatter_anim2.b3d") ) );
        m_scene_node->setRotation(core::vector3df(0.0, -180.0, 0.0));
        m_scene_node->setAnimationSpeed(0.9f);
        m_scene_node->setCurrentFrame(0.0f);
        m_scene_node->setLoopMode(false);
    }
    else
    {
        m_scene_node->setAnimationSpeed(0);
    }

    if (kart->getIdent() == "nolok")
        m_swat_sound = SFXManager::get()->createSoundSource("hammer");
    else
        m_swat_sound = SFXManager::get()->createSoundSource("swatter");
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
    if (m_swat_sound)
    {
        m_swat_sound->deleteSFX();
    }
}   // ~Swatter

// ----------------------------------------------------------------------------
/** Updates an armed swatter: it checks for any karts that are close enough
 *  and not invulnerable, it swats the kart.
 *  \param dt Time step size.
 *  \return True if the attachment should be discarded.
 */
bool Swatter::updateAndTestFinished(float dt)
{
    if (m_removing_bomb)
    {
        m_swat_bomb_frame += dt*25.0f;
        m_scene_node->setRotation(core::vector3df(0.0, -180.0, 0.0));

        m_scene_node->setCurrentFrame(m_swat_bomb_frame);

        if (m_swat_bomb_frame >= 32.5f && m_bomb_scene_node != NULL)
        {
            m_bomb_scene_node->setPosition(m_bomb_scene_node->getPosition() +
                                      core::vector3df(-dt*15.0f, 0.0f, 0.0f) );
            m_bomb_scene_node->setRotation(m_bomb_scene_node->getRotation() +
                                      core::vector3df(-dt*15.0f, 0.0f, 0.0f) );
        }

        if (m_swat_bomb_frame >= m_scene_node->getEndFrame())
        {
            return true;
        }
        else if (m_swat_bomb_frame >= 35)
        {
            if (m_bomb_scene_node != NULL)
            {
                irr_driver->removeNode(m_bomb_scene_node);
                m_bomb_scene_node = NULL;
            }
        }   // bom_frame > 35

        return false;
    }   // if removing bomb

    switch(m_animation_phase)
    {
    case SWATTER_AIMING:
        {
            chooseTarget();
            pointToTarget();
            if(!m_target) break;

            // Is the target too near?
            float dist_to_target2 =
                (m_target->getXYZ()- Vec3(m_scene_node->getAbsolutePosition()))
                .length2();
            float min_dist2
                 = m_kart->getKartProperties()->getSwatterDistance2();
            if(dist_to_target2 < min_dist2)
            {
                // Start squashing
                m_animation_phase = SWATTER_TO_TARGET;

                // Setup the animation
                m_scene_node->setCurrentFrame(0.0f);
                m_scene_node->setLoopMode(false);
                m_scene_node->setAnimationSpeed(SWATTER_ANIMATION_SPEED);
            }
        }
        break;
    case SWATTER_TO_TARGET:
        {
            pointToTarget();

            const float middle_frame    = m_scene_node->getEndFrame()/2.0f;
            float       current_frame   = m_scene_node->getFrameNr();

            // Did we just finish the first part of the movement?
            if(current_frame >= middle_frame)
            {
                // Squash the karts and items around and
                // change the current phase
                squashThingsAround();
                m_animation_phase = SWATTER_FROM_TARGET;
            }
        }
        break;

    case SWATTER_FROM_TARGET:
        break;
    }

    // If the swatter is used up, trigger cleaning up
    // TODO: use a timeout
    // TODO: how does it work currently...?
    return false;
}   // updateAndTestFinished

// ----------------------------------------------------------------------------
/** When the animation ends, the swatter is ready again.
 */
void Swatter::onAnimationEnd()
{
    m_animation_phase = SWATTER_AIMING;
}   // onAnimationEnd

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

        float dist2 = (kart->getXYZ()-m_kart->getXYZ()).length2();
        if(dist2<min_dist2)
        {
            min_dist2 = dist2;
            closest_kart = kart;
        }
    }
    m_target = closest_kart;    // may be NULL
}

// ----------------------------------------------------------------------------
/** If there is a current target, point in its direction, otherwise adopt the
 *  default position. */
void Swatter::pointToTarget()
{
    if(!m_target)
    {
        m_scene_node->setRotation(core::vector3df());
    }
    else
    {
        Vec3 swatter_to_target = m_target->getXYZ()
                               -Vec3(m_scene_node->getAbsolutePosition());
        float dy = -swatter_to_target.getZ();
        float dx = swatter_to_target.getX();
        float angle = SWAT_ANGLE_OFFSET + (atan2(dy, dx)-m_kart->getHeading())
                                        * 180.0f/M_PI;

        m_scene_node->setRotation(core::vector3df(0.0, angle, 0.0));
    }
}   // pointToTarget

// ----------------------------------------------------------------------------
/** Squash karts or items that are around the end position (determined using
 *  a joint) of the swatter.
 */
void Swatter::squashThingsAround()
{
    const KartProperties*  kp           = m_kart->getKartProperties();
    // Square of the minimum distance
    float                  min_dist2    = kp->getSwatterDistance2();
    const World*           world        = World::getWorld();

    // Get the node corresponding to the joint at the center of the swatter
    // (by swatter, I mean the thing hold in the hand, not the whole thing)
    scene::ISceneNode* swatter_node = m_scene_node->getJointNode("Swatter");
    assert(swatter_node);
    Vec3 swatter_pos = swatter_node->getAbsolutePosition();

    m_swat_sound->setPosition(swatter_pos);
    m_swat_sound->play();

    // Squash karts around
    for(unsigned int i=0; i<world->getNumKarts(); i++)
    {
        AbstractKart *kart = world->getKart(i);
        // TODO: isSwatterReady()
        if(kart->isEliminated() || kart==m_kart)
            continue;
        // don't swat an already hurt kart
        if (kart->isInvulnerable() || kart->isSquashed())
            continue;

        float dist2 = (kart->getXYZ()-swatter_pos).length2();

        if(dist2 >= min_dist2) continue;   // too far away, ignore this kart

        kart->setSquash(kp->getSquashDuration(), kp->getSquashSlowdown());

        if (kart->getAttachment()->getType()==Attachment::ATTACH_BOMB)
        {   // make bomb explode
            kart->getAttachment()->update(10000);
            HitEffect *he = new Explosion(m_kart->getXYZ(),  "explosion", "explosion.xml");
            if(m_kart->getController()->isPlayerController())
                he->setPlayerKartHit();
            projectile_manager->addHitEffect(he);
            ExplosionAnimation::create(kart);
        }   // if kart has bomb attached
        World::getWorld()->kartHit(kart->getWorldKartId());
    }   // for i < num_kartrs

    // TODO: squash items
}   // squashThingsAround
