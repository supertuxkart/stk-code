
#include "testUtils.h"

using namespace irr;
using namespace core;

namespace
{

// test camera changes with terrain scene node recalculation
bool terrainRecalc(void)
{
	IrrlichtDevice *device =
		createDevice(video::EDT_BURNINGSVIDEO, dimension2du(160, 120), 32);

	if (!device)
		return true;

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();

	scene::ITerrainSceneNode* terrain = smgr->addTerrainSceneNode(
		"../media/terrain-heightmap.bmp");
	terrain->setScale(core::vector3df(40.f, .1f, 40.f));

	terrain->setMaterialFlag(video::EMF_LIGHTING, false);
	terrain->setMaterialTexture(0, driver->getTexture("../media/terrain-texture.jpg"));
	terrain->setDebugDataVisible(scene::EDS_FULL);

	scene::ICameraSceneNode* camera = smgr->addCameraSceneNode();

	const core::vector3df center(terrain->getBoundingBox().getCenter());
	camera->setTarget(center);

	// yes, Y is intentionally being set to X here
	const core::vector3df above (center.X, center.X, center.Z);
	camera->setPosition (above);
	camera->setUpVector(vector3df(1.f, 0.f, 0.f));
	camera->setFarValue(above.Y);

	device->run();
	smgr->drawAll();


	// This shouldn't cause a recalc
	camera->setUpVector(vector3df(1.f, 0.f, .01f).normalize());
	device->run();
	driver->beginScene(true, true, video::SColor(255,100,101,140));
	smgr->drawAll();
	driver->endScene();

	// Note that this has to be a slightly fuzzier than usual compare to satisfy multiple OpenGL environments
	bool result = takeScreenshotAndCompareAgainstReference(driver, "-terrainSceneNode-1.png", 97.98f);
	if(!result)
	{
		logTestString("Small camera up rotation caused bad recalc.\n");
	}

	// This is big enough to cause a recalc
	camera->setUpVector(vector3df(1.f, 0.f, .1f).normalize());
	device->run();
	driver->beginScene(true, true, video::SColor(255,100,101,140));
	smgr->drawAll();
	driver->endScene();

	result &= takeScreenshotAndCompareAgainstReference(driver, "-terrainSceneNode-2.png", 98.38f);
	if(!result)
	{
		logTestString("Large camera up rotation caused bad recalc.\n");
	}

	device->closeDevice();
	device->run();
	device->drop();
	return result;
}

bool terrainGaps()
{
	IrrlichtDevice* device = createDevice(video::EDT_BURNINGSVIDEO, dimension2d<u32>(160, 120));
	if (!device)
		return true;

	video::IVideoDriver * irrVideo = device->getVideoDriver();
	scene::ISceneManager* irrScene = device->getSceneManager();

	// Add camera
	scene::ICameraSceneNode* camera = irrScene->addCameraSceneNode();
	camera->setPosition(vector3df(20000, 500, 12800));
	camera->setTarget(vector3df(16800, 0, 12800));
	camera->setFarValue(42000.0f);

	// Add terrain scene node
	for (u32 i = 0; i < 2; i++)
	{
		const char* name="media/ter1.png";
		scene::ITerrainSceneNode* terrain = irrScene->addTerrainSceneNode(
				name, 0, -1,
				vector3df((f32)(256*50), 0.f, (f32)(i*256*50)),// position 12800(==imgsize*scale)
				vector3df(0.f, 0.f, 0.f), // rotation
				vector3df(50.f, 15.0f, 50.f) // scale 50 15 50
		);

		terrain->setMaterialFlag(video::EMF_LIGHTING, false);
		terrain->setMaterialFlag(video::EMF_WIREFRAME, !terrain->getMaterial(0).Wireframe);
	}
   
	irrVideo->beginScene(true, true, video::SColor(0,150,150,150));
	irrScene->drawAll();
	irrVideo->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(irrVideo, "-terrainGap.png");

	device->closeDevice();
	device->run();
	device->drop();
	return result;
}

}

bool terrainSceneNode()
{
	bool result = terrainRecalc();
	result &= terrainGaps();
	return result;
}

