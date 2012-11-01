// Copyright (C) 2008-2012 Christian Stehno, Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;

namespace{

class MyEventReceiver : public IEventReceiver
{
public:
   // This is the one method that we have to implement
   virtual bool OnEvent(const SEvent& event)
   {
      // Remember whether each key is down or up
      if (event.EventType == EET_KEY_INPUT_EVENT)
         KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;

      return false;
   }

   // This is used to check whether a key is being held down
   virtual bool IsKeyDown(EKEY_CODE keyCode) const
   {
      return KeyIsDown[keyCode];
   }

   MyEventReceiver()
   {
      for (u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
         KeyIsDown[i] = false;
   }

private:
   // We use this array to store the current state of each key
   bool KeyIsDown[KEY_KEY_CODES_COUNT];
};

//! Tests using octree selector
bool octree()
{
	IrrlichtDevice *device = createDevice (video::EDT_OPENGL, core::dimension2d < u32 > (160, 120));
	if (!device)
		return true; // No error if device does not exist

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();

	stabilizeScreenBackground(driver);

	scene::IMetaTriangleSelector * meta = smgr->createMetaTriangleSelector();

	device->getFileSystem()->addFileArchive("../media/map-20kdm2.pk3");
	scene::IAnimatedMesh* q3levelmesh = smgr->getMesh("20kdm2.bsp");
	if (q3levelmesh)
	{
		scene::ISceneNode* q3node = smgr->addOctreeSceneNode(q3levelmesh->getMesh(0));

		q3node->setPosition(core::vector3df(-1350,-130,-1400));

		scene::ITriangleSelector * selector =
			smgr->createOctreeTriangleSelector(q3levelmesh->getMesh(0), q3node, 128);
		meta->addTriangleSelector(selector);
		selector->drop();
	}

	scene::ICameraSceneNode* camera = smgr->addCameraSceneNode();
	camera->setPosition(core::vector3df(-100,50,-150));
	camera->updateAbsolutePosition();
	camera->setTarget(camera->getAbsolutePosition() + core::vector3df(0, 0, 20));

	device->getCursorControl()->setVisible(false);

	enum
	{
		MAX_TRIANGLES = 4096, // Large to test getting all the triangles
		BOX_SIZE1 = 300,
		BOX_SIZE2 = 50
	};

	core::triangle3df triangles[MAX_TRIANGLES];
	core::vector3df boxPosition(camera->getAbsolutePosition());

	video::SMaterial unlit;
	unlit.Lighting = false;
	unlit.Thickness = 3.f;
	unlit.PolygonOffsetFactor=1;

	bool result = true;
	{
		camera->setPosition(core::vector3df(-620,-20,550));
		driver->beginScene(true, true, 0);
		smgr->drawAll();

		core::aabbox3df box(boxPosition.X - BOX_SIZE1, boxPosition.Y - BOX_SIZE1, boxPosition.Z - BOX_SIZE1,
		boxPosition.X + BOX_SIZE1, boxPosition.Y + BOX_SIZE1, boxPosition.Z + BOX_SIZE1);

		driver->setTransform(video::ETS_WORLD, core::matrix4());
		driver->setMaterial(unlit);
		driver->draw3DBox(box, video::SColor(255, 0, 255, 0));

		if(meta)
		{
			s32 found;
			meta->getTriangles(triangles, MAX_TRIANGLES, found, box);

			while(--found >= 0)
				driver->draw3DTriangle(triangles[found], video::SColor(255, 255, 0, 0));
		}

		driver->endScene();
		result &= takeScreenshotAndCompareAgainstReference(driver, "-octree_select1.png");
	}
	{
		camera->setPosition(core::vector3df(120,40,50));
		driver->beginScene(true, true, 0);
		smgr->drawAll();

		core::aabbox3df box(boxPosition.X - BOX_SIZE2, boxPosition.Y - BOX_SIZE2, boxPosition.Z - BOX_SIZE2,
		boxPosition.X + BOX_SIZE2, boxPosition.Y + BOX_SIZE2, boxPosition.Z + BOX_SIZE2);

		driver->setTransform(video::ETS_WORLD, core::matrix4());
		driver->setMaterial(unlit);
		driver->draw3DBox(box, video::SColor(255, 0, 255, 0));

		if(meta)
		{
			s32 found;
			meta->getTriangles(triangles, MAX_TRIANGLES, found, box);

			while(--found >= 0)
				driver->draw3DTriangle(triangles[found], video::SColor(255, 255, 0, 0));
		}

		driver->endScene();
		result &= takeScreenshotAndCompareAgainstReference(driver, "-octree_select2.png");
	}

	meta->drop();

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

//! Tests using triangle selector
bool triangle()
{
	IrrlichtDevice *device = createDevice (video::EDT_OPENGL, core::dimension2d < u32 > (160, 120));
	if (!device)
		return true; // No error if device does not exist

	MyEventReceiver receiver;
	device->setEventReceiver(&receiver);

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();

	stabilizeScreenBackground(driver);

	scene::IMetaTriangleSelector * meta = smgr->createMetaTriangleSelector();

	scene::IAnimatedMesh * mesh = smgr->getMesh("../media/sydney.md2");
	scene::IAnimatedMeshSceneNode * sydney = smgr->addAnimatedMeshSceneNode( mesh );
	if (sydney)
	{
		sydney->setPosition(core::vector3df(15, -10, 15));
		sydney->setMaterialFlag(video::EMF_LIGHTING, false);
		sydney->setMD2Animation ( scene::EMAT_STAND );
		sydney->setAnimationSpeed(0.f);
		sydney->setMaterialTexture( 0, driver->getTexture("../media/sydney.bmp") );

		scene::ITriangleSelector * selector =
		smgr->createTriangleSelector(sydney->getMesh()->getMesh(0), sydney);
		meta->addTriangleSelector(selector);
		selector->drop();
	}

	scene::ICameraSceneNode* camera =
	smgr->addCameraSceneNodeFPS();
	camera->setPosition(core::vector3df(70,0,-30));
	camera->updateAbsolutePosition();
	camera->setTarget(camera->getAbsolutePosition() + core::vector3df(-20, 0, 20));

	device->getCursorControl()->setVisible(false);

	enum
	{
		MAX_TRIANGLES = 5000, // Large to test getting all the triangles
		BOX_SIZE = 30
	};

	core::triangle3df triangles[MAX_TRIANGLES];
	core::vector3df boxPosition(0,0,0);

	video::SMaterial unlit;
	unlit.Lighting = false;
	unlit.Thickness = 3.f;
	unlit.PolygonOffsetFactor=1;

	bool result = true;
	{
		driver->beginScene(true, true, 0xff00ffff);
		smgr->drawAll();

		core::aabbox3df box(boxPosition.X - BOX_SIZE, boxPosition.Y - BOX_SIZE, boxPosition.Z - BOX_SIZE,
		boxPosition.X + BOX_SIZE, boxPosition.Y + BOX_SIZE, boxPosition.Z + BOX_SIZE);

		driver->setTransform(video::ETS_WORLD, core::matrix4());
		driver->setMaterial(unlit);
		driver->draw3DBox(box, video::SColor(255, 0, 255, 0));

		if(meta)
		{
			s32 found;
			meta->getTriangles(triangles, MAX_TRIANGLES, found, box);

			while(--found >= 0)
				driver->draw3DTriangle(triangles[found], video::SColor(255, 255, 0, 0));
		}

		driver->endScene();
		result &= takeScreenshotAndCompareAgainstReference(driver, "-tri_select1.png");
	}
	{
		boxPosition.Z -= 10.f;
		driver->beginScene(true, true, 0xff00ffff);
		smgr->drawAll();

		core::aabbox3df box(boxPosition.X - BOX_SIZE, boxPosition.Y - BOX_SIZE, boxPosition.Z - BOX_SIZE,
		boxPosition.X + BOX_SIZE, boxPosition.Y + BOX_SIZE, boxPosition.Z + BOX_SIZE);

		driver->setTransform(video::ETS_WORLD, core::matrix4());
		driver->setMaterial(unlit);
		driver->draw3DBox(box, video::SColor(255, 0, 255, 0));

		if(meta)
		{
			s32 found;
			meta->getTriangles(triangles, MAX_TRIANGLES, found, box);

			while(--found >= 0)
				driver->draw3DTriangle(triangles[found], video::SColor(255, 255, 0, 0));
		}

		driver->endScene();
		result &= takeScreenshotAndCompareAgainstReference(driver, "-tri_select2.png");
	}
	{
		boxPosition.Z -= 20.f;
		driver->beginScene(true, true, 0xff00ffff);
		smgr->drawAll();

		core::aabbox3df box(boxPosition.X - BOX_SIZE, boxPosition.Y - BOX_SIZE, boxPosition.Z - BOX_SIZE,
		boxPosition.X + BOX_SIZE, boxPosition.Y + BOX_SIZE, boxPosition.Z + BOX_SIZE);

		driver->setTransform(video::ETS_WORLD, core::matrix4());
		driver->setMaterial(unlit);
		driver->draw3DBox(box, video::SColor(255, 0, 255, 0));

		if(meta)
		{
			s32 found;
			meta->getTriangles(triangles, MAX_TRIANGLES, found, box);

			while(--found >= 0)
				driver->draw3DTriangle(triangles[found], video::SColor(255, 255, 0, 0));
		}

		driver->endScene();
		result &= takeScreenshotAndCompareAgainstReference(driver, "-tri_select3.png");
	}

	meta->drop();

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}
}

// Tests need not be accurate, as we just need to include at least
// the triangles that match the criteria. But we try to be as close
// as possible of course, to reduce the collision checks done afterwards
bool triangleSelector(void)
{
	bool result = true;

	result &= octree();
	result &= triangle();

	return result;
}
