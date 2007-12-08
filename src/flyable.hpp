//  $Id: flyable.hpp 1284 2007-11-08 12:31:54Z hikerstk $
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

#ifndef HEADER_FLYABLE_H
#define HEADER_FLYABLE_H

#include "moveable.hpp"
#include "kart.hpp"

class Flyable : public Moveable
{
    sgCoord           m_last_pos;
    bool              m_has_hit_something;
    int               m_last_radar_beep;
    bool              m_exploded;
    int               m_HAT_counter;        // compute HAT only every N timesteps

protected:
    const Kart*       m_owner;              // the kart which released this flyable
    btCollisionShape *m_shape;
    float             m_max_height;
    float             m_min_height;
    float             m_force_updown;
    float             m_speed;
    float             m_mass;
    btVector3         m_extend;
    // The flyable class stores the values for each flyable type, e.g. 
    // speed, min_height, max_height. These variables must be static,
    // so we need arrays of these variables to have different values
    // for sparks, missiles, ...
    static float      m_st_speed[COLLECT_MAX];         // Speed of the projectile
    static ssgEntity* m_st_model[COLLECT_MAX];         // 3d model
    static float      m_st_min_height[COLLECT_MAX];    // min height above track
    static float      m_st_max_height[COLLECT_MAX];    // max height above track
    static float      m_st_force_updown[COLLECT_MAX];  // force pushing up/down 
    static btVector3  m_st_extend[COLLECT_MAX];        // size of the model

    float             m_current_HAT;        // height  above terrain
    void              getClosestKart(const Kart **minKart, float *minDist, 
                                     btVector3 *minDelta) const;
    virtual btCollisionShape *createShape()=0;
    virtual void      too_low  (float dt)    
                        {m_body->applyCentralForce(
                                 btVector3(0.0f, 0.0f, m_force_updown)); }
    virtual void      too_high(float dt)
                        {m_body->applyCentralForce(
                                 btVector3(0.0f, 0.0f, -m_force_updown)); }
    virtual void      right_height(float dt)
                        {btVector3 v=m_body->getLinearVelocity();
                         v.setZ(0.0f);
                         m_body->setLinearVelocity(v);  }
    void              createPhysics(const btVector3& offset, 
                                    const btVector3 velocity);
public:

                 Flyable     (Kart* kart, CollectableType type);
    virtual     ~Flyable     ();
    static void  init        (const lisp::Lisp* lisp, ssgEntity *model, 
                              CollectableType type);
    virtual void update      (float);

    void placeModel          ();
    virtual void hitTrack    () {};
    void         explode     (Kart* kart);
    bool         hasHit      ()               { return m_has_hit_something; }
    void         reset       () { Moveable::reset();
                                  sgCopyCoord(&m_last_pos,&m_reset_pos );   }
    void OutsideTrack        (int isReset)    { explode(NULL);              }
};   // Flyable

#endif
