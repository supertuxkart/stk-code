// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

//! Tests view ports and text rendering.
/** The result should be as follows: We have three times the same image, a billboard with a cube and the text in front.
They are replicated three times, one centered in the screen without viewport, one in the upper left (qurter screen) and
one on the right (half screen). The latter is stretched due to the changed aspect ratio.
The two viewport scenes get the texture drawn over the center as well, using draw2dimage. This should mark the proper
image position with the top left corner of the texture.
Finally, each scene has a checkbox drawn at the left side, vertically centered. This will show whether GUI elements adhere
to viewports as well. */
static bool viewPortText(E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice( driverType, dimension2d<u32>(160, 120), 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager * smgr = device->getSceneManager();
	IGUIEnvironment* env = smgr->getGUIEnvironment();

	stabilizeScreenBackground(driver);

	env->addCheckBox(true, core::recti(10,60,28,82));

	logTestString("Testing driver %ls\n", driver->getName());

	IBillboardSceneNode * bnode = smgr->addBillboardSceneNode(0,dimension2d<f32>(32,32),core::vector3df(0,0,10));
	bnode->setMaterialFlag(video::EMF_LIGHTING, false);
	bnode->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	bnode->setMaterialTexture(0, driver->getTexture("../media/fire.bmp"));

	smgr->addTextSceneNode(device->getGUIEnvironment()->getBuiltInFont(), L"TEST", video::SColor(255,255,255,255), 0);
	smgr->addCubeSceneNode();
	smgr->addCameraSceneNode(0, vector3df(0,30,-40), vector3df(0,5,0));

	driver->beginScene(true, true, SColor(255,100,101,140));
	smgr->drawAll();
	env->drawAll();
	driver->setViewPort(rect<s32>(0,0,160/2,120/2));
	smgr->drawAll();
	env->drawAll();
	driver->draw2DImage(driver->getTexture("../media/fire.bmp"), core::vector2di(160/2,120/2));
	driver->setViewPort(rect<s32>(160/2,0,160,120));
	smgr->drawAll();
	env->drawAll();
	driver->draw2DImage(driver->getTexture("../media/fire.bmp"), core::vector2di(160/2,120/2));
	driver->endScene(); 

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-viewPortText.png", 98.71f);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}


bool viewPort(void)
{
	bool result = true;

	// TODO: software driver and burnings don't use view port for
	// 2d rendering, so result is pretty wrong.
	TestWithAllDrivers(viewPortText);

	return result;
}

