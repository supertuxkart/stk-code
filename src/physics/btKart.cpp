/*
 * Copyright (C) 2005-2015 Erwin Coumans http://continuousphysics.com/Bullet/
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies.
 * Erwin Coumans makes no representations about the suitability
 * of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
*/

#include "LinearMath/btVector3.h"
#include "btKart.hpp"

#include "BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.h"
#include "BulletDynamics/ConstraintSolver/btJacobianEntry.h"
#include "LinearMath/btQuaternion.h"
#include "BulletDynamics/Dynamics/btDynamicsWorld.h"
#include "BulletDynamics/Vehicle/btVehicleRaycaster.h"
#include "BulletDynamics/Vehicle/btWheelInfo.h"
#include "LinearMath/btMinMax.h"
#include "LinearMath/btIDebugDraw.h"
#include "BulletDynamics/ConstraintSolver/btContactConstraint.h"

#include "graphics/material.hpp"
#include "karts/kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/terrain_info.hpp"
#include "tracks/track.hpp"

#define ROLLING_INFLUENCE_FIX

// ============================================================================
btKart::btKart(btRigidBody* chassis, btVehicleRaycaster* raycaster,
               Kart *kart)
      : m_vehicleRaycaster(raycaster), m_fixed_body(0, 0, 0)
{
    m_chassisBody               = chassis;
    m_indexRightAxis            = 0;
    m_indexUpAxis               = 1;
    m_indexForwardAxis          = 2;
    m_kart                      = kart;
    m_fixed_body.setMassProps(btScalar(0.),btVector3(btScalar(0.),
        btScalar(0.),btScalar(0.)));
    reset();
}   // btKart

// ----------------------------------------------------------------------------
btKart::~btKart()
{
}   // ~btKart

// ----------------------------------------------------------------------------

//
// basically most of the code is general for 2 or 4 wheel vehicles, but some
// of it needs to be reviewed
//
btWheelInfo& btKart::addWheel(const btVector3& connectionPointCS,
                              const btVector3& wheelDirectionCS0,
                              const btVector3& wheelAxleCS,
                              btScalar suspensionRestLength,
                              btScalar wheelRadius,
                              const btVehicleTuning& tuning,
                              bool isFrontWheel)
{

    btWheelInfoConstructionInfo ci;

    ci.m_chassisConnectionCS      = connectionPointCS;
    ci.m_wheelDirectionCS         = wheelDirectionCS0;
    ci.m_wheelAxleCS              = wheelAxleCS;
    ci.m_suspensionRestLength     = suspensionRestLength;
    ci.m_wheelRadius              = wheelRadius;
    ci.m_bIsFrontWheel            = isFrontWheel;
    ci.m_suspensionStiffness      = tuning.m_suspensionStiffness;
    ci.m_wheelsDampingCompression = tuning.m_suspensionCompression;
    ci.m_wheelsDampingRelaxation  = tuning.m_suspensionDamping;
    ci.m_frictionSlip             = tuning.m_frictionSlip;
    ci.m_maxSuspensionTravel      = tuning.m_maxSuspensionTravel;
    ci.m_maxSuspensionForce       = tuning.m_maxSuspensionForce;

    m_wheelInfo.push_back( btWheelInfo(ci));

    btWheelInfo& wheel = m_wheelInfo[getNumWheels()-1];
    memset(&wheel.m_raycastInfo, 0, sizeof(wheel.m_raycastInfo));

    updateWheelTransformsWS(wheel, getChassisWorldTransform(), false);
    updateWheelTransform(getNumWheels()-1,false);

    m_forwardWS.resize(m_wheelInfo.size());
    m_axle.resize(m_wheelInfo.size());
    m_forwardImpulse.resize(m_wheelInfo.size());
    m_sideImpulse.resize(m_wheelInfo.size());

    return wheel;
}   // addWheel

// ----------------------------------------------------------------------------
/** Resets the kart before a (re)start, to make sure all physics variable
 *  are properly defined. This is especially important for physics replay.
 */
void btKart::reset()
{
    for(int i=0; i<getNumWheels(); i++)
    {
        btWheelInfo &wheel                     = m_wheelInfo[i];
        wheel.m_raycastInfo.m_suspensionLength = 0;
        updateWheelTransform(i, true);
    }
    m_visual_wheels_touch_ground = false;
    m_allow_sliding              = false;
    m_num_wheels_on_ground       = 0;
    m_additional_impulse         = btVector3(0,0,0);
    m_ticks_additional_impulse   = 0;
    m_additional_rotation        = 0;
    m_ticks_additional_rotation  = 0;
    m_max_speed                  = -1.0f;
    m_min_speed                  = 0.0f;

    // Set the brakes so that karts don't slide downhill
    setAllBrakes(5.0f);

}   // reset

// ----------------------------------------------------------------------------
const btTransform& btKart::getWheelTransformWS( int wheelIndex ) const
{
    btAssert(wheelIndex < getNumWheels());
    const btWheelInfo& wheel = m_wheelInfo[wheelIndex];
    return wheel.m_worldTransform;

}   // getWheelTransformWS

