//  $Id: homing.cpp 1284 2007-11-08 12:31:54Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
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

#include "homing.hpp"
#include "constants.hpp"

float Homing::m_st_max_distance;
float Homing::m_st_max_turn_angle;

// -----------------------------------------------------------------------------
/** A homing missile is handled as a kinematic object, since this simplifies
 *  computation of turning (otherwise rotational forces would have to be 
 *  applied). As a result, the mass must be zero, and linear velocity of the
 *  body can not be set (asserts in bullet). So this object implements its
 *  own setting/getting of velocity, to be able to use flyables functions.
 */
Homing::Homing (Kart *kart) : Flyable(kart, COLLECT_HOMING)
{
    m_mass = 0.0f;    // a kinematik object must have mass=0, otherwise warnings
                      // will be printed during bullet collision handling.
    float y_offset=kart->getKartLength()+2.0f*m_extend.getY();
    
    m_initial_velocity = btVector3(0.0f, m_speed, 0.0f);
    createPhysics(y_offset, m_initial_velocity, new btCylinderShape(0.5f*m_extend));
    m_body->setCollisionFlags(m_body->getCollisionFlags()           |
                              btCollisionObject::CF_KINEMATIC_OBJECT );
    m_body->setActivationState(DISABLE_DEACTIVATION);
}   // Homing

// -----------------------------------------------------------------------------
void Homing::init(const lisp::Lisp* lisp, ssgEntity *homing)
{
    Flyable::init(lisp, homing, COLLECT_HOMING);
    m_st_max_turn_angle = 15.0f;
    m_st_max_distance   = 20.0f;
    lisp->get("max-distance",    m_st_max_distance  );
    lisp->get("max-turn-angle",  m_st_max_turn_angle);
}   // init

// -----------------------------------------------------------------------------
void Homing::update(float dt)
{
    Flyable::update(dt);

    const Kart *kart=0;
    btVector3 direction;
    float minDistance;

    getClosestKart(&kart, &minDistance, &direction);
    btTransform my_trans=getTrans();
    if(minDistance<m_st_max_distance)   // move homing towards kart
    {
        btTransform target=kart->getTrans();
        
        float steer=steerTowards(my_trans, target.getOrigin());
        if(fabsf(steer)>90.0f) steer=0.0f;
        if(steer<-m_st_max_turn_angle)  steer = -m_st_max_turn_angle;
        if(steer> m_st_max_turn_angle)  steer =  m_st_max_turn_angle;
        btMatrix3x3 steerMatrix(btQuaternion(0.0f,0.0f,DEGREE_TO_RAD(steer)));
        my_trans.setBasis(my_trans.getBasis()*steerMatrix);
    }   // minDistance<m_st_max_distance
    btVector3 v =my_trans.getBasis()*m_initial_velocity;
    my_trans.setOrigin(my_trans.getOrigin()+dt*v);
    setTrans(my_trans);
}   // update
// -----------------------------------------------------------------------------
float Homing::steerTowards(btTransform& trans, btVector3& target)
{
    btMatrix3x3 m(trans.getBasis());
    btVector3 forwards(0.f,1.f,0.0f);
    btVector3 direction=m*forwards;
    float heading = RAD_TO_DEGREE(atan2(direction.getY(),direction.getX()));

    btVector3 pos=trans.getOrigin();
    float angle = RAD_TO_DEGREE(atan2(target.getY()-pos.getY(), target.getX()-pos.getX()));
    angle -=heading;
    if(angle> 180.0f) angle=angle-360.0f;
    if(angle<-180.0f) angle=angle+360.0f;
    return angle;
}   // steerTowards
