//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011 Joerg Henrichs
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
#include "items/attachment.hpp"
#include "modes/world.hpp"
#include "karts/kart.hpp"

#define SWAT_POS_OFFSET        core::vector3df(0.0, 0.2f, -0.4f)
#define SWAT_ANGLE_MIN  45
#define SWAT_ANGLE_MAX  135
#define SWAT_ANGLE_OFFSET (90.0f + 15.0f)
#define SWATTER_ANIMATION_SPEED 100.0f

Swatter::Swatter(Attachment *attachment, Kart *kart)
       : AttachmentPlugin(attachment, kart)
{
    m_animation_phase  = SWATTER_AIMING;
    m_target = NULL;

    // Setup the node
    scene::IAnimatedMeshSceneNode* node = m_attachment->getNode();
    node->setPosition(SWAT_POS_OFFSET);
    node->setAnimationSpeed(0);
    
    m_swat_sound = sfx_manager->createSoundSource("swatter");
}   // Swatter

//-----------------------------------------------------------------------------
Swatter::~Swatter()
{
    if (m_swat_sound)
    {
        sfx_manager->deleteSFX(m_swat_sound);
    }
}   // ~Swatter

//-----------------------------------------------------------------------------
/** Updates an armed swatter: it checks for any karts that are close enough
 *  and not invulnerable, it swats the kart.
 *  \param dt Time step size.
 *  \return True if the attachment should be discarded.
 */
bool Swatter::updateAndTestFinished(float dt)
{
    switch(m_animation_phase)
    {
    case SWATTER_AIMING:
        {
            chooseTarget();
            pointToTarget();

            if(!m_target)
                break;

            // Is the target too near?
            float   dist_to_target2 = (m_target->getXYZ() - m_attachment->getNode()->getAbsolutePosition()).length2();
            float   min_dist2       = m_kart->getKartProperties()->getSwatterDistance2();
            if(dist_to_target2 < min_dist2)
            {
                // Start squashing
                m_animation_phase = SWATTER_TO_TARGET;

                // Setup the animation
                scene::IAnimatedMeshSceneNode* node = m_attachment->getNode();
                node->setCurrentFrame(0.0f);
                node->setLoopMode(false);
                node->setAnimationSpeed(SWATTER_ANIMATION_SPEED);
            }
        }
        break;
    case SWATTER_TO_TARGET:
        {
            pointToTarget();

            scene::IAnimatedMeshSceneNode *node = m_attachment->getNode();
            const float middle_frame    = node->getEndFrame()/2.0f;
            float       current_frame   = node->getFrameNr();

            // Did we just finish the first part of the movement?
            if(current_frame >= middle_frame)
            {
                // Squash the karts and items around and change the current phase
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

/** When the animation ends, the swatter is ready again */
void Swatter::onAnimationEnd()
{
    m_animation_phase = SWATTER_AIMING;
}   // onAnimationEnd

/** Determine the nearest kart or item and update the current target accordingly */
void Swatter::chooseTarget()
{
    // TODO: for the moment, only handle karts...
    const World *world         = World::getWorld();
    Kart        *closest_kart  = NULL;
    float       min_dist2      = FLT_MAX;

    for(unsigned int i=0; i<world->getNumKarts(); i++)
    {
        Kart *kart = world->getKart(i);
        // TODO: isSwatterReady(), isSquashable()?
        if(kart->isEliminated() || kart==m_kart || kart->isSquashed())
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

/** If there is a current target, point in its direction, otherwise adopt the default position */
void Swatter::pointToTarget()
{
    if(!m_target)
    {
        m_attachment->getNode()->setRotation(core::vector3df());
    }
    else
    {
        Vec3 swatter_to_target = m_target->getXYZ() - m_attachment->getNode()->getAbsolutePosition();
        float dy = -swatter_to_target.getZ();
        float dx = swatter_to_target.getX();
        float angle = SWAT_ANGLE_OFFSET + (atan2(dy, dx) - m_kart->getHeading()) * 180.0f/M_PI;

        m_attachment->getNode()->setRotation(core::vector3df(0.0, angle, 0.0));
    }
}

/** Squash karts or items that are around the end position (determined using a joint) of the swatter */
void Swatter::squashThingsAround()
{
    const KartProperties*  kp          = m_kart->getKartProperties();
    float                  min_dist2   = kp->getSwatterDistance2();    // Square of the minimum distance
    const World*           world       = World::getWorld();
    scene::IAnimatedMeshSceneNode *node  = m_attachment->getNode();

    // Get the node corresponding to the joint at the center of the swatter (by swatter, I mean
    // the thing hold in the hand, not the whole thing)
    scene::ISceneNode* swatter_node = node->getJointNode("Swatter");
    assert(swatter_node);
    Vec3 swatter_pos = swatter_node->getAbsolutePosition();

    m_swat_sound->position(swatter_pos);
    m_swat_sound->play();
    
    // Squash karts around
    for(unsigned int i=0; i<world->getNumKarts(); i++)
    {
        Kart *kart = world->getKart(i);
        // TODO: isSwatterReady()
        if(kart->isEliminated() || kart==m_kart || kart->isSquashed())
            continue;
        float dist2 = (kart->getXYZ()-swatter_pos).length2();

        if(dist2 < min_dist2)
        {
            kart->setSquash(kp->getSquashDuration(),
                            kp->getSquashSlowdown());
            World::getWorld()->kartHit(kart->getWorldKartId());
        }
    }

    // TODO: squash items
}
