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
#ifndef BTKARTRAYCASTER_HPP
#define BTKARTRAYCSATER_HPP

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "BulletDynamics/ConstraintSolver/btTypedConstraint.h"
#include "BulletDynamics/Vehicle/btVehicleRaycaster.h"
class btDynamicsWorld;
#include "LinearMath/btAlignedObjectArray.h"
#include "BulletDynamics/Vehicle/btWheelInfo.h"
#include "BulletDynamics/Dynamics/btActionInterface.h"


class btKartRaycaster : public btVehicleRaycaster
{
	btDynamicsWorld*	m_dynamicsWorld;
public:
	btKartRaycaster(btDynamicsWorld* world)
		:m_dynamicsWorld(world)
	{
	}

	virtual void* castRay(const btVector3& from,const btVector3& to, 
                          btVehicleRaycasterResult& result);

};


#endif //RAYCASTVEHICLE_H

