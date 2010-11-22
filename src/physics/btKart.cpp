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

/* Based on btRayCastVehicle, but modified for STK.
 * projectVehicleToSurface function and shorter raycast functions added.
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

    m_num_wheels_on_ground = 0;
    
    m_zipper_active = false;
    m_zipper_velocity = btScalar(0);
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

    btVehicleRaycaster::btVehicleRaycasterResult    rayResults;

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
//Shorter version of above raycast function. This is used when projecting
//vehicles towards the ground at the start of a race
btScalar btKart::rayCast(btWheelInfo& wheel, const btVector3& ray)
{
    updateWheelTransformsWS( wheel,false);

    btScalar depth          = -1;

    const btVector3& source = wheel.m_raycastInfo.m_hardPointWS;
    wheel.m_raycastInfo.m_contactPointWS = source + ray;
    const btVector3& target = source + ray;

    btVehicleRaycaster::btVehicleRaycasterResult    rayResults;

    assert(m_vehicleRaycaster);

    void* object            = m_vehicleRaycaster->castRay(source,target,rayResults);

    wheel.m_raycastInfo.m_groundObject = 0;

    if (object)
    {
        depth = ray.length() * rayResults.m_distFraction;

        wheel.m_raycastInfo.m_contactPointWS   = rayResults.m_hitPointInWorld;
        wheel.m_raycastInfo.m_contactNormalWS  = rayResults.m_hitNormalInWorld;
        wheel.m_raycastInfo.m_isInContact      = true;
    }

    return depth;
}

// ----------------------------------------------------------------------------
//Project vehicle onto surface in a particular direction.
//Used in reseting kart positions.
//Please align wheel direction with ray direction first.
bool btKart::projectVehicleToSurface(const btVector3& ray, bool translate_vehicle)
{
    if (ray.length() <= btScalar(0))
        return false;

    btVector3 ray_dir = ray / ray.length();

    for (int i=0;i<getNumWheels();i++)
    {
        updateWheelTransform(i,false);
    }


    btScalar min_depth(-1);   //minimum distance of wheel to surface
    int min_wheel_index  = 0; //wheel with 1st minimum distance to surface
    int min_wheel_index2 = 0; //wheel with 2nd minimum distance to surface
    int min_wheel_index3 = 0; //wheel with 3nd minimum distance to surface

    btScalar depth[4];

    for (int i=0;i<m_wheelInfo.size();i++)
    {
        depth[i] = rayCast( m_wheelInfo[i], ray);
        depth[i] -= m_wheelInfo[i].m_wheelsRadius;

        if (!(m_wheelInfo[i].m_raycastInfo.m_isInContact))
        {
            return false; //a wheel is not over ground
        }

        if (depth[i]<min_depth || i ==0)
        {
            min_depth = depth[i];
            min_wheel_index = i;
        }

    }

    bool flag = true;

    for (int i=0;i<m_wheelInfo.size();i++)
    {
        if (i==min_wheel_index)
            continue;

        if (depth[i]<min_depth || flag)
        {
            min_depth = depth[i];
            min_wheel_index2 = i;
            flag = false;
        }
    }

    flag = true;
    for (int i=0;i<m_wheelInfo.size();i++)
    {
        if (i==min_wheel_index || i==min_wheel_index2)
            continue;

        if (depth[i]<min_depth || flag)
        {
            min_depth = depth[i];
            min_wheel_index3 = i;
            flag = false;
        }
    }

    min_depth = depth[min_wheel_index];

    btWheelInfo& min_wheel  = m_wheelInfo[min_wheel_index];
    btWheelInfo& min_wheel2 = m_wheelInfo[min_wheel_index2];
    btWheelInfo& min_wheel3 = m_wheelInfo[min_wheel_index3];

    btTransform trans = getRigidBody()->getCenterOfMassTransform();
    btTransform rot_trans;
    rot_trans.setIdentity();
    rot_trans.setRotation(trans.getRotation());
    rot_trans = rot_trans.inverse();

    btTransform offset_trans;
    offset_trans.setIdentity();
    btVector3 offset= min_wheel.m_raycastInfo.m_hardPointWS + min_wheel.m_wheelsRadius * ray_dir;
    offset -= getRigidBody()->getCenterOfMassPosition();
    offset_trans.setOrigin(rot_trans*offset);


    //the effect of the following rotations is to make the 3 wheels with initial
    //minimum distance to surface (in the ray direction) in contact with the
    //plane between the points of intersection (between the ray and surface).

    //Note - For possible complex surfaces with lots of bumps directly under vehicle,
    //       the raycast needs to be done from a slightly higher above the surface.
    //       For such surfaces, the end result should be that at least 1 wheel touches
    //       the surface, and no wheel goes below the surface.

    //We need to rotate vehicle, using above contact point as a pivot to put
    //2nd closest wheel nearer to the surface of the track
    btScalar d_hpws  = (min_wheel.m_raycastInfo.m_hardPointWS - min_wheel2.m_raycastInfo.m_hardPointWS).length();
    btScalar d_depth = (min_wheel2.m_raycastInfo.m_contactPointWS - min_wheel2.m_raycastInfo.m_hardPointWS - ray_dir * min_wheel.m_wheelsRadius).length();
    d_depth -= min_depth;

    //calculate rotation angle from pivot point and plane perpendicular to ray
    float rot_angle = atanf(d_depth / d_hpws);
    rot_angle -= atanf((min_wheel2.m_wheelsRadius - min_wheel.m_wheelsRadius) / d_hpws);

    getRigidBody()->setAngularVelocity(btVector3(0,0,0));
    getRigidBody()->setLinearVelocity(btVector3(0,0,0));


    btVector3 rot_axis = (min_wheel2.m_raycastInfo.m_hardPointWS - min_wheel.m_raycastInfo.m_hardPointWS).cross(ray_dir);

    btTransform operator_trans;
    operator_trans.setIdentity();

    //perform pivot rotation
    if (rot_axis.length() != btScalar(0))
    {
        //rotate kart about pivot point, about line perpendicular to
        //ray and vector between the 2 wheels
        operator_trans *= offset_trans;
        operator_trans.setRotation(btQuaternion(rot_trans*rot_axis.normalize(), rot_angle));
        offset_trans.setOrigin(-(rot_trans*offset));
        operator_trans *= offset_trans;
    }

    //apply tranform
    trans *= operator_trans;
    getRigidBody()->setCenterOfMassTransform(trans);

    //next, rotate about axis which is a vector between 2 wheels above, so that
    //the 3rd wheel is correctly positioned.

    rot_axis = min_wheel2.m_raycastInfo.m_contactPointWS - min_wheel.m_raycastInfo.m_contactPointWS;
    btVector3 wheel_dist = min_wheel3.m_raycastInfo.m_hardPointWS - min_wheel.m_raycastInfo.m_hardPointWS;
    if (rot_axis.length() != btScalar(0))
    {
        btVector3 proj = wheel_dist.dot(rot_axis) * rot_axis.normalize();

        //calculate position on axis when a perpendicular line would go through
        //3rd wheel position when translated in ray position and rotated as above
        btVector3 pos_on_axis = min_wheel.m_raycastInfo.m_contactPointWS + proj;

        btVector3 to_contact_pt = min_wheel3.m_raycastInfo.m_contactPointWS - pos_on_axis;
        btScalar dz = to_contact_pt.dot(ray_dir);
        btScalar dw = (to_contact_pt - dz * ray_dir).length();
        rot_angle = atanf (dz / dw);

        btVector3 rot_point = getRigidBody()->getCenterOfMassPosition() + min_depth * ray_dir - min_wheel.m_raycastInfo.m_contactPointWS;
        rot_point = rot_point.dot(rot_axis) * rot_axis.normalize() - rot_point;


        //calculate translation offset to axis from center of mass along perpendicular
        offset_trans.setIdentity();

        offset= rot_point;
        offset_trans.setOrigin(rot_trans*offset);

        btVector3 a = min_wheel3.m_raycastInfo.m_hardPointWS - min_wheel.m_raycastInfo.m_hardPointWS;
        btVector3 b = min_wheel2.m_raycastInfo.m_hardPointWS - min_wheel.m_raycastInfo.m_hardPointWS;

        if ( (a.cross(b)).dot(ray_dir) > 0 )
        {
            rot_angle *= btScalar(-1);
        }

        //rotate about new axis
        operator_trans.setIdentity();
        operator_trans *= offset_trans;
        operator_trans.setRotation(btQuaternion(rot_trans*rot_axis.normalize(), rot_angle));
        offset_trans.setOrigin(-(rot_trans*offset));
        operator_trans *= offset_trans;

        //apply tranform
        trans *= operator_trans;
        getRigidBody()->setCenterOfMassTransform(trans);
    }

    if (!translate_vehicle)
        return true;


    for (int i=0;i<getNumWheels();i++)
    {
        updateWheelTransform(i,false);
    }

    min_depth = btScalar(-1);   //minimum distance of wheel to surface

    for (int i=0;i<m_wheelInfo.size();i++)
    {
        btScalar depth = rayCast( m_wheelInfo[i], ray);
        depth -= m_wheelInfo[i].m_wheelsRadius;

        if (!(m_wheelInfo[i].m_raycastInfo.m_isInContact))
        {
            return false; //a wheel is not over ground
        }

        if (depth<min_depth || i==0)
        {
            min_depth  = depth;
        }
    }

    //translate along ray so wheel closest to surface is exactly on the surface
    getRigidBody()->translate((min_depth) * ray_dir);
    //offset for suspension rest length
    getRigidBody()->translate(-min_wheel.getSuspensionRestLength() *
                               min_wheel.m_raycastInfo.m_wheelDirectionWS);
    return true;
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
    m_num_wheels_on_ground = 0;
    
    for (i=0;i<m_wheelInfo.size();i++)
    {
        btScalar depth;
        depth = rayCast( m_wheelInfo[i]);
        
        if (m_wheelInfo[i].m_raycastInfo.m_isInContact)
            m_num_wheels_on_ground++;
    }

    // Work around: make sure that either both wheels on one axis
    // are on ground, or none of them. This avoids the problem of
    // the kart suddenly getting additional angular velocity because
    // e.g. only one rear wheel is on the ground.
    for(i=0; i<m_wheelInfo.size(); i+=2)
    {
        if(m_wheelInfo[i].m_raycastInfo.m_isInContact != m_wheelInfo[i+1].m_raycastInfo.m_isInContact)
        {
            int wheel_air_index = i;
            int wheel_ground_index = i+1;
            
            if (m_wheelInfo[i].m_raycastInfo.m_isInContact)
            {
                wheel_air_index = i+1;
                wheel_ground_index = i;
            }

            btWheelInfo& wheel_air = m_wheelInfo[wheel_air_index];
            btWheelInfo& wheel_ground = m_wheelInfo[wheel_ground_index];

            wheel_air.m_raycastInfo = wheel_ground.m_raycastInfo;
        }
    }   // for i=0; i<m_wheelInfo.size(); i+=2

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
            const btTransform&  chassisWorldTransform = getChassisWorldTransform();

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
            //  Spring
            {
                btScalar    susp_length         = wheel_info.getSuspensionRestLength();
                btScalar    current_length = wheel_info.m_raycastInfo.m_suspensionLength;

                btScalar length_diff = (susp_length - current_length);

                force = wheel_info.m_suspensionStiffness
                    * length_diff * wheel_info.m_clippedInvContactDotSuspension;
            }

            // Damper
            {
                btScalar projected_rel_vel = wheel_info.m_suspensionRelativeVelocity;
                {
                    btScalar    susp_damping;
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
    btVector3   m_frictionPositionWorld;
    btVector3   m_frictionDirectionWorld;
    btScalar    m_jacDiagABInv;
    btScalar    m_maxImpulse;


    btWheelContactPoint(btRigidBody* body0,btRigidBody* body1,const btVector3& frictionPosWorld,const btVector3& frictionDirectionWorld, btScalar maxImpulse)
        :m_body0(body0),
        m_body1(body1),
        m_frictionPositionWorld(frictionPosWorld),
        m_frictionDirectionWorld(frictionDirectionWorld),
        m_maxImpulse(maxImpulse)
    {
        btScalar denom0 = body0->computeImpulseDenominator(frictionPosWorld,frictionDirectionWorld);
        btScalar denom1 = body1->computeImpulseDenominator(frictionPosWorld,frictionDirectionWorld);
        btScalar    relaxation = 1.f;
        m_jacDiagABInv = relaxation/(denom0+denom1);
    }


};

// ----------------------------------------------------------------------------
void    btKart::updateFriction(btScalar timeStep)
{

    //calculate the impulse, so that the wheels don't move sidewards
    int numWheel = getNumWheels();
    if (!numWheel)
        return;

    m_forwardWS.resize(numWheel);
    m_axle.resize(numWheel);
    m_forwardImpulse.resize(numWheel);
    m_sideImpulse.resize(numWheel);


    //collapse all those loops into one!
    for (int i=0;i<getNumWheels();i++)
    {
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

            }


        }
    }

    btScalar sideFactor = btScalar(1.);
    btScalar fwdFactor = 0.5;

    bool sliding = false;
    {
        for (int wheel =0;wheel <getNumWheels();wheel++)
        {
            btWheelInfo& wheelInfo = m_wheelInfo[wheel];
            class btRigidBody* groundObject = (class btRigidBody*) wheelInfo.m_raycastInfo.m_groundObject;

            btScalar    rollingFriction = 0.f;

            m_forwardImpulse[wheel] = btScalar(0.);
            m_wheelInfo[wheel].m_skidInfo= btScalar(1.);

            if (groundObject)
            {
                if (m_zipper_active)
                {
                    if (wheel==2 || wheel==3)
                    {
                        // The zipper velocity is the speed that should be
                        // reached. So compute the impulse to accelerate the
                        // kart up to that speed:
                        m_forwardImpulse[wheel] = 0.5f*(m_zipper_velocity - getRigidBody()->getLinearVelocity().length()) / m_chassisBody->getInvMass();
                    }
                }
                else
                {

                    if (wheelInfo.m_engineForce != 0.f)
                    {
                        rollingFriction = wheelInfo.m_engineForce* timeStep;
                    } else
                    {
                        //switch between active rolling (throttle), braking and non-active rolling friction (no throttle/break)
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

                    m_forwardImpulse[wheel] = rollingFriction;//wheelInfo.m_engineForce* timeStep;
                }
                btScalar maximp = wheelInfo.m_wheelsSuspensionForce * timeStep * wheelInfo.m_frictionSlip;
                btScalar maximpSide = maximp;

                btScalar maximpSquared = maximp * maximpSide;

                btScalar x = (m_forwardImpulse[wheel] ) * fwdFactor;
                btScalar y = (m_sideImpulse[wheel] ) * sideFactor;

                btScalar impulseSquared = (x*x + y*y);

                if (impulseSquared > maximpSquared)
                {
                    sliding = true;

                    btScalar factor = maximp / btSqrt(impulseSquared);

                    m_wheelInfo[wheel].m_skidInfo *= factor;
                }
            }    // if groundObject

        }   // for wheel<getNumWheels()

        m_zipper_active = false;
    }   // close block


    if (sliding)
    {
        for (int wheel = 0;wheel < getNumWheels(); wheel++)
        {
            if (m_sideImpulse[wheel] != btScalar(0.) && m_allow_sliding)
            {
                if (m_wheelInfo[wheel].m_skidInfo< btScalar(1.))
                {
                    m_forwardImpulse[wheel] *= m_wheelInfo[wheel].m_skidInfo;
                    m_sideImpulse[wheel] *= m_wheelInfo[wheel].m_skidInfo;
                }
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

                rel_pos[m_indexUpAxis] *= wheelInfo.m_rollInfluence;
                m_chassisBody->applyImpulse(sideImp,rel_pos);

                //apply friction impulse on the ground
                groundObject->applyImpulse(-sideImp,rel_pos2);
            }
        }
    }


}
