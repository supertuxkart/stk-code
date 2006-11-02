//  $Id: physics.hpp 839 2006-10-24 00:01:56Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Joerg Henrichs, Steve Baker
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

#include "physics.hpp"
#include "ssg_help.hpp"
#include "world.hpp"

#ifdef BULLET
#include "../bullet/Demos/OpenGL/GL_ShapeDrawer.h"

// -----------------------------------------------------------------------------
/// Initialise physics.
Physics::Physics(float gravity)
{
    m_dynamics_world = new btDiscreteDynamicsWorld();
    m_dynamics_world->setGravity(btVector3(0.0f, 0.0f, -gravity));
    m_debug_drawer=new GLDebugDrawer();
    m_debug_drawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    m_dynamics_world->setDebugDrawer(m_debug_drawer);

}   // Physics

// -----------------------------------------------------------------------------
Physics::~Physics()
{
    delete m_dynamics_world;
}   // ~Physics

// -----------------------------------------------------------------------------
//* Set track
void Physics::set_track(ssgEntity* track)
{
}   // set_track

// -----------------------------------------------------------------------------
//* Adds a kart to the physics engine
void Physics::addKart(Kart *kart, btRaycastVehicle **vehicle, 
                      btRaycastVehicle::btVehicleTuning **tuning)
{

    // First: Create the chassis of the kart
    // -------------------------------------
    // The size for bullet must be specified in half extends!
    ssgEntity *model = kart->getModel();
    float x_min, x_max, y_min, y_max, z_min, z_max;
    MinMax(model, &x_min, &x_max, &y_min, &y_max, &z_min, &z_max);
    float kart_width  = x_max-x_min;
    float kart_length = y_max-y_min;
    btCollisionShape *kart_chassis = new btBoxShape(btVector3(0.5*kart_width,
                                                              0.5*kart_length,
                                                              0.5*(z_max-z_min)));
    // Set mass and inertia
    // --------------------
    float mass=kart->getMass();
    btVector3 inertia;
    kart_chassis->calculateLocalInertia(mass, inertia);

    // Position the chassis
    // --------------------
    btTransform trans;
    trans.setIdentity();
    sgCoord *pos=kart->getCoord();
    trans.setOrigin(btVector3(pos->xyz[0], pos->xyz[1], pos->xyz[2]+100.0f));
    btDefaultMotionState* myMotionState = new btDefaultMotionState(trans);

    // Then create a rigid body
    // ------------------------
    btRigidBody* kart_body = new btRigidBody(mass, myMotionState, 
                                             kart_chassis, inertia);
    kart_body->setCenterOfMassTransform(btTransform::getIdentity());
    m_dynamics_world->addRigidBody(kart_body);

    // Reset velocities
    // ----------------
    kart_body->setLinearVelocity (btVector3(0.0f,0.0f,0.0f));
    kart_body->setAngularVelocity(btVector3(0.0f,0.0f,0.0f));

    // Create the actual vehicle
    // -------------------------
    btVehicleRaycaster *vehicle_raycaster = new btDefaultVehicleRaycaster(m_dynamics_world);
    *tuning = new btRaycastVehicle::btVehicleTuning();
    *vehicle = new btRaycastVehicle(**tuning, kart_body, vehicle_raycaster);

    // never deactivate the vehicle
    kart_body->SetActivationState(DISABLE_DEACTIVATION);
    m_dynamics_world->addVehicle(*vehicle);
    (*vehicle)->setCoordinateSystem(/*right: */ 0,  /*up: */ 2,  /*forward: */ 1);
    
    // Add wheels
    // ----------
    float wheel_width  = kart->getWheelWidth();
    float wheel_radius = kart->getWheelRadius();
    float suspension_rest = 0.6f;
    btVector3 wheel_coord(0.5f*kart_width-(0.3f*wheel_width), 
                          0.5f*kart_length-wheel_radius,
                          0.0f);
    btVector3 wheel_direction(0.0f, 0.0f, -1.0f);
    btVector3 wheel_axle(1.0f,0.0f,0.0f);

    // right front wheel
    (*vehicle)->addWheel(wheel_coord, wheel_direction, wheel_axle,
                         suspension_rest, wheel_radius, **tuning,
                         /* isFrontWheel: */ true);

    // left front wheel
    wheel_coord = btVector3(- (0.5f*kart_width-(0.3f*wheel_width)), 
                            0.5f*kart_length-wheel_radius,
                            0.0f);
    (*vehicle)->addWheel(wheel_coord, wheel_direction, wheel_axle,
                         suspension_rest, wheel_radius, **tuning,
                         /* isFrontWheel: */ true);

    // right rear wheel
    wheel_coord = btVector3(0.5*kart_width-(0.3f*wheel_width), 
                            -0.5*(kart_length-wheel_radius),
                            0.0f);
    (*vehicle)->addWheel(wheel_coord, wheel_direction, wheel_axle,
                         suspension_rest, wheel_radius, **tuning,
                         /* isFrontWheel: */ false);

    // right rear wheel
    wheel_coord = btVector3(-(0.5*kart_width-(0.3f*wheel_width)),
                            -0.5*(kart_length-wheel_radius),
                            0.0f);
    (*vehicle)->addWheel(wheel_coord, wheel_direction, wheel_axle,
                         suspension_rest, wheel_radius, **tuning,
                         /* isFrontWheel: */ false);
}   // addKart

