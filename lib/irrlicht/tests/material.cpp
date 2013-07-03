#include "testUtils.h"

using namespace irr;

static bool polygonOffset(video::E_DRIVER_TYPE type)
{
	IrrlichtDevice* device = createDevice(type, core::dimension2d<u32>(160, 120));

	if (device == 0)
		return true;

	video::IVideoDriver* driver = device->getVideoDriver();
	if (!driver->queryFeature(video::EVDF_POLYGON_OFFSET))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	stabilizeScreenBackground(driver);

	scene::ISceneManager* smgr = device->getSceneManager();

	// create first plane
	scene::ISceneNode* plane = smgr->addMeshSceneNode(smgr->addHillPlaneMesh(
		"plane", core::dimension2df(10,10), core::dimension2du(2,2)), 0, -1,
		core::vector3df(0,0,20), core::vector3df(270,0,0));

	if (plane)
	{
		plane->setMaterialTexture(0, driver->getTexture("../media/t351sml.jpg"));
		plane->setMaterialFlag(video::EMF_LIGHTING, false);
		plane->setMaterialFlag(video::EMF_BACK_FACE_CULLING, true);
	}

	// create second plane exactly on top of the first one
	scene::ISceneNode* plane2 = smgr->addMeshSceneNode(smgr->addHillPlaneMesh(
		"plane2", core::dimension2df(5,5), core::dimension2du(2,2)), 0, -1,
		core::vector3df(0,0,20), core::vector3df(270,0,0));
	plane2->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);

	smgr->addCameraSceneNode();

	// test back plane to back
	plane->getMaterial(0).PolygonOffsetDirection=video::EPO_BACK;
	plane->getMaterial(0).PolygonOffsetFactor=7;

	driver->beginScene(true, true, video::SColor(255,113,113,133));
	smgr->drawAll();
	driver->endScene();
	bool result = takeScreenshotAndCompareAgainstReference(driver, "-polygonBack.png");

	//reset back plane
	plane->getMaterial(0).PolygonOffsetFactor=0;
	// test front plane to front
	plane2->getMaterial(0).PolygonOffsetDirection=video::EPO_FRONT;
	plane2->getMaterial(0).PolygonOffsetFactor=7;
	driver->beginScene(true, true, video::SColor(255,113,113,133));
	smgr->drawAll();
	driver->endScene();
	result &= takeScreenshotAndCompareAgainstReference(driver, "-polygonFront.png");

	device->closeDevice();
	device->run();
	device->drop();
    return result;
}

bool material()
{
	bool result = true;
	TestWithAllDrivers(polygonOffset);
	return result;
}
