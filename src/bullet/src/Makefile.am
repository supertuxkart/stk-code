noinst_LIBRARIES	= libbulletmath.a libbulletcollision.a libbulletdynamics.a

libbulletmath_a_SOURCES	= \
	LinearMath/btAabbUtil2.h  LinearMath/btList.h LinearMath/btQuickprof.h	 \
	LinearMath/btAlignedAllocator.cpp 	LinearMath/btMatrix3x3.h	 \
	LinearMath/btRandom.h 			LinearMath/btAlignedAllocator.h  \
	LinearMath/btMinMax.h	 		LinearMath/btScalar.h		 \
	LinearMath/btAlignedObjectArray.h	LinearMath/btMotionState.h	 \
	LinearMath/btPoolAllocator.h		LinearMath/btDefaultMotionState.h\
	LinearMath/btPoint3.h	 		LinearMath/btStackAlloc.h	 \
	LinearMath/btGeometryUtil.cpp		LinearMath/btQuadWord.h		 \
	LinearMath/btTransform.h 		LinearMath/btGeometryUtil.h	 \
	LinearMath/btQuaternion.h		LinearMath/btTransformUtil.h	 \
	LinearMath/btIDebugDraw.h		LinearMath/btQuickprof.cpp  	 \
	LinearMath/btTypedUserInfo.h		LinearMath/btTypedUserInfo.h	 \
	LinearMath/btVector3.h			LinearMath/btHashMap.h		 \
	LinearMath/btConvexHull.cpp		LinearMath/btConvexHull.h

