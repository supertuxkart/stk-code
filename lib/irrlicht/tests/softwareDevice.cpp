// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;

//! Tests the basic functionality of the software device.
bool softwareDevice(void)
{
	IrrlichtDevice *device = createDevice(video::EDT_SOFTWARE, dimension2d<u32>(160, 120), 32);
	if (!device)
		return false;

	video::IVideoDriver* driver = device->getVideoDriver();
	ISceneManager * smgr = device->getSceneManager();

	// Test a particular rotation that highlighted a clipping issue from matrix::transformPlane()
	video::S3DVertex vertices[3];
	vertices[0] = video::S3DVertex(10,0,-10, 1,0,0,
		video::SColor(255,255,0,255), 1, 1);
	vertices[1] = video::S3DVertex(0,20,0, 0,1,1,
		video::SColor(255,255,255,0), 1, 0);
	vertices[2] = video::S3DVertex(-10,0,-10, 0,0,1,
		video::SColor(255,0,255,0), 0, 0);

	video::SMaterial material;
	material.Lighting = false;
	material.Wireframe = false;
	const u16 indices[] = { 1,0,2, };

	matrix4 transform(matrix4::EM4CONST_IDENTITY);
	vector3df rotations(290, 0, 290); // <-- This rotation used to clip the triangle incorrectly
	transform.setRotationDegrees(rotations);

	(void)smgr->addCameraSceneNode(0, core::vector3df(0,0,-40), core::vector3df(0,0,0));

	driver->beginScene(true, true, video::SColor(255,255,255,0));
	smgr->drawAll();

	driver->setMaterial(material);

	driver->setTransform(video::ETS_WORLD, transform);
	driver->drawIndexedTriangleList(&vertices[0], 3, &indices[0], 1);
	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-softwareDevice-rotatedClip.png");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}
