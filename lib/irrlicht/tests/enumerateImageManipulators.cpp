// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace scene;
using namespace video;

bool enumerateImageManipulators(void)
{
    IrrlichtDevice *device = createDevice(video::EDT_NULL);
    if (!device)
        return false;

    IVideoDriver* driver = device->getVideoDriver();

	const char* filenames[] =
	{
		"foo.bmp",
		"foo.jpg",
		"foo.pcx",
		"foo.png",
		"foo.ppm",
		"foo.psd",
		"foo.tga",
		// the following have no writers
		"foo.wal",
		"foo.pgm",
		"foo.pbm",
		"foo.rgb",
		"foo.rgba",
		"foo.sgi",
		"foo.int",
		"foo.inta",
		"foo.bw"
	};
	// how many formats have loaders?
	const u32 writersUntil = 7;

	const u32 numberOfFilenames = sizeof(filenames) / sizeof(filenames[0]);
	bool loaderForFilename[numberOfFilenames] = { false }; // and the rest get 0 == false
	bool writerForFilename[numberOfFilenames] = { false }; // and the rest get 0 == false

	bool result = true;

	u32 i;
	const u32 loaders = driver->getImageLoaderCount();
	for (i = 0; i < loaders; ++i)
	{
		IImageLoader * loader = driver->getImageLoader(i);

		if(!loader)
		{
			logTestString("Failed to get image loader %d\n", i);
			assert_log(false);
			result = false;
		}

		for(u32 filename = 0; filename < numberOfFilenames; ++filename)
		{
			if(loader->isALoadableFileExtension(filenames[filename]))
			{
				loaderForFilename[filename] = true;
			}
		}
	}

	IImageLoader * loader = driver->getImageLoader(i);
	assert_log(loader == 0);
	if(loader)
	{
		logTestString("Got a loader when none was expected (%d)\n", i);
		result = false;
	}

	for(u32 filename = 0; filename < numberOfFilenames; ++filename)
	{
		if(!loaderForFilename[filename])
		{
			logTestString("File type '%s' doesn't have a loader\n", filenames[filename]);
			assert_log(false);
			result = false;
		}
	}

	const u32 writers = driver->getImageWriterCount();
	for (i = 0; i < writers; ++i)
	{
		IImageWriter * writer = driver->getImageWriter(i);

		if(!writer)
		{
			logTestString("Failed to get image writer %d\n", i);
			assert_log(false);
			result = false;
		}

		for(u32 filename = 0; filename < numberOfFilenames; ++filename)
		{
			if(writer->isAWriteableFileExtension(filenames[filename]))
			{
				writerForFilename[filename] = true;
				break;
			}
		}
	}

	IImageWriter * writer = driver->getImageWriter(i);
	assert_log(writer == 0);
	if(writer)
	{
		logTestString("Got a writer when none was expected (%d)\n", i);
		result = false;
	}

	for(u32 filename = 0; filename < numberOfFilenames; ++filename)
	{
		// There's no writer for the .WAL file type.
		if(!writerForFilename[filename] && (filename<writersUntil))
		{
			logTestString("File type '%s' doesn't have a writer\n", filenames[filename]);
			assert_log(false);
			result = false;
		}
	}

	device->closeDevice();
	device->run();
    device->drop();

    return result;
}
