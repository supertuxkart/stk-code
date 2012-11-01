// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace scene;
using namespace video;

/** Tests the Burning Video driver */
bool burningsVideo(void)
{
    IrrlichtDevice *device = createDevice(video::EDT_BURNINGSVIDEO,
										core::dimension2du(160,120), 32);
    if (!device)
        return false;

    IVideoDriver* driver = device->getVideoDriver();
    ISceneManager* smgr = device->getSceneManager();

    smgr->addCubeSceneNode(10.f, 0, -1, core::vector3df(0.f, 0.f, 20.f));
    smgr->addCameraSceneNode();
    // Test that ambient lighting works when there are no other lights in the scene
    smgr->setAmbientLight(video::SColorf(.7f, .1f, .1f, 1.f));

    bool result = false;
    device->run();
	if (driver->beginScene(true, true, video::SColor(0, 80, 80, 80)))
	{
		smgr->drawAll();
		driver->endScene();
		result = takeScreenshotAndCompareAgainstReference(driver, "-ambient-lighting.png", 100);
	}

	device->closeDevice();
	device->run();
    device->drop();

    return result;
}
