//  $Id: spark.cpp 1284 2007-11-08 12:31:54Z hikerstk $
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

#include "spark.hpp"
float Spark::m_st_max_distance;   // maximum distance for a spark to be attracted
float Spark::m_st_force_to_target;

// -----------------------------------------------------------------------------
Spark::Spark(Kart *kart) : Flyable(kart, COLLECT_SPARK)
{
    btVector3 offset(0.0f, 
                     -0.5f*kart->getKartLength()-2.0f*m_extend.getY(), 
                     0.5f*kart->getKartHeight());
    float     speed=-m_speed;
    // if the kart is driving backwards, release from the front
    if(m_owner->getSpeed()<0) 
    {
        offset.setY(0.5f*kart->getKartLength()+2.0f*m_extend.getY());
        speed   = m_speed;
    }

    createPhysics(offset, btVector3(0.0f, speed, 0.0f));

    // unset no_contact_response flags, so that the spark 
    // will bounce off the track
    int flag = getBody()->getCollisionFlags();
    flag = flag & (~ btCollisionObject::CF_NO_CONTACT_RESPONSE);
    getBody()->setCollisionFlags(flag);
}   // Spark

// -----------------------------------------------------------------------------
void Spark::init(const lisp::Lisp* lisp, ssgEntity *spark)
{
    Flyable::init(lisp, spark, COLLECT_SPARK);
    m_st_max_distance    = 20.0f;
    m_st_force_to_target = 10.0f;
 
    lisp->get("max-distance",    m_st_max_distance   );
    lisp->get("force-to-target", m_st_force_to_target);
}   // init

// -----------------------------------------------------------------------------
btCollisionShape *Spark::createShape()
{
    return new btSphereShape(0.5f*m_extend.getY());
    
}   // createShape

// -----------------------------------------------------------------------------
void Spark::update(float dt)
{
    Flyable::update(dt);
    const Kart *kart=0;
    btVector3 direction;
    float minDistance;
    getClosestKart(&kart, &minDistance, &direction);
    if(minDistance<m_st_max_distance)   // move spark towards kart
    {
        direction*=1/direction.length()*m_st_force_to_target;
        m_body->applyCentralForce(direction);
    }
    else
    {   // Sparks lose energy (e.g. when hitting the track), so increase the
        // speed if the spark is too slow, but only if it's not too high (if
        // the spark is too high, it is 'pushed down', which can reduce the
        // speed, which causes the speed to increase, which in turn causes
        // the spark to fly higher and higher.
        btVector3 v=m_body->getLinearVelocity();
        if (m_current_HAT<= m_max_height)
        {
            float vlen = v.length2();
            if(vlen<0.8*m_speed*m_speed)
            {   // spark lost energy (less than 80%), i.e. it's too slow - speed it up:
                if(vlen==0.0f) {
                    v    = btVector3(.5f, .5f, .5f);  // avoid 0 div.
                    vlen = 0.75;
                }
                else
                {
                    m_body->setLinearVelocity(v*m_speed/sqrt(vlen));
                }
            }   // vlen < 0.8*m_speed*m_speed
        }   // m_current_HAT < m_max_height  
    }   // spar lose energy
}   // update
// -----------------------------------------------------------------------------
