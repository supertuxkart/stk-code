// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;

// Tests skinned meshes.
bool skinnedMesh(void)
{
	// Use EDT_BURNINGSVIDEO since it is not dependent on (e.g.) OpenGL driver versions.
	IrrlichtDevice *device = createDevice(video::EDT_BURNINGSVIDEO, core::dimension2d<u32>(160, 120), 32);
	if (!device)
		return false;

	scene::ISceneManager * smgr = device->getSceneManager();

	logTestString("Testing setMesh()\n");

	scene::ISkinnedMesh* mesh = (scene::ISkinnedMesh*)smgr->getMesh("../media/ninja.b3d");
	if (!mesh)
	{
		logTestString("Could not load ninja.\n");
		return false;
	}

	scene::IAnimatedMeshSceneNode* node = smgr->addAnimatedMeshSceneNode(mesh);
	if (!node)
	{
		logTestString("Could not add ninja node.\n");
		return false;
	}

	// test if certain joint is found
	bool result = (node->getJointNode("Joint1") != 0);
	if (!result)
		logTestString("Could not find joint in ninja.\n");

	mesh = (scene::ISkinnedMesh*)smgr->getMesh("../media/dwarf.x");
	if (!mesh)
	{
		logTestString("Could not load dwarf.\n");
		return false;
	}
	node->setMesh(mesh);

	// make sure old joint is non-existant anymore
	logTestString("Ignore error message in log, this is intended.\n");
	result &= (node->getJointNode("Joint1")==0);
	if (!result)
		logTestString("Found non-existing joint in dwarf.\n");

	// and check that a new joint can be found
	// we use a late one, in order to see also inconsistencies in the joint cache
	result &= (node->getJointNode("hit") != 0);
	if (!result)
		logTestString("Could not find joint in dwarf.\n");

	node = smgr->addAnimatedMeshSceneNode(mesh);
	if (!node)
	{
		logTestString("Could not add dwarf node.\n");
		return false;
	}
	// check that a joint can really be found
	result &= (node->getJointNode("hit") != 0);
	if (!result)
		logTestString("Could not find joint in dwarf.\n");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}
