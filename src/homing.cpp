//  $Id: homing.cpp 1284 2007-11-08 12:31:54Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
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

#include "homing.hpp"
#include "constants.hpp"

#include <iostream>

float Homing::m_st_max_distance;
float Homing::m_st_max_distance_squared;
float Homing::m_st_max_turn_angle;

Homing::Homing (Kart *kart) : Flyable(kart, COLLECT_HOMING)
{
    m_target = NULL;
    
    // A bit of a hack: the mass of this kinematic object is still 1.0 
    // (see flyable), which enables collisions. I tried setting 
    // collisionFilterGroup/mask, but still couldn't get this object to 
    // collide with the track. By setting the mass to 1, collisions happen.
    // (if bullet is compiled with _DEBUG, a warning will be printed the first
    // time a homing-track collision happens).
    float y_offset=kart->getKartLength()+2.0f*m_extend.getY();
    
    float up_velocity = m_speed/7.0f;
    
    btTransform trans = kart->getTrans();
    
    const float pitch = 0.0f; //getTerrainPitch(heading);
    
    // find closest kart in front of the current one
    const Kart *closest_kart=0;   btVector3 direction;   float kartDistSquared;
    getClosestKart(&closest_kart, &kartDistSquared, &direction, kart /* search in front of this kart */);
    
    // aim at this kart if 1) it's not too far, 2) if the aimed kart's speed
    // allows the projectile to catch up with it
    if(closest_kart != NULL && kartDistSquared < m_st_max_distance_squared && m_speed>closest_kart->getSpeed())
    {
        m_target = (Kart*)closest_kart;
        //std::cout << "aiming at " << closest_kart->getName().c_str() << std::endl;
        
        // calculate appropriate initial up velocity so that the
        // projectile lands on the aimed kart (9.8 is the gravity)
        const float time = sqrt(kartDistSquared) / (m_speed - closest_kart->getSpeed()/2.5f); // the division is an empirical estimation
        up_velocity = time*9.8f;
        
        // calculate the approximate location of the aimed kart in 'time' seconds
        btVector3 closestKartLoc = closest_kart->getTrans().getOrigin();
        closestKartLoc += time*closest_kart->getVelocity();
        
        // calculate the angle at which the projectile should be thrown
        // to hit the aimed kart
        float projectileAngle=atan2(-(closestKartLoc.getX() - kart->getTrans().getOrigin().getX()),
                                      closestKartLoc.getY() - kart->getTrans().getOrigin().getY() );

        btMatrix3x3 thisKartDirMatrix = kart->getKartHeading().getBasis();
        btVector3 thisKartDirVector(thisKartDirMatrix[0][1],
                                    thisKartDirMatrix[1][1],
                                    thisKartDirMatrix[2][1]);

        // apply transformation to the bullet object
        btMatrix3x3 m;
        m.setEulerZYX(pitch, 0.0f, projectileAngle /*+thisKartAngle*/);
        trans.setBasis(m);
        
    }
    else
    {
        m_target = NULL;
        // kart is too far to be hit. so throw the projectile in a generic way,
        // straight ahead, without trying to hit anything in particular
        trans = kart->getKartHeading(pitch);
        /*
        std::cout << "Using generic direction because ";
        
        if(closest_kart == NULL) std::cout << "no closest kart was found ";
        else if(!(kartDistSquared < m_st_max_distance_squared)) std::cout <<  closest_kart->getName().c_str() << " is too far : " << sqrt(kartDistSquared) << " (" << kartDistSquared << ")";
        if(closest_kart != NULL && !(m_speed>closest_kart->getSpeed())) std::cout << "kart is too fast ";
        std::cout << std::endl;
        */
    }
    

    m_initial_velocity = btVector3(0.0f, m_speed, up_velocity);
    
    createPhysics(y_offset, m_initial_velocity, 
                  new btCylinderShape(0.5f*m_extend), true /* gravity */, true /* rotation */, &trans);
   // m_body->setCollisionFlags(m_body->getCollisionFlags()           |
   //                           btCollisionObject::CF_KINEMATIC_OBJECT );
    m_body->setActivationState(DISABLE_DEACTIVATION);
    
    m_body->applyTorque( btVector3(5,-3,7) );
    
}   // Homing

// -----------------------------------------------------------------------------
void Homing::init(const lisp::Lisp* lisp, ssgEntity *homing)
{
    Flyable::init(lisp, homing, COLLECT_HOMING);
    m_st_max_turn_angle = 15.0f;
    m_st_max_distance   = 80.0f;
    m_st_max_distance_squared = 80.0f * 80.0f;
    
    lisp->get("max-distance",    m_st_max_distance  );
    m_st_max_distance_squared = m_st_max_distance*m_st_max_distance;
    
    lisp->get("max-turn-angle",  m_st_max_turn_angle);
}   // init

// -----------------------------------------------------------------------------
void Homing::update(float dt)
{
    
    if(m_target != NULL)
    {
        // correct direction to go towards aimed kart
        btTransform my_trans = getTrans();
        btTransform target   = m_target->getTrans();
        
        btVector3 ideal_direction = target.getOrigin() - my_trans.getOrigin();
        ideal_direction.normalize();
        
        const btVector3& actual_direction = m_body -> getLinearVelocity();

        ideal_direction.setInterpolate3(actual_direction.normalized(), ideal_direction, dt);
        
        const int current_xy_speed = sqrt( actual_direction.getX()*actual_direction.getX() +
                                           actual_direction.getY()*actual_direction.getY());
        
        m_body->setLinearVelocity( btVector3(ideal_direction.getX()*current_xy_speed,
                                             ideal_direction.getY()*current_xy_speed,
                                             actual_direction.getZ()) );
        
        /*
        // pull towards aimed kart
        btVector3 pullForce = target.getOrigin() - my_trans.getOrigin();
        pullForce.setZ(0);
        pullForce.normalize();
        pullForce *= 10;
        m_body->applyCentralImpulse( pullForce );
        */
        /*
        // if over aimed kart, pull down
        if(fabsf(my_trans.getOrigin().getX() - target.getOrigin().getX()) < 5.0 &&
           fabsf(my_trans.getOrigin().getY() - target.getOrigin().getY()) < 5.0)
        {
            m_body->applyCentralForce( btVector3(0, 0, -20.0f) );
        }
        */
    }
    
    Flyable::update(dt);
}   // update