// ----------------------------------------------------------------------------
void btKart::updateWheelTransform(int wheelIndex, bool interpolatedTransform)
{

    btWheelInfo& wheel = m_wheelInfo[ wheelIndex ];
    updateWheelTransformsWS(wheel, getChassisWorldTransform(), interpolatedTransform);
    btVector3 up = -wheel.m_raycastInfo.m_wheelDirectionWS;
    const btVector3& right = wheel.m_raycastInfo.m_wheelAxleWS;
    btVector3 fwd = up.cross(right);
    fwd = fwd.normalize();

    //rotate around steering over de wheelAxleWS
    btScalar steering = wheel.m_steering;

    btQuaternion steeringOrn(up,steering);//wheel.m_steering);
    btMatrix3x3 steeringMat(steeringOrn);

    btMatrix3x3 basis2(
        right[0],fwd[0],up[0],
        right[1],fwd[1],up[1],
        right[2],fwd[2],up[2]
    );

    wheel.m_worldTransform.setBasis(steeringMat * basis2);
    wheel.m_worldTransform.setOrigin(
                                     wheel.m_raycastInfo.m_hardPointWS
                                    + wheel.m_raycastInfo.m_wheelDirectionWS
                                     *wheel.m_raycastInfo.m_suspensionLength);
}   // updateWheelTransform

// ----------------------------------------------------------------------------
void btKart::resetSuspension()
{

    int i;
    for (i=0;i<m_wheelInfo.size(); i++)
    {
            btWheelInfo& wheel = m_wheelInfo[i];
            wheel.m_raycastInfo.m_suspensionLength =
                wheel.getSuspensionRestLength();
            wheel.m_suspensionRelativeVelocity = btScalar(0.0);

            wheel.m_raycastInfo.m_contactNormalWS =
                - wheel.m_raycastInfo.m_wheelDirectionWS;
            //wheel_info.setContactFriction(btScalar(0.0));
            wheel.m_clippedInvContactDotSuspension = btScalar(1.0);
    }
}   // resetSuspension

// ----------------------------------------------------------------------------
void btKart::updateWheelTransformsWS(btWheelInfo& wheel,
                                     btTransform chassis_trans,
                                     bool interpolatedTransform,
                                     float fraction)
{
    wheel.m_raycastInfo.m_isInContact = false;

    if (interpolatedTransform && (getRigidBody()->getMotionState()))
    {
        getRigidBody()->getMotionState()->getWorldTransform(chassis_trans);
    }

    wheel.m_raycastInfo.m_hardPointWS =
        chassis_trans( wheel.m_chassisConnectionPointCS*fraction );
    wheel.m_raycastInfo.m_wheelDirectionWS = chassis_trans.getBasis() *
                                                wheel.m_wheelDirectionCS ;
    wheel.m_raycastInfo.m_wheelAxleWS      = chassis_trans.getBasis() *
                                                wheel.m_wheelAxleCS;
}   // updateWheelTransformsWS

// ----------------------------------------------------------------------------
/** Updates all wheel transform informations. This is used just after a rewind
 *  to update all m_hardPointWS (which is used by stk to determine the terrain
 *  under the kart).
 */
void btKart::updateAllWheelTransformsWS()
{
    updateAllWheelPositions();

    const btTransform& chassisTrans = getChassisWorldTransform();

    btVector3 forwardW(chassisTrans.getBasis()[0][m_indexForwardAxis],
                       chassisTrans.getBasis()[1][m_indexForwardAxis],
                       chassisTrans.getBasis()[2][m_indexForwardAxis]);

    // Simulate suspension
    // -------------------

    m_num_wheels_on_ground       = 0;
    m_visual_wheels_touch_ground = true;
    for (int i=0;i<m_wheelInfo.size();i++)
    {
        rayCast( i);
        if(m_wheelInfo[i].m_raycastInfo.m_isInContact)
            m_num_wheels_on_ground++;
        else
        {
            // If the original raycast did not hit the ground,
            // try a little bit (5%) closer to the centre of the chassis.
            // Some tracks have very minor gaps that would otherwise
            // trigger odd physical behaviour.
            rayCast(i, 0.95f);
            if (m_wheelInfo[i].m_raycastInfo.m_isInContact)
                m_num_wheels_on_ground++;
        }
    }
}   // updateAllWheelTransformsWS

// ----------------------------------------------------------------------------
/**
 */
