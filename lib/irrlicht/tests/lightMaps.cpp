// Copyright (C) 2008-2012 Christian Stehno, Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

//! Tests lightmaps under all drivers that support them
static bool runTestWithDriver(E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice( driverType, dimension2d<u32>(160, 120), 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager * smgr = device->getSceneManager();

	logTestString("Testing driver %ls\n", driver->getName());
	if (driver->getDriverAttributes().getAttributeAsInt("MaxTextures")<2)
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	stabilizeScreenBackground(driver);

	bool result = true;
	bool added = device->getFileSystem()->addFileArchive("../media/map-20kdm2.pk3");
	assert_log(added);

	if(added)
	{
		ISceneNode * node = smgr->addOctreeSceneNode(smgr->getMesh("20kdm2.bsp")->getMesh(0), 0, -1, 1024);
		assert_log(node);

		if (node)
		{
			node->setMaterialFlag(EMF_LIGHTING, false);
			node->setPosition(core::vector3df(-1300,-820,-1249));
			node->setScale(core::vector3df(1, 5, 1));

			(void)smgr->addCameraSceneNode(0, core::vector3df(0,0,0), core::vector3df(40,100,30));

			driver->beginScene(true, true, video::SColor(255,255,255,0));
			smgr->drawAll();
			driver->endScene();

			result = takeScreenshotAndCompareAgainstReference(driver, "-lightmaps.png", 96);
		}
	}

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}


bool lightMaps(void)
{
	bool result = true;
	TestWithAllDrivers(runTestWithDriver);
	return result;
}

