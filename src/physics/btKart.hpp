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
#ifndef BT_KART_HPP
#define BT_KART_HPP

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "BulletDynamics/ConstraintSolver/btTypedConstraint.h"
#include "physics/btKartRaycast.hpp"
class btDynamicsWorld;
#include "LinearMath/btAlignedObjectArray.h"
#include "BulletDynamics/Vehicle/btWheelInfo.h"
#include "BulletDynamics/Dynamics/btActionInterface.h"

class btVehicleTuning;
class Kart;
struct btWheelContactPoint;

///rayCast vehicle, very special constraint that turn a rigidbody into a vehicle.
class btKart : public btActionInterface
{
private:
		btAlignedObjectArray<btVector3>	m_forwardWS;
		btAlignedObjectArray<btVector3>	m_axle;
		btAlignedObjectArray<btScalar>	m_forwardImpulse;
		btAlignedObjectArray<btScalar>	m_sideImpulse;
	
		///backwards compatibility
		int	m_userConstraintType;
		int	m_userConstraintId;

        static btRigidBody& getFixedBody();
        btScalar calcRollingFriction(btWheelContactPoint& contactPoint);

public:
	class btVehicleTuning
		{
			public:

			btVehicleTuning()
				:m_suspensionStiffness(btScalar(5.88)),
				m_suspensionCompression(btScalar(0.83)),
				m_suspensionDamping(btScalar(0.88)),
				m_maxSuspensionTravelCm(btScalar(500.)),
				m_frictionSlip(btScalar(10.5)),
				m_maxSuspensionForce(btScalar(6000.))
			{
			}
			btScalar	m_suspensionStiffness;
			btScalar	m_suspensionCompression;
			btScalar	m_suspensionDamping;
			btScalar	m_maxSuspensionTravelCm;
			btScalar	m_frictionSlip;
			btScalar	m_maxSuspensionForce;

		};
protected:

	btScalar	        m_damping;
	btVehicleRaycaster *m_vehicleRaycaster;
	btScalar            m_currentVehicleSpeedKmHour;
    /** True if a zipper is active for that kart. */
    bool                m_zipper_active;

    /** The zipper velocity (i.e. the velocity the kart should reach in
     *  the first frame that the zipper is active). */
    btScalar            m_zipper_velocity;

    /** The angular velocity to be applied when the kart skids. 
     *  0 means no skidding. */
    btScalar            m_skid_angular_velocity;

    /** True if the kart is currently skidding. This is used to detect
     *  the end of skidding (i.e. m_skid_angular_velocity=0 and
     *  m_is_skidding=true), and triggers adjusting of the velocity 
     *  direction. */
    bool                m_is_skidding;
    
    /** Sliding (skidding) will only be permited when this is true. Also check
     *  the friction parameter in the wheels since friction directly affects 
     *  skidding.
     */
    bool                m_allow_sliding;

    /** The center point of the front (in car coordinates). */
    btVector3           m_front_center_pointCS;

	btRigidBody* m_chassisBody;

    int m_num_wheels_on_ground;
	int m_indexRightAxis;
	int m_indexUpAxis;
	int	m_indexForwardAxis;

    /** The STK kart object which uses this vehicle. This is mostly used to 
     *  get access to the kart properties, which also define physics 
     *  properties. */
    Kart *m_kart;

	void defaultInit();

public:

	//constructor to create a car from an existing rigidbody
	btKart(btRigidBody* chassis, btVehicleRaycaster* raycaster,
           Kart *kart);

	virtual ~btKart() ;

    void reset();

	///btActionInterface interface
	virtual void updateAction( btCollisionWorld* collisionWorld, btScalar step)
	{
        (void) collisionWorld;
		updateVehicle(step);
	}
	
	///btActionInterface interface
	void	debugDraw(btIDebugDraw* debugDrawer);
			
	const btTransform& getChassisWorldTransform() const;
	
	btScalar rayCast(btWheelInfo& wheel);

	virtual void updateVehicle(btScalar step);
	
	
	void resetSuspension();

	btScalar	getSteeringValue(int wheel) const;