btScalar btKart::rayCast(unsigned int index, float fraction)
{
    btWheelInfo &wheel = m_wheelInfo[index];

    // Work around a bullet problem: when using a convex hull the raycast
    // would sometimes hit the chassis (which does not happen when using a
    // box shape). Therefore set the collision mask in the chassis body so
    // that it is not hit anymore.
    short int old_group=0;
    if(m_chassisBody->getBroadphaseHandle())
    {
        old_group = m_chassisBody->getBroadphaseHandle()
                                 ->m_collisionFilterGroup;
        m_chassisBody->getBroadphaseHandle()->m_collisionFilterGroup = 0;
    }

    updateWheelTransformsWS(wheel, getChassisWorldTransform(), false, fraction);

    btScalar max_susp_len = wheel.getSuspensionRestLength()
                          + wheel.m_maxSuspensionTravel;

    // Do a slightly longer raycast to see if the kart might soon hit the 
    // ground and some 'cushioning' is needed to avoid that the chassis
    // hits the ground.
    btScalar raylen = max_susp_len + 0.5f;

    btVector3 rayvector = wheel.m_raycastInfo.m_wheelDirectionWS * (raylen);
    const btVector3& source = wheel.m_raycastInfo.m_hardPointWS;
    wheel.m_raycastInfo.m_contactPointWS = source + rayvector;
    const btVector3& target = wheel.m_raycastInfo.m_contactPointWS;

    btVehicleRaycaster::btVehicleRaycasterResult rayResults;

    btAssert(m_vehicleRaycaster);

    void* object = m_vehicleRaycaster->castRay(source,target,rayResults);

    wheel.m_raycastInfo.m_groundObject = 0;

    btScalar depth =  raylen * rayResults.m_distFraction;
    if (object &&  depth < max_susp_len)
    {
        wheel.m_raycastInfo.m_contactNormalWS  = rayResults.m_hitNormalInWorld;
        wheel.m_raycastInfo.m_contactNormalWS.normalize();
        wheel.m_raycastInfo.m_isInContact = true;
        ///@todo for driving on dynamic/movable objects!;
        wheel.m_raycastInfo.m_triangle_index = rayResults.m_triangle_index;;
        wheel.m_raycastInfo.m_groundObject = &m_fixed_body;

        wheel.m_raycastInfo.m_suspensionLength = depth;

        //clamp on max suspension travel
        btScalar minSuspensionLength = wheel.getSuspensionRestLength()
                                - wheel.m_maxSuspensionTravel;
        btScalar maxSuspensionLength = wheel.getSuspensionRestLength()
                                + wheel.m_maxSuspensionTravel;
        if (wheel.m_raycastInfo.m_suspensionLength < minSuspensionLength)
        {
            wheel.m_raycastInfo.m_suspensionLength = minSuspensionLength;
        }
        if (wheel.m_raycastInfo.m_suspensionLength > maxSuspensionLength)
        {
            wheel.m_raycastInfo.m_suspensionLength = maxSuspensionLength;
        }

        wheel.m_raycastInfo.m_contactPointWS = rayResults.m_hitPointInWorld;

        btScalar denominator = wheel.m_raycastInfo.m_contactNormalWS.dot(
                                      wheel.m_raycastInfo.m_wheelDirectionWS );

        btVector3 chassis_velocity_at_contactPoint;
        btVector3 relpos = wheel.m_raycastInfo.m_contactPointWS
                         - getRigidBody()->getCenterOfMassPosition();

        chassis_velocity_at_contactPoint =
            getRigidBody()->getVelocityInLocalPoint(relpos);

        btScalar projVel = wheel.m_raycastInfo.m_contactNormalWS.dot(
                                            chassis_velocity_at_contactPoint );

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
        depth = btScalar(-1.0);
        //put wheel info as in rest position
        wheel.m_raycastInfo.m_suspensionLength = wheel.getSuspensionRestLength();
        wheel.m_suspensionRelativeVelocity = btScalar(0.0);
        wheel.m_raycastInfo.m_contactNormalWS =
            - wheel.m_raycastInfo.m_wheelDirectionWS;
        wheel.m_clippedInvContactDotSuspension = btScalar(1.0);
    }

    if(m_chassisBody->getBroadphaseHandle())
    {
        m_chassisBody->getBroadphaseHandle()->m_collisionFilterGroup
            = old_group;
    }

    return depth;

}   // rayCast

// ----------------------------------------------------------------------------
/** Returns the contact point of a visual wheel.
*  \param n Index of the wheel, must be 2 or 3 since only the two rear
*           wheels define the visual position
*/
void btKart::getVisualContactPoint(const btTransform& chassis_trans,
                                   btVector3 *left, btVector3 *right)
{
    btAssert(m_vehicleRaycaster);

    m_visual_wheels_touch_ground = true;

    short int old_group = 0;
    if (m_chassisBody->getBroadphaseHandle())
    {
        old_group = m_chassisBody->getBroadphaseHandle()
                  ->m_collisionFilterGroup;
        m_chassisBody->getBroadphaseHandle()->m_collisionFilterGroup = 0;
    }
    for (int index = 2; index <= 3; index++)
    {
        // Map index 0-1 to wheel 2-3 (which are the rear wheels)
        btWheelInfo &wheel = m_wheelInfo[index];
        updateWheelTransformsWS(wheel, chassis_trans, false);
        btScalar max_susp_len = wheel.getSuspensionRestLength()
                              + wheel.m_maxSuspensionTravel;

        // Do a slightly longer raycast to see if the kart might soon hit the 
        // ground and some 'cushioning' is needed to avoid that the chassis
        // hits the ground.
        btScalar raylen = max_susp_len + 0.5f;
        btVector3 rayvector = wheel.m_raycastInfo.m_wheelDirectionWS * (raylen);
        btVector3 pos = m_kart->getKartModel()->getWheelGraphicsPosition(index);
        pos.setZ(pos.getZ()*0.9f);
        btVector3 source = chassis_trans(pos);
        btVector3 target = source + rayvector;
        btVehicleRaycaster::btVehicleRaycasterResult rayResults;

        void* object = m_vehicleRaycaster->castRay(source, target, rayResults);
        if(index == 2) *left  = rayResults.m_hitPointInWorld;
        else           *right = rayResults.m_hitPointInWorld;
        m_visual_wheels_touch_ground &= (object != NULL);
    }   // for index in [2,3]

    if (m_chassisBody->getBroadphaseHandle())
    {
        m_chassisBody->getBroadphaseHandle()->m_collisionFilterGroup = old_group;
    }
}   // getVisualContactPoint

