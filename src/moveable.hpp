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
#include "btBulletDynamicsCommon.h"
#include "user_pointer.hpp"

/* Limits of Kart performance */
#define CRASH_PITCH          -45.0f

#define MAX_HERRING_EATEN    20


class Moveable 
{
private:
    btVector3     m_velocityLC;      /* velocity in kart coordinates                */
protected:
    UserPointer   m_user_pointer;
    sgCoord       m_reset_pos;       /* Where to start in case of a reset           */
    sgCoord       m_curr_pos;        /* current position                            */
    sgVec4*       m_normal_hot;      /* plane on which HOT was computed             */
    Material*     m_material_hot;    /* Material at HOT                             */
    ssgTransform* m_model_transform;            // The transform where the model is under
    ssgTransform* m_shadow;
    int           m_collided;
    int           m_crashed;
    sgVec3        m_surface_avoidance_vector ;
    int           m_first_time ;
    sgCoord*      m_history_velocity;
    sgCoord*      m_history_position;
    btRigidBody*          m_body;
    btDefaultMotionState* m_motion_state;
    btTransform   m_transform;

public:

    Moveable (bool bHasHistory=false);
    virtual ~Moveable();

    ssgTransform* getModelTransform()          {return m_model_transform;          }
    virtual const btVector3 &getVelocity() const {return m_body->getLinearVelocity();}
    const btVector3 &getVelocityLC() const     {return m_velocityLC;               }
    virtual void  setVelocity(const btVector3& v) {m_body->setLinearVelocity(v);   }
    sgCoord*      getCoord     ()              {return &m_curr_pos;                }
    const btVector3 getPos     ()  const       {return m_transform.getOrigin();    }
    const sgCoord* getCoord    ()  const       {return &m_curr_pos;                }
    const sgVec4* getNormalHOT ()  const       {return m_normal_hot;               }
    void          setCoord     (sgCoord* pos)  {sgCopyCoord ( &m_curr_pos,pos);    }
    virtual void  placeModel   ();
    virtual void  handleZipper ()              {};
    virtual void  reset        ();
    virtual void  update       (float dt) ;
    void          WriteHistory (char* s, int kartNumber, int indx);
    void          ReadHistory  (char* s, int kartNumber, int indx);
    btRigidBody*  getBody   () const {return m_body; }
    void          createBody(float mass, btTransform& trans, 
                             btCollisionShape *shape);
    const btTransform&  getTrans() const {return m_transform;}
    void          setTrans  (btTransform& t){m_transform=t;m_motion_state->setWorldTransform(t);}
}
;   // class Moveable

#endif
