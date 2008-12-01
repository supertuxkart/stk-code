//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, Steve Baker
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

#ifndef HEADER_MOVEABLE_H
#define HEADER_MOVEABLE_H

#define _WINSOCKAPI_
#include <plib/ssg.h>
#include "btBulletDynamicsCommon.h"
#include "user_pointer.hpp"
#include "physics/kart_motion_state.hpp"
#include "utils/vec3.hpp"

class Material;

/* Limits of Kart performance */
#define CRASH_PITCH          -45.0f

#define MAX_ITEMS_COLLECTED    20


class Moveable 
{
private:
    btVector3        m_velocityLC;      /* velocity in kart coordinates                */
    btTransform      m_transform;
    Vec3             m_hpr;
protected:
    UserPointer      m_user_pointer;
    sgVec4          *m_normal_hot;      /* plane on which HOT was computed             */
    Material        *m_material_hot;    /* Material at HOT                             */
    ssgTransform    *m_model_transform;            // The transform where the model is under
    ssgTransform    *m_shadow;
    int              m_first_time ;
    btRigidBody     *m_body;
    KartMotionState *m_motion_state;
public:

    Moveable ();
    virtual ~Moveable();

    ssgTransform* getModelTransform()          {return m_model_transform;          }
    virtual const btVector3 &getVelocity() const {return m_body->getLinearVelocity();}
    const btVector3 &getVelocityLC() const     {return m_velocityLC;               }
    virtual void  setVelocity(const btVector3& v) {m_body->setLinearVelocity(v);   }
    const Vec3&   getXYZ       () const        {return (Vec3&)m_transform.getOrigin();}
    const Vec3&   getHPR       () const        {return m_hpr;                      }
    const btQuaternion getRotation() const     {return m_transform.getRotation();  }
    void          setXYZ       (const Vec3& a) {m_transform.setOrigin(a);
                                                m_motion_state->setWorldTransform(m_transform);}
    void          setRotation  (const btQuaternion&a){m_transform.setRotation(a);
                                                m_motion_state->setWorldTransform(m_transform);}
    void          setXYZRotation(const Vec3& xyz, const btQuaternion& a)
                                               {m_transform.setRotation(a);
                                                m_transform.setOrigin(xyz);
                                                m_motion_state->setWorldTransform(m_transform);}
    const sgVec4* getNormalHOT () const        {return m_normal_hot;               }
    virtual void  updateGraphics(const Vec3& off_xyz,  const Vec3& off_hpr);
    virtual void  handleZipper ()              {};
    virtual void  reset        ();
    virtual void  update       (float dt) ;
    btRigidBody*  getBody   () const {return m_body; }
    void          createBody(float mass, btTransform& trans, 
                             btCollisionShape *shape);
    const btTransform&  getTrans() const {return m_transform;}
    void          setTrans  (const btTransform& t);
}
;   // class Moveable

#endif
