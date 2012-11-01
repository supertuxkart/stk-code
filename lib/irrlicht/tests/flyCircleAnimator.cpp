// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;

/** Tests the offset capability of the fly circle animator */
bool flyCircleAnimator(void)
{
	IrrlichtDevice *device = createDevice(video::EDT_BURNINGSVIDEO,
										core::dimension2du(160,120), 32);
	if (!device)
		return false;

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager* smgr = device->getSceneManager();

	const f32 offsetDegrees[] = { 0.f, 45.f, 135.f, 270.f };

	for(u32 i = 0; i < sizeof(offsetDegrees) / sizeof(offsetDegrees[0]); ++i)
	{
		IBillboardSceneNode * node = smgr->addBillboardSceneNode();
		// Have the animator rotate around the Z axis plane, rather than the default Y axis
		ISceneNodeAnimator * animator = smgr->createFlyCircleAnimator(
				vector3df(0, 0, 0), 30.f, 0.001f,
				vector3df(0, 0, 1), (offsetDegrees[i] / 360.f));
		if(!node || !animator)
			return false;

		node->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR );
		node->setMaterialTexture(0, driver->getTexture("../media/particle.bmp"));
		node->setMaterialFlag(video::EMF_LIGHTING, false);

		node->addAnimator(animator);
		animator->drop();
	}

	(void)smgr->addCameraSceneNode(0, vector3df(0, 0, -50), vector3df(0, 0, 0));

	bool result = false;

	// Don't do device->run() since I need the time to remain at 0.
	if (driver->beginScene(true, true, video::SColor(0, 80, 80, 80)))
	{
		smgr->drawAll();
		driver->endScene();
		result = takeScreenshotAndCompareAgainstReference(driver, "-flyCircleAnimator.png", 100);
	}

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

