// Copyright (C) 2008-2012 Christian Stehno, Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;

static bool testLightTypes(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice (driverType, core::dimension2d<u32>(128,128));
	if (!device)
		return true; // No error if device does not exist

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();
	if (!driver->getDriverAttributes().getAttributeAsInt("MaxLights"))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	stabilizeScreenBackground(driver);

	logTestString("Testing driver %ls\n", driver->getName());

//	smgr->setAmbientLight(video::SColorf(0.3f,0.3f,0.3f));
	scene::ICameraSceneNode* cam = smgr->addCameraSceneNode();
	cam->setPosition(core::vector3df(0,200,0));
	cam->setTarget(core::vector3df());
	smgr->addAnimatedMeshSceneNode(device->getSceneManager()->addHillPlaneMesh("plane", core::dimension2df(4,4), core::dimension2du(128,128)));
	scene::ILightSceneNode* light1 = smgr->addLightSceneNode(0, core::vector3df(-100,30,-100));
	light1->setLightType(video::ELT_POINT);
	light1->setRadius(100.f);
	light1->getLightData().DiffuseColor.set(0,1,1);
//	smgr->addCubeSceneNode(10, light1)->setMaterialFlag(video::EMF_LIGHTING, false);
	scene::ILightSceneNode* light2 = smgr->addLightSceneNode(0, core::vector3df(100,30,100));
	light2->setRotation(core::vector3df(90,0,0));
	light2->setLightType(video::ELT_SPOT);
	light2->setRadius(100.f);
	light2->getLightData().DiffuseColor.set(1,0,0);
	light2->getLightData().InnerCone=10.f;
	light2->getLightData().OuterCone=30.f;
//	smgr->addCubeSceneNode(10, light2)->setMaterialFlag(video::EMF_LIGHTING, false);
	scene::ILightSceneNode* light3 = smgr->addLightSceneNode();
	light3->setRotation(core::vector3df(15,0,0));
	light3->setLightType(video::ELT_DIRECTIONAL);
	light1->getLightData().DiffuseColor.set(0,1,0);

	driver->beginScene (true, true, 0);
	smgr->drawAll();
	driver->endScene();

	const bool result = takeScreenshotAndCompareAgainstReference(driver, "-lightType.png", 99.91f);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

bool lights(void)
{
	bool result = true;
	// no lights in sw renderer
	TestWithAllDrivers(testLightTypes);
	return result;
}
