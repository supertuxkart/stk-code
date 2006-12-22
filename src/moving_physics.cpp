//  $Id: moving_physics.cpp 839 2006-10-24 00:01:56Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include "moving_physics.hpp"
#include "world.hpp"

#ifdef BULLET
// -----------------------------------------------------------------------------
MovingPhysics::MovingPhysics(char *data, bodyTypes type) 
             : ssgTransform(), Callback()
{
    if(type==BODY_CONE)
    {
        m_shape = new btConeShape(1, 1);
        setName("cone");
    }

    btTransform trans;
    trans.setIdentity();
    trans.setOrigin(btVector3(0, 8, 5+rand()%10));
    m_motion_state = new btDefaultMotionState(trans);
    float mass     = 0.1;
    btVector3 inertia;
    m_shape->calculateLocalInertia(mass, inertia);
    m_body = new btRigidBody(mass, m_motion_state, m_shape, inertia);
    setUserData(new ssgBase());
    world->getPhysics()->getPhysicsWorld()->addRigidBody(m_body);
}

// -----------------------------------------------------------------------------
MovingPhysics::~MovingPhysics()
{
    world->getPhysics()->getPhysicsWorld()->removeRigidBody(m_body);
    delete m_shape;
    delete m_motion_state;
    delete m_body;
}  // ~MovingPhysics

// -----------------------------------------------------------------------------
void MovingPhysics::update(float dt)
{
    btTransform t;
    m_motion_state->getWorldTransform(t);
    float m[4][4];
    t.getOpenGLMatrix((float*)&m);

    printf("%lx is %f %f %f\n",this, t.getOrigin().x(),t.getOrigin().y(),t.getOrigin().z());
    // Transfer the new position and hpr to m_curr_pos
    sgCoord m_curr_pos;
    sgSetCoord(&m_curr_pos, m);
    setTransform(&m_curr_pos);
    //    printf("Update moving physics called\n");
}   // update
// -----------------------------------------------------------------------------
#endif

/* EOF */