// ----------------------------------------------------------------------------
const btTransform& btKart::getChassisWorldTransform() const
{
    return getRigidBody()->getCenterOfMassTransform();
}   // getChassisWorldTransform

// ----------------------------------------------------------------------------
void btKart::updateAllWheelPositions()
{
    for (int i=0;i<getNumWheels();i++)
    {
        updateWheelTransform(i,false);
    }

}   // updateAllWheelPositions

// ----------------------------------------------------------------------------
void btKart::updateVehicle( btScalar step )
{
    updateAllWheelTransformsWS();

    for(int i=0; i<m_wheelInfo.size(); i++)
        m_wheelInfo[i].m_was_on_ground = m_wheelInfo[i].m_raycastInfo.m_isInContact;


    // If the kart is flying, try to keep it parallel to the ground.
    // -------------------------------------------------------------
    if(m_num_wheels_on_ground==0)
    {
        btVector3 kart_up    = getChassisWorldTransform().getBasis().getColumn(1);
        btVector3 terrain_up = -m_chassisBody->getGravity();
        terrain_up = terrain_up.normalize();
        // Length of axis depends on the angle - i.e. the further awat
        // the kart is from being upright, the larger the applied impulse
        // will be, resulting in fast changes when the kart is on its
        // side, but not overcompensating (and therefore shaking) when
        // the kart is not much away from being upright.
        btVector3 axis = kart_up.cross(terrain_up);

        // To avoid the kart going backwards/forwards (or rolling sideways),
        // set the pitch/roll to 0 before applying the 'straightening' impulse.
        // TODO: make this works if gravity is changed.
        btVector3 av = m_chassisBody->getAngularVelocity();
        av.setX(0);
        av.setZ(0);
        m_chassisBody->setAngularVelocity(av);
        // Give a nicely balanced feeling for rebalancing the kart
        float smoothing = m_kart->getKartProperties()
                                ->getStabilitySmoothFlyingImpulse();
        m_chassisBody->applyTorqueImpulse(axis * smoothing);
    }

    // Apply suspension forcen (i.e. upwards force)
    // --------------------------------------------
    updateSuspension(step);

    for (int i=0;i<m_wheelInfo.size();i++)
    {
        //apply suspension force
        btWheelInfo& wheel = m_wheelInfo[i];

        btScalar suspensionForce = wheel.m_wheelsSuspensionForce;

        if (suspensionForce > wheel.m_maxSuspensionForce)
        {
            suspensionForce = wheel.m_maxSuspensionForce;
        }
        btVector3 impulse = wheel.m_raycastInfo.m_contactNormalWS
                            * suspensionForce * step;
        btVector3 relpos = wheel.m_raycastInfo.m_contactPointWS
                         - getRigidBody()->getCenterOfMassPosition();

        getRigidBody()->applyImpulse(impulse, relpos);

    }

    // Update friction (i.e. forward force)
    // ------------------------------------
    updateFriction( step);

    // If configured, add a force to keep karts on the track
    // -----------------------------------------------------
    float dif = m_kart->getKartProperties()->getStabilityDownwardImpulseFactor();
    if(dif!=0 && m_num_wheels_on_ground==4)
    {
        float f = -fabsf(m_kart->getSpeed()) * dif;
        btVector3 downwards_impulse = m_chassisBody->getWorldTransform().getBasis()
                                    * btVector3(0, f, 0);
        m_chassisBody->applyCentralImpulse(downwards_impulse);
    }

    // Apply additional impulse set by supertuxkart
    // --------------------------------------------
    if(m_ticks_additional_impulse>0)
    {
        // We have fixed timestep
        float dt = stk_config->ticks2Time(1);
        m_chassisBody->applyCentralImpulse(m_additional_impulse*dt);
        m_ticks_additional_impulse--;
    }

    // Apply additional rotation set by supertuxkart
    // ---------------------------------------------
    if(m_ticks_additional_rotation>0)
    {
        btTransform &t = m_chassisBody->getWorldTransform();
        // We have fixed timestep
        float dt = stk_config->ticks2Time(1);
        btQuaternion add_rot(m_additional_rotation * dt,
                             0.0f,
                             0.0f);
        t.setRotation(t.getRotation()*add_rot);
        m_chassisBody->setWorldTransform(t);
        // Also apply the rotation to the interpolated world transform.
        // This is important (at least if the rotation is only applied
        // in one frame) since STK will actually use the interpolated
        // transform, which would otherwise only be updated one frame
        // later, resulting in a one-frame incorrect rotation of the
        // kart, or a strongly 'visual jolt' of the kart
        btTransform &iwt=m_chassisBody->getInterpolationWorldTransform();
        iwt.setRotation(iwt.getRotation()*add_rot);
        m_ticks_additional_rotation--;
    }
    adjustSpeed(m_min_speed, m_max_speed);
}   // updateVehicle

