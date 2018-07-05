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
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/rewind_manager.hpp"
#include "tracks/track.hpp"

#include "ISceneNode.h"

Moveable::Moveable()
{
    m_body            = 0;
    m_motion_state    = 0;
    m_mesh            = NULL;
    m_node            = NULL;
    m_heading         = 0;

    m_smoothed_transform = btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f));
    m_start_smoothing_postion = m_adjust_position =
        std::make_pair(Vec3(0.0f, 0.0f, 0.0f),
        btQuaternion(0.0f, 0.0f, 0.0f, 1.0f));
    m_prev_position_data = std::make_pair(m_smoothed_transform, Vec3());
    m_smoothing = SS_NONE;
    m_adjust_time = m_adjust_time_dt = 0.0f;
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
void Moveable::prepareSmoothing()
{
    // Continuous smooth enabled
    //if (m_smoothing != SS_NONE)
    //    return;

    m_prev_position_data = std::make_pair(m_transform, getVelocity());
}   // prepareSmoothing

//-----------------------------------------------------------------------------
/** Adds a new error between graphical and physical position/rotation. Called
 *  in case of a rewind to allow to for smoothing the visuals in case of
 *  incorrect client prediction.
 */
void Moveable::checkSmoothing()
{
    // Continuous smooth enabled
    //if (m_smoothing != SS_NONE)
    //    return;

    float adjust_length = (m_transform.getOrigin() -
        m_prev_position_data.first.getOrigin()).length();
    if (adjust_length < 0.1f || adjust_length > 4.0f)
        return;

    float speed = m_prev_position_data.second.length();
    speed = std::max(speed, getVelocity().length());
    if (speed < 0.3f)
        return;

    float adjust_time = (adjust_length * 2.0f) / speed;
    if (adjust_time > 2.0f)
        return;

    m_smoothing = SS_TO_ADJUST;
    m_adjust_time_dt = 0.0f;
    m_adjust_time = adjust_time;

    m_start_smoothing_postion.first = m_smoothing == SS_NONE ?
        m_prev_position_data.first.getOrigin() :
        m_smoothed_transform.getOrigin();
    m_start_smoothing_postion.second = m_smoothing == SS_NONE ?
        m_prev_position_data.first.getRotation() :
        m_smoothed_transform.getRotation();
    m_start_smoothing_postion.second.normalize();

    m_adjust_control_point = m_start_smoothing_postion.first +
        m_prev_position_data.second * m_adjust_time;
    Vec3 p2 = m_transform.getOrigin() + getVelocity() *
        m_adjust_time;

    m_adjust_position.first.setInterpolate3(m_adjust_control_point, p2, 0.5f);
    m_adjust_position.second = m_transform.getRotation();
    m_adjust_position.second.normalize();
}   // checkSmoothing

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
    Vec3 cur_xyz = getXYZ();
    btQuaternion cur_rot = getRotation();

    float ratio = 0.0f;
    if (m_smoothing != SS_NONE)
    {
        float adjust_time_dt = m_adjust_time_dt + dt;
        ratio = adjust_time_dt / m_adjust_time;
        if (ratio > 1.0f)
        {
            ratio -= 1.0f;
            m_adjust_time_dt = adjust_time_dt - m_adjust_time;
            if (m_smoothing == SS_TO_ADJUST)
            {
                m_smoothing = SS_TO_REAL;
                m_adjust_control_point = m_adjust_position.first +
                    getVelocity() * m_adjust_time;
            }
            else
                m_smoothing = SS_NONE;
        }
        else
            m_adjust_time_dt = adjust_time_dt;
    }

    assert(m_adjust_time_dt >= 0.0f);
    assert(ratio >= 0.0f);
    if (m_smoothing == SS_TO_ADJUST)
    {
        cur_xyz.setInterpolate3(m_start_smoothing_postion.first,
            m_adjust_position.first, ratio);
        Vec3 to_control;
        to_control.setInterpolate3(m_start_smoothing_postion.first,
            m_adjust_control_point, ratio);
        cur_xyz.setInterpolate3(cur_xyz, to_control, 1.0f - ratio);
        if (smoothRotation())
        {
            cur_rot = m_start_smoothing_postion.second;
            if (dot(cur_rot, m_adjust_position.second) < 0.0f)
                cur_rot = -cur_rot;
            cur_rot = cur_rot.slerp(m_adjust_position.second, ratio);
        }
    }
    else if (m_smoothing == SS_TO_REAL)
    {
        Vec3 to_control;
        to_control.setInterpolate3(m_adjust_position.first,
            m_adjust_control_point, ratio);
        float ratio_sqrt = sqrtf(ratio);
        cur_xyz.setInterpolate3(m_adjust_position.first, cur_xyz, ratio_sqrt);
        cur_xyz.setInterpolate3(to_control, cur_xyz, ratio);
        if (smoothRotation())
        {
            cur_rot.normalize();
            if (dot(cur_rot, m_adjust_position.second) < 0.0f)
                cur_rot = -cur_rot;
            cur_rot = cur_rot.slerp(m_adjust_position.second, 1.0f - ratio);
        }
    }

    m_smoothed_transform.setOrigin(cur_xyz);
    m_smoothed_transform.setRotation(cur_rot);

    if (m_smoothing != SS_NONE)
    {
        Vec3 lc = m_transform.inverse()(cur_xyz);
        // Adjust vertical position for up/down-sloping
        cur_xyz = m_smoothed_transform(Vec3(0.0f, -lc.y(), 0.0f));
        m_smoothed_transform.setOrigin(cur_xyz);
    }

#undef DEBUG_SMOOTHING
#ifdef DEBUG_SMOOTHING
    // Gnuplot compare command
    // plot "stdout.log" u 6:8 w lp lw 2, "stdout.log" u 10:12 w lp lw 2
    Log::verbose("Smoothing", "%s smoothed-xyz(6-8) %f %f %f "
        "xyz(10-12) %f %f %f", getIdent().c_str(),
        cur_xyz.getX(), cur_xyz.getY(), cur_xyz.getZ(),
        getXYZ().getX(), getXYZ().getY(), getXYZ().getZ());
#endif

    Vec3 xyz = cur_xyz + offset_xyz;
    m_node->setPosition(xyz.toIrrVector());
    btQuaternion r_all = cur_rot * rotation;
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
#ifndef SERVER_ONLY
    m_node->setVisible(true);  // In case that the objects was eliminated
#endif

    m_smoothed_transform = m_transform;
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
 *  \param ticks Number of physics time steps - should be 1.
 */
void Moveable::update(int ticks)
{
    if(m_body->getInvMass()!=0)
        m_motion_state->getWorldTransform(m_transform);
    m_velocityLC = getVelocity()*m_transform.getBasis();
    updatePosition();
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
