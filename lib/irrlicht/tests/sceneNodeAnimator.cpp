// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;

/** Test functionality of the ISceneNodeAnimator implementations. */
bool sceneNodeAnimator(void)
{
	IrrlichtDevice * device = irr::createDevice(video::EDT_NULL, dimension2d<u32>(160, 120));
	assert_log(device);
	if(!device)
		return false;

	ISceneManager * smgr = device->getSceneManager();

	// Test the hasFinished() method.
	ISceneNodeAnimatorCollisionResponse* collisionResponseAnimator
		= smgr->createCollisionResponseAnimator(0, 0);

	ISceneNodeAnimator* deleteAnimator = smgr->createDeleteAnimator(1);

	ISceneNodeAnimator* flyCircleAnimator = smgr->createFlyCircleAnimator();

	ISceneNodeAnimator* flyStraightAnimator
		= smgr->createFlyStraightAnimator(vector3df(0, 0, 0), vector3df(0, 0, 0), 1, false);

	ISceneNodeAnimator* flyStraightAnimatorLooping
		= smgr->createFlyStraightAnimator(vector3df(0, 0, 0), vector3df(0, 0, 0), 1, true);

	ISceneNodeAnimator* rotationAnimator = smgr->createRotationAnimator(vector3df(0, 0, 0));

	array<vector3df> points;
	points.push_back(vector3df(0, 0, 0));
	points.push_back(vector3df(0, 0, 0));
	ISceneNodeAnimator* followSplineAnimator = smgr->createFollowSplineAnimator(0, points, 1000.f);

	array<video::ITexture*> textures;
	textures.push_back(0);
	textures.push_back(0);

	ISceneNodeAnimator* textureAnimator = smgr->createTextureAnimator(textures, 1, false);
	ISceneNodeAnimator* textureAnimatorLooping = smgr->createTextureAnimator(textures, 1, true);

	bool result = true;

	ISceneNode * deletedNode = smgr->addEmptySceneNode();
	deletedNode->addAnimator(deleteAnimator);

	ISceneNode * testNode = smgr->addEmptySceneNode();
	testNode->addAnimator(collisionResponseAnimator);
	testNode->addAnimator(deleteAnimator);
	testNode->addAnimator(flyCircleAnimator);
	testNode->addAnimator(flyStraightAnimator);
	testNode->addAnimator(flyStraightAnimatorLooping);
	testNode->addAnimator(rotationAnimator);
	testNode->addAnimator(followSplineAnimator);
	testNode->addAnimator(textureAnimator);
	testNode->addAnimator(textureAnimatorLooping);

	result &= !collisionResponseAnimator->hasFinished();
	result &= !deleteAnimator->hasFinished();
	result &= !flyCircleAnimator->hasFinished();
	result &= !flyStraightAnimator->hasFinished();
	result &= !flyStraightAnimatorLooping->hasFinished();
	result &= !rotationAnimator->hasFinished();
	result &= !followSplineAnimator->hasFinished();
	result &= !textureAnimator->hasFinished();
	result &= !textureAnimatorLooping->hasFinished();

	device->run();
	device->sleep(10);
	device->run();
	smgr->drawAll();

	// These animators don't have an endpoint.
	result &= !collisionResponseAnimator->hasFinished();
	result &= !flyCircleAnimator->hasFinished();
	result &= !rotationAnimator->hasFinished();
	result &= !followSplineAnimator->hasFinished();

	// These animators are looping and so can't finish.
	result &= !flyStraightAnimatorLooping->hasFinished();
	result &= !textureAnimatorLooping->hasFinished();

	// These have an endpoint and have reached it.
	result &= deleteAnimator->hasFinished();
	result &= flyStraightAnimator->hasFinished();
	result &= textureAnimator->hasFinished();

	collisionResponseAnimator->drop();
	deleteAnimator->drop();
	flyCircleAnimator->drop();
	flyStraightAnimator->drop();
	flyStraightAnimatorLooping->drop();
	rotationAnimator->drop();
	followSplineAnimator->drop();
	textureAnimator->drop();
	textureAnimatorLooping->drop();

	device->closeDevice();
	device->run();
	device->drop();

	if(!result)
	{
		logTestString("One or more animators has a bad hasFinished() state\n.");
		assert_log(false);
	}

	return result;
}


