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
#ifndef BT_KART_HPP
#define BT_KART_HPP

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "BulletDynamics/ConstraintSolver/btTypedConstraint.h"
#include "physics/btKartRaycast.hpp"
class btDynamicsWorld;
#include "LinearMath/btAlignedObjectArray.h"
#include "BulletDynamics/Vehicle/btWheelInfo.h"
#include "BulletDynamics/Dynamics/btActionInterface.h"

#include "config/stk_config.hpp"

class btVehicleTuning;
class Kart;
struct btWheelContactPoint;

/** rayCast vehicle, very special constraint that turn a rigidbody into a
 *  vehicle.
 */
class btKart : public btActionInterface
{
public:
    class btVehicleTuning
    {
    public:

        btVehicleTuning()
            :m_suspensionStiffness(btScalar(5.88)),
             m_suspensionCompression(btScalar(0.83)),
             m_suspensionDamping(btScalar(0.88)),
             m_maxSuspensionTravel(btScalar(5.)),
             m_frictionSlip(btScalar(10.5)),
             m_maxSuspensionForce(btScalar(6000.))
        {
        }   // btVehicleTuning

        btScalar    m_suspensionStiffness;
        btScalar    m_suspensionCompression;
        btScalar    m_suspensionDamping;
        btScalar    m_maxSuspensionTravel;
        btScalar    m_frictionSlip;
        btScalar    m_maxSuspensionForce;

    };   // class btVehicleTuning

private:

    btAlignedObjectArray<btVector3> m_forwardWS;
    btAlignedObjectArray<btVector3> m_axle;
    btAlignedObjectArray<btScalar>  m_forwardImpulse;
    btAlignedObjectArray<btScalar>  m_sideImpulse;

    ///backwards compatibility
    int m_userConstraintType;
    int m_userConstraintId;

    static btRigidBody& getFixedBody();
    btScalar calcRollingFriction(btWheelContactPoint& contactPoint);

    btScalar            m_damping;
    btVehicleRaycaster *m_vehicleRaycaster;

    /** Sliding (skidding) will only be permited when this is true. Also check
     *  the friction parameter in the wheels since friction directly affects
     *  skidding.
     */
    bool                m_allow_sliding;

    /** An additional impulse that is applied for a certain amount of time. */
    btVector3           m_additional_impulse;

    /** The time the additional impulse should be applied. */
    uint16_t            m_ticks_additional_impulse;

    /** Additional rotation in y-axis that is applied over a certain amount of time. */
    float               m_additional_rotation;

    /** Duration over which the additional rotation is applied. */
    uint16_t            m_ticks_additional_rotation;

    /** The rigid body that is the chassis of the kart. */
    btRigidBody        *m_chassisBody;

    /** Used to replace the ground object. */
    btRigidBody         m_fixed_body;

    /** Number of wheels that touch the ground. */
    int                 m_num_wheels_on_ground;

    /** Index of the right axis. */
    int                 m_indexRightAxis;
    /** Index of the up axis. */
    int                 m_indexUpAxis;
    /** Index of the forward axis. */
    int                 m_indexForwardAxis;

    /** The STK kart object which uses this vehicle. This is mostly used to
     *  get access to the kart properties, which also define physics
     *  properties. */
    Kart               *m_kart;

    /** Minimum speed for the kart. Used e.g. for zippers. Setting this value
     *  will potentially instantaneously accelerate the kart to the minimum
     *  speed requested (in the next physics step). */
    btScalar m_min_speed;

    /** Maximum speed for the kart. It is reset to -1 at the end of each
     *  physics steps, so need to be set again by the application. */
    btScalar m_max_speed;

    /** True if the visual wheels touch the ground. */
    bool m_visual_wheels_touch_ground;

    btAlignedObjectArray<btWheelInfo> m_wheelInfo;

    void     defaultInit();
    btScalar rayCast(btWheelInfo& wheel, const btVector3& ray);
    void     updateWheelTransformsWS(btWheelInfo& wheel,
                                     btTransform chassis_trans,
                                     bool interpolatedTransform=true,
                                     float fraction = 1.0f);

public:

