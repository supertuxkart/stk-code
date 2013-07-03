// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;

static bool testGetCollisionResultPosition(IrrlichtDevice * device,
					   ISceneManager * smgr,
					   ISceneCollisionManager * collMgr)
{
	IMeshSceneNode * cubeNode = smgr->addCubeSceneNode(10.f);
	ITriangleSelector * cubeSelector = smgr->createTriangleSelectorFromBoundingBox(cubeNode);

	triangle3df triOut;
	vector3df hitPosition;
	bool falling;
	ISceneNode* hitNode;

	vector3df resultPosition =
		collMgr->getCollisionResultPosition(cubeSelector,
						vector3df(0, 50, 0),
						vector3df(10, 20, 10),
						vector3df(0, -100, 0),
						triOut,
						hitPosition,
						falling,
						hitNode);

	bool result = true;

	if(hitNode != cubeNode)
	{
		logTestString("Unexpected collision node\n");
		result = false;
	}

	if(!equals(resultPosition.Y, 25.f, 0.01f))
	{
		logTestString("Unexpected collision response position\n");
		result = false;
	}

	if(!equals(hitPosition.Y, 5.f, 0.01f))
	{
		logTestString("Unexpected collision position\n");
		result = false;
	}

	resultPosition =
		collMgr->getCollisionResultPosition(cubeSelector,
						vector3df(-20, 0, 0),
						vector3df(10, 20, 10),
						vector3df(100, 0, 0),
						triOut,
						hitPosition,
						falling,
						hitNode);

	if(hitNode != cubeNode)
	{
		logTestString("Unexpected collision node\n");
		result = false;
	}

	if(!equals(resultPosition.X, -15.f, 0.01f))
	{
		logTestString("Unexpected collision response position\n");
		result = false;
	}

	if(!equals(hitPosition.X, -5.f, 0.01f))
	{
		logTestString("Unexpected collision position\n");
		result = false;
	}

	assert_log(result);

	cubeSelector->drop();
	smgr->clear();

	return result;
}


// Test that getCollisionPoint() actually uses the closest point, not the closest triangle.
static bool getCollisionPoint_ignoreTriangleVertices(IrrlichtDevice * device,
						ISceneManager * smgr,
						ISceneCollisionManager * collMgr)
{
	// Create a cube with a Z face at 5, but corners close to 0
	ISceneNode * farSmallCube = smgr->addCubeSceneNode(10, 0, -1, vector3df(0, 0, 10));

	// Create a cube with a Z face at 0, but corners far from 0
	ISceneNode * nearBigCube = smgr->addCubeSceneNode(100, 0, -1, vector3df(0, 0, 50));

	IMetaTriangleSelector * meta = smgr->createMetaTriangleSelector();

	ITriangleSelector * selector = smgr->createTriangleSelectorFromBoundingBox(farSmallCube);
	meta->addTriangleSelector(selector);
	selector->drop();

	// We should expect a hit on this cube
	selector = smgr->createTriangleSelectorFromBoundingBox(nearBigCube);
	meta->addTriangleSelector(selector);
	selector->drop();

	line3df ray(0, 0, -5, 0, 0, 100);
	vector3df hitPosition;
	triangle3df hitTriangle;
	ISceneNode* hitNode;

	bool collision = collMgr->getCollisionPoint(ray, meta, hitPosition, hitTriangle, hitNode);

	meta->drop();

	if(hitNode != nearBigCube)
	{
		logTestString("getCollisionPoint_ignoreTriangleVertices: hit the wrong node.\n");
		return false;
	}

	if(!collision)
	{
		logTestString("getCollisionPoint_ignoreTriangleVertices: didn't get a hit.\n");
		return false;
	}

	if(hitPosition != vector3df(0, 0, 0))
	{
		logTestString("getCollisionPoint_ignoreTriangleVertices: unexpected hit position %f %f %f.\n",
			hitPosition.X, hitPosition.Y, hitPosition.Z );
		return false;
	}

	smgr->clear();

	return true;
}


