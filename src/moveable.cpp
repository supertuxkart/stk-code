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

#include "player_kart.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "user_config.hpp"
#include "coord.hpp"

Moveable::Moveable()
{
    m_body            = 0;
    m_motion_state    = 0;
    m_shadow          = 0;
    m_first_time      = true ;
    m_model_transform = new ssgTransform();

    m_model_transform->ref();
}   // Moveable

//-----------------------------------------------------------------------------
Moveable::~Moveable()
{
    // The body is being removed from the world in kart/projectile
    if(m_body)         delete m_body;
    if(m_motion_state) delete m_motion_state;
    // FIXME what about model?
}   // ~Moveable

//-----------------------------------------------------------------------------
// The reset position must be set before calling reset
void Moveable::reset()
{
    m_material_hot     = NULL;
    m_normal_hot       = NULL;
    if(m_body)
    {
        m_body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
        m_body->setAngularVelocity(btVector3(0, 0, 0));
        m_body->setCenterOfMassTransform(m_transform);
    }
    Coord c(m_transform);
    m_hpr = c.getHPR();
}   // reset

//-----------------------------------------------------------------------------
void Moveable::createBody(float mass, btTransform& trans, 
                          btCollisionShape *shape) {
    
    btVector3 inertia;
    shape->calculateLocalInertia(mass, inertia);
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

//-----------------------------------------------------------------------------
void Moveable::update(float dt)
{
    m_motion_state->getWorldTransform(m_transform);
    m_velocityLC = getVelocity()*getTrans().getBasis();
    m_hpr.setHPR(m_transform.getBasis());

    updateGraphics(Vec3(0,0,0), Vec3(0,0,0));
    m_first_time = false ;
}   // update

//-----------------------------------------------------------------------------
void Moveable::updateGraphics(const Vec3& off_xyz, const Vec3& off_hpr)
{
    Vec3 xyz=getXYZ()+off_xyz;
    Vec3 hpr=getHPR()+off_hpr;
    sgCoord c=Coord(xyz, hpr).toSgCoord();
    
    m_model_transform->setTransform(&c);
}   // updateGraphics