    /** Constructor to create a car from an existing rigidbody.
     *  \param chassis The rigid body to use as chassis.
     *  \param raycaster The raycast object to use.
     *  \paran kart The STK kart object that uses this vehicle
     *         (this is used to get access to the kart properties).
     */
                       btKart(btRigidBody* chassis,
                              btVehicleRaycaster* raycaster,
                              Kart *kart);
     virtual          ~btKart();
    void               reset();
    void               debugDraw(btIDebugDraw* debugDrawer);
    const btTransform& getChassisWorldTransform() const;
    btScalar           rayCast(unsigned int index, float fraction=1.0f);
    virtual void       updateVehicle(btScalar step);
    void               resetSuspension();
    btScalar           getSteeringValue(int wheel) const;
    void               setSteeringValue(btScalar steering,int wheel);
    void               applyEngineForce(btScalar force, int wheel);
    const btTransform& getWheelTransformWS( int wheelIndex ) const;
    void               updateWheelTransform(int wheelIndex,
                                            bool interpolatedTransform=true);
    btWheelInfo&       addWheel(const btVector3& connectionPointCS0,
                                const btVector3& wheelDirectionCS0,
                                const btVector3& wheelAxleCS,
                                btScalar suspensionRestLength,
                                btScalar wheelRadius,
                                const btVehicleTuning& tuning,
                                bool isFrontWheel);
    const btWheelInfo& getWheelInfo(int index) const;
    btWheelInfo&       getWheelInfo(int index);
    void               updateAllWheelTransformsWS();
    void               setAllBrakes(btScalar brake);
    void               updateSuspension(btScalar deltaTime);
    virtual void       updateFriction(btScalar timeStep);
    void               setSliding(bool active);
    void               instantSpeedIncreaseTo(btScalar speed);
    void               adjustSpeed(btScalar min_speed, btScalar max_speed);
    void               updateAllWheelPositions();
    void               getVisualContactPoint(const btTransform& chassis_trans,
                                             btVector3 *left, btVector3 *right);
        // ------------------------------------------------------------------------
    /** Returns true if both rear visual wheels touch the ground. */
    bool visualWheelsTouchGround() const
    {
        return m_visual_wheels_touch_ground;
    }   // visualWheelsTouchGround
    // ------------------------------------------------------------------------
    /** btActionInterface interface. */
    virtual void updateAction(btCollisionWorld* collisionWorld,
                              btScalar step)
    {
        (void) collisionWorld;
        updateVehicle(step);
    }   // updateAction
    // ------------------------------------------------------------------------
    /** Returns the number of wheels of this vehicle. */
    inline int getNumWheels() const { return int(m_wheelInfo.size());}
    // ------------------------------------------------------------------------
    /** Returns the chassis (rigid) body. */
    inline btRigidBody* getRigidBody() { return m_chassisBody; }
    // ------------------------------------------------------------------------
    /** Returns the chassis (rigid) body. */
    const btRigidBody* getRigidBody() const { return m_chassisBody; }
    // ------------------------------------------------------------------------
    /** Returns the index of the right axis. */
    inline int getRightAxis() const { return m_indexRightAxis; }
    // ------------------------------------------------------------------------
    /** Returns the index of the up axis. */
    inline int getUpAxis() const { return m_indexUpAxis; }
    // ------------------------------------------------------------------------
    /** Returns the index of the forward axis. */
    inline int getForwardAxis() const { return m_indexForwardAxis; }
    // ------------------------------------------------------------------------
    /** Backwards compatibility. */
    int getUserConstraintType() const { return m_userConstraintType ; }
    // ------------------------------------------------------------------------
    void setUserConstraintType(int userConstraintType)
    {
        m_userConstraintType = userConstraintType;
    }   // setUserConstraintType
    // ------------------------------------------------------------------------
    void setUserConstraintId(int uid) { m_userConstraintId = uid; }
    // ------------------------------------------------------------------------
    int getUserConstraintId() const { return m_userConstraintId; }
    // ------------------------------------------------------------------------
    /** Returns the number of wheels on the ground. */
    unsigned int getNumWheelsOnGround() const {return m_num_wheels_on_ground;}
    // ------------------------------------------------------------------------
    /** Sets an impulse that is applied for a certain amount of time.
     *  \param t Ticks for the impulse to be active.
     *  \param imp The impulse to apply.  */
    void setTimedCentralImpulse(uint16_t t, const btVector3 &imp,
                                bool rewind = false)
    {
        // Only add impulse if no other impulse is active.
        if (m_ticks_additional_impulse > 0 && !rewind) return;
        m_additional_impulse      = imp;
        m_ticks_additional_impulse = t;
    }   // setTimedImpulse
    // ------------------------------------------------------------------------
    /** Returns the time an additional impulse is activated. */
    uint16_t getCentralImpulseTicks() const
                                         { return m_ticks_additional_impulse; }
    // ------------------------------------------------------------------------
    const btVector3& getAdditionalImpulse() const
                                               { return m_additional_impulse; }
    // ------------------------------------------------------------------------
    /** Sets a rotation that is applied over a certain amount of time (to avoid
     *  a too rapid changes in the kart).
     *  \param t Ticks for the rotation to be applied.
     *  \param rot_in_y_axis The rotation in y-axis to apply.  */
    void setTimedRotation(uint16_t t, float rot_in_y_axis)
    {
        if (t > 0)
        {
            m_additional_rotation =
                rot_in_y_axis / (stk_config->ticks2Time(t));
        }
        m_ticks_additional_rotation = t;
    }   // setTimedTorque
    // ------------------------------------------------------------------------
    float getTimedRotation() const            { return m_additional_rotation; }
    // ------------------------------------------------------------------------
    uint16_t getTimedRotationTicks() const
                                        { return m_ticks_additional_rotation; }
    // ------------------------------------------------------------------------
    /** Sets the maximum speed for this kart. */
    void setMaxSpeed(float new_max_speed) 
    {
        // Only change m_max_speed if it has not been set (<0), or
        // the new value is smaller than the current maximum. For example,
        // overworld will set the max_speed to 0 in case of teleporting to
        // a bubble, but set it again later (based on zipper etc activated).
        // We need to make sure that the 0 is maintained.
        if(m_max_speed <0 || m_max_speed > new_max_speed)
            m_max_speed = new_max_speed;
    }   // setMaxSpeed
    // ------------------------------------------------------------------------
    /** Resets the maximum so any new maximum value from the application will
     *  be accepted. */
    virtual void resetMaxSpeed() { m_max_speed = -1.0f; m_min_speed = 0.0f; }
    // ------------------------------------------------------------------------
    /** Sets the minimum speed for this kart. */
    void setMinSpeed(float s)
    {
        if(s > m_min_speed) m_min_speed = s; 
    }
    // ------------------------------------------------------------------------
    /** Returns the minimum speed for this kart. */
    btScalar getMinSpeed() const { return m_min_speed; }
};   // class btKart

#endif //BT_KART_HPP

