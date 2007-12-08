//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, Steve Baker
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

#ifndef HEADER_MOVEABLE_H
#define HEADER_MOVEABLE_H

#include <plib/ssg.h>
#include "material.hpp"
#ifdef BULLET
#include "btBulletDynamicsCommon.h"
#endif

/* Limits of Kart performance */
#define CRASH_PITCH          -45.0f

#define MAX_NATURAL_VELOCITY    ( 60.0f * KILOMETERS_PER_HOUR )
#define MIN_CRASH_VELOCITY   (MAX_NATURAL_VELOCITY * 0.2f)
#define MIN_COLLIDE_VELOCITY (MAX_NATURAL_VELOCITY * 0.1f)
#define COLLIDE_BRAKING_RATE (MAX_NATURAL_VELOCITY * 1.0f)

#define MAX_HERRING_EATEN    20


class Moveable
{
public:
    enum   MoveableType {MOV_KART, MOV_PROJECTILE, MOV_TRACK} ;
protected:
    MoveableType  m_moveable_type;  /* used when upcasting bullet user pointers    */
    sgCoord       m_reset_pos;      /* Where to start in case of a reset           */
    sgCoord       m_curr_pos;       /* current position                            */
    sgCoord       m_velocity;       /* current velocity in local coordinates       */
    sgVec3        m_abs_velocity;   /* world coordinates' velocity vector          */
    sgVec4*       m_normal_hot;      /* plane on which HOT was computed             */
    Material*     m_material_hot;    /* Material at HOT                             */
    ssgTransform* m_model_transform; // The transform where the model is under
    ssgTransform* m_shadow;
    int           m_collided;
    int           m_crashed;
    sgVec3        m_surface_avoidance_vector ;
    int           m_first_time ;

    float collectIsectData ( sgVec3 start, sgVec3 end ) ;
    sgCoord*      m_history_velocity;
    sgCoord*      m_history_position;
#ifdef BULLET
    btRigidBody*          m_body;
    btDefaultMotionState* m_motion_state;
#endif

public:

    /* start - New Physics */


    Moveable (bool bHasHistory=false);
    virtual ~Moveable();

    ssgTransform* getModelTransform()          {return m_model_transform;         }
    MoveableType  getMoveableType()  const     {return m_moveable_type;           }
    void          setMoveableType(MoveableType m){m_moveable_type=m;              }
    sgCoord*      getVelocity  ()              {return & m_velocity;              }
    sgCoord*      getCoord     ()              {return &m_curr_pos;               }
    const sgCoord* getCoord    ()  const       {return &m_curr_pos;               }
    const sgVec4* getNormalHOT ()  const       {return m_normal_hot;              }
    void          setCoord     (sgCoord* pos)  {sgCopyCoord ( &m_curr_pos,pos);   }
    virtual void  placeModel   ()              {m_model_transform->setTransform(&m_curr_pos); }
    virtual void  handleZipper ()              {};
    virtual void  reset        ();
    virtual void  update       (float dt) ;
    virtual void  updatePosition(float dt, sgMat4 result);
#ifndef BULLET
    virtual void  doCollisionAnalysis(float dt, float hot);
#endif

    // Gets called when no high of terrain can be determined (isReset=0), or
    // there is a 'reset' material under the moveable --> karts need to be
    // rescued, missiles should explode.
    virtual void  OutsideTrack (int isReset) {}

    float         getIsectData (sgVec3 start, sgVec3 end );
    void          WriteHistory (char* s, int kartNumber, int indx);
    void          ReadHistory  (char* s, int kartNumber, int indx);
    btRigidBody*  getBody   () const {return m_body; }
    void          createBody(float mass, btTransform& trans, 
                             btCollisionShape *shape, MoveableType m);
    void          getTrans  (btTransform* t) const 
                                            {m_motion_state->getWorldTransform(*t);}
    void          setTrans  (btTransform& t){m_motion_state->setWorldTransform(t);}
}
;   // class Moveable

#endif
