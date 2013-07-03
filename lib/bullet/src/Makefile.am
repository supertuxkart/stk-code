noinst_LIBRARIES	= libbulletmath.a libbulletcollision.a libbulletdynamics.a

libbulletmath_a_SOURCES	= \
	LinearMath/btAabbUtil2.h  LinearMath/btList.h LinearMath/btQuickprof.h	 \
	LinearMath/btAlignedAllocator.cpp 	LinearMath/btMatrix3x3.h	 \
	LinearMath/btRandom.h 			LinearMath/btAlignedAllocator.h  \
	LinearMath/btMinMax.h	 		LinearMath/btScalar.h		 \
	LinearMath/btAlignedObjectArray.h	LinearMath/btMotionState.h	 \
	LinearMath/btPoolAllocator.h		LinearMath/btDefaultMotionState.h\
	LinearMath/btStackAlloc.h	  	LinearMath/btGrahamScan2dConvexHull.h \
	LinearMath/btGeometryUtil.cpp		LinearMath/btQuadWord.h		 \
	LinearMath/btTransform.h 		LinearMath/btGeometryUtil.h	 \
	LinearMath/btQuaternion.h		LinearMath/btTransformUtil.h	 \
	LinearMath/btIDebugDraw.h		LinearMath/btQuickprof.cpp  	 \
	LinearMath/btVector3.h			LinearMath/btHashMap.h		 \
	LinearMath/btConvexHull.cpp		LinearMath/btConvexHull.h        \
	LinearMath/btConvexHullComputer.cpp	LinearMath/btConvexHullComputer.h \
        LinearMath/btSerializer.cpp             LinearMath/btSerializer.h


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
        BulletCollision/BroadphaseCollision/btDbvt.cpp                           \
        BulletCollision/BroadphaseCollision/btDbvt.h                             \
        BulletCollision/BroadphaseCollision/btDbvtBroadphase.cpp                 \
        BulletCollision/BroadphaseCollision/btDbvtBroadphase.h                   \
	BulletCollision/BroadphaseCollision/btDispatcher.cpp			 \
	BulletCollision/BroadphaseCollision/btDispatcher.h			 \
	BulletCollision/BroadphaseCollision/btMultiSapBroadphase.cpp		 \
	BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h		 \
	BulletCollision/BroadphaseCollision/btOverlappingPairCache.cpp		 \
	BulletCollision/BroadphaseCollision/btOverlappingPairCache.h		 \
	BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h		 \
        BulletCollision/BroadphaseCollision/btQuantizedBvh.cpp                   \
        BulletCollision/BroadphaseCollision/btQuantizedBvh.h                     \
	BulletCollision/BroadphaseCollision/btSimpleBroadphase.cpp		 \
	BulletCollision/BroadphaseCollision/btSimpleBroadphase.h		 \
	BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.cpp	 \
	BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.h		 \
	BulletCollision/CollisionDispatch/btBoxBoxDetector.cpp			 \
	BulletCollision/CollisionDispatch/btBoxBoxDetector.h			 \
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
	BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.cpp	 \
	BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.h	 \
	BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.cpp		 \
	BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.h		 \
	BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.cpp	 \
	BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.h	 \
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
        BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.cpp     \
        BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h       \
        BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.cpp     \
        BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.h       \
        BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.cpp        \
        BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.h          \
        BulletCollision/CollisionDispatch/btGhostObject.cpp                      \
        BulletCollision/CollisionDispatch/btGhostObject.h                        \
        BulletCollision/CollisionDispatch/btInternalEdgeUtility.cpp              \
        BulletCollision/CollisionDispatch/btInternalEdgeUtility.h                \
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
	BulletCollision/CollisionShapes/btConvexPolyhedron.cpp			 \
	BulletCollision/CollisionShapes/btConvexPolyhedron.h			 \
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
        BulletCollision/CollisionShapes/btBox2dShape.cpp                         \
        BulletCollision/CollisionShapes/btBox2dShape.h                           \
        BulletCollision/CollisionShapes/btConvex2dShape.cpp                      \
        BulletCollision/CollisionShapes/btConvex2dShape.h                        \
        BulletCollision/CollisionShapes/btConvexPointCloudShape.cpp              \
        BulletCollision/CollisionShapes/btConvexPointCloudShape.h                \
        BulletCollision/CollisionShapes/btMaterial.h                             \
        BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.cpp     \
        BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.h       \
        BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.cpp         \
        BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.h           \
        BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.cpp   \
        BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.h     \
        BulletCollision/CollisionShapes/btTriangleInfoMap.h                      \
        BulletCollision/Gimpact/btBoxCollision.h                                 \
        BulletCollision/Gimpact/btClipPolygon.h                                  \
        BulletCollision/Gimpact/btContactProcessing.cpp                          \
        BulletCollision/Gimpact/btContactProcessing.h                            \
        BulletCollision/Gimpact/btGImpactBvh.cpp                                 \
        BulletCollision/Gimpact/btGImpactBvh.h                                   \
        BulletCollision/Gimpact/btGImpactCollisionAlgorithm.cpp                  \
        BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h                    \
        BulletCollision/Gimpact/btGImpactMassUtil.h                              \
        BulletCollision/Gimpact/btGImpactQuantizedBvh.cpp                        \
        BulletCollision/Gimpact/btGImpactQuantizedBvh.h                          \
        BulletCollision/Gimpact/btGImpactShape.cpp                               \
        BulletCollision/Gimpact/btGImpactShape.h                                 \
        BulletCollision/Gimpact/btGenericPoolAllocator.cpp                       \
        BulletCollision/Gimpact/btGenericPoolAllocator.h                         \
        BulletCollision/Gimpact/btGeometryOperations.h                           \
        BulletCollision/Gimpact/btQuantization.h                                 \
        BulletCollision/Gimpact/btTriangleShapeEx.cpp                            \
        BulletCollision/Gimpact/btTriangleShapeEx.h                              \
        BulletCollision/Gimpact/gim_array.h                                      \
        BulletCollision/Gimpact/gim_basic_geometry_operations.h                  \
        BulletCollision/Gimpact/gim_bitset.h                                     \
        BulletCollision/Gimpact/gim_box_collision.h                              \
        BulletCollision/Gimpact/gim_box_set.cpp                                  \
        BulletCollision/Gimpact/gim_box_set.h                                    \
        BulletCollision/Gimpact/gim_clip_polygon.h                               \
        BulletCollision/Gimpact/gim_contact.cpp                                  \
        BulletCollision/Gimpact/gim_contact.h                                    \
        BulletCollision/Gimpact/gim_geom_types.h                                 \
        BulletCollision/Gimpact/gim_geometry.h                                   \
        BulletCollision/Gimpact/gim_hash_table.h                                 \
        BulletCollision/Gimpact/gim_linear_math.h                                \
        BulletCollision/Gimpact/gim_math.h                                       \
        BulletCollision/Gimpact/gim_memory.cpp                                   \
        BulletCollision/Gimpact/gim_memory.h                                     \
        BulletCollision/Gimpact/gim_radixsort.h                                  \
        BulletCollision/Gimpact/gim_tri_collision.cpp                            \
        BulletCollision/Gimpact/gim_tri_collision.h                              \
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
	BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.cpp     \
	BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.h       \
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
        BulletDynamics/Character/btCharacterControllerInterface.h                \
        BulletDynamics/Character/btKinematicCharacterController.cpp              \
        BulletDynamics/Character/btKinematicCharacterController.h                \
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
	BulletDynamics/ConstraintSolver/btPoint2PointConstraint.cpp		 \
	BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h		 \
	BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp	 \
	BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h	 \
	BulletDynamics/ConstraintSolver/btSliderConstraint.cpp			 \
	BulletDynamics/ConstraintSolver/btSliderConstraint.h			 \
	BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.cpp		 \
	BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.h		 \
	BulletDynamics/ConstraintSolver/btSolverBody.h				 \
	BulletDynamics/ConstraintSolver/btSolverConstraint.h			 \
	BulletDynamics/ConstraintSolver/btTypedConstraint.cpp			 \
	BulletDynamics/ConstraintSolver/btTypedConstraint.h			 \
        BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.cpp        \
        BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h          \
        BulletDynamics/ConstraintSolver/btHinge2Constraint.cpp                   \
        BulletDynamics/ConstraintSolver/btHinge2Constraint.h                     \
        BulletDynamics/ConstraintSolver/btUniversalConstraint.cpp                \
        BulletDynamics/ConstraintSolver/btUniversalConstraint.h                  \
        BulletDynamics/Dynamics/btActionInterface.h                              \
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
