//  $Id: missile.hpp 1284 2007-11-08 12:31:54Z hikerstk $
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

#ifndef HEADER_HOMING_H
#define HEADER_HOMING_H

#include "flyable.hpp"

class Homing : public Flyable
{
private:
    static float m_st_max_distance;    // maximum distance for a spark to be attracted
    static float m_st_max_turn_angle;
    btVector3    m_initial_velocity;
    float        steerTowards(btTransform& trans, btVector3& target);

public:
    Homing (Kart *kart);
    static  void init     (const lisp::Lisp* lisp, ssgEntity* homing);
    virtual void update   (float dt);
    virtual void hitTrack ()                      { explode(NULL);           }
    // Kinematic objects are not allowed to have a velocity (assertion in 
    // bullet), so we have to do our own velocity handling here
    virtual const btVector3 &getVelocity() const  {return m_initial_velocity;}
    virtual void  setVelocity(const btVector3& v) {m_initial_velocity=v;     }

};   // Homing

#endif
