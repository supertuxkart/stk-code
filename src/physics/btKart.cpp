/*
 * Copyright (c) 2005 Erwin Coumans http://continuousphysics.com/Bullet/
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies.
 * Erwin Coumans makes no representations about the suitability
 * of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
*/

#include "physics/btKart.hpp"

#include "LinearMath/btMinMax.h"
#include "LinearMath/btVector3.h"
#include "LinearMath/btQuaternion.h"
#include "BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.h"
#include "BulletDynamics/ConstraintSolver/btJacobianEntry.h"
#include "BulletDynamics/Dynamics/btDynamicsWorld.h"
#include "BulletDynamics/Vehicle/btVehicleRaycaster.h"
#include "BulletDynamics/Vehicle/btWheelInfo.h"
#include "BulletDynamics/ConstraintSolver/btContactConstraint.h"

struct btWheelContactPoint;
btScalar calcRollingFriction(btWheelContactPoint& contactPoint);


static btRigidBody s_fixedObject( 0,0,0);

btKart::btKart(const btVehicleTuning& tuning,btRigidBody* chassis,
               btVehicleRaycaster* raycaster, float track_connect_accel )
: btRaycastVehicle(tuning, chassis, raycaster)
{
    m_track_connect_accel = track_connect_accel;
}

// ----------------------------------------------------------------------------
btKart::~btKart()
{
}

// ----------------------------------------------------------------------------
btScalar btKart::rayCast(btWheelInfo& wheel)
{
	updateWheelTransformsWS( wheel,false);


	btScalar depth          = -1;

	btScalar raylen         = wheel.getSuspensionRestLength()+wheel.m_wheelsRadius+
                              wheel.m_maxSuspensionTravelCm*0.01f;

	btVector3 rayvector     = wheel.m_raycastInfo.m_wheelDirectionWS * (raylen);
	const btVector3& source = wheel.m_raycastInfo.m_hardPointWS;
	wheel.m_raycastInfo.m_contactPointWS = source + rayvector;
	const btVector3& target = wheel.m_raycastInfo.m_contactPointWS;

	btScalar param = btScalar(0.);

	btVehicleRaycaster::btVehicleRaycasterResult	rayResults;

	assert(m_vehicleRaycaster);

	void* object            = m_vehicleRaycaster->castRay(source,target,rayResults);

	wheel.m_raycastInfo.m_groundObject = 0;

	if (object)
	{
    	param = rayResults.m_distFraction;
		depth          = raylen * rayResults.m_distFraction;
		wheel.m_raycastInfo.m_contactNormalWS  = rayResults.m_hitNormalInWorld;
		wheel.m_raycastInfo.m_isInContact      = true;

		wheel.m_raycastInfo.m_groundObject     = &s_fixedObject;//todo for driving on dynamic/movable objects!;
		//wheel.m_raycastInfo.m_groundObject = object;


		btScalar hitDistance = param*raylen;
		wheel.m_raycastInfo.m_suspensionLength = hitDistance - wheel.m_wheelsRadius;
		//clamp on max suspension travel

		btScalar  minSuspensionLength = wheel.getSuspensionRestLength() - wheel.m_maxSuspensionTravelCm*btScalar(0.01);
		btScalar maxSuspensionLength = wheel.getSuspensionRestLength()+ wheel.m_maxSuspensionTravelCm*btScalar(0.01);
		if (wheel.m_raycastInfo.m_suspensionLength < minSuspensionLength)
		{
			wheel.m_raycastInfo.m_suspensionLength = minSuspensionLength;
		}
		if (wheel.m_raycastInfo.m_suspensionLength > maxSuspensionLength)
		{
			wheel.m_raycastInfo.m_suspensionLength = maxSuspensionLength;
		}

		wheel.m_raycastInfo.m_contactPointWS = rayResults.m_hitPointInWorld;

		btScalar denominator= wheel.m_raycastInfo.m_contactNormalWS.dot( wheel.m_raycastInfo.m_wheelDirectionWS );

		btVector3 chassis_velocity_at_contactPoint;
		btVector3 relpos = wheel.m_raycastInfo.m_contactPointWS-getRigidBody()->getCenterOfMassPosition();

		chassis_velocity_at_contactPoint = getRigidBody()->getVelocityInLocalPoint(relpos);

		btScalar projVel = wheel.m_raycastInfo.m_contactNormalWS.dot( chassis_velocity_at_contactPoint );

		if ( denominator >= btScalar(-0.1))
		{
			wheel.m_suspensionRelativeVelocity = btScalar(0.0);
			wheel.m_clippedInvContactDotSuspension = btScalar(1.0) / btScalar(0.1);
		}
		else
		{
			btScalar inv = btScalar(-1.) / denominator;
			wheel.m_suspensionRelativeVelocity = projVel * inv;
			wheel.m_clippedInvContactDotSuspension = inv;
		}

	} else
	{
		//put wheel info as in rest position
		wheel.m_raycastInfo.m_suspensionLength = wheel.getSuspensionRestLength();
		wheel.m_suspensionRelativeVelocity = btScalar(0.0);
		wheel.m_raycastInfo.m_contactNormalWS = - wheel.m_raycastInfo.m_wheelDirectionWS;
		wheel.m_clippedInvContactDotSuspension = btScalar(1.0);

	}

	return depth;
}

