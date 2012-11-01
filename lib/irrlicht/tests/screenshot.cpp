// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;



// Tests screenshots features
bool testShots(video::E_DRIVER_TYPE type)
{
	IrrlichtDevice *device = createDevice(type, core::dimension2d<u32>(160, 120), 32);
	if (!device)
		return true;

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager * smgr = device->getSceneManager();

	logTestString("Testing driver %ls\n", driver->getName());

	scene::IAnimatedMesh* mesh = smgr->getMesh("../media/sydney.md2");
	scene::IAnimatedMeshSceneNode* node;

	if (!mesh)
		return false;
	node = smgr->addAnimatedMeshSceneNode(mesh);
	if (!node)
		return false;

	stabilizeScreenBackground(driver);

	node->setPosition(core::vector3df(20, 0, 30));
	node->setMaterialFlag(video::EMF_LIGHTING, false);
	node->setMaterialTexture(0, driver->getTexture("../media/sydney.bmp"));
	node->setLoopMode(false);

	smgr->addCameraSceneNode();

	node->setMD2Animation(scene::EMAT_DEATH_FALLBACK);
	node->setCurrentFrame((f32)(node->getEndFrame()));
	node->setAnimationSpeed(0);

	device->run();
	driver->beginScene(true, true, video::SColor(255, 255, 255, 0));
	smgr->drawAll();
	driver->endScene();

	for (s32 i=0; i<video::ECF_UNKNOWN; ++i)
	{
		video::IImage* img = driver->createScreenShot((video::ECOLOR_FORMAT)i);
		logTestString("Color Format %d %ssupported\n", i, (img && img->getColorFormat() == i)?"":"un");

#if 0	// Enable for a quick check while testing.
		// If someone adds comparison images please use another scene without animation
		// and maybe a texture using the full color spectrum.
		if ( img )
		{
			irr::core::stringc screenshotFilename = "results/";
			screenshotFilename += shortDriverName(driver);
			screenshotFilename += "screenshot";
			screenshotFilename += core::stringc(i);
			screenshotFilename += ".png";
			driver->writeImageToFile(img, screenshotFilename.c_str());
		}
#endif

		if (img)
			img->drop();
	}
	device->closeDevice();
	device->run();
	device->drop();

	return true;
}

bool screenshot()
{
	bool result = true;
	TestWithAllHWDrivers(testShots);
	return result;
}