static bool testGetSceneNodeFromScreenCoordinatesBB(IrrlichtDevice * device,
						ISceneManager * smgr,
						ISceneCollisionManager * collMgr)
{
	// Create 3 nodes. The nearest node actually contains the camera.
	IMeshSceneNode * cubeNode1 = smgr->addCubeSceneNode(10.f, 0, -1, vector3df(0, 0, 4));
	IMeshSceneNode * cubeNode2 = smgr->addCubeSceneNode(10.f, 0, -1, vector3df(0, 0, 30));
	cubeNode2->setRotation(vector3df(90.f, 90.f, 90.f)); // Just check that rotation doesn't stop us hitting it.
	IMeshSceneNode * cubeNode3 = smgr->addCubeSceneNode(10.f, 0, -1, vector3df(0, 0, 40));
	cubeNode3->setRotation(vector3df(180.f, 180.f, 180.f)); // Just check that rotation doesn't stop us hitting it.

	ICameraSceneNode * camera = smgr->addCameraSceneNode();
	device->run();
	smgr->drawAll(); // Get the camera in a good state

	ISceneNode * hitNode = collMgr->getSceneNodeFromScreenCoordinatesBB(position2d<s32>(80, 60));

	// Expect the first node to be hit, since we're starting the check from inside it.
	bool result = true;
	if(hitNode != cubeNode1)
	{
		logTestString("Unexpected node hit. Expected cubeNode1.\n");
		result = false;
	}

	// Now make cubeNode1 invisible and check that cubeNode2 is hit.
	cubeNode1->setVisible(false);
	hitNode = collMgr->getSceneNodeFromScreenCoordinatesBB(position2d<s32>(80, 60));
	if(hitNode != cubeNode2)
	{
		logTestString("Unexpected node hit. Expected cubeNode2.\n");
		result = false;
	}

	// Make cubeNode1 the parent of cubeNode2.
	cubeNode2->setParent(cubeNode1);

	// Check visibility.
	bool visible = cubeNode2->isVisible();
	if(!visible)
	{
		logTestString("cubeNode2 should think that it (in isolation) is visible.\n");
		result = false;
	}

	visible = cubeNode2->isTrulyVisible();
	if(visible)
	{
		logTestString("cubeNode2 should know that it (recursively) is invisible.\n");
		result = false;
	}

	// cubeNode2 should now be an invalid target as well, and so the final cube node should be hit.
	hitNode = collMgr->getSceneNodeFromScreenCoordinatesBB(position2d<s32>(80, 60));
	if(hitNode != cubeNode3)
	{
		logTestString("Unexpected node hit. Expected cubeNode3.\n");
		result = false;
	}


	// Make cubeNode3 invisible and check that the camera node is hit (since it has a valid bounding box).
	cubeNode3->setVisible(false);
	hitNode = collMgr->getSceneNodeFromScreenCoordinatesBB(position2d<s32>(80, 60));
	if(hitNode != camera)
	{
		logTestString("Unexpected node hit. Expected the camera node.\n");
		result = false;
	}

	// Now verify bitmasking
	camera->setID(0xAAAAAAAA); // == 101010101010101010101010101010
	hitNode = collMgr->getSceneNodeFromScreenCoordinatesBB(position2d<s32>(80, 60), 0x02);
	if(hitNode != camera)
	{
		logTestString("Unexpected node hit. Expected the camera node.\n");
		result = false;
	}

	// Test the 01010101010101010101010101010101 bitmask (0x55555555)
	hitNode = collMgr->getSceneNodeFromScreenCoordinatesBB(position2d<s32>(80, 60), 0x55555555);
	if(hitNode != 0)
	{
		logTestString("A node was hit when none was expected.\n");
		result = false;
	}
	assert_log(result);

	smgr->clear();

	return result;
}