// ----------------------------------------------------------------------------
void btKart::updateVehicle( btScalar step )
{
	{
		for (int i=0;i<getNumWheels();i++)
		{
			updateWheelTransform(i,false);
		}
	}


	m_currentVehicleSpeedKmHour = btScalar(3.6) * getRigidBody()->getLinearVelocity().length();

	const btTransform& chassisTrans = getChassisWorldTransform();

	btVector3 forwardW (
		chassisTrans.getBasis()[0][m_indexForwardAxis],
		chassisTrans.getBasis()[1][m_indexForwardAxis],
		chassisTrans.getBasis()[2][m_indexForwardAxis]);

	if (forwardW.dot(getRigidBody()->getLinearVelocity()) < btScalar(0.))
	{
		m_currentVehicleSpeedKmHour *= btScalar(-1.);
	}

	//
	// simulate suspension
	//

	int i=0;
	for (i=0;i<m_wheelInfo.size();i++)
	{
		btScalar depth;
		depth = rayCast( m_wheelInfo[i]);
	}

    // Work around: make sure that either both wheels on one axis
    // are on ground, or none of them. This avoids the problem of
    // the kart suddenly getting additional angular velocity because
    // e.g. only one rear wheel is on the ground.
    for(i=0; i<4; i+=2)
    {
        if(m_wheelInfo[i].m_raycastInfo.m_isInContact &&
           !(m_wheelInfo[i+1].m_raycastInfo.m_isInContact))
        {
            m_wheelInfo[i+1].m_raycastInfo = m_wheelInfo[i].m_raycastInfo;
        }
        if(!(m_wheelInfo[i].m_raycastInfo.m_isInContact) &&
           m_wheelInfo[i+1].m_raycastInfo.m_isInContact)
        {
           m_wheelInfo[i].m_raycastInfo = m_wheelInfo[i+1].m_raycastInfo;
        }
    }   // for i=0; i<4; i+=2

	updateSuspension(step);

	for (i=0;i<m_wheelInfo.size();i++)
	{
		//apply suspension force
		btWheelInfo& wheel = m_wheelInfo[i];

		btScalar suspensionForce = wheel.m_wheelsSuspensionForce;

		btScalar gMaxSuspensionForce = btScalar(6000.);
		if (suspensionForce > gMaxSuspensionForce)
		{
			suspensionForce = gMaxSuspensionForce;
		}
		btVector3 impulse = wheel.m_raycastInfo.m_contactNormalWS * suspensionForce * step;
		btVector3 relpos = wheel.m_raycastInfo.m_contactPointWS - getRigidBody()->getCenterOfMassPosition();

		getRigidBody()->applyImpulse(impulse, relpos);

	}



	updateFriction( step);


	for (i=0;i<m_wheelInfo.size();i++)
	{
		btWheelInfo& wheel = m_wheelInfo[i];
		btVector3 relpos = wheel.m_raycastInfo.m_hardPointWS - getRigidBody()->getCenterOfMassPosition();
		btVector3 vel = getRigidBody()->getVelocityInLocalPoint( relpos );

		if (wheel.m_raycastInfo.m_isInContact)
		{
			const btTransform&	chassisWorldTransform = getChassisWorldTransform();

			btVector3 fwd (
				chassisWorldTransform.getBasis()[0][m_indexForwardAxis],
				chassisWorldTransform.getBasis()[1][m_indexForwardAxis],
				chassisWorldTransform.getBasis()[2][m_indexForwardAxis]);

			btScalar proj = fwd.dot(wheel.m_raycastInfo.m_contactNormalWS);
			fwd -= wheel.m_raycastInfo.m_contactNormalWS * proj;

			btScalar proj2 = fwd.dot(vel);

			wheel.m_deltaRotation = (proj2 * step) / (wheel.m_wheelsRadius);
			wheel.m_rotation += wheel.m_deltaRotation;

		} else
		{
			wheel.m_rotation += wheel.m_deltaRotation;
		}

		wheel.m_deltaRotation *= btScalar(0.99);//damping of rotation when not in contact

	}

}