	void	setSteeringValue(btScalar steering,int wheel);


	void	applyEngineForce(btScalar force, int wheel);

	const btTransform&	getWheelTransformWS( int wheelIndex ) const;

	void	updateWheelTransform( int wheelIndex, bool interpolatedTransform = true );
	
//	void	setRaycastWheelInfo( int wheelIndex , bool isInContact, const btVector3& hitPoint, const btVector3& hitNormal,btScalar depth);

	btWheelInfo&	addWheel( const btVector3& connectionPointCS0, const btVector3& wheelDirectionCS0,const btVector3& wheelAxleCS,btScalar suspensionRestLength,btScalar wheelRadius,const btVehicleTuning& tuning, bool isFrontWheel);

	inline int		getNumWheels() const {
		return int (m_wheelInfo.size());
	}
	
	btAlignedObjectArray<btWheelInfo>	m_wheelInfo;


	const btWheelInfo&	getWheelInfo(int index) const;

	btWheelInfo&	getWheelInfo(int index);

	void	updateWheelTransformsWS(btWheelInfo& wheel , bool interpolatedTransform = true);

	
	void setBrake(btScalar brake,int wheelIndex);
	
	void	updateSuspension(btScalar deltaTime);

	virtual void	updateFriction(btScalar	timeStep);



	inline btRigidBody* getRigidBody()
	{
		return m_chassisBody;
	}

	const btRigidBody* getRigidBody() const
	{
		return m_chassisBody;
	}

	inline int	getRightAxis() const
	{
		return m_indexRightAxis;
	}
	inline int getUpAxis() const
	{
		return m_indexUpAxis;
	}

	inline int getForwardAxis() const
	{
		return m_indexForwardAxis;
	}

	
	///Worldspace forward vector
	btVector3 getForwardVector() const
	{
		const btTransform& chassisTrans = getChassisWorldTransform(); 

		btVector3 forwardW ( 
			  chassisTrans.getBasis()[0][m_indexForwardAxis], 
			  chassisTrans.getBasis()[1][m_indexForwardAxis], 
			  chassisTrans.getBasis()[2][m_indexForwardAxis]); 

		return forwardW;
	}

	///Velocity of vehicle (positive if velocity vector has same direction as foward vector)
	btScalar	getCurrentSpeedKmHour() const
	{
		return m_currentVehicleSpeedKmHour;
	}

	virtual void	setCoordinateSystem(int rightIndex,int upIndex,int forwardIndex)
	{
		m_indexRightAxis = rightIndex;
		m_indexUpAxis = upIndex;
		m_indexForwardAxis = forwardIndex;
	}


	///backwards compatibility
	int getUserConstraintType() const
	{
		return m_userConstraintType ;
	}

	void	setUserConstraintType(int userConstraintType)
	{
		m_userConstraintType = userConstraintType;
	};

	void	setUserConstraintId(int uid)
	{
		m_userConstraintId = uid;
	}

	int getUserConstraintId() const
	{
		return m_userConstraintId;
	}
private:
    btScalar rayCast(btWheelInfo& wheel, const btVector3& ray);
public:
    void setSliding(bool active);
    void activateZipper(float speed);
    void deactivateZipper();
    bool projectVehicleToSurface(const btVector3& ray, 
                                 bool translate_vehicle);
    // ------------------------------------------------------------------------
    /** Sets the angular velocity to be used when skidding 
     *  (0 means no skidding). */
    void setSkidAngularVelocity(float v) {m_skid_angular_velocity = v; }
    // ------------------------------------------------------------------------
    /** Returns the number of wheels on the ground. */
    unsigned int getNumWheelsOnGround() const {return m_num_wheels_on_ground;}
    // ------------------------------------------------------------------------
    /** Returns the middle point of the front wheel connection points in
     *  world coordinates. */
    btVector3 getFrontCenterPointWS() const 
    {
        return m_chassisBody->getWorldTransform()(m_front_center_pointCS);
    }   // getFrontCenterPointWS
};

#endif //BT_RAYCASTVEHICLE_H