# Certain g++ versions causes stk to crash if btVoronoiSimplexSolver.cpp is
# compiled with optimisations. Since it's rather difficult to specify certain
# flags for one file only with auto-tools (you can only specify flags per
# library, so you have to put this one file into a separate library) we compile
# the whole libbulletcollision with the modified flags - i.e. by adding 
# -fno-elide-constructors to the flags. The overall performance impact is
# quite low anyway.
libbulletcollision_a_CXXFLAGS = @NOELIDE@ 
libbulletcollision_a_SOURCES = \
	btBulletCollisionCommon.h						 \
	BulletCollision/BroadphaseCollision/btAxisSweep3.cpp			 \
	BulletCollision/BroadphaseCollision/btAxisSweep3.h			 \
	BulletCollision/BroadphaseCollision/btBroadphaseInterface.h		 \
	BulletCollision/BroadphaseCollision/btBroadphaseProxy.cpp		 \
	BulletCollision/BroadphaseCollision/btBroadphaseProxy.h			 \
	BulletCollision/BroadphaseCollision/btCollisionAlgorithm.cpp		 \
	BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h		 \
	BulletCollision/BroadphaseCollision/btDispatcher.cpp			 \
	BulletCollision/BroadphaseCollision/btDispatcher.h			 \
	BulletCollision/BroadphaseCollision/btMultiSapBroadphase.cpp		 \
	BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h		 \
	BulletCollision/BroadphaseCollision/btOverlappingPairCache.cpp		 \
	BulletCollision/BroadphaseCollision/btOverlappingPairCache.h		 \
	BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h		 \
	BulletCollision/BroadphaseCollision/btSimpleBroadphase.cpp		 \
	BulletCollision/BroadphaseCollision/btSimpleBroadphase.h		 \
	BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.h		 \
	BulletCollision/CollisionDispatch/btBoxBoxDetector.h			 \
	BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.cpp	 \
	BulletCollision/CollisionDispatch/btBoxBoxDetector.cpp			 \
	BulletCollision/CollisionDispatch/btCollisionConfiguration.h		 \
	BulletCollision/CollisionDispatch/btCollisionCreateFunc.h		 \
	BulletCollision/CollisionDispatch/btCollisionDispatcher.cpp		 \
	BulletCollision/CollisionDispatch/btCollisionDispatcher.h		 \
	BulletCollision/CollisionDispatch/btCollisionObject.cpp			 \
	BulletCollision/CollisionDispatch/btCollisionObject.h			 \
	BulletCollision/CollisionDispatch/btCollisionWorld.cpp			 \
	BulletCollision/CollisionDispatch/btCollisionWorld.h			 \
	BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.cpp	 \
	BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.h	 \
	BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.cpp	 \
	BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.h	 \
	BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.cpp	 \
	BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.h	 \
	BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.cpp		 \
	BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.h		 \
	BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.cpp	 \
	BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h	 \
	BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.cpp		 \
	BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.h		 \
	BulletCollision/CollisionDispatch/btManifoldResult.cpp			 \
	BulletCollision/CollisionDispatch/btManifoldResult.h			 \
	BulletCollision/CollisionDispatch/btSimulationIslandManager.cpp		 \
	BulletCollision/CollisionDispatch/btSimulationIslandManager.h		 \
	BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.cpp	 \
	BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.h	 \
	BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.cpp	 \
	BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h	 \
	BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.cpp \
	BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.h	 \
	BulletCollision/CollisionDispatch/btUnionFind.cpp			 \
	BulletCollision/CollisionDispatch/btUnionFind.h				 \
	BulletCollision/CollisionDispatch/SphereTriangleDetector.cpp		 \
	BulletCollision/CollisionDispatch/SphereTriangleDetector.h		 \
	BulletCollision/CollisionShapes/btBoxShape.cpp				 \
	BulletCollision/CollisionShapes/btBoxShape.h				 \
	BulletCollision/CollisionShapes/btBvhTriangleMeshShape.cpp		 \
	BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h   	 	 \
	BulletCollision/CollisionShapes/btCapsuleShape.cpp			 \
	BulletCollision/CollisionShapes/btCapsuleShape.h			 \
	BulletCollision/CollisionShapes/btCollisionMargin.h			 \
	BulletCollision/CollisionShapes/btCollisionShape.cpp			 \
	BulletCollision/CollisionShapes/btCollisionShape.h			 \
	BulletCollision/CollisionShapes/btCompoundShape.cpp			 \
	BulletCollision/CollisionShapes/btCompoundShape.h			 \
	BulletCollision/CollisionShapes/btConcaveShape.cpp			 \
	BulletCollision/CollisionShapes/btConcaveShape.h			 \
	BulletCollision/CollisionShapes/btConeShape.cpp				 \
	BulletCollision/CollisionShapes/btConeShape.h				 \
	BulletCollision/CollisionShapes/btConvexHullShape.cpp			 \
	BulletCollision/CollisionShapes/btConvexHullShape.h			 \
	BulletCollision/CollisionShapes/btConvexInternalShape.cpp		 \
	BulletCollision/CollisionShapes/btConvexInternalShape.h			 \
	BulletCollision/CollisionShapes/btConvexShape.cpp			 \
	BulletCollision/CollisionShapes/btConvexShape.h			 	 \
	BulletCollision/CollisionShapes/btConvexTriangleMeshShape.cpp		 \
	BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h		 \
	BulletCollision/CollisionShapes/btCylinderShape.cpp			 \
	BulletCollision/CollisionShapes/btCylinderShape.h			 \
	BulletCollision/CollisionShapes/btEmptyShape.cpp			 \
	BulletCollision/CollisionShapes/btEmptyShape.h				 \
	BulletCollision/CollisionShapes/btHeightfieldTerrainShape.cpp		 \
	BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h    		 \
	BulletCollision/CollisionShapes/btMinkowskiSumShape.cpp			 \
	BulletCollision/CollisionShapes/btMinkowskiSumShape.h			 \
	BulletCollision/CollisionShapes/btMultiSphereShape.cpp			 \
	BulletCollision/CollisionShapes/btMultiSphereShape.h			 \
	BulletCollision/CollisionShapes/btOptimizedBvh.cpp			 \
	BulletCollision/CollisionShapes/btOptimizedBvh.h			 \
	BulletCollision/CollisionShapes/btPolyhedralConvexShape.cpp		 \
	BulletCollision/CollisionShapes/btPolyhedralConvexShape.h		 \
	BulletCollision/CollisionShapes/btShapeHull.cpp				 \
	BulletCollision/CollisionShapes/btShapeHull.h				 \
	BulletCollision/CollisionShapes/btSphereShape.cpp			 \
	BulletCollision/CollisionShapes/btSphereShape.h				 \
	BulletCollision/CollisionShapes/btStaticPlaneShape.cpp			 \
	BulletCollision/CollisionShapes/btStaticPlaneShape.h			 \
	BulletCollision/CollisionShapes/btStridingMeshInterface.cpp		 \
	BulletCollision/CollisionShapes/btStridingMeshInterface.h		 \
	BulletCollision/CollisionShapes/btTetrahedronShape.cpp			 \
	BulletCollision/CollisionShapes/btTetrahedronShape.h			 \
	BulletCollision/CollisionShapes/btTriangleBuffer.cpp			 \
	BulletCollision/CollisionShapes/btTriangleBuffer.h			 \
	BulletCollision/CollisionShapes/btTriangleCallback.cpp			 \
	BulletCollision/CollisionShapes/btTriangleCallback.h			 \
	BulletCollision/CollisionShapes/btTriangleIndexVertexArray.cpp		 \
	BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h		 \
	BulletCollision/CollisionShapes/btTriangleMesh.cpp			 \
	BulletCollision/CollisionShapes/btTriangleMesh.h			 \
	BulletCollision/CollisionShapes/btTriangleMeshShape.cpp			 \
	BulletCollision/CollisionShapes/btTriangleMeshShape.h			 \
	BulletCollision/CollisionShapes/btTriangleShape.h			 \
	BulletCollision/CollisionShapes/btUniformScalingShape.cpp		 \
	BulletCollision/CollisionShapes/btUniformScalingShape.h			 \
	BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.cpp	 \
	BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.h	 \
	BulletCollision/NarrowPhaseCollision/btConvexCast.cpp			 \
	BulletCollision/NarrowPhaseCollision/btConvexCast.h			 \
	BulletCollision/NarrowPhaseCollision/btConvexPenetrationDepthSolver.h	 \
	BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h\
	BulletCollision/NarrowPhaseCollision/btGjkConvexCast.cpp		 \
	BulletCollision/NarrowPhaseCollision/btGjkConvexCast.h			 \
	BulletCollision/NarrowPhaseCollision/btGjkEpa2.cpp			 \
	BulletCollision/NarrowPhaseCollision/btGjkEpa2.h			 \
	BulletCollision/NarrowPhaseCollision/btGjkEpa.cpp			 \
	BulletCollision/NarrowPhaseCollision/btGjkEpa.h				 \
	BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.cpp	 \
	BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h	 \
	BulletCollision/NarrowPhaseCollision/btGjkPairDetector.cpp		 \
	BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h		 \
	BulletCollision/NarrowPhaseCollision/btManifoldPoint.h			 \
	BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.cpp\
	BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.h \
	BulletCollision/NarrowPhaseCollision/btPersistentManifold.cpp		 \
	BulletCollision/NarrowPhaseCollision/btPersistentManifold.h		 \
	BulletCollision/NarrowPhaseCollision/btPointCollector.h			 \
	BulletCollision/NarrowPhaseCollision/btRaycastCallback.cpp		 \
	BulletCollision/NarrowPhaseCollision/btRaycastCallback.h		 \
	BulletCollision/NarrowPhaseCollision/btSimplexSolverInterface.h		 \
	BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.cpp		 \
	BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.h		 \
	BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.cpp		 \
	BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.h

