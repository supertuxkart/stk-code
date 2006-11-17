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

/* Limits of Kart performance */
#define CRASH_PITCH          -45.0f

#define MAX_NATURAL_VELOCITY    ( 60.0f * KILOMETERS_PER_HOUR )
#define MIN_CRASH_VELOCITY   (MAX_NATURAL_VELOCITY * 0.2f)
#define MIN_COLLIDE_VELOCITY (MAX_NATURAL_VELOCITY * 0.1f)
#define COLLIDE_BRAKING_RATE (MAX_NATURAL_VELOCITY * 1.0f)

#define MAX_HERRING_EATEN    20


class Moveable
{
protected:
    sgCoord       m_reset_pos;      /* Where to start in case of a reset           */
    sgCoord       m_curr_pos;       /* current position                            */
    sgCoord       m_velocity;       /* current velocity in local coordinates       */
    sgVec3        m_abs_velocity;   /* world coordinates' velocity vector          */
    sgVec4*       m_normal_hot;      /* plane on which HOT was computed             */
    Material*     m_material_hot;    /* Material at HOT                             */
    ssgTransform* m_model;
    ssgTransform* m_shadow;
    int           m_collided;
    int           m_crashed;
    sgVec3        m_surface_avoidance_vector ;
    int           m_first_time ;
    float         m_wheelie_angle ;
    int           m_on_ground ;

    float collectIsectData ( sgVec3 start, sgVec3 end ) ;
    sgCoord*      m_history_velocity;
    sgCoord*      m_history_position;

public:

    /* start - New Physics */


    Moveable (bool bHasHistory=false);
    virtual ~Moveable();

    void          setReset     (sgCoord* pos)  {sgCopyCoord( &m_reset_pos, pos ); }
    ssgTransform* getModel     ()              {return m_model ;                  }
    int           isOnGround   ()              {return m_on_ground;               }
    sgCoord*      getVelocity  ()              {return & m_velocity;              }
    sgCoord*      getCoord     ()              {return &m_curr_pos;               }
    void          setCoord     (sgCoord* pos)  {sgCopyCoord ( &m_curr_pos,pos);   }
    virtual void  placeModel   ()              {m_model->setTransform(&m_curr_pos); }
    virtual void  handleZipper ()              {};
    virtual void  reset        ();
    virtual void  update       (float dt) ;
    virtual void  updatePosition(float dt, sgMat4 result);
    virtual void  doCollisionAnalysis(float dt, float hot);

    // Gets called when no high of terrain can be determined (isReset=0), or
    // there is a 'reset' material under the moveable --> karts need to be
    // rescued, missiles should explode.
    virtual void  OutsideTrack (int isReset) {}

    float         getIsectData (sgVec3 start, sgVec3 end );
    void          WriteHistory (char* s, int kartNumber, int indx);
    void          ReadHistory  (char* s, int kartNumber, int indx);
}
;   // class Moveable

#endif