// ----------------------------------------------------------------------------
void btKart::updateSuspension(btScalar deltaTime)
{
	(void)deltaTime;

	btScalar chassisMass = btScalar(1.) / m_chassisBody->getInvMass();

	for (int w_it=0; w_it<getNumWheels(); w_it++)
	{
		btWheelInfo &wheel_info = m_wheelInfo[w_it];

		if ( wheel_info.m_raycastInfo.m_isInContact )
		{
			btScalar force;
			//	Spring
			{
				btScalar	susp_length			= wheel_info.getSuspensionRestLength();
				btScalar	current_length = wheel_info.m_raycastInfo.m_suspensionLength;

				btScalar length_diff = (susp_length - current_length);

                force = wheel_info.m_suspensionStiffness
					* length_diff * wheel_info.m_clippedInvContactDotSuspension;
			}

			// Damper
			{
				btScalar projected_rel_vel = wheel_info.m_suspensionRelativeVelocity;
				{
					btScalar	susp_damping;
					if ( projected_rel_vel < btScalar(0.0) )
					{
						susp_damping = wheel_info.m_wheelsDampingCompression;
					}
					else
					{
						susp_damping = wheel_info.m_wheelsDampingRelaxation;
					}
					force -= susp_damping * projected_rel_vel;
				}
			}

			// RESULT
			wheel_info.m_wheelsSuspensionForce = force * chassisMass;
			if (wheel_info.m_wheelsSuspensionForce < btScalar(0.))
			{
				wheel_info.m_wheelsSuspensionForce = btScalar(0.);
			}
		}
		else
		{
            // A very unphysical thing to handle slopes that are a bit too steep
            // or uneven (resulting in only one wheel on the ground)
            // If only the front or only the rear wheels are on the ground, add
            // a force pulling the axis down (towards the ground). Note that it
            // is already guaranteed that either both or no wheels on one axis
            // are on the ground, so we have to test only one of the wheels
            wheel_info.m_wheelsSuspensionForce = -m_track_connect_accel*chassisMass ;
		}
    }   // for w_it<number of wheels

}

// ----------------------------------------------------------------------------
// FIXME: This structure has to be the same as the one declared in btRaycastVehicle.
//        Unfortunately bullet (atm) does not declare this struct in the header file.
struct btWheelContactPoint
{
	btRigidBody* m_body0;
	btRigidBody* m_body1;
	btVector3	m_frictionPositionWorld;
	btVector3	m_frictionDirectionWorld;
	btScalar	m_jacDiagABInv;
	btScalar	m_maxImpulse;


	btWheelContactPoint(btRigidBody* body0,btRigidBody* body1,const btVector3& frictionPosWorld,const btVector3& frictionDirectionWorld, btScalar maxImpulse)
		:m_body0(body0),
		m_body1(body1),
		m_frictionPositionWorld(frictionPosWorld),
		m_frictionDirectionWorld(frictionDirectionWorld),
		m_maxImpulse(maxImpulse)
	{
		btScalar denom0 = body0->computeImpulseDenominator(frictionPosWorld,frictionDirectionWorld);
		btScalar denom1 = body1->computeImpulseDenominator(frictionPosWorld,frictionDirectionWorld);
		btScalar	relaxation = 1.f;
		m_jacDiagABInv = relaxation/(denom0+denom1);
	}


};