libbulletdynamics_a_SOURCES	= \
	Bullet-C-Api.h					 			 \
	btBulletDynamicsCommon.h			 			 \
	BulletDynamics/ConstraintSolver/btConeTwistConstraint.cpp		 \
	BulletDynamics/ConstraintSolver/btConeTwistConstraint.h			 \
	BulletDynamics/ConstraintSolver/btConstraintSolver.h			 \
	BulletDynamics/ConstraintSolver/btContactConstraint.cpp			 \
	BulletDynamics/ConstraintSolver/btContactConstraint.h			 \
	BulletDynamics/ConstraintSolver/btContactSolverInfo.h			 \
	BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.cpp  		 \
	BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h		 \
	BulletDynamics/ConstraintSolver/btHingeConstraint.cpp			 \
	BulletDynamics/ConstraintSolver/btHingeConstraint.h			 \
	BulletDynamics/ConstraintSolver/btJacobianEntry.h			 \
	BulletDynamics/ConstraintSolver/btOdeJoint.cpp				 \
	BulletDynamics/ConstraintSolver/btOdeTypedJoint.cpp			 \
	BulletDynamics/ConstraintSolver/btOdeMacros.h				 \
	BulletDynamics/ConstraintSolver/btOdeSolverBody.h			 \
	BulletDynamics/ConstraintSolver/btOdeContactJoint.cpp			 \
	BulletDynamics/ConstraintSolver/btOdeContactJoint.h			 \
	BulletDynamics/ConstraintSolver/btSliderConstraint.cpp			 \
	BulletDynamics/ConstraintSolver/btSliderConstraint.h			 \
	BulletDynamics/ConstraintSolver/btSorLcp.cpp				 \
	BulletDynamics/ConstraintSolver/btSorLcp.h				 \
	BulletDynamics/ConstraintSolver/btOdeQuickstepConstraintSolver.h	 \
	BulletDynamics/ConstraintSolver/btOdeQuickstepConstraintSolver.cpp	 \
	BulletDynamics/ConstraintSolver/btOdeJoint.h				 \
	BulletDynamics/ConstraintSolver/btOdeTypedJoint.h			 \
	BulletDynamics/ConstraintSolver/btPoint2PointConstraint.cpp		 \
	BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h		 \
	BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp	 \
	BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h	 \
	BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.cpp		 \
	BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.h		 \
	BulletDynamics/ConstraintSolver/btSolverBody.h				 \
	BulletDynamics/ConstraintSolver/btSolverConstraint.h			 \
	BulletDynamics/ConstraintSolver/btTypedConstraint.cpp			 \
	BulletDynamics/ConstraintSolver/btTypedConstraint.h			 \
	BulletDynamics/Dynamics/btContinuousDynamicsWorld.cpp			 \
	BulletDynamics/Dynamics/btContinuousDynamicsWorld.h			 \
	BulletDynamics/Dynamics/btDiscreteDynamicsWorld.cpp			 \
	BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h			 \
	BulletDynamics/Dynamics/btDynamicsWorld.h				 \
	BulletDynamics/Dynamics/btRigidBody.cpp					 \
	BulletDynamics/Dynamics/btRigidBody.h 					 \
	BulletDynamics/Dynamics/btSimpleDynamicsWorld.cpp			 \
	BulletDynamics/Dynamics/btSimpleDynamicsWorld.h				 \
	BulletDynamics/Dynamics/Bullet-C-API.cpp				 \
	BulletDynamics/Vehicle/btRaycastVehicle.cpp				 \
	BulletDynamics/Vehicle/btRaycastVehicle.h				 \
	BulletDynamics/Vehicle/btVehicleRaycaster.h				 \
	BulletDynamics/Vehicle/btWheelInfo.cpp					 \
	BulletDynamics/Vehicle/btWheelInfo.h
