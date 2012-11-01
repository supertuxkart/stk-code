// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;

/** Tests that the geometry creator does what it says it does. */
bool testGeometryCreator(void)
{
    IrrlichtDevice *device = createDevice(video::EDT_BURNINGSVIDEO,
										core::dimension2du(160,120), 32);
    if (!device)
        return false;

    IVideoDriver* driver = device->getVideoDriver();
    ISceneManager* smgr = device->getSceneManager();
	(void)smgr->addCameraSceneNode(0, vector3df(0, 0, -50));

	const IGeometryCreator * geom = smgr->getGeometryCreator();

	ITexture * wall = driver->getTexture("../media/wall.bmp");

	SMaterial material;
	material.Lighting = false;
	material.TextureLayer[0].Texture = wall;

	irr::scene::IMesh * meshHill = geom->createHillPlaneMesh(dimension2df(10, 5), dimension2du(5, 5), 
									&material, 10, dimension2df(2, 2), dimension2df(3, 3) );
	IMeshSceneNode * node = smgr->addMeshSceneNode(meshHill, 0, -1,
									vector3df(0, 10, 0), vector3df(-60, 0, 0));
	meshHill->drop();

	irr::scene::IMesh * meshArrow = geom->createArrowMesh(4, 8, 10, 6, 3, 6,
								SColor(255, 255, 0, 0), SColor(255, 0, 255, 0));
	node = smgr->addMeshSceneNode(meshArrow, 0, -1, vector3df(-10, -20, 0));
	node->setMaterialFlag(video::EMF_LIGHTING, false);
	meshArrow->drop();

	irr::scene::IMesh * meshCone = geom->createConeMesh(5.f, 10.f, 16);
	node = smgr->addMeshSceneNode(meshCone, 0, -1, vector3df(-35, -20, 0));
	node->setMaterialFlag(video::EMF_LIGHTING, false);
	node->setMaterialTexture(0, wall);
	meshCone->drop();

	irr::scene::IMesh * meshCube = geom->createCubeMesh();
	node = smgr->addMeshSceneNode(meshCube, 0, -1, vector3df(-20, -20, 0));
	node->setMaterialFlag(video::EMF_LIGHTING, false);
	node->setMaterialTexture(0, wall);
	meshCube->drop();

	irr::scene::IMesh * meshCylinder = geom->createCylinderMesh(3, 10, 16);
	node = smgr->addMeshSceneNode(meshCylinder, 0, -1, vector3df(0, -20, 10), core::vector3df(45,0,0));
	node->setMaterialFlag(video::EMF_LIGHTING, false);
	node->setMaterialTexture(0, wall);
	meshCylinder->drop();

	irr::scene::IMesh * meshSphere = geom->createSphereMesh();
	node = smgr->addMeshSceneNode(meshSphere, 0, -1, vector3df(10, -15, 0));
	node->setMaterialFlag(video::EMF_LIGHTING, false);
	node->setMaterialTexture(0, wall);
	meshSphere->drop();

	irr::scene::IMesh * meshVolumeLight = geom->createVolumeLightMesh();
	node = smgr->addMeshSceneNode(meshVolumeLight, 0, -1, vector3df(20, -20, -10));
	node->setMaterialFlag(video::EMF_LIGHTING, false);
	node->setMaterialTexture(0, wall);
	node->setScale(core::vector3df(4.f,4.f,4.f));
	meshVolumeLight->drop();

	bool result = false;
	device->run();
	if (driver->beginScene(true, true, video::SColor(0, 80, 80, 80)))
	{
		smgr->drawAll();
		driver->endScene();
		result = takeScreenshotAndCompareAgainstReference(driver, "-testGeometryCreator.png", 99.994f);
	}

	smgr->clear();

	driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);

	// add camera
	scene::ICameraSceneNode* camera = smgr->addCameraSceneNodeFPS(0,100.0f,2.0f);
	camera->setPosition(core::vector3df(2000.0f,5000.f,0.0f));
	camera->setTarget(core::vector3df(0.0f,0.0f,0.0f));
	camera->setFarValue(20000.0f);
	device->getCursorControl()->setVisible(false); // disable mouse cursor

	video::IImage* colorMapImage = driver->createImageFromFile("../media/terrain-texture.jpg");
	video::IImage* heightMapImage = driver->createImageFromFile("../media/terrain-heightmap.bmp");

	scene::IAnimatedMesh* terrain = smgr->addTerrainMesh("TerrainMeshName", colorMapImage, heightMapImage,
		core::dimension2d<f32>(40, 40), // size of a pixel
		8*40); // maximum height
	colorMapImage->drop();
	colorMapImage = 0;
	heightMapImage->drop();
	heightMapImage = 0;

	scene::IAnimatedMeshSceneNode* anode = smgr->addAnimatedMeshSceneNode(terrain);
	if (anode)
	{
		anode->setMaterialFlag(video::EMF_LIGHTING, false);
		anode->setPosition(core::vector3df(-5000,0,-5000));
	}

	driver->beginScene();
	smgr->drawAll();
	driver->endScene();

	// This screenshot shows some mipmap problems, but this seems to be
	// no fault of the mesh
	result = takeScreenshotAndCompareAgainstReference(driver, "-testTerrainMesh.png", 99.989f);

	device->closeDevice();
	device->run();
    device->drop();

    return result;
}
