#include "testUtils.h"

using namespace irr;

namespace
{
bool testImageCreation()
{
	// create device

	IrrlichtDevice *device = createDevice(video::EDT_SOFTWARE, core::dimension2d<u32>(160,120));

	if (device == 0)
		return true; // could not create selected driver.

	video::IVideoDriver* driver = device->getVideoDriver();
	video::ITexture* tex=driver->getTexture("../media/water.jpg");
	video::IImage* img1=driver->createImage(tex, core::vector2di(0,0), core::dimension2du(32,32));
	video::ITexture* tex1=driver->addTexture("new1", img1);
	img1->drop();
	img1=0;
	video::IImage* img2=driver->createImage(tex, core::vector2di(0,0), tex->getSize());
	video::ITexture* tex2=driver->addTexture("new2", img2);
	img2->drop();
	img2 = 0;

	driver->beginScene(true, true, video::SColor(255,255,0,255));//Backbuffer background is pink

	driver->draw2DImage(tex, core::position2d<s32>(0,0), core::recti(0,0,32,32));
	driver->draw2DImage(tex1, core::position2d<s32>(32,0));
	driver->draw2DImage(tex2, core::position2d<s32>(64,0), core::recti(0,0,32,32));

	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-createImage.png");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

bool testImageFormats()
{
	IrrlichtDevice *device = createDevice(video::EDT_BURNINGSVIDEO, core::dimension2d<u32>(256,128));

	if (device == 0)
		return true; // could not create selected driver.

	video::IVideoDriver* driver = device->getVideoDriver();
	video::ITexture* tex=driver->getTexture("../media/water.jpg");
	video::ITexture* tex1=driver->getTexture("media/grey.tga");
	driver->beginScene(true, true);

	driver->draw2DImage(tex, core::position2d<s32>(0,0), core::recti(0,0,64,64));
	driver->draw2DImage(tex1, core::position2d<s32>(0,64), core::recti(0,0,64,64));
	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-testImageFormats.png", 99.5f);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}
}

bool createImage()
{
	bool result = testImageCreation();
	result &= testImageFormats();
	return result;
}
