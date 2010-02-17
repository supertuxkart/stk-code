//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, Steve Baker
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

#include "karts/moveable.hpp"

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "utils/coord.hpp"

Moveable::Moveable()
{
    m_body            = 0;
    m_motion_state    = 0;
    m_first_time      = true;
    m_mesh            = NULL;
    m_animated_mesh   = NULL;
    m_node            = NULL;
    m_animated_node   = NULL;
}   // Moveable

//-----------------------------------------------------------------------------
Moveable::~Moveable()
{
    // The body is being removed from the world in kart/projectile
    if(m_body)         delete m_body;
    if(m_motion_state) delete m_motion_state;
    if(m_node) irr_driver->removeNode(m_node);
    if(m_animated_node) irr_driver->removeNode(m_animated_node);
    if(m_mesh) irr_driver->removeMesh(m_mesh);
    if(m_animated_mesh) irr_driver->removeMesh(m_animated_mesh);
}   // ~Moveable

//-----------------------------------------------------------------------------
/** Sets this model to be non-animated.
 *  \param n The scene node.
 */
void Moveable::setNode(scene::ISceneNode *n)
{
    m_node          = n; 
    m_animated_node = NULL;
}   // setNode

//-----------------------------------------------------------------------------
/** Sets this model to be animated.
 *  \param n The animated scene node.
 */
void Moveable::setAnimatedNode(scene::IAnimatedMeshSceneNode *n)
{
    m_node          = NULL; 
    m_animated_node = n;
}   // setAnimatedNode

//-----------------------------------------------------------------------------
void Moveable::updateGraphics(const Vec3& off_xyz, const Vec3& off_hpr)
{
    Vec3 xyz=getXYZ()+off_xyz;
    Vec3 hpr=getHPR()+off_hpr;
    //sgCoord c=Coord(xyz, hpr).toSgCoord();
    if(m_node)
    {
        m_node->setPosition(xyz.toIrrVector());
        m_node->setRotation(hpr.toIrrHPR());
    }
    else if(m_animated_node)
    {
        m_animated_node->setPosition(xyz.toIrrVector());
        m_animated_node->setRotation(hpr.toIrrHPR());
    }
}   // updateGraphics

//-----------------------------------------------------------------------------
// The reset position must be set before calling reset
void Moveable::reset()
{
    if(m_body)
    {
        m_body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
        m_body->setAngularVelocity(btVector3(0, 0, 0));
        m_body->setCenterOfMassTransform(m_transform);
    }
    if(m_node)
        m_node->setVisible(true);  // In case that the objects was eliminated
    if(m_animated_node)
        m_animated_node->setVisible(true);

    Coord c(m_transform);
    m_hpr = c.getHPR();
}   // reset

//-----------------------------------------------------------------------------
void Moveable::update(float dt)
{
    m_motion_state->getWorldTransform(m_transform);
    m_velocityLC = getVelocity()*m_transform.getBasis();
    // The following code would synchronise bullet to irrlicht rotations, but
    // heading etc. might not be 'correct', e.g. a 180 degree heading rotation
    // would be reported as 180 degree roll and pitch, and 0 degree heading.
    // So to get heading, pitch etc. the way needed elsewhere (camera etc),
    // we would still have to rotate unit vectors and compute heading etc.
    // with atan.
    //btQuaternion q = m_transform.getRotation();
    //core::quaternion qirr(q.getX(), q.getZ(), q.getY(), -q.getW());
    //core::vector3df r;
    //qirr.toEuler(r);
    // Note: toIrrHPR mixes the axis back etc., so the assignments below
    // mean that getIrrHPR returns exactly (r.x,r.y,r.z)*RAD_TO_DEGREE
    //m_hpr.setX(-r.Y);
    //m_hpr.setY(-r.X);
    //m_hpr.setZ(-r.Z);


    m_hpr.setHPR(m_transform.getBasis());
    // roll is not set correctly, I assume due to a different HPR order.
    // So we compute the proper roll (by taking the angle between the up
    // vector and the rotated up vector).
    Vec3 up(0,0,1);
    Vec3 roll_vec = m_transform.getBasis()*up;
    float roll = atan2(roll_vec.getX(), roll_vec.getZ());
    m_hpr.setRoll(roll);

    updateGraphics(Vec3(0,0,0), Vec3(0,0,0));
    m_first_time = false ;
}   // update


//-----------------------------------------------------------------------------
void Moveable::createBody(float mass, btTransform& trans,
                          btCollisionShape *shape) {
    btVector3 inertia;
    shape->calculateLocalInertia(mass, inertia);
    m_transform = trans;
    m_motion_state = new KartMotionState(trans);

    btRigidBody::btRigidBodyConstructionInfo info(mass, m_motion_state, shape, inertia);
    info.m_restitution=0.5f;

    // Then create a rigid body
    // ------------------------
    m_body = new btRigidBody(info);
    // The value of user_pointer must be set from the actual class, otherwise this
    // is only a pointer to moveable, not to (say) kart, and virtual 
    // functions are not called correctly. So only init the pointer to zero.
    m_user_pointer.zero();
    m_body->setUserPointer(&m_user_pointer);
    const btMatrix3x3& basis=m_body->getWorldTransform().getBasis();
    m_hpr.setHPR(basis);
}   // createBody

//-----------------------------------------------------------------------------
/** Places this moveable at a certain location and stores this transform in
 *  this Moveable, so that it can be accessed easily.
 *  \param t New transform for this moveable.
 */
void Moveable::setTrans(const btTransform &t)
{
    m_transform=t;
    m_motion_state->setWorldTransform(t);
}   // setTrans
