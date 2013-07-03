// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

//! Tests IVideoDriver::drawPixel().
/** Expect to see two diagonal lines overlaying a wall texture cube.
	One line should run from green at the top left to red at the bottom right.
	The other should run from cyan 100% transparent at the bottom left to
	cyan 100% opaque at the top right. */
static bool lineRender(E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice( driverType, dimension2d<u32>(160, 120), 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager * smgr = device->getSceneManager();

	stabilizeScreenBackground(driver);

	logTestString("Testing driver %ls\n", driver->getName());

	// Draw a cube background so that we can check that the pixels' alpha is working.
	ISceneNode * cube = smgr->addCubeSceneNode(50.f, 0, -1, vector3df(0, 0, 60));
	cube->setMaterialTexture(0, driver->getTexture("../media/wall.bmp"));
	cube->setMaterialFlag(video::EMF_LIGHTING, false);
	(void)smgr->addCameraSceneNode();

	driver->beginScene(true, true, SColor(255,100,101,140));
	smgr->drawAll();

	// Test for benign handling of offscreen pixel values as well as onscreen ones.
	for(s32 x = -10; x < 170; ++x)
	{
		s32 y = 120 * x / 160;
		driver->drawPixel((u32)x, (u32)y, SColor(255, 255 * x / 640, 255 * (640 - x) / 640, 0));
		y = 120 - y;
		driver->drawPixel((u32)x, (u32)y, SColor(255 * x / 640, 0, 255, 255));
	}

	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-drawPixel.png", 98.81f);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

// this test draws alternating white and black borders with
// increasing thickness. Intended use of this test is to ensure
// the corect pixel alignment within the render window.
static bool pixelAccuracy(E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice( driverType, dimension2d<u32>(160, 120), 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	IVideoDriver* driver = device->getVideoDriver();

	stabilizeScreenBackground(driver);

	logTestString("Testing driver %ls\n", driver->getName());

	device->getSceneManager()->addCameraSceneNode();

	driver->beginScene(true, true, SColor(255,100,101,140));
	u32 start=0;
	for (u32 count=1; count<10; ++count)
	{
		for (u32 j=0; j<count; ++j)
		{
			for (u32 x=0; x<100-start; ++x)
			{
				driver->drawPixel(start+x, start, (count%2==1)?0xffffffff:0xff000000);
			}
			++start;
		}
	}
	start=0;
	for (u32 count=1; count<10; ++count)
	{
		for (u32 j=0; j<count; ++j)
		{
			for (u32 x=0; x<100-start; ++x)
			{
				driver->drawPixel(start, start+x, (count%2==1)?0xffffffff:0xff000000);
			}
			++start;
		}
	}
	for (u32 x=0; x<100; ++x)
	{
		driver->drawPixel(x, x, 0xffff0000);
	}
	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-pixelAccuracy.png");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

// this test draws lines of different lengths and compares
// them with pixel placement
// grey pixels denote start and end of the white drawn lines
// black pixels only make those grey points better visible
// yellow and magenta lines should start and end next toa black pixel,
// yellow one right to the last black pixel down, magenta below the last
// black pixel to the right
// white lines are always double drawn, lines back and forth.
static bool drawLine(E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice( driverType, dimension2d<u32>(160, 120), 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	IVideoDriver* driver = device->getVideoDriver();

	stabilizeScreenBackground(driver);

	logTestString("Testing driver %ls\n", driver->getName());

	device->getSceneManager()->addCameraSceneNode();

	driver->beginScene(true, true, SColor(255,100,101,140));
	// horizontal lines
	for (u32 i=0; i<20; ++i)
	{
		driver->draw2DLine(core::vector2di(10,10+3*i), core::vector2di(10+2*i,10+3*i));
		// mark start point
		driver->drawPixel(9,10+3*i+1, video::SColor(0xff000000));
		driver->drawPixel(10,10+3*i+1, video::SColor(0xff888888));
		driver->drawPixel(11,10+3*i+1, video::SColor(0xff000000));
		// mark end point
		driver->drawPixel(9+2*i,10+3*i+1, video::SColor(0xff000000));
		driver->drawPixel(10+2*i,10+3*i+1, video::SColor(0xff888888));
		driver->drawPixel(11+2*i,10+3*i+1, video::SColor(0xff000000));
		driver->draw2DLine(core::vector2di(10+2*i,10+3*i+2), core::vector2di(10,10+3*i+2));
	}
	// vertical lines
	for (u32 i=0; i<20; ++i)
	{
		driver->draw2DLine(core::vector2di(11+3*i,10), core::vector2di(11+3*i,10+2*i));
		// mark start point
		driver->drawPixel(11+3*i+1,9, video::SColor(0xff000000));
		driver->drawPixel(11+3*i+1,10, video::SColor(0xff888888));
		driver->drawPixel(11+3*i+1,11, video::SColor(0xff000000));
		// mark end point
		driver->drawPixel(11+3*i+1,9+2*i, video::SColor(0xff000000));
		driver->drawPixel(11+3*i+1,10+2*i, video::SColor(0xff888888));
		driver->drawPixel(11+3*i+1,11+2*i, video::SColor(0xff000000));
		driver->draw2DLine(core::vector2di(11+3*i+2,10+2*i), core::vector2di(11+3*i+2, 10));
	}
	// diagonal lines
	driver->draw2DLine(core::vector2di(14,14),core::vector2di(50,68), video::SColor(0xffffff00));
	driver->draw2DLine(core::vector2di(15,14),core::vector2di(69,50), video::SColor(0xffff00ff));
	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-drawLine.png");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

bool drawPixel(void)
{
	bool result = true;

	TestWithAllDrivers(lineRender);
	TestWithAllDrivers(pixelAccuracy);
	TestWithAllDrivers(drawLine);

	return result;
}