// ----------------------------------------------------------------------------
void btKart::setSteeringValue(btScalar steering, int wheel)
{
    btAssert(wheel>=0 && wheel < getNumWheels());

    btWheelInfo& wheelInfo = getWheelInfo(wheel);
    wheelInfo.m_steering = steering;
}   // setSteeringValue

// ----------------------------------------------------------------------------
btScalar btKart::getSteeringValue(int wheel) const
{
    return getWheelInfo(wheel).m_steering;
}

// ----------------------------------------------------------------------------
void btKart::applyEngineForce(btScalar force, int wheel)
{
    btAssert(wheel>=0 && wheel < getNumWheels());
    btWheelInfo& wheelInfo = getWheelInfo(wheel);
    wheelInfo.m_engineForce = force;
}


// ----------------------------------------------------------------------------
const btWheelInfo& btKart::getWheelInfo(int index) const
{
    btAssert((index >= 0) && (index < getNumWheels()));

    return m_wheelInfo[index];
}

// ----------------------------------------------------------------------------
btWheelInfo& btKart::getWheelInfo(int index)
{
    btAssert((index >= 0) && (index < getNumWheels()));

    return m_wheelInfo[index];
}

// ----------------------------------------------------------------------------
void btKart::setAllBrakes(btScalar brake)
{
    for(int i=0; i<getNumWheels(); i++)
        getWheelInfo(i).m_brake = brake;
}   // setAllBrakes


// ----------------------------------------------------------------------------
void btKart::updateSuspension(btScalar deltaTime)
{
    (void)deltaTime;

    btScalar chassisMass = btScalar(1.) / m_chassisBody->getInvMass();

    for (int w_it=0; w_it<getNumWheels(); w_it++)
    {
        btWheelInfo &wheel_info = m_wheelInfo[w_it];
        if ( !wheel_info.m_raycastInfo.m_isInContact )
        {
            // A very unphysical thing to handle slopes that are a bit too
            // steep or uneven (resulting in only one wheel on the ground)
            // If only the front or only the rear wheels are on the ground, add
            // a force pulling the axis down (towards the ground). Note that it
            // is already guaranteed that either both or no wheels on one axis
            // are on the ground, so we have to test only one of the wheels
            // In hindsight it turns out that this code basically adds
            // additional gravity when a kart is flying. So if this code would
            // be removed some jumps (esp. Enterprise) do not work as expected
            // anymore.
            wheel_info.m_wheelsSuspensionForce =
                 -m_kart->getKartProperties()->getStabilityTrackConnectionAccel()
                * chassisMass;
            continue;
        }

        btScalar force;

        // Spring
        btScalar susp_length    = wheel_info.getSuspensionRestLength();
        btScalar current_length = wheel_info.m_raycastInfo.m_suspensionLength;
        btScalar length_diff    = (susp_length - current_length);
        if(m_kart->getKartProperties()->getSuspensionExpSpringResponse())
            length_diff *= fabsf(length_diff)/susp_length;
        force = wheel_info.m_suspensionStiffness * length_diff
              * wheel_info.m_clippedInvContactDotSuspension;

        // Damper
        btScalar projected_rel_vel = wheel_info.m_suspensionRelativeVelocity;
        btScalar susp_damping = projected_rel_vel < btScalar(0.0)
                              ? wheel_info.m_wheelsDampingCompression
                              : wheel_info.m_wheelsDampingRelaxation;
        force -= susp_damping * projected_rel_vel;

        // RESULT
        wheel_info.m_wheelsSuspensionForce = force * chassisMass;
        if (wheel_info.m_wheelsSuspensionForce < btScalar(0.))
        {
            wheel_info.m_wheelsSuspensionForce = btScalar(0.);
        }
    }   //  for (int w_it=0; w_it<getNumWheels(); w_it++)

}   // updateSuspension

// ----------------------------------------------------------------------------
struct btWheelContactPoint
{
    btRigidBody* m_body0;
    btRigidBody* m_body1;
    btVector3    m_frictionPositionWorld;
    btVector3    m_frictionDirectionWorld;
    btScalar     m_jacDiagABInv;
    btScalar     m_maxImpulse;


