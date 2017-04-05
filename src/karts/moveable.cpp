//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 Joerg Henrichs, Steve Baker
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

#include <math.h>
#include "karts/moveable.hpp"

#include "config/stk_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "tracks/track.hpp"

#include "ISceneNode.h"

Moveable::Moveable()
{
    m_body            = 0;
    m_motion_state    = 0;
    m_mesh            = NULL;
    m_node            = NULL;
    m_heading         = 0;
}   // Moveable

//-----------------------------------------------------------------------------
Moveable::~Moveable()
{
    // The body is being removed from the world in kart/projectile
    if(m_body)         delete m_body;
    if(m_motion_state) delete m_motion_state;
    if(m_node) irr_driver->removeNode(m_node);
    if(m_mesh) irr_driver->removeMeshFromCache(m_mesh);
}   // ~Moveable

//-----------------------------------------------------------------------------
/** Sets the mesh for this model.
 *  \param n The scene node.
 */
void Moveable::setNode(scene::ISceneNode *n)
{
    m_node          = n;
}   // setNode

//-----------------------------------------------------------------------------
/** Updates the graphics model. Mainly set the graphical position to be the
 *  same as the physics position, but uses offsets to position and rotation
 *  for special gfx effects (e.g. skidding will turn the karts more).
 *  \param offset_xyz Offset to be added to the position.
 *  \param rotation Additional rotation.
 */
void Moveable::updateGraphics(float dt, const Vec3& offset_xyz,
                              const btQuaternion& rotation)
{
#ifndef SERVER_ONLY
    Vec3 xyz=getXYZ()+offset_xyz;
    m_node->setPosition(xyz.toIrrVector());
    btQuaternion r_all = getRotation()*rotation;
    if(btFuzzyZero(r_all.getX()) && btFuzzyZero(r_all.getY()-0.70710677f) &&
       btFuzzyZero(r_all.getZ()) && btFuzzyZero(r_all.getW()-0.70710677f)   )
        r_all.setX(0.000001f);
    Vec3 hpr;
    hpr.setHPR(r_all);
    m_node->setRotation(hpr.toIrrHPR());
#endif
}   // updateGraphics

//-----------------------------------------------------------------------------
/** The reset position must be set before calling reset
 */
void Moveable::reset()
{
    if(m_body)
    {
        m_body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
        m_body->setAngularVelocity(btVector3(0, 0, 0));
        m_body->setCenterOfMassTransform(m_transform);
    }
    m_node->setVisible(true);  // In case that the objects was eliminated

    Vec3 up       = getTrans().getBasis().getColumn(1);
    m_pitch       = atan2(up.getZ(), fabsf(up.getY()));
    m_roll        = atan2(up.getX(), up.getY());
    m_velocityLC  = Vec3(0, 0, 0);
    Vec3 forw_vec = m_transform.getBasis().getColumn(0);
    m_heading     = -atan2f(forw_vec.getZ(), forw_vec.getX());

}   // reset

//-----------------------------------------------------------------------------

void Moveable::flyUp()
{
    m_body->setGravity(btVector3(0.0, 8.0, 0.0));
    m_body->applyCentralImpulse(btVector3(0.0, 100.0, 0.0));
}   // flyUp

// ----------------------------------------------------------------------------
void Moveable::flyDown()
{
    m_body->applyCentralImpulse(btVector3(0.0, -100.0, 0.0));
}   // flyDown

// ----------------------------------------------------------------------------
void Moveable::stopFlying()
{
    m_body->setGravity(btVector3(0.0, -Track::getCurrentTrack()->getGravity(), 0.0));
}   // stopFlying

//-----------------------------------------------------------------------------
/** Updates the current position and rotation from the corresponding physics
 *  body, and then calls updateGraphics to position the model correctly.
 *  \param float dt Time step size.
 */
void Moveable::update(float dt)
{
    if(m_body->getInvMass()!=0)
        m_motion_state->getWorldTransform(m_transform);
    m_velocityLC = getVelocity()*m_transform.getBasis();
    updatePosition();

    updateGraphics(dt, Vec3(0,0,0), btQuaternion(0, 0, 0, 1));
}   // update

//-----------------------------------------------------------------------------
/** Updates the current position and rotation. This function is also called
 *  by ghost karts for getHeading() to work.
 */
void Moveable::updatePosition()
{
    Vec3 forw_vec = m_transform.getBasis().getColumn(2);
    m_heading     = atan2f(forw_vec.getX(), forw_vec.getZ());

    // The pitch in hpr is in between -pi and pi. But for the camera it
    // must be restricted to -pi/2 and pi/2 - so recompute it by restricting
    // y to positive values, i.e. no pitch of more than pi/2.
    Vec3 up       = getTrans().getBasis().getColumn(1);
    m_pitch       = atan2(up.getZ(), fabsf(up.getY()));
    m_roll        = atan2(up.getX(), up.getY());
}   // updatePosition

//-----------------------------------------------------------------------------
/** Creates the bullet rigid body for this moveable.
 *  \param mass Mass of this object.
 *  \param trans Transform (=position and orientation) for this object).
 *  \param shape Bullet collision shape for this object.
 */
void Moveable::createBody(float mass, btTransform& trans,
                          btCollisionShape *shape,
                          float restitution)
{
    btVector3 inertia;
    shape->calculateLocalInertia(mass, inertia);
    m_transform = trans;
    m_motion_state = new KartMotionState(trans);

    btRigidBody::btRigidBodyConstructionInfo info(mass, m_motion_state,
                                                  shape, inertia);
    info.m_restitution = restitution;
    info.m_friction = stk_config->m_default_moveable_friction;

    // Then create a rigid body
    // ------------------------
    m_body = new btRigidBody(info);
    if(mass==0)
    {
        // Create a kinematic object
        m_body->setCollisionFlags(m_body->getCollisionFlags() |
                                  btCollisionObject::CF_KINEMATIC_OBJECT );
        m_body->setActivationState(DISABLE_DEACTIVATION);
    }

    // The value of user_pointer must be set from the actual class, otherwise this
    // is only a pointer to moveable, not to (say) kart, and virtual
    // functions are not called correctly. So only init the pointer to zero.
    m_user_pointer.zero();
    m_body->setUserPointer(&m_user_pointer);
}   // createBody

//-----------------------------------------------------------------------------
/** Places this moveable at a certain location and stores this transform in
 *  this Moveable, so that it can be accessed easily.
 *  \param t New transform for this moveable.
 */
void Moveable::setTrans(const btTransform &t)
{
    m_transform=t;
    if(m_motion_state)
        m_motion_state->setWorldTransform(t);
}   // setTrans
