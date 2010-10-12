//  $Id: three_d_animation.cpp 1681 2008-04-09 13:52:48Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009  Joerg Henrichs
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

#include "animations/three_d_animation.hpp"

#include <stdio.h>

#include "animations/ipo.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/mesh_tools.hpp"
#include "io/xml_node.hpp"
#include "modes/world.hpp"
#include "physics/physics.hpp"
#include "physics/kart_motion_state.hpp"
#include "tracks/bezier_curve.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

ThreeDAnimation::ThreeDAnimation(const Track &track,
                                 const XMLNode &node)
               : AnimationBase(node)
{
    /** Save the initial position and rotation in the base animation object. */
    setInitialTransform(m_animated_node->getPosition(), m_animated_node->getRotation());

    m_body            = NULL;
    m_motion_state    = NULL;
    m_collision_shape = NULL;
    std::string shape;
    node.get("shape", &shape);
    if(shape!="")
    {
        createPhysicsBody(shape);
    }
}   // ThreeDAnimation

// ----------------------------------------------------------------------------
/** Creates a bullet rigid body for this animated model. */
void ThreeDAnimation::createPhysicsBody(const std::string &shape)
{
    // 1. Determine size of the object
    // -------------------------------
    Vec3 min, max;
    MeshTools::minMax3D(m_animated_mesh, &min, &max);
    Vec3 extend = max-min;
    if(shape=="box")
    {
        m_collision_shape = new btBoxShape(0.5*extend);
    }
    else if(shape=="coneX")
    {
        float radius = 0.5f*std::max(extend.getY(), extend.getZ());
        m_collision_shape = new btConeShapeX(radius, extend.getX());
    }
    else if(shape=="coneY" || shape=="cone")
    {
        float radius = 0.5f*std::max(extend.getX(), extend.getZ());
        m_collision_shape = new btConeShape(radius, extend.getY());
    }
    else if(shape=="coneZ")
    {
        // Note that the b3d model and therefore the extend has the
        // irrlicht axis, i.e. Y and Z swapped. Also we need to
        // convert 
        float radius = 0.5f*std::max(extend.getX(), extend.getY());
        m_collision_shape = new btConeShapeZ(radius, extend.getZ());
    }
    else
    {
        fprintf(stderr, "Shape '%s' is not supported, ignored.\n", shape.c_str());
        return;
    }
    const core::vector3df &hpr=m_animated_node->getRotation()*DEGREE_TO_RAD;
    btQuaternion q(hpr.X, hpr.Y, hpr.Z);
    const core::vector3df &xyz=m_animated_node->getPosition();
    Vec3 p(xyz);
    btTransform trans(q,p);
    m_motion_state = new KartMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo info(0, m_motion_state, 
                                                  m_collision_shape);

    m_body = new btRigidBody(info);
    m_user_pointer.set(this);
    m_body->setUserPointer(&m_user_pointer);
    World::getWorld()->getPhysics()->addBody(m_body);
    m_body->setCollisionFlags( m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    m_body->setActivationState(DISABLE_DEACTIVATION);
}   // createPhysicsBody

// ----------------------------------------------------------------------------
/** Destructor. */
ThreeDAnimation::~ThreeDAnimation()
{
    if(m_body)
    {
        World::getWorld()->getPhysics()->removeBody(m_body);
        delete m_body;
        delete m_motion_state;
        delete m_collision_shape;
    }
}   // ~ThreeDAnimation

// ----------------------------------------------------------------------------
/** Updates position and rotation of this model. Called once per time step.
 *  \param dt Time since last call.
 */
void ThreeDAnimation::update(float dt)
{
    core::vector3df xyz = m_animated_node->getPosition();
    core::vector3df hpr = m_animated_node->getRotation();
    core::vector3df scale = m_animated_node->getScale();
    AnimationBase::update(dt, &xyz, &hpr, &scale);     //updates all IPOs
    printf("xyz = %f %f %f\n", xyz.X, xyz.Y, xyz.Z);
    m_animated_node->setPosition(xyz);
    m_animated_node->setRotation(hpr);
    m_animated_node->setScale(scale);

    // Now update the position of the bullet body if there is one:
    if(m_body)
    {
        hpr = DEGREE_TO_RAD*hpr;
        btQuaternion q(-hpr.Z, -hpr.X, -hpr.Y);
        Vec3 p(xyz);
        btTransform trans(q,p);
        m_motion_state->setWorldTransform(trans);
    }
}   // update
