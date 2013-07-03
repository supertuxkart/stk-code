// Copyright (C) 2008-2012 Christian Stehno, Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;

namespace
{
//! check miplevels by visual test
bool renderMipLevels(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice( driverType, dimension2d<u32>(160, 120), 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager * smgr = device->getSceneManager();
	if (!driver->queryFeature(video::EVDF_MIP_MAP))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	stabilizeScreenBackground(driver);

	logTestString("Testing driver %ls\n", driver->getName());

	scene::ISceneNode* n = smgr->addCubeSceneNode();
	scene::ISceneNode* n2 = smgr->addCubeSceneNode(10, 0, -1, core::vector3df(20,0,30), core::vector3df(0,45,0));

	// we use a main texture with blue on top and red below
	// and mipmap with pink on top and cyan below
	if (n && n2)
	{
		// create the texture and miplevels with distinct colors
		u32 texData[16*16];
		for (u32 i=0; i<16*16; ++i)
			texData[i]=(i<8*16?0xff0000ff:0xffff0000);
		video::IImage* image = driver->createImageFromData(video::ECF_A8R8G8B8, core::dimension2du(16,16), texData, false);
		u32 mipdata[8*16];
		u32 index=0;
		for (u32 j=8; j>0; j/=2)
		{
			for (u32 i=0; i<j; ++i)
			{
				u32 val=(i<j/2?0xffff00ff:0xff00ffff);
				for (u32 k=0; k<j; ++k)
					mipdata[index++]=val;
			}
		}

		video::ITexture* tex = driver->addTexture("miptest", image, mipdata);
		if (!tex)
			// is probably an error in the mipdata handling
			return false;
		else
		{
			n->setMaterialFlag(video::EMF_LIGHTING, false);
			n->setMaterialTexture(0, tex);
			n2->setMaterialFlag(video::EMF_LIGHTING, false);
			n2->setMaterialTexture(0, tex);
		}
		image->drop();
	}

	(void)smgr->addCameraSceneNode(0, core::vector3df(10,0,-30));

	driver->beginScene(true, true, video::SColor(255,100,101,140));
	smgr->drawAll();
	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-renderMipmap.png");

	if (!result)
		logTestString("mipmap render failed.\n", driver->getName());
	else
		logTestString("Passed\n");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}


//! Tests locking miplevels
bool lockAllMipLevels(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice( driverType, dimension2d<u32>(160, 120), 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager * smgr = device->getSceneManager();

	if (!driver->queryFeature(video::EVDF_MIP_MAP))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	stabilizeScreenBackground(driver);

	logTestString("Testing driver %ls\n", driver->getName());

	scene::ISceneNode* n = smgr->addCubeSceneNode();

	if (n)
	{
		// create the texture and miplevels with distinct colors
		u32 texData[16*16];
		for (u32 i=0; i<16*16; ++i)
			texData[i]=0xff0000ff-i;
		video::IImage* image = driver->createImageFromData(video::ECF_A8R8G8B8, core::dimension2du(16,16), texData, false);
		u32 mipdata[8*16];
		u32 index=0;
		for (u32 j=8; j>0; j/=2)
		{
			u32 val=(j==8?0x00ff00ff:(j==4?0x0000ffff:(j==2?0xc2c200ff:0x001212ff)));
			for (u32 i=0; i<j; ++i)
			{
				for (u32 k=0; k<j; ++k)
					mipdata[index++]=val-i;
			}
		}

		video::ITexture* tex = driver->addTexture("miptest", image, mipdata);
		if (!tex)
			// is probably an error in the mipdata handling
			return false;
		else
			n->setMaterialTexture(0, tex);
		image->drop();
	}

	(void)smgr->addCameraSceneNode();

	driver->beginScene(true, true, video::SColor(255,100,101,140));
	smgr->drawAll();
	driver->endScene();

	video::ITexture* tex = driver->findTexture("miptest");
	video::SColor* bits = (video::SColor*)tex->lock(video::ETLM_READ_ONLY, 0);
	bool result = (bits[0].color==0xff0000ff);
	tex->unlock();
	bits = (video::SColor*)tex->lock(video::ETLM_READ_ONLY, 1);
	result &= (bits[0].color==0x00ff00ff);
	tex->unlock();
	bits = (video::SColor*)tex->lock(video::ETLM_READ_ONLY, 2);
	result &= (bits[0].color==0x0000ffff);
	tex->unlock();
	bits = (video::SColor*)tex->lock(video::ETLM_READ_ONLY, 3);
	result &= (bits[0].color==0xc2c200ff);
	tex->unlock();
	bits = (video::SColor*)tex->lock(video::ETLM_READ_ONLY, 4);
	result &= (bits[0].color==0x001212ff);
	tex->unlock();

	if (!result)
		logTestString("mipmap lock after init with driver %ls failed.\n", driver->getName());

	// test with updating a lower level, and reading upper and lower
	bits = (video::SColor*)tex->lock(video::ETLM_READ_WRITE, 3);
	bits[0]=0xff00ff00;
	bits[1]=0xff00ff00;
	tex->unlock();
	bits = (video::SColor*)tex->lock(video::ETLM_READ_WRITE, 4);
	result &= (bits[0].color==0x001212ff);
	tex->unlock();
	bits = (video::SColor*)tex->lock(video::ETLM_READ_ONLY, 3);
	result &= ((bits[0].color==0xff00ff00)&&(bits[2].color==0xc2c200fe));
	tex->unlock();

	if (!result)
		logTestString("mipmap lock after mipmap write with driver %ls failed.\n", driver->getName());

	// now test locking level 0
	bits = (video::SColor*)tex->lock(video::ETLM_READ_WRITE, 0);
	bits[0]=0xff00ff00;
	bits[1]=0xff00ff00;
	tex->unlock();
	bits = (video::SColor*)tex->lock(video::ETLM_READ_WRITE, 4);
	result &= (bits[0].color==0x001212ff);
	tex->unlock();
	bits = (video::SColor*)tex->lock(video::ETLM_READ_ONLY, 0);
	result &= ((bits[0].color==0xff00ff00)&&(bits[2].color==0xff0000fd));
	tex->unlock();

	if (!result)
		logTestString("mipmap lock at level 0 after mipmap write with driver %ls failed.\n", driver->getName());
	else
		logTestString("Passed\n");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}


//! Tests locking miplevels after texture was created with auto mipmap update
bool lockWithAutoMipmap(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice( driverType, dimension2d<u32>(160, 120), 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager * smgr = device->getSceneManager();

	if (!driver->queryFeature(video::EVDF_MIP_MAP))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	stabilizeScreenBackground(driver);

	logTestString("Testing driver %ls\n", driver->getName());

	scene::ISceneNode* n = smgr->addCubeSceneNode();

	if (n)
	{
		// create the texture
		u32 texData[16*16];
		for (u32 i=0; i<16*16; ++i)
			texData[i]=0xff0000ff-i;
		video::IImage* image = driver->createImageFromData(video::ECF_A8R8G8B8, core::dimension2du(16,16), texData, false);

		video::ITexture* tex = driver->addTexture("miptest", image);
		if (!tex)
			return false;
		else
			n->setMaterialTexture(0, tex);
		image->drop();
	}
	(void)smgr->addCameraSceneNode();

	driver->beginScene(true, true, video::SColor(255,100,101,140));
	smgr->drawAll();
	driver->endScene();

	video::ITexture* tex = driver->findTexture("miptest");
	video::SColor* bits = (video::SColor*)tex->lock(video::ETLM_READ_ONLY, 0);
	bool result = (bits[0].color==0xff0000ff);
	tex->unlock();
	if (!result)
		logTestString("mipmap lock after init with driver %ls failed.\n", driver->getName());

	// test with updating a lower level, and reading upper and lower
	bits = (video::SColor*)tex->lock(video::ETLM_READ_WRITE, 3);
	bits[0]=0xff00ff00;
	bits[1]=0xff00ff00;
	tex->unlock();
	// lock another texture just to invalidate caches in the driver
	bits = (video::SColor*)tex->lock(video::ETLM_READ_WRITE, 4);
	tex->unlock();
	bits = (video::SColor*)tex->lock(video::ETLM_READ_ONLY, 3);
	result &= ((bits[0].color==0xff00ff00)&&(bits[2].color!=0xff00ff00));
	tex->unlock();

	if (!result)
		logTestString("mipmap lock after mipmap write with driver %ls failed.\n", driver->getName());

	// now test locking level 0
	bits = (video::SColor*)tex->lock(video::ETLM_READ_WRITE, 0);
	bits[0]=0x00ff00ff;
	bits[1]=0x00ff00ff;
	tex->unlock();
	bits = (video::SColor*)tex->lock(video::ETLM_READ_ONLY, 3);
	result &= ((bits[0].color==0xff00ff00)&&(bits[2].color!=0xff00ff00));
	tex->unlock();

	if (!result)
		logTestString("mipmap lock at level 0 after mipmap write with driver %ls failed.\n", driver->getName());
	else
		logTestString("Passed\n");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}
}


bool textureFeatures(void)
{
	bool result = true;

	TestWithAllDrivers(renderMipLevels);
	TestWithAllDrivers(lockAllMipLevels);
	TestWithAllDrivers(lockWithAutoMipmap);

	return result;
}
