// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

/** This tests verifies that textures opened from different places in the
	filesystem don't create duplicated textures. */
bool loadFromFileFolder(void)
{
	IrrlichtDevice *device =
		createDevice( video::EDT_NULL, dimension2du(160, 120));

	if (!device)
	{
		logTestString("Unable to create EDT_NULL device\n");
		return false;
	}

	IVideoDriver * driver = device->getVideoDriver();

	u32 numTexs = driver->getTextureCount();

	ITexture * tex1 = driver->getTexture("../media/tools.png");
	assert_log(tex1);
	if(!tex1)
		logTestString("Unable to open ../media/tools.png\n");
	if (driver->getTextureCount()!=numTexs+1)
	{
		logTestString("No additional texture in the texture cache %s:%d\n", __FILE__, __LINE__);
		return false;
	}

	IReadFile * readFile = device->getFileSystem()->createAndOpenFile("../media/tools.png");
	assert_log(readFile);
	if(!readFile)
		logTestString("Unable to open ../media/tools.png\n");
	if (driver->getTextureCount()!=numTexs+1)
	{
		logTestString("Additional texture in the texture cache %s:%d\n", __FILE__, __LINE__);
		return false;
	}

	ITexture * tex2 = driver->getTexture(readFile);
	assert_log(tex2);
	if(!readFile)
		logTestString("Unable to create texture from ../media/tools.png\n");
	if (driver->getTextureCount()!=numTexs+1)
	{
		logTestString("Additional texture in the texture cache %s:%d\n", __FILE__, __LINE__);
		return false;
	}

	readFile->drop();

	// adding  a folder archive
	device->getFileSystem()->addFileArchive( "../media/" );

	ITexture * tex3 = driver->getTexture("tools.png");
	assert_log(tex3);
	if(!tex3)
		logTestString("Unable to open tools.png\n");
	if (driver->getTextureCount()!=numTexs+1)
	{
		logTestString("Additional texture in the texture cache %s:%d\n", __FILE__, __LINE__);
		return false;
	}

	ITexture * tex4 = driver->getTexture("tools.png");
	assert_log(tex4);
	if(!tex4)
		logTestString("Unable to open tools.png\n");
	if (driver->getTextureCount()!=numTexs+1)
	{
		logTestString("Additional texture in the texture cache %s:%d\n", __FILE__, __LINE__);
		return false;
	}

	device->closeDevice();
	device->run();
	device->drop();
	return ((tex1 == tex2) && (tex1 == tex3) && (tex1 == tex4));
}

bool loadTextures()
{
	bool result = true;
	result &= loadFromFileFolder();
	return result;
}