    btWheelContactPoint(btRigidBody* body0, btRigidBody* body1,
                        const btVector3& frictionPosWorld,
                        const btVector3& frictionDirectionWorld,
                        btScalar maxImpulse)
        :m_body0(body0),
         m_body1(body1),
         m_frictionPositionWorld(frictionPosWorld),
         m_frictionDirectionWorld(frictionDirectionWorld),
         m_maxImpulse(maxImpulse)
    {
        btScalar denom0 = body0->computeImpulseDenominator(frictionPosWorld,
                                                       frictionDirectionWorld);
        btScalar denom1 = body1->computeImpulseDenominator(frictionPosWorld,
                                                       frictionDirectionWorld);
        btScalar relaxation = 1.f;
        m_jacDiagABInv = relaxation/(denom0+denom1);
    }



};   // struct btWheelContactPoint

// ----------------------------------------------------------------------------
btScalar btKart::calcRollingFriction(btWheelContactPoint& contactPoint)
{

    const btVector3& contactPosWorld = contactPoint.m_frictionPositionWorld;

    btVector3 rel_pos1 = contactPosWorld
                       - contactPoint.m_body0->getCenterOfMassPosition();
    btVector3 rel_pos2 = contactPosWorld
                       - contactPoint.m_body1->getCenterOfMassPosition();

    btScalar maxImpulse  = contactPoint.m_maxImpulse;

    btVector3 vel1 = contactPoint.m_body0->getVelocityInLocalPoint(rel_pos1);
    btVector3 vel2 = contactPoint.m_body1->getVelocityInLocalPoint(rel_pos2);
    btVector3 vel = vel1 - vel2;

    btScalar vrel = contactPoint.m_frictionDirectionWorld.dot(vel);

    // calculate j that moves us to zero relative velocity
    // Note that num_wheels_on_ground > 0 since this function is called
    // for wheels that touch the ground/
    btScalar j1 = -vrel * contactPoint.m_jacDiagABInv / m_num_wheels_on_ground;
    btSetMin(j1, maxImpulse);
    btSetMax(j1, -maxImpulse);

    return j1;
}   // calcRollingFriction

// ----------------------------------------------------------------------------

