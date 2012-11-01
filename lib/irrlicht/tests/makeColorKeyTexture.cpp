// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;

/** Test the behaviour of makeColorKeyTexture() using both 16 bit (software)
	and 32 bit (Burning) textures, with the new behaviour and the legacy
	behaviour. */
static bool doTestWith(E_DRIVER_TYPE driverType,
						bool zeroTexels)
{
	IrrlichtDevice *device = createDevice( driverType,
											dimension2d<u32>(160, 120), 32);
	if (!device)
		return false;

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager * smgr = device->getSceneManager();

	// Draw a cube background so that we can check that the keying is working.
	ISceneNode * cube = smgr->addCubeSceneNode(50.f, 0, -1, vector3df(0, 0, 60));
	cube->setMaterialTexture(0, driver->getTexture("../media/wall.bmp"));
	cube->setMaterialFlag(video::EMF_LIGHTING, false);

	ITexture * Texture = device->getVideoDriver()->getTexture("../media/portal2.bmp");

	device->getVideoDriver()->makeColorKeyTexture(Texture,
												  position2d<s32>(64,64),
												  zeroTexels);
	device->getVideoDriver()->makeColorKeyTexture(Texture,
												  position2d<s32>(64,64),
												  zeroTexels);
	(void)smgr->addCameraSceneNode();

	driver->beginScene(true, true, SColor(255,100,101,140));
	smgr->drawAll();

	driver->draw2DImage(Texture,
						position2di(40, 40),
						rect<s32>(0, 0, Texture->getSize().Width, Texture->getSize().Height),
						0,
						SColor(255,255,255,255),
						true);
	driver->endScene();

	char screenshotName[256];
	(void)snprintf(screenshotName, 256, "-makeColorKeyTexture-%s.png",
		zeroTexels? "old" : "new");

	bool result = takeScreenshotAndCompareAgainstReference(driver, screenshotName);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

bool makeColorKeyTexture(void)
{
	bool result = true;
	
	result &= doTestWith(EDT_BURNINGSVIDEO, false);
	result &= doTestWith(EDT_SOFTWARE, false);
	result &= doTestWith(EDT_BURNINGSVIDEO, true);
	result &= doTestWith(EDT_SOFTWARE, true);

	return result;
}