// ----------------------------------------------------------------------------
void	btKart::updateFriction(btScalar	timeStep)
{

    //calculate the impulse, so that the wheels don't move sidewards
    int numWheel = getNumWheels();
    if (!numWheel)
        return;

    m_forwardWS.resize(numWheel);
    m_axle.resize(numWheel);
    m_forwardImpulse.resize(numWheel);
    m_sideImpulse.resize(numWheel);

    int numWheelsOnGround = 0;


    //collapse all those loops into one!
    for (int i=0;i<getNumWheels();i++)
    {
        btWheelInfo& wheelInfo = m_wheelInfo[i];
        class btRigidBody* groundObject = (class btRigidBody*) wheelInfo.m_raycastInfo.m_groundObject;
        if (groundObject)
            numWheelsOnGround++;
        m_sideImpulse[i] = btScalar(0.);
        m_forwardImpulse[i] = btScalar(0.);

    }

    {

        for (int i=0;i<getNumWheels();i++)
        {

            btWheelInfo& wheelInfo = m_wheelInfo[i];

            class btRigidBody* groundObject = (class btRigidBody*) wheelInfo.m_raycastInfo.m_groundObject;

            if (groundObject)
            {

                const btTransform& wheelTrans = getWheelTransformWS( i );

                btMatrix3x3 wheelBasis0 = wheelTrans.getBasis();
                m_axle[i] = btVector3(
                    wheelBasis0[0][m_indexRightAxis],
                    wheelBasis0[1][m_indexRightAxis],
                    wheelBasis0[2][m_indexRightAxis]);

                const btVector3& surfNormalWS = wheelInfo.m_raycastInfo.m_contactNormalWS;
                btScalar proj = m_axle[i].dot(surfNormalWS);
                m_axle[i] -= surfNormalWS * proj;
                m_axle[i] = m_axle[i].normalize();

                m_forwardWS[i] = surfNormalWS.cross(m_axle[i]);
                m_forwardWS[i].normalize();


                resolveSingleBilateral(*m_chassisBody, wheelInfo.m_raycastInfo.m_contactPointWS,
                    *groundObject, wheelInfo.m_raycastInfo.m_contactPointWS,
                    btScalar(0.), m_axle[i],m_sideImpulse[i],timeStep);

                //m_sideImpulse[i] *= sideFrictionStiffness2; // hiker: sideFrictionStiffness2 is a global(!) variable = 1.0

            }


        }
    }

    btScalar sideFactor = btScalar(1.);
    btScalar fwdFactor = 0.5;

    {
        for (int wheel =0;wheel <getNumWheels();wheel++)
        {
            btWheelInfo& wheelInfo = m_wheelInfo[wheel];
            class btRigidBody* groundObject = (class btRigidBody*) wheelInfo.m_raycastInfo.m_groundObject;

            btScalar	rollingFriction = 0.f;

            if (groundObject)
            {
                if (wheelInfo.m_engineForce != 0.f)
                {
                    rollingFriction = wheelInfo.m_engineForce* timeStep;
                } else
                {
                    btScalar defaultRollingFrictionImpulse = 0.f;
                    btScalar maxImpulse = wheelInfo.m_brake ? wheelInfo.m_brake : defaultRollingFrictionImpulse;
                    btWheelContactPoint contactPt(m_chassisBody,groundObject,wheelInfo.m_raycastInfo.m_contactPointWS,m_forwardWS[wheel],maxImpulse);
                    rollingFriction = calcRollingFriction(contactPt);
                    // This is a work around for the problem that a kart shakes
                    // if it is braking: we get a minor impulse forward, which
                    // bullet then tries to offset by applying a backward
                    // impulse - which is a bit too big, causing a impulse
                    // backwards, ... till the kart is shaking backwards and
                    // forwards
                    if(wheelInfo.m_brake && fabsf(rollingFriction)<10)
                        rollingFriction=0;
                }
            }

            //switch between active rolling (throttle), braking and non-active rolling friction (no throttle/break)




            m_forwardImpulse[wheel] = btScalar(0.);
            m_wheelInfo[wheel].m_skidInfo= btScalar(1.);

            if (groundObject)
            {
                m_forwardImpulse[wheel] = rollingFriction;//wheelInfo.m_engineForce* timeStep;
            }

        }
    }





    // apply the impulses
    {
        for (int wheel = 0;wheel<getNumWheels() ; wheel++)
        {
            btWheelInfo& wheelInfo = m_wheelInfo[wheel];

            btVector3 rel_pos = wheelInfo.m_raycastInfo.m_contactPointWS -
                m_chassisBody->getCenterOfMassPosition();

            if (m_forwardImpulse[wheel] != btScalar(0.))
            {
                m_chassisBody->applyImpulse(m_forwardWS[wheel]*(m_forwardImpulse[wheel]),
                                            btVector3(0,0,0));
            }
            if (m_sideImpulse[wheel] != btScalar(0.))
            {
                class btRigidBody* groundObject = (class btRigidBody*) m_wheelInfo[wheel].m_raycastInfo.m_groundObject;

                btVector3 rel_pos2 = wheelInfo.m_raycastInfo.m_contactPointWS -
                    groundObject->getCenterOfMassPosition();

                //adjust relative position above ground so that force only acts sideways
                btVector3 delta_vec = (wheelInfo.m_raycastInfo.m_hardPointWS - wheelInfo.m_raycastInfo.m_contactPointWS);
                if (delta_vec.length() != btScalar (0))
                {
                    delta_vec = delta_vec.normalize();
                    rel_pos -= delta_vec * rel_pos.dot(delta_vec);
                }

                btVector3 sideImp = m_axle[wheel] * m_sideImpulse[wheel];

                rel_pos[2] *= wheelInfo.m_rollInfluence;
                m_chassisBody->applyImpulse(sideImp,rel_pos);

                //apply friction impulse on the ground
                groundObject->applyImpulse(-sideImp,rel_pos2);
            }
        }
    }


}