void btKart::updateFriction(btScalar timeStep)
{
    //calculate the impulse, so that the wheels don't move sidewards
    for (int i=0;i<getNumWheels();i++)
    {
        m_sideImpulse[i]       = btScalar(0.);
        btWheelInfo& wheelInfo = m_wheelInfo[i];

        btRigidBody* groundObject =
            (btRigidBody*) wheelInfo.m_raycastInfo.m_groundObject;

        if(!groundObject) continue;
        const btTransform& wheelTrans = getWheelTransformWS( i );

        btMatrix3x3 wheelBasis0 = wheelTrans.getBasis();
        m_axle[i] = btVector3(wheelBasis0[0][m_indexRightAxis],
                              wheelBasis0[1][m_indexRightAxis],
                              wheelBasis0[2][m_indexRightAxis]  );

        const btVector3& surfNormalWS =
                        wheelInfo.m_raycastInfo.m_contactNormalWS;
        btScalar proj = m_axle[i].dot(surfNormalWS);
        m_axle[i]    -= surfNormalWS * proj;
        m_axle[i]     = m_axle[i].normalize();

        m_forwardWS[i] = surfNormalWS.cross(m_axle[i]);
        m_forwardWS[i].normalize();

        resolveSingleBilateral(*m_chassisBody,
                               wheelInfo.m_raycastInfo.m_contactPointWS,
                               *groundObject,
                               wheelInfo.m_raycastInfo.m_contactPointWS,
                               btScalar(0.), m_axle[i],m_sideImpulse[i],
                               timeStep);

        btScalar sideFrictionStiffness2 = btScalar(1.0);
        m_sideImpulse[i] *= sideFrictionStiffness2;
    }

    btScalar sideFactor = btScalar(1.);
    btScalar fwdFactor = 0.5;

    bool sliding = false;
    for (int wheel=0; wheel<getNumWheels(); wheel++)
    {
        btWheelInfo& wheelInfo        = m_wheelInfo[wheel];
        m_wheelInfo[wheel].m_skidInfo = btScalar(1.);
        m_forwardImpulse[wheel]       = btScalar(0.);

        btRigidBody* groundObject =
            (btRigidBody*) wheelInfo.m_raycastInfo.m_groundObject;
        if(!groundObject) continue;

        btScalar rollingFriction = 0.f;

        if (wheelInfo.m_engineForce != 0.f)
        {
            rollingFriction = wheelInfo.m_engineForce* timeStep;
        }
        else
        {
            btScalar defaultRollingFrictionImpulse = 0.f;
            btScalar maxImpulse = wheelInfo.m_brake
                ? wheelInfo.m_brake
                : defaultRollingFrictionImpulse;
            btWheelContactPoint contactPt(m_chassisBody, groundObject,
                wheelInfo.m_raycastInfo.m_contactPointWS,
                m_forwardWS[wheel], maxImpulse);
            rollingFriction = calcRollingFriction(contactPt);
            // This is a work around for the problem that a kart shakes
            // if it is braking: we get a minor impulse forward, which
            // bullet then tries to offset by applying a backward
            // impulse - which is a bit too big, causing a impulse
            // backwards, ... till the kart is shaking backwards and
            // forwards. By only applying half of the impulse in case
            // of low friction this goes away.
            if (wheelInfo.m_brake && fabsf(rollingFriction) < 10)
                rollingFriction *= 0.5f;
        }

        m_forwardImpulse[wheel] = rollingFriction;

        if(m_ticks_additional_impulse>0)
        {
            sliding = true;
            m_wheelInfo[wheel].m_skidInfo = 0.0f;
        }
        else
        {
            btScalar maximp         = wheelInfo.m_wheelsSuspensionForce
                                    * timeStep * wheelInfo.m_frictionSlip;
            btScalar maximpSide     = maximp;
            btScalar maximpSquared  = maximp * maximpSide;

            btScalar x = (m_forwardImpulse[wheel] ) * fwdFactor;
            btScalar y = (m_sideImpulse[wheel]    ) * sideFactor;

            btScalar impulseSquared = (x*x + y*y);

            if (impulseSquared > maximpSquared)
            {
                sliding = true;
                btScalar factor = maximp / btSqrt(impulseSquared);
                m_wheelInfo[wheel].m_skidInfo *= factor;
            }   // if impulseSquared > maximpSquared
        }   // else (!m_timed_impulse
    }   // for (int wheel=0; wheel<getNumWheels(); wheel++)


    // Note: don't reset zipper speed, or the kart rewinder will
    // get incorrect zipper information.

    if (sliding && (m_allow_sliding || m_ticks_additional_impulse>0) )
    {
        for (int wheel = 0; wheel < getNumWheels(); wheel++)
        {
            if (m_sideImpulse[wheel] != btScalar(0.)       &&
                m_wheelInfo[wheel].m_skidInfo< btScalar(1.)   )
            {
                m_forwardImpulse[wheel] *= m_wheelInfo[wheel].m_skidInfo;
                m_sideImpulse[wheel] *= m_wheelInfo[wheel].m_skidInfo;
            }
        }   // for wheel <getNumWheels
    }   // if sliding

    // Apply the impulses
    // ------------------
    for (int wheel = 0;wheel<getNumWheels() ; wheel++)
    {
        btWheelInfo& wheelInfo = m_wheelInfo[wheel];
        btVector3 rel_pos      = wheelInfo.m_raycastInfo.m_contactPointWS
                                 - m_chassisBody->getCenterOfMassPosition();

        if (m_forwardImpulse[wheel] != btScalar(0.))
        {
            m_chassisBody->applyImpulse(
                                  m_forwardWS[wheel]*(m_forwardImpulse[wheel]),
#define COMPATIBLE_0_7_3
#ifdef COMPATIBLE_0_7_3
                                  // This was apparently done to help hexley
                                  btVector3(0,0,0));
#else
                                  rel_pos);
#endif
        }
        if (m_sideImpulse[wheel] != btScalar(0.))
        {
            btRigidBody* groundObject =
                (btRigidBody*) m_wheelInfo[wheel].m_raycastInfo.m_groundObject;
            btVector3 rel_pos2 = wheelInfo.m_raycastInfo.m_contactPointWS
                               - groundObject->getCenterOfMassPosition();
            //adjust relative position above ground so that force only
            // acts sideways
            btVector3 delta_vec = (wheelInfo.m_raycastInfo.m_hardPointWS
                                - wheelInfo.m_raycastInfo.m_contactPointWS);
            if (delta_vec.length() != btScalar (0))
            {
                delta_vec = delta_vec.normalize();
                rel_pos -= delta_vec * rel_pos.dot(delta_vec);
            }

            btVector3 sideImp = m_axle[wheel] * m_sideImpulse[wheel];

#if defined ROLLING_INFLUENCE_FIX && !defined COMPATIBLE_0_7_3
            // fix. It only worked if car's up was along Y - VT.
            btVector3 vChassisWorldUp =
                                    getRigidBody()->getCenterOfMassTransform()
                                   .getBasis().getColumn(m_indexUpAxis);
            rel_pos -= vChassisWorldUp * (vChassisWorldUp.dot(rel_pos) *
                                          (1.f-wheelInfo.m_rollInfluence) );
#else
            rel_pos[m_indexUpAxis] *= wheelInfo.m_rollInfluence;
#endif
            m_chassisBody->applyImpulse(sideImp,rel_pos);

            //apply friction impulse on the ground
            groundObject->applyImpulse(-sideImp,rel_pos2);
        }   // if (m_sideImpulse[wheel] != btScalar(0.))
    }   // for wheel<getNumWheels()



}   // updateFriction

