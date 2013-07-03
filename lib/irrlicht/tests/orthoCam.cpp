// Copyright (C) 2008-2012 Christian Stehno, Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;

static bool testOrthoCam(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice (driverType, core::dimension2d<u32>(160,120));
	if (!device)
		return true; // No error if device does not exist

	stabilizeScreenBackground(device->getVideoDriver());

	scene::ICameraSceneNode* cam = device->getSceneManager()->addCameraSceneNode();
	cam->setPosition(core::vector3df(500,200,-500));
	cam->setTarget(core::vector3df());
	cam->setProjectionMatrix(core::matrix4().buildProjectionMatrixOrthoLH(240,180,0.9f,2000.f), true);

	device->getSceneManager()->addAnimatedMeshSceneNode(device->getSceneManager()->addHillPlaneMesh("plane", core::dimension2df(32,32), core::dimension2du(16,16)));//->setMaterialFlag(video::EMF_WIREFRAME, true);
	device->getSceneManager()->addCubeSceneNode(20.f)->setPosition(core::vector3df(50,20,50));
	device->getSceneManager()->addCubeSceneNode(20.f)->setPosition(core::vector3df(50,20,-50));
	device->getSceneManager()->addCubeSceneNode(20.f)->setPosition(core::vector3df(50,50,0));

	device->getSceneManager()->addCubeSceneNode(20.f)->setPosition(core::vector3df(-50,10,0));
	device->getSceneManager()->addCubeSceneNode(20.f)->setPosition(core::vector3df(100,10,-100));
	device->getSceneManager()->addCubeSceneNode(20.f)->setPosition(core::vector3df(150,10,0));

	scene::IAnimatedMeshSceneNode* node = device->getSceneManager()->addAnimatedMeshSceneNode(device->getSceneManager()->getMesh("../media/ninja.b3d"), 0, -1, core::vector3df(-50,2,-50), core::vector3df(),core::vector3df(5,5,5));
	node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
	node->setAnimationSpeed(0.f);

	scene::ILightSceneNode* light = device->getSceneManager()->addLightSceneNode(0, core::vector3df(0,100,0));
	light->setLightType(video::ELT_POINT);
	light->setRadius(500.f);
	light->getLightData().DiffuseColor.set(0,1,1);

	device->getVideoDriver()->beginScene (true, true, 0);
	device->getSceneManager()->drawAll();
	device->getVideoDriver()->endScene();

	const bool result = takeScreenshotAndCompareAgainstReference(device->getVideoDriver(), "-orthoCam.png", 99.91f);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

static bool testOrthoStencil(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice (driverType, core::dimension2d<u32>(160,120), 16, false, true);
	if (!device)
		return true; // No error if device does not exist

	stabilizeScreenBackground(device->getVideoDriver());

	scene::ICameraSceneNode* cam = device->getSceneManager()->addCameraSceneNode();
	cam->setPosition(core::vector3df(300,250,-300));
	cam->setTarget(core::vector3df(0,20,0));
	cam->setProjectionMatrix(core::matrix4().buildProjectionMatrixOrthoLH(120,90,0.9f,2000.f), true);

	device->getSceneManager()->addAnimatedMeshSceneNode(device->getSceneManager()->addHillPlaneMesh("plane", core::dimension2df(32,32), core::dimension2du(16,16)));//->setMaterialFlag(video::EMF_WIREFRAME, true);

	scene::IAnimatedMeshSceneNode* node = device->getSceneManager()->addAnimatedMeshSceneNode(device->getSceneManager()->getMesh("../media/ninja.b3d"), 0, -1, core::vector3df(0,2,0), core::vector3df(),core::vector3df(5,5,5));
	node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
	node->addShadowVolumeSceneNode();
	node->setAnimationSpeed(0.f);

	scene::ILightSceneNode* light = device->getSceneManager()->addLightSceneNode(0, core::vector3df(100,150,100));
	light->setLightType(video::ELT_POINT);
	light->setRadius(500.f);
	light->getLightData().DiffuseColor.set(0,1,1);

	device->getVideoDriver()->beginScene (true, true, 0);
	device->getSceneManager()->drawAll();
	device->getVideoDriver()->endScene();

	const bool result = takeScreenshotAndCompareAgainstReference(device->getVideoDriver(), "-orthoStencil.png", 99.91f);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

bool orthoCam(void)
{
	bool passed = true;

	passed &= testOrthoCam(video::EDT_OPENGL);
	// no lights in sw renderer
//	passed &= testOrthoCam(video::EDT_SOFTWARE);
	passed &= testOrthoCam(video::EDT_BURNINGSVIDEO);
	passed &= testOrthoCam(video::EDT_DIRECT3D9);
	passed &= testOrthoCam(video::EDT_DIRECT3D8);

	passed &= testOrthoStencil(video::EDT_OPENGL);
	passed &= testOrthoStencil(video::EDT_DIRECT3D9);

	return passed;
}
