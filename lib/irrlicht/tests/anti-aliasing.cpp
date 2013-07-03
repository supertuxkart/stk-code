#include "testUtils.h"

using namespace irr;

static bool testLineRendering(video::E_DRIVER_TYPE type)
{
	SIrrlichtCreationParameters params;
	params.AntiAlias = 2;
	params.Bits = 16;
	params.WindowSize = core::dimension2d<u32>(160, 120);
	params.DriverType = type;

	IrrlichtDevice *device = createDeviceEx(params);

	if (!device)
		return true; // in case the driver type does not exist

	video::IVideoDriver* driver = device->getVideoDriver();
	// if no AntiAliasing supported, skip this test
	if (driver->getDriverAttributes().getAttributeAsInt("AntiAlias")<2)
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	logTestString("Testing driver %ls\n", driver->getName());

	scene::ISceneManager* smgr = device->getSceneManager();

	scene::IAnimatedMesh* mesh = smgr->getMesh("./media/sydney.md2");
	if (!mesh)
	{
		device->closeDevice();
		device->run();
		device->drop();
		return false;
	}

	stabilizeScreenBackground(driver);

	scene::IAnimatedMeshSceneNode* node = smgr->addAnimatedMeshSceneNode( mesh );

	if (node)
	{
		node->setMaterialFlag(video::EMF_LIGHTING, false);
		node->setMD2Animation(scene::EMAT_STAND);
		node->setMaterialTexture( 0, driver->getTexture("./media/sydney.bmp") );
	}

	smgr->addCameraSceneNode(0, core::vector3df(0,30,-40), core::vector3df(0,5,0));

	device->getTimer()->setTime(0);	// scene has animations and current scene seems to be saved at that time
	driver->beginScene(true, true, video::SColor(255,100,101,140));
	smgr->drawAll();
	driver->draw3DBox(node->getBoundingBox(), video::SColor(0,255,0,0));
	driver->draw2DLine(core::position2di(10,10), core::position2di(100,100), video::SColor(255,0,0,0));
	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-lineAntiAliasing.png", 99.4f );

	device->closeDevice();
	device->run();
	device->drop();
    return result;
}

bool antiAliasing()
{
	bool result = true;
	TestWithAllHWDrivers(testLineRendering);
	return result;
}
