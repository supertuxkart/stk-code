//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 Joerg Henrichs, Steve Baker
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

#ifndef HEADER_MOVEABLE_HPP
#define HEADER_MOVEABLE_HPP

namespace irr
{
    namespace scene { class IMesh; class IMeshSceneNode; class ISceneNode; }
}
using namespace irr;
#include "btBulletDynamicsCommon.h"

#include "physics/kart_motion_state.hpp"
#include "physics/user_pointer.hpp"
#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

class Material;

/**
  * \ingroup karts
  */
class Moveable: public NoCopy
{
private:
    btVector3              m_velocityLC;      /**<Velocity in kart coordinates. */
    btTransform            m_transform;
    /** The 'real' heading between -180 to 180 degrees. */
    float                  m_heading;
    /** The pitch between -90 and 90 degrees. */
    float                  m_pitch;
    /** The roll between -180 and 180 degrees. */
    float                  m_roll;

protected:
    UserPointer            m_user_pointer;
    scene::IMesh          *m_mesh;
    scene::ISceneNode     *m_node;
    btRigidBody           *m_body;
    KartMotionState       *m_motion_state;

public:
                  Moveable();
    virtual      ~Moveable();
    /** Returns the scene node of this moveable. */
    scene::ISceneNode
                 *getNode() const { return m_node; }
    void          setNode(scene::ISceneNode *n);
    virtual const btVector3
                 &getVelocity()   const        {return m_body->getLinearVelocity();}
    const btVector3
                 &getVelocityLC() const        {return m_velocityLC;               }
    virtual void  setVelocity(const btVector3& v) {m_body->setLinearVelocity(v);   }
    const Vec3&   getXYZ()        const        {return (Vec3&)m_transform.getOrigin();}
    /** Returns the heading between -pi and pi. */
    float         getHeading()    const        {return m_heading;                  }
   /** Returns the pitch of the kart, restricted to between -pi/2 and pi/2. */
    float         getPitch()      const        {return m_pitch;                    }
    /** Returns the roll of the kart between -pi and pi.  */
    float         getRoll()       const        {return m_roll;                     }
    const btQuaternion
                  getRotation()   const        {return m_transform.getRotation();  }

    /** Enter flying mode */
    virtual void flyUp();
    virtual void flyDown();
    virtual void stopFlying();

    /** Sets the XYZ coordinates of the moveable. */
    virtual void setXYZ(const Vec3& a)
    {
        m_transform.setOrigin(a);
        if(m_motion_state)
            m_motion_state->setWorldTransform(m_transform);
    }   // setXYZ
    // ------------------------------------------------------------------------
    /** Sets the rotation of the physical body this moveable. */
    void setRotation(const btMatrix3x3 &m)
    {
        m_transform.setBasis(m);
        if(m_motion_state)
            m_motion_state->setWorldTransform(m_transform);
    }   // setRotation(btMatrix3x3)
    // ------------------------------------------------------------------------
    /** Sets the rotation of the physical body this moveable. */
    void setRotation(const btQuaternion &q)
    {
        m_transform.setRotation(q);
        if(m_motion_state)
            m_motion_state->setWorldTransform(m_transform);
    }   // setRotation(btQuaternion)
    // ------------------------------------------------------------------------
    virtual void  updateGraphics(float dt, const Vec3& off_xyz,
                                 const btQuaternion& off_rotation);
    virtual void  reset();
    virtual void  update(float dt) ;
    btRigidBody  *getBody() const {return m_body; }
    void          createBody(float mass, btTransform& trans,
                             btCollisionShape *shape,
                             float restitution);
    const btTransform
                 &getTrans() const {return m_transform;}
    void          setTrans(const btTransform& t);
    void          updatePosition();
}
;   // class Moveable

#endif
