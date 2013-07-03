// Copyright (C) 2009-2012 Christian Stehno
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

// Tests mesh transformations via mesh manipulator.
bool meshTransform(void)
{
	// Use EDT_BURNINGSVIDEO since it is not dependent on (e.g.) OpenGL driver versions.
	IrrlichtDevice *device = createDevice(EDT_BURNINGSVIDEO, dimension2d<u32>(160, 120), 32);
	assert_log(device);
	if (!device)
		return false;

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager * smgr = device->getSceneManager();

	IMeshSceneNode* node1 = smgr->addCubeSceneNode(50);
	IAnimatedMesh* amesh = smgr->getMesh("../media/sydney.md2");
	IAnimatedMesh* smesh = smgr->getMesh("../media/ninja.b3d");
	assert_log(node1 && amesh && smesh);

	bool result = false;
	if (!node1 || !amesh || !smesh)
		return false;

//	node1->setPosition(core::vector3df(-60,0,150));
	node1->setDebugDataVisible(scene::EDS_BBOX_ALL);

	IMeshSceneNode* node2 = smgr->addMeshSceneNode(amesh->getMesh(10));
	assert_log(node2);

	if (!node2)
		return false;

//	node2->setPosition(core::vector3df(30,10,150));
	node2->setDebugDataVisible(scene::EDS_BBOX_ALL);
	node2->setMaterialFlag(EMF_LIGHTING, false);

	IMeshSceneNode* node3 = smgr->addMeshSceneNode(smesh->getMesh(10));
	assert_log(node3);

	if (!node3)
		return false;

//	node3->setPosition(core::vector3df(10,0,0));
	node3->setDebugDataVisible(scene::EDS_BBOX_ALL);
	node3->setMaterialFlag(EMF_LIGHTING, false);

	smgr->addCameraSceneNode()->setPosition(core::vector3df(0,0,-20));

	// Just jump to the last frame since that's all we're interested in.
	device->run();
	driver->beginScene(true, true, SColor(255, 60, 60, 60));
	smgr->drawAll();
	driver->endScene();

	core::matrix4 mat;
	mat.setTranslation(core::vector3df(-60,0,150));
	driver->getMeshManipulator()->transform(node1->getMesh(), mat);

	mat.setTranslation(core::vector3df(30,10,150));
	driver->getMeshManipulator()->transform(node2->getMesh(), mat);

	mat.setTranslation(core::vector3df(10,0,0));
	driver->getMeshManipulator()->transform(node3->getMesh(), mat);

	// Just jump to the last frame since that's all we're interested in.
	device->run();
	driver->beginScene(true, true, SColor(255, 60, 60, 60));
	smgr->drawAll();
	driver->endScene();

	result = takeScreenshotAndCompareAgainstReference(driver, "-meshTransform.png");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}
