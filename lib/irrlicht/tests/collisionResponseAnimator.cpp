// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;

static bool expectedCollisionCallbackPositions = true;

class CMyCollisionCallback : public ICollisionCallback
{
public:
	bool onCollision(const ISceneNodeAnimatorCollisionResponse& animator)
	{
		const vector3df & collisionPoint = animator.getCollisionPoint();

		logTestString("Collision callback at %f %f %f\n",
			collisionPoint.X, collisionPoint.Y, collisionPoint.Z);

		if(collisionPoint != ExpectedCollisionPoint)
		{
			logTestString("*** Error: collision point, expected %f %f %f\n",
				ExpectedCollisionPoint.X, ExpectedCollisionPoint.Y, ExpectedCollisionPoint.Z);
			expectedCollisionCallbackPositions = false;
			assert_log(false);
		}

		const vector3df & nodePosition = animator.getCollisionResultPosition();
		if(nodePosition != ExpectedNodePosition)
		{
			logTestString("*** Error: result position, expected %f %f %f\n",
				ExpectedNodePosition.X, ExpectedNodePosition.Y, ExpectedNodePosition.Z);
			expectedCollisionCallbackPositions = false;
			assert_log(false);
		}

		if(animator.getTargetNode() != ExpectedTarget)
		{
			logTestString("*** Error: wrong node\n");
			expectedCollisionCallbackPositions = false;
			assert_log(false);
		}

		return ConsumeNextCollision;
	}

	void setNextExpectedCollision(ISceneNode* target,
					const vector3df& expectedPoint,
					const vector3df& expectedPosition,
					bool consume)
	{
		ExpectedTarget = target;
		ExpectedCollisionPoint = expectedPoint;
		ExpectedNodePosition = expectedPosition;
		ConsumeNextCollision = consume;
	}

private:

	ISceneNode * ExpectedTarget;
	vector3df ExpectedCollisionPoint;
	vector3df ExpectedNodePosition;
	bool ConsumeNextCollision;

};

/** Test that collision response animator will reset itself when removed from a
	scene node, so that the scene node can then be moved without the animator
	jumping it back again. */
bool collisionResponseAnimator(void)
{
	IrrlichtDevice * device = irr::createDevice(video::EDT_NULL);
	assert_log(device);
	if(!device)
		return false;

	ISceneManager * smgr = device->getSceneManager();

	// Create 2 nodes to the left of a "wall"
	ISceneNode * testNode1 = smgr->addEmptySceneNode();
	ISceneNode * testNode2 = smgr->addEmptySceneNode();
	testNode1->setPosition(vector3df(-50, 0,0));
	testNode2->setPosition(vector3df(-50, 0,0));

	// Create a "wall" node, and collision response animators for each test node.
	IMeshSceneNode * wallNode = smgr->addCubeSceneNode(10.f);

	ITriangleSelector * wallSelector = smgr->createTriangleSelectorFromBoundingBox(wallNode);
	ISceneNodeAnimatorCollisionResponse * collisionAnimator1 =
		smgr->createCollisionResponseAnimator(wallSelector,
												testNode1,
												vector3df(10,10,10),
												vector3df(0, 0, 0));
	testNode1->addAnimator(collisionAnimator1);

	CMyCollisionCallback collisionCallback;
	collisionAnimator1->setCollisionCallback(&collisionCallback);

	collisionAnimator1->drop();
	collisionAnimator1 = 0;

	ISceneNodeAnimatorCollisionResponse * collisionAnimator2 =
		smgr->createCollisionResponseAnimator(wallSelector,
												testNode2,
												vector3df(10,10,10),
												vector3df(0, 0, 0));
	testNode2->addAnimator(collisionAnimator2);
	collisionAnimator2->setCollisionCallback(&collisionCallback);

	wallSelector->drop();
	// Don't drop() collisionAnimator2 since we're going to use it.

	// Get the system in a good state
	device->run();
	smgr->drawAll();

	// Try to move both nodes to the right of the wall.
	// This one should be stopped by its animator.
	testNode1->setPosition(vector3df(50, 0,0));
	collisionCallback.setNextExpectedCollision(testNode1,
												vector3df(-5.005f, 0, 0),
												vector3df(-15.005f, 0, 0),
												false);

	// Whereas this one, by forcing the animator to update its target node, should be
	// able to pass through the wall. (In <=1.6 it was stopped by the wall even if
	// the animator was removed and later re-added);
	testNode2->setPosition(vector3df(50, 0,0));
	collisionAnimator2->setTargetNode(testNode2);
	collisionAnimator2->drop(); // We're done using this now.

	device->run();
	smgr->drawAll();

	bool result = true;

	if(testNode1->getAbsolutePosition().X > -15.f)
	{
		logTestString("collisionResponseAnimator test node 1 wasn't stopped from moving.\n");
		assert_log(false);
		result = false;
	}

	if(testNode2->getAbsolutePosition().X < 50.f)
	{
		logTestString("collisionResponseAnimator test node 2 was stopped from moving.\n");
		assert_log(false);
		result = false;
	}

	// Now try to move the second node back through the wall again. Now it should be
	// stopped by the wall.
	testNode2->setPosition(vector3df(-50, 0, 0));

	// We'll consume this collision, so the node will actually move all the way through.
	collisionCallback.setNextExpectedCollision(testNode2,
												vector3df(5.005f, 0, 0),
												vector3df(15.005f, 0, 0),
												true);

	device->run();
	smgr->drawAll();

	if(testNode2->getAbsolutePosition().X != -50.f)
	{
		logTestString("collisionResponseAnimator test node 2 was stopped from moving.\n");
		assert_log(false);
		result = false;
	}

	// Now we'll try to move it back to the right and allow it to be stopped.
	collisionCallback.setNextExpectedCollision(testNode2,
												vector3df(-5.005f, 0, 0),
												vector3df(-15.005f, 0, 0),
												false);
	testNode2->setPosition(vector3df(50, 0, 0));

	device->run();
	smgr->drawAll();

	if(testNode2->getAbsolutePosition().X > -15.f)
	{
		logTestString("collisionResponseAnimator test node 2 moved too far.\n");
		assert_log(false);
		result = false;
	}

	device->closeDevice();
	device->run();
	device->drop();

	result &= expectedCollisionCallbackPositions;
	return result;
}

