// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;

// Tests mesh loading features and the mesh cache.
/** This won't test render results. Currently, not all mesh loaders are tested. */
bool meshLoaders(void)
{
	IrrlichtDevice *device = createDevice(video::EDT_NULL, core::dimension2d<u32>(160, 120), 32);
	assert_log(device);
	if (!device)
		return false;

	scene::ISceneManager * smgr = device->getSceneManager();
	scene::IAnimatedMesh* mesh = smgr->getMesh("../media/ninja.b3d");
	assert_log(mesh);

	bool result = (mesh != 0);

	if (mesh)
	{
		if (mesh != smgr->getMesh("../media/ninja.b3d"))
		{
			logTestString("Loading from same file results in different meshes!");
				result=false;
		}
	}

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}
