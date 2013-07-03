// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

/** This tests verifies that textures opened from different places in the filesystem
	can be distinguished, even if they have the same filename. */
bool disambiguateTextures(void)
{
	IrrlichtDevice *device =
		createDevice( video::EDT_NULL, dimension2d<u32>(640, 480));

	if (!device)
	{
		logTestString("Unable to create EDT_NULL device\n");
		return false;
	}

	// Expects an empty tmp/tmp directory under this app's wd and
	// a media directory under this apps' directory with tools.png in it.
	stringc wd = device->getFileSystem()->getWorkingDirectory();

	if(-1 == wd.find("/tests") && -1 == wd.find("\\tests"))
	{
		logTestString("The tests must be run from the /tests directory, regardless of where\n"\
			"the test executable was built.\n");
		device->drop();
		return false;
	}

	IVideoDriver * driver = device->getVideoDriver();

	ITexture * tex1 = driver->getTexture("../media/tools.png");
	assert_log(tex1);
	if(!tex1)
		logTestString("Unable to open ../media/tools.png\n");

	ITexture * tex2 = driver->getTexture("../media/tools.png");
	assert_log(tex2);
	if(!tex2)
		logTestString("Unable to open ../media/tools.png\n");

	IReadFile * readFile = device->getFileSystem()->createAndOpenFile("../media/tools.png");
	assert_log(readFile);
	if(!readFile)
		logTestString("Unable to open ../media/tools.png\n");

	ITexture * tex3 = driver->getTexture(readFile);
	assert_log(tex3);
	if(!readFile)
		logTestString("Unable to create texture from ../media/tools.png\n");

	readFile->drop();

	// All 3 of the above textures should be identical.
	assert_log(tex1 == tex2);
	assert_log(tex1 == tex3);

	stringc newWd = wd + "/empty/empty";
	bool changed = device->getFileSystem()->changeWorkingDirectoryTo(newWd.c_str());
	assert_log(changed);
	ITexture * tex4 = driver->getTexture("../../media/tools.png");
	assert_log(tex4);
	if(!tex4)
		logTestString("Unable to open ../../media/tools.png\n");
	assert_log(tex1 != tex4);

	// The working directory must be restored for the other tests to work.
	changed &= device->getFileSystem()->changeWorkingDirectoryTo(wd.c_str());

	device->closeDevice();
	device->run();
	device->drop();

	return (changed && tex1 == tex2 && tex1 == tex3 && tex1 != tex4) ? true : false;
}

