// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;

namespace
{
// Test billboard sizing
bool billboardSize(void)
{
	// Use EDT_BURNINGSVIDEO since it is not dependent on (e.g.) OpenGL driver versions.
	IrrlichtDevice *device = createDevice(video::EDT_BURNINGSVIDEO, core::dimension2d<u32>(160, 120), 32);
	assert_log(device);
	if (!device)
		return false;

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager * smgr = device->getSceneManager();

	smgr->addCameraSceneNode();
	scene::IBillboardSceneNode* bill = smgr->addBillboardSceneNode();
	bill->setPosition(core::vector3df(0,0,50));
	bill = smgr->addBillboardSceneNode();
	bill->setPosition(core::vector3df(-30,0,50));
	bill->getMaterial(0).Lighting=false;
	bill = smgr->addBillboardSceneNode();
	bill->setPosition(core::vector3df(30,0,50));
	bill->getMaterial(0).Lighting=false;
	bill->getMaterial(0).Wireframe=true;
	bill = smgr->addBillboardSceneNode();
	bill->setPosition(core::vector3df(30,0,50));
	bill->setSize(2,2,2);

	bill = smgr->addBillboardSceneNode();
	bill->setSize(10,20,2);
	bill->setPosition(core::vector3df(0,30,50));
	bill = smgr->addBillboardSceneNode();
	bill->setSize(10,2,20);
	bill->setPosition(core::vector3df(-30,30,50));
	bill->getMaterial(0).Lighting=false;
	bill = smgr->addBillboardSceneNode();
	bill->setSize(10,2,20);
	bill->setPosition(core::vector3df(30,30,50));
	bill->getMaterial(0).Lighting=false;
	bill->getMaterial(0).Wireframe=true;
	bill = smgr->addBillboardSceneNode();
	bill->setPosition(core::vector3df(30,30,50));
	bill->setSize(2,2,2);

	video::ITexture* tex = driver->getTexture("../media/fireball.bmp");
	bill = smgr->addBillboardSceneNode();
	bill->getMaterial(0).Lighting=false;
	bill->getMaterial(0).TextureLayer[0].AnisotropicFilter=true;
	bill->getMaterial(0).setTexture(0, tex);
	bill->setSize(10,20,2);
	bill->setPosition(core::vector3df(0,-30,50));
	bill = smgr->addBillboardSceneNode();
	bill->setSize(10,2,20);
	bill->setPosition(core::vector3df(-30,-30,50));
	bill->getMaterial(0).TextureLayer[0].AnisotropicFilter=true;
	bill->getMaterial(0).setTexture(0, tex);
	bill->getMaterial(0).Lighting=false;
	bill = smgr->addBillboardSceneNode();
	bill->setSize(10,2,20);
	bill->setPosition(core::vector3df(30,-30,50));
	bill->getMaterial(0).TextureLayer[0].AnisotropicFilter=true;
	bill->getMaterial(0).setTexture(0, tex);
	bill->getMaterial(0).Lighting=false;
	bill->getMaterial(0).Wireframe=true;
	bill = smgr->addBillboardSceneNode();
	bill->setPosition(core::vector3df(30,-30,50));
	bill->setSize(2,2,2);

	bill = smgr->addBillboardSceneNode();
	bill->getMaterial(0).Lighting=false;
	bill->getMaterial(0).setTexture(0, tex);
	bill->setSize(10,20,14);
	bill->setPosition(core::vector3df(0,-15,50));
	bill = smgr->addBillboardSceneNode();
	bill->setSize(10,14,20);
	bill->setPosition(core::vector3df(-30,-15,50));
	bill->getMaterial(0).setTexture(0, tex);
	bill->getMaterial(0).Lighting=false;
	bill = smgr->addBillboardSceneNode();
	bill->setSize(10,14,20);
	bill->setPosition(core::vector3df(30,-15,50));
	bill->getMaterial(0).setTexture(0, tex);
	bill->getMaterial(0).Lighting=false;
	bill->getMaterial(0).Wireframe=true;
	bill = smgr->addBillboardSceneNode();
	bill->setPosition(core::vector3df(30,-15,50));
	bill->setSize(2,2,2);

	bool result = false;

	device->run();
	driver->beginScene(true, true, video::SColor(255, 60, 60, 60));
	smgr->drawAll();
	driver->endScene();

	result = takeScreenshotAndCompareAgainstReference(driver, "-billboard.png");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

// Test billboard orientation
// Should generate a properly readable (i.e. not mirrored or flipped)
// font file display.
bool billboardOrientation(void)
{
	// Use EDT_BURNINGSVIDEO since it is not dependent on (e.g.) OpenGL driver versions.
	IrrlichtDevice *device = createDevice(video::EDT_BURNINGSVIDEO, core::dimension2d<u32>(160, 120), 32);
	assert_log(device);
	if (!device)
		return false;

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager * smgr = device->getSceneManager();

	smgr->addCameraSceneNode();
	scene::IBillboardSceneNode* bill = smgr->addBillboardSceneNode(0, core::dimension2df(40,40));
	bill->setPosition(core::vector3df(0,-15,10));
	bill->getMaterial(0).Lighting=false;
	bill->getMaterial(0).setTexture(0, driver->getTexture("../media/fontcourier.bmp"));

	bool result = false;

	device->run();
	driver->beginScene(true, true, video::SColor(255, 60, 60, 60));
	smgr->drawAll();
	driver->endScene();

	result = takeScreenshotAndCompareAgainstReference(driver, "-billboardOrientation.png");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

} // end anonymous namespace

// Test billboards
bool billboards(void)
{
	bool result = billboardSize();
	result &= billboardOrientation();
	return result;
}
