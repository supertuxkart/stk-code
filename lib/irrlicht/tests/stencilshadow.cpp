// Copyright (C) 2008-2012 Christian Stehno, Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;

static bool shadows(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice (driverType, core::dimension2d<u32>(160,120), 16, false, true);
	if (!device)
		return true; // No error if device does not exist

	stabilizeScreenBackground(device->getVideoDriver());

	scene::ICameraSceneNode* cam = device->getSceneManager()->addCameraSceneNodeFPS();
	cam->setPosition(core::vector3df(-15,55,10));
	cam->setTarget(core::vector3df(-5,-5,-15));

	device->getSceneManager()->setAmbientLight(video::SColorf(.5f,.5f,.5f));
	scene::IMeshSceneNode* cube = device->getSceneManager()->addCubeSceneNode(100, 0, -1, core::vector3df(0,50,0));
	cube->setScale(core::vector3df(-1,-1,-1));

	scene::IAnimatedMeshSceneNode* node = device->getSceneManager()->addAnimatedMeshSceneNode(device->getSceneManager()->getMesh("../media/ninja.b3d"), 0, -1, core::vector3df(0,2,0), core::vector3df(),core::vector3df(5,5,5));
	node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
	node->addShadowVolumeSceneNode();
	node->setAnimationSpeed(0.f);

	scene::ILightSceneNode* light = device->getSceneManager()->addLightSceneNode(0, core::vector3df(10,10,10));
	light->setLightType(video::ELT_POINT);
	light->setRadius(500.f);
	light->getLightData().DiffuseColor.set(0,1,1);

	device->getVideoDriver()->beginScene (true, true, 0);
	device->getSceneManager()->drawAll();
	device->getVideoDriver()->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(device->getVideoDriver(), "-stencilShadow.png", 99.91f);

	node->remove();
	cube->remove();
	// test self-shadowing
	node = device->getSceneManager()->addAnimatedMeshSceneNode(device->getSceneManager()->getMesh("../media/dwarf.x"));
	node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
	node->addShadowVolumeSceneNode();
	node->setAnimationSpeed(0.f);

	cam->setPosition(core::vector3df(0,55,-30));
	cam->setTarget(core::vector3df(60,45,150));

	device->getVideoDriver()->beginScene (true, true, 0);
	device->getSceneManager()->drawAll();
	device->getVideoDriver()->endScene();

	result = takeScreenshotAndCompareAgainstReference(device->getVideoDriver(), "-stencilSelfShadow.png", 99.41f);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

bool stencilShadow(void)
{
	bool passed = true;

	passed &= shadows(video::EDT_OPENGL);
	// no shadows in these renderers
//	passed &= shadows(video::EDT_SOFTWARE);
//	passed &= shadows(video::EDT_BURNINGSVIDEO);
	passed &= shadows(video::EDT_DIRECT3D9);
	passed &= shadows(video::EDT_DIRECT3D8);

	return passed;
}