// -----------------------------------------------------------------------------
//* 
void Physics::update(float dt)
{

    for(int i=0; i<world->getNumKarts(); i++)
    {
        Kart *k = world->getKart(i);
        sgCoord *curr_pos = k->getCoord();

        btRaycastVehicle *vehicle=k->getVehicle();
        btRigidBody *b=vehicle->getRigidBody();
        btMotionState *m = b->getMotionState();
        btTransform trans;
        trans.setIdentity();
        trans.setOrigin(btVector3(curr_pos->xyz[0],curr_pos->xyz[1],curr_pos->xyz[2]));
        m->setWorldTransform(trans);
        //        m->getWorldTransform(trans);
        btVector3 p = trans.getOrigin();
        printf("%lx Pos %f %f %f\n",vehicle, p.x(), p.y(), p.z());

        btCylinderShapeX wheelShape(btVector3(k->getWheelWidth(),
                                              k->getWheelRadius(),
                                              k->getWheelRadius()));
        btVector3 wheelColor(1,0,0);
        for(int j=0; j<vehicle->getNumWheels(); j++)
        {
            vehicle->updateWheelTransform(j);
            float m[16];
            vehicle->getWheelInfo(j).m_worldTransform.getOpenGLMatrix(m);
            GL_ShapeDrawer::drawOpenGL(m, &wheelShape, wheelColor,btIDebugDraw::DBG_DrawWireframe);
        }
    }

    int num_objects = m_dynamics_world->getNumCollisionObjects();
    for(int i=0; i<num_objects; i++) 
    {
        Kart *k=world->getKart(i);
        btCollisionObject *obj = m_dynamics_world->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if(!body) continue;
        const btVector3 &pos=body->getCenterOfMassPosition();
        printf("body %d: %f %f %f dt %f\n",i, pos.x(), pos.y(), pos.z(),dt);
        sgCoord *p = k->getCoord();
        p->xyz[0]=pos.x();
        p->xyz[1]=pos.y();
        p->xyz[2]=pos.z();
        float m[16];
		btVector3 wireColor(1,0,0);
        btDefaultMotionState *myMotion = (btDefaultMotionState*)body->getMotionState();
        myMotion->m_graphicsWorldTrans.getOpenGLMatrix(m);
        GL_ShapeDrawer::drawOpenGL(m,obj->m_collisionShape,wireColor,
                                   btIDebugDraw::DBG_DrawWireframe);
        
    }
	// ???  m_dynamicsWorld->updateAabbs();

}   // update

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif
/* EOF */
  
