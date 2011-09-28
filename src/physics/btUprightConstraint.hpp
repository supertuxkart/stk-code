/*

Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim
   that you wrote the original software. If you use this software in a 
   product, an acknowledgment in the product documentation would be 
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef HEADER_UPRIGHT_CONSTRAINT_HPP
#define HEADER_UPRIGHT_CONSTRAINT_HPP

#include "LinearMath/btVector3.h"
#include "BulletDynamics/ConstraintSolver/btJacobianEntry.h"
#include "BulletDynamics/ConstraintSolver/btTypedConstraint.h"

class btRigidBody;
class Kart;

/**
  * \ingroup physics
  */
class btUprightConstraint : public btTypedConstraint
{
private:
    class btUprightConstraintLimit
    {
    public:
        btVector3   m_axis;
        btScalar    m_angle;
        btScalar    m_accumulatedImpulse;
        btScalar    m_currentLimitError;
    };

    //! relative_frames

    //!@{
    btTransform     m_frameInA;//!< the constraint space w.r.t body A
    //!@}

    //! Jacobians
    //!@{
    btJacobianEntry m_jacAng[ 2 ];//!< angular constraint
    //!@}

    const Kart     *m_kart;
protected:

    //! temporal variables
    //!@{
        btScalar    m_timeStep;
        btScalar    m_ERP;
        btScalar    m_bounce;
        btScalar    m_damping;
        btScalar    m_maxLimitForce;
        btScalar    m_limitSoftness;
        btScalar    m_hiLimit;
        btScalar    m_loLimit;
        btScalar    m_disable_time;

        btUprightConstraintLimit        m_limit[ 2 ];

    //!@}

    btUprightConstraint& operator=(btUprightConstraint& other)
    {
        btAssert(0);
        (void) other;
        return *this;
    }

    void buildAngularJacobian(btJacobianEntry & jacAngular,
                              const btVector3 & jointAxisW);

    void solveAngularLimit(btUprightConstraintLimit *limit,
                           btScalar timeStep, btScalar jacDiagABInv,
                           btRigidBody * body0 );

public:

         btUprightConstraint(const Kart* kart, const btTransform& frameInA);

    // -PI,+PI                      is the full range
    // 0,0                          is no rotation around x or z
    // -PI*0.2,+PI*0.2      is a nice bit of tilt
    void setLimit( btScalar range )            { m_loLimit = -range;
                                                 m_hiLimit = +range;        }
    // Error correction scaling
    // 0 - 1
    void setErp( btScalar erp )                { m_ERP = erp;                }
    void setBounce( btScalar bounce )          { m_bounce = bounce;          }
    void setMaxLimitForce( btScalar force )    { m_maxLimitForce = force;    }
    void setLimitSoftness( btScalar softness ) { m_limitSoftness = softness; }
    void setDamping( btScalar damping )        { m_damping = damping;        }
    void setDisableTime( btScalar t )          { m_disable_time = t;         }
    virtual void buildJacobian();
    virtual void solveConstraintObsolete(btRigidBody& /*bodyA*/,
                                         btRigidBody& /*bodyB*/, btScalar  
                                         timeStep);
    virtual void getInfo1 (btConstraintInfo1* info);
    virtual void getInfo2 (btConstraintInfo2* info);
    virtual	void setParam(int num, btScalar value, int axis = -1);
    virtual btScalar getParam(int num, int axis) const;

};

 

#endif //UPRIGHT_CONSTRAINT_H

