//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Joerg Henrichs
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

#include "karts/rescue_animation.hpp"

#include "graphics/callbacks.hpp"
#include "graphics/camera.hpp"
#include "graphics/referee.hpp"
#include "items/attachment.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "modes/three_strikes_battle.hpp"
#include "modes/world.hpp"
#include "physics/physics.hpp"

#include "ISceneNode.h"

/** The constructor stores a pointer to the kart this object is animating,
 *  and initialised the timer.
 *  \param kart Pointer to the kart which is animated.
 */
RescueAnimation::RescueAnimation(AbstractKart *kart, bool is_auto_rescue)
               : AbstractKartAnimation(kart, "RescueAnimation")
{
    m_referee     = new Referee(*m_kart);
    m_kart->getNode()->addChild(m_referee->getSceneNode());
    m_timer       = m_kart->getKartProperties()->getRescueDuration();
    m_velocity    = m_kart->getKartProperties()->getRescueHeight() / m_timer;
    m_xyz         = m_kart->getXYZ();

    m_kart->getAttachment()->clear();

    // Get the current rotation of the kart
    m_curr_rotation = m_kart->getNode()->getRotation() * DEGREE_TO_RAD;

    // Determine the rotation that will rotate the kart from the current
    // up direction to the right up direction it should have according to
    // the normal at the kart's location
    Vec3 up = m_kart->getTrans().getBasis().getColumn(1);
    btQuaternion q = shortestArcQuat(up, m_kart->getNormal());
   
    // Store this rotation as 'delta HPR', which is added over time to the
    // current rotation to end up (after m_timer seconds) with the right up
    // rotation
    m_add_rotation.setHPR(q);
    m_add_rotation /= m_timer;

    // Add a hit unless it was auto-rescue
    if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_3_STRIKES &&
        !is_auto_rescue)
    {
        ThreeStrikesBattle *world=(ThreeStrikesBattle*)World::getWorld();
        world->kartHit(m_kart->getWorldKartId());
        if (UserConfigParams::m_arena_ai_stats)
            world->increaseRescueCount();
    }
};   // RescueAnimation

//-----------------------------------------------------------------------------
/** This object is automatically destroyed when the timer expires.
 */
RescueAnimation::~RescueAnimation()
{
    // If m_timer >=0, this object is deleted because the kart
    // is deleted (at the end of a race), which means that
    // world is in the process of being deleted. In this case
    // we can't call removeKartAfterRescue() or getPhysics anymore.
    if(m_timer < 0)
        World::getWorld()->moveKartAfterRescue(m_kart);
    m_kart->getNode()->removeChild(m_referee->getSceneNode());
    delete m_referee;
    m_referee = NULL;
    if(m_timer < 0)
    {
        m_kart->getBody()->setLinearVelocity(btVector3(0,0,0));
        m_kart->getBody()->setAngularVelocity(btVector3(0,0,0));
        World::getWorld()->getPhysics()->addKart(m_kart);
        for(unsigned int i=0; i<Camera::getNumCameras(); i++)
        {
            Camera *camera = Camera::getCamera(i);
            if(camera && camera->getKart()==m_kart &&
                camera->getType() != Camera::CM_TYPE_END)
                camera->setMode(Camera::CM_NORMAL);
        }
    }
}   // ~RescueAnimation

// ----------------------------------------------------------------------------
/** Updates the kart animation.
 *  \param dt Time step size.
 *  \return True if the explosion is still shown, false if it has finished.
 */
void RescueAnimation::update(float dt)
{
    m_xyz += dt*m_velocity * m_kart->getNormal();
    m_kart->setXYZ(m_xyz);
    m_curr_rotation += dt*m_add_rotation;
    btMatrix3x3 m;
    m.setEulerZYX(m_curr_rotation.getPitch(), m_curr_rotation.getHeading(),
                   m_curr_rotation.getRoll());
    m_kart->setRotation(m);

    AbstractKartAnimation::update(dt);

}   // update