static bool getScaledPickedNodeBB(IrrlichtDevice * device,
				ISceneManager * smgr,
				ISceneCollisionManager * collMgr)
{
	ISceneNode* farTarget = smgr->addCubeSceneNode(1.f);
	farTarget->setScale(vector3df(100.f, 100.f, 10.f));
	farTarget->setPosition(vector3df(0.f, 0.f, 500.f));
	farTarget->updateAbsolutePosition();

	// Create a node that's slightly further away than the closest node,
	// but thinner.  Its furthest corner is closer, but the collision
	// position is further, so it should not be selected.
	ISceneNode* middleTarget = smgr->addCubeSceneNode(10.f);
	middleTarget->setPosition(vector3df(0.f, 0.f, 101.f));
	middleTarget->setScale(vector3df(1.f, 1.f, 0.5f));
	middleTarget->updateAbsolutePosition();

	ISceneNode* nearTarget = smgr->addCubeSceneNode(10.f);
	nearTarget->setPosition(vector3df(0.f, 0.f, 100.f));
	nearTarget->updateAbsolutePosition();
	// We'll rotate this node 90 degrees to show that we can hit its side.
	nearTarget->setRotation(vector3df(0.f, 90.f, 0.f));

	line3df ray(0.f, 0.f, 0.f, 0.f, 0.f, 500.f);

	const ISceneNode * const hit = collMgr->getSceneNodeFromRayBB(ray);

	bool result = (hit == nearTarget);

	if(hit == 0)
		logTestString("getSceneNodeFromRayBB() didn't hit anything.\n");
	else if(hit == farTarget)
		logTestString("getSceneNodeFromRayBB() hit the far (scaled) target.\n");
	else if(hit == middleTarget)
		logTestString("getSceneNodeFromRayBB() hit the middle (scaled) target.\n");

	assert_log(result);

	smgr->clear();

	return result;
}


// box intersection according to Kay et al., code from gamedev.net
static bool IntersectBox(const core::vector3df& origin, const core::vector3df& dir, const core::aabbox3df& box)
{
	core::vector3df minDist = (box.MinEdge - origin)/dir;
	core::vector3df maxDist = (box.MaxEdge - origin)/dir;

	core::vector3df realMin(core::min_(minDist.X, maxDist.X),core::min_(minDist.Y, maxDist.Y),core::min_(minDist.Z, maxDist.Z));
	core::vector3df realMax(core::max_(minDist.X, maxDist.X),core::max_(minDist.Y, maxDist.Y),core::max_(minDist.Z, maxDist.Z));

	f32 minmax = core::min_(realMax.X, realMax.Y, realMax.Z);
	// nearest distance to intersection
	f32 maxmin = core::max_(realMin.X, realMin.Y, realMin.Z);

	return (maxmin >=0 && minmax >= maxmin);
}

static bool checkBBoxIntersection(IrrlichtDevice * device,
				ISceneManager * smgr)
{
	video::IVideoDriver* driver = device->getVideoDriver();

	// add camera
	scene::ICameraSceneNode* camera = smgr->addCameraSceneNode();
	camera->setPosition(core::vector3df(30, 30, 30));
	camera->setTarget(core::vector3df(8.f, 8.f, 8.f));
	camera->setID(0);

	// add a cube to pick
	scene::ISceneNode* cube = smgr->addCubeSceneNode(30, 0, -1, core::vector3df(0,0,0),core::vector3df(30,40,50));

	bool result=true;
	for (u32 round=0; round<2; ++round)
	{
		driver->beginScene(true, true, video::SColor(100, 50, 50, 100));
		smgr->drawAll();
		driver->endScene();

		core::matrix4 invMat = cube->getAbsoluteTransformation();
		invMat.makeInverse();

		s32 hits=0;
		u32 start = device->getTimer()->getRealTime();
		for (u32 i=10; i<150; ++i)
		{
			for (u32 j=10; j<110; ++j)
			{
				const core::position2di pos(i, j);

				// get the line used for picking
				core::line3df ray = smgr->getSceneCollisionManager()->getRayFromScreenCoordinates(pos, camera);

				invMat.transformVect(ray.start);
				invMat.transformVect(ray.end);

				hits += (cube->getBoundingBox().intersectsWithLine(ray)?1:0);
			}
		}
		u32 duration = device->getTimer()->getRealTime()-start;
		logTestString("bbox intersection checks %d hits (of 14000).\n", hits);
		hits = -hits;

		start = device->getTimer()->getRealTime();
		for (u32 i=10; i<150; ++i)
		{
			for (u32 j=10; j<110; ++j)
			{
				const core::position2di pos(i, j);

				// get the line used for picking
				core::line3df ray = smgr->getSceneCollisionManager()->getRayFromScreenCoordinates(pos, camera);

				invMat.transformVect(ray.start);
				invMat.transformVect(ray.end);

				hits += (IntersectBox(ray.start, (ray.end-ray.start).normalize(), cube->getBoundingBox())?1:0);
			}
		}
		u32 duration2 = device->getTimer()->getRealTime()-start;
		logTestString("bbox intersection resulted in %d misses at a speed of %d (old) compared to %d (new).\n", abs(hits), duration, duration2);
		if (duration>(duration2*1.2f))
			logTestString("Consider replacement of bbox intersection test.\n");

		result &= (hits==0);
		assert_log(result);
		// second round without any hits, so check opposite direction
		camera->setTarget(core::vector3df(80.f, 80.f, 80.f));
	}

	ISceneNode* node = smgr->addSphereSceneNode(5.f, 16, 0, -1, core::vector3df(0, 0, 1), core::vector3df(), core::vector3df(0.3f, 0.3f, 0.3f));
	cube->remove();
	cube = smgr->addCubeSceneNode(10.f, 0, -1, core::vector3df(0, 6.5f, 1), core::vector3df(), core::vector3df(10, 0.1f, 1.f));
	camera->setPosition(core::vector3df(0, 0, 10));
	camera->setTarget(core::vector3df());

	u32 count=0;
	for (u32 i=0; i<30; ++i)
	{
		driver->beginScene(true, true, video::SColor(100, 50, 50, 100));
		smgr->drawAll();
		driver->endScene();

		count += node->getTransformedBoundingBox().intersectsWithBox(cube->getTransformedBoundingBox())?1:0;
		node->setPosition(node->getPosition()+core::vector3df(.5f,.5f,0));
		if (i==8 && count != 0)
			result=false;
		if (i==17 && count != 9)
			result=false;
	}
	if (count != 9)
		result=false;

	smgr->clear();

	return result;
}