// ----------------------------------------------------------------------------
void btKart::debugDraw(btIDebugDraw* debugDrawer)
{
    const btVector3 &from = m_kart->getTerrainInfo()->getOrigin();
    const btVector3 &to = m_kart->getTerrainInfo()->getHitPoint();
    debugDrawer->drawLine(from, to, btVector3(0.5, 0.5, 0));

    for (int v=0;v<getNumWheels();v++)
    {
        btVector3 wheelColor(0,1,1);
        const btWheelInfo &w = getWheelInfo(v);
        if (w.m_raycastInfo.m_isInContact)
        {
            wheelColor.setValue(0,0,1);
        } else
        {
            wheelColor.setValue(1,0,1);
        }

        btVector3 wheelPosWS = w.m_worldTransform.getOrigin();

        btVector3 axle = btVector3(
                            w.m_worldTransform.getBasis()[0][getRightAxis()],
                            w.m_worldTransform.getBasis()[1][getRightAxis()],
                            w.m_worldTransform.getBasis()[2][getRightAxis()]);

        //debug wheels (cylinders)
        debugDrawer->drawLine(wheelPosWS,wheelPosWS+axle,wheelColor);
        debugDrawer->drawLine(wheelPosWS,
                              w.m_raycastInfo.m_contactPointWS,
                              wheelColor);
        // Draw the (interpolated) normal of the ground at the wheel position
        btVector3 white(1.0f, 1.0f, 1.0f);
        debugDrawer->drawLine(w.m_raycastInfo.m_contactPointWS,
                              w.m_raycastInfo.m_contactPointWS+
                                 w.m_raycastInfo.m_contactNormalWS,
                              white);
        int n = w.m_raycastInfo.m_triangle_index;
        if (n > -1)
        {
            const TriangleMesh &tm = Track::getCurrentTrack()->getTriangleMesh();
            btVector3 p1, p2, p3;
            tm.getTriangle(n, &p1, &p2, &p3);
            btVector3 n1, n2, n3;
            tm.getNormals(n, &n1, &n2, &n3);
            // Draw the normals at the vertices
            debugDrawer->drawLine(p1, p1 + n1, white);
            debugDrawer->drawLine(p2, p2 + n2, white);
            debugDrawer->drawLine(p3, p3 + n3, white);
            // Also draw the triangle in white, it can make it easier
            // to identify which triangle a wheel is on
            debugDrawer->drawTriangle(p1, p2, p3, white, 1.0f);
        }

    }   // for i < getNumWheels
}   // debugDraw


// ----------------------------------------------------------------------------
/** Enables or disables sliding.
 *  \param active Enable (true) or disable sliding.
 */
void btKart::setSliding(bool active)
{
    m_allow_sliding = active;
}   // setSliding

// ----------------------------------------------------------------------------
/** Adjusts the velocity of this kart to be at least the specified minimum,
 *  and less than or equal to the maximum. If necessary the kart will
 *  instantaneously change its speed.
 *  \param min_speed Minimum speed, 0 means no effect.
 *  \param max_speed Maximum speed the kart is allowed to have.
 */
void btKart::adjustSpeed(btScalar min_speed, btScalar max_speed)
{
    const btVector3 &velocity = m_chassisBody->getLinearVelocity();
    float speed = velocity.length();


    if (speed < min_speed && min_speed > 0)
    {
        if (speed > 0)
        {
            // The speedup is only for the direction of the normal.
            const btVector3 &normal = m_kart->getNormal();
            btVector3 upright_component = normal * normal.dot(velocity);
            // Subtract the upright velocity component,
            btVector3 v = velocity - upright_component;
            if (!v.fuzzyZero())
            {
                const float velocity_ratio = min_speed / v.length();
                // Scale the velocity in the plane, then add the upright component
                // of the velocity back in.
                m_chassisBody->setLinearVelocity( v*velocity_ratio
                                                + upright_component );
            }
        }
    }
    else if (speed >0 && max_speed >= 0 && speed > max_speed)
    {
        const float velocity_ratio = max_speed / speed;
        m_chassisBody->setLinearVelocity(velocity * velocity_ratio);
    }
}   // adjustSpeed

// ----------------------------------------------------------------------------
//Shorter version of above raycast function. This is used when projecting
//vehicles towards the ground at the start of a race
btScalar btKart::rayCast(btWheelInfo& wheel, const btVector3& ray)
{
    updateWheelTransformsWS(wheel, getChassisWorldTransform(), false);

    btScalar depth          = -1;

    const btVector3& source = wheel.m_raycastInfo.m_hardPointWS;
    wheel.m_raycastInfo.m_contactPointWS = source + ray;
    const btVector3& target = source + ray;

    btVehicleRaycaster::btVehicleRaycasterResult    rayResults;

    assert(m_vehicleRaycaster);

    void* object = m_vehicleRaycaster->castRay(source,target,rayResults);

    wheel.m_raycastInfo.m_groundObject = 0;

    if (object)
    {
        depth = ray.length() * rayResults.m_distFraction;

        wheel.m_raycastInfo.m_contactPointWS   = rayResults.m_hitPointInWorld;
        wheel.m_raycastInfo.m_contactNormalWS  = rayResults.m_hitNormalInWorld;
        wheel.m_raycastInfo.m_isInContact      = true;
        wheel.m_raycastInfo.m_triangle_index   = rayResults.m_triangle_index;
    }

    return depth;
}   // rayCast(btWheelInfo& wheel, const btVector3& ray

// ----------------------------------------------------------------------------

