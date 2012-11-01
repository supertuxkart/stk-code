#include "testUtils.h"

using namespace irr;

static bool withSphere(video::E_DRIVER_TYPE type)
{
	IrrlichtDevice* device = createDevice(type, core::dimension2d<u32>(160, 120));

	if (device == 0)
		return true;

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();

	driver->setClipPlane(0, core::plane3df(core::vector3df(0,18,0), core::vector3df(0,-1,0)), true);
	smgr->addLightSceneNode(0, core::vector3df(30,30,50));
	// create first sphere
	scene::ISceneNode* sphere = smgr->addMeshSceneNode(smgr->addSphereMesh("sphere", 10, 64, 64));

	if (sphere)
	{
		sphere->setPosition(core::vector3df(0,10,0));
		sphere->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
	}

	sphere = smgr->addMeshSceneNode(smgr->addHillPlaneMesh("mesh", core::dimension2df(10,10), core::dimension2du(10,10)));
	sphere->setPosition(core::vector3df(0,10,0));

	smgr->addCameraSceneNode(0, core::vector3df(-25,30,20), core::vector3df());

	driver->setClipPlane(1, core::plane3df(core::vector3df(0,2,0), core::vector3df(0,1,0)), true);
	driver->setClipPlane(2, core::plane3df(core::vector3df(8,0,0), core::vector3df(-1,0,0)));

	device->run();
//	while(device->run())
	{
	driver->beginScene(true, true, video::SColor(255,113,113,133));
	driver->setClipPlane(3, core::plane3df(core::vector3df(-8,0,0), core::vector3df(1,0,0)), true);
	driver->setClipPlane(4, core::plane3df(core::vector3df(0,0,8), core::vector3df(0,0,-1)));
	driver->setClipPlane(5, core::plane3df(core::vector3df(0,0,-8), core::vector3df(0,0,1)));
	driver->enableClipPlane(2, true);
	driver->enableClipPlane(4, true);
	driver->enableClipPlane(5, true);
	smgr->drawAll();
	driver->enableClipPlane(1, false);
	driver->enableClipPlane(2, false);
	driver->enableClipPlane(4, false);
	driver->enableClipPlane(5, false);
	driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
	driver->setMaterial(video::IdentityMaterial);
	driver->draw3DLine(core::vector3df(), core::vector3df(0,0,50));
	driver->endScene();
	}
	bool result = takeScreenshotAndCompareAgainstReference(driver, "-ucpsphere.png");

	device->closeDevice();
	device->drop();
    return result;
}

bool userclipplane()
{
	bool result = true;
	TestWithAllHWDrivers(withSphere);
	return result;
}