static bool compareGetSceneNodeFromRayBBWithBBIntersectsWithLine(IrrlichtDevice * device,
				ISceneManager * smgr,
				ISceneCollisionManager * collMgr)
{
	video::IVideoDriver* driver = device->getVideoDriver();

	// add camera
	scene::ICameraSceneNode* camera = smgr->addCameraSceneNodeFPS();
	camera->setPosition(core::vector3df(30, 30, 30));
	camera->setTarget(core::vector3df(-8.f, 8.f, -8.f));
	camera->setID(0);

	// add a dynamic light (this causes weirdness)
	smgr->addLightSceneNode(0, core::vector3df(4, 4, 4), video::SColorf(.2f, .3f, .2f));

	// add a cube to pick
	scene::ISceneNode* cube = smgr->addCubeSceneNode(15);

	driver->beginScene(true, true, video::SColor(100, 50, 50, 100));
	smgr->drawAll();
	driver->endScene();

	core::matrix4 invMat = cube->getAbsoluteTransformation();
	invMat.makeInverse();

	bool result = true;
	for (u32 i=76; i<82; ++i)
	{
		for (u32 j=56; j<64; ++j)
		{
			const core::position2di pos(i, j);

			// get the line used for picking
			core::line3df ray = smgr->getSceneCollisionManager()->getRayFromScreenCoordinates(pos, camera);

			// find a selected node
			scene::ISceneNode* pick = smgr->getSceneCollisionManager()->getSceneNodeFromRayBB(ray, 1);

			invMat.transformVect(ray.start);
			invMat.transformVect(ray.end);

			const int a_hit = (pick == cube);
			const int b_hit = cube->getBoundingBox().intersectsWithLine(ray);
			result = (a_hit==b_hit);
		}
	}

	assert_log(result);

	smgr->clear();

	return result;
}


/** Test functionality of the sceneCollisionManager */
bool sceneCollisionManager(void)
{
	IrrlichtDevice * device = irr::createDevice(video::EDT_NULL, dimension2d<u32>(160, 120));
	assert_log(device);
	if(!device)
		return false;

	ISceneManager * smgr = device->getSceneManager();
	ISceneCollisionManager * collMgr = smgr->getSceneCollisionManager();

	bool result = testGetCollisionResultPosition(device, smgr, collMgr);

	smgr->clear();

	result &= testGetSceneNodeFromScreenCoordinatesBB(device, smgr, collMgr);

	result &= getScaledPickedNodeBB(device, smgr, collMgr);

	result &= getCollisionPoint_ignoreTriangleVertices(device, smgr, collMgr);

	result &= checkBBoxIntersection(device, smgr);

	result &= compareGetSceneNodeFromRayBBWithBBIntersectsWithLine(device, smgr, collMgr);

	device->closeDevice();
	device->run();
	device->drop();
	return result;
}
