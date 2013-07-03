// Copyright (C) 2008-2012 Christian Stehno, Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;

//! Tests rendering RTTs with draw2DImage
/** This test is very special in its setup, problematic situation was found by stefbuet. */
static bool testWith2DImage(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice(driverType, core::dimension2d<u32> (128, 128));
	if (!device)
		return true; // No error if device does not exist

	video::IVideoDriver *driver = device->getVideoDriver ();
	scene::ISceneManager *smgr = device->getSceneManager ();

	if (!driver->queryFeature(video::EVDF_RENDER_TO_TARGET))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	stabilizeScreenBackground(driver);

	logTestString("Testing driver %ls\n", driver->getName());

	video::ITexture *image = driver->getTexture ("../media/irrlichtlogo2.png");
	video::ITexture *RTT_texture = driver->addRenderTargetTexture (core::dimension2d < u32 > (128, 128));

	smgr->addCameraSceneNode (0, core::vector3df (100, 100, 100),
			      core::vector3df (0, 0, 0));

	/*to reproduce the bug :
	-draw the image : it's normal
	-apply an RTT texture to a model
	-remove the model
	-draw the image again : it's flipped
	*/

	video::SColor colors[4]={0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
	//draw the image :
	driver->beginScene (true, true, video::SColor (255, 200, 200, 200));
	driver->draw2DImage (image,
		       core::rect < s32 >
		       (64 - image->getSize ().Width / 2,
			64 - image->getSize ().Height / 2,
			64 + image->getSize ().Width / 2,
			64 + image->getSize ().Height / 2),
		       core::rect < s32 > (0, 0, image->getSize ().Width,
					   image->getSize ().Height), 0, colors,
		       true);
	driver->endScene ();

	//then create a model and apply to it the RTT Texture
	//rendering the model is important, if not rendered 1 time, bug won't appear.
	//after the render, we remove the node : important, if not done, bug won't appear too.
	scene::IMesh *modelMesh = smgr->getMesh ("../media/earth.x");
	scene::ISceneNode *modelNode = smgr->addMeshSceneNode(modelMesh);
	modelNode->setMaterialTexture (0, RTT_texture);

	driver->beginScene (true, true, video::SColor (255, 200, 200, 200));
	smgr->drawAll();
	driver->endScene();

	modelNode->remove();

	//then we render the image normaly
	//it's now fliped...
	for (u32 i=0; i<10; ++i)
	{
		driver->beginScene (true, true, video::SColor (255, 200, 200, 200));

		//draw img
		driver->draw2DImage (image,
				   core::rect < s32 >
				   (64 - image->getSize ().Width / 2,
				    64 - image->getSize ().Height / 2,
				    64 + image->getSize ().Width / 2,
				    64 + image->getSize ().Height / 2),
				   core::rect < s32 > (0, 0, image->getSize ().Width,
						       image->getSize ().Height), 0,
				   colors, true);

		//call this is important :
		//if not called, the bug won't appear
		smgr->drawAll();

		driver->endScene();
	}

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-rttWith2DImage.png", 99.9f);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}


bool rttAndZBuffer(video::E_DRIVER_TYPE driverType)
{
	SIrrlichtCreationParameters cp;
	cp.WindowSize.set(160,120);
	cp.Bits = 32;
	cp.AntiAlias = 4;
	cp.DriverType = driverType;

	IrrlichtDevice* nullDevice = createDevice(video::EDT_NULL);
	cp.WindowSize = nullDevice->getVideoModeList()->getDesktopResolution();
	nullDevice->closeDevice();
	nullDevice->run();
	nullDevice->drop();

	cp.WindowSize -= core::dimension2d<u32>(100, 100);

	IrrlichtDevice* device = createDeviceEx(cp);
	if (!device)
		return true;

	video::IVideoDriver* vd = device->getVideoDriver();
	scene::ISceneManager* sm = device->getSceneManager();

	if	(!vd->queryFeature(video::EVDF_RENDER_TO_TARGET))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	stabilizeScreenBackground(vd);

	logTestString("Testing driver %ls\n", vd->getName());

	video::ITexture* rt = vd->addRenderTargetTexture(cp.WindowSize, "rt", video::ECF_A32B32G32R32F);
	video::S3DVertex vertices[4];
	vertices[0].Pos.Z = vertices[1].Pos.Z = vertices[2].Pos.Z = vertices[3].Pos.Z = 1.0f;
	vertices[0].Pos.Y = vertices[1].Pos.Y = 1.0f;
	vertices[2].Pos.Y = vertices[3].Pos.Y = -1.0f;
	vertices[0].Pos.X = vertices[3].Pos.X = -1.0f;
	vertices[1].Pos.X = vertices[2].Pos.X = 1.0f;
	vertices[0].TCoords.Y = vertices[1].TCoords.Y = 0.0f;
	vertices[2].TCoords.Y = vertices[3].TCoords.Y = 1.0f;
	vertices[0].TCoords.X = vertices[3].TCoords.X = 1.0f;
	vertices[1].TCoords.X = vertices[2].TCoords.X = 0.0f;

	u16 indices[6] = {0, 1, 3, 1, 2, 3};

	video::SMaterial rtMat;
	rtMat.BackfaceCulling = false;
	rtMat.Lighting = false;
	rtMat.TextureLayer[0].TextureWrapU =
		rtMat.TextureLayer[0].TextureWrapV = video::ETC_CLAMP_TO_EDGE;

	sm->addLightSceneNode(NULL, core::vector3df(0, 50, 0),
		video::SColorf(1, 1, 1), 100);

	sm->addCameraSceneNode(NULL, core::vector3df(0, 10, 0));

	const scene::IGeometryCreator* geom = sm->getGeometryCreator();
	scene::IMeshManipulator* manip = sm->getMeshManipulator();
	scene::IMesh* mesh;
	scene::ISceneNode* node;
   
	mesh = geom->createCubeMesh(core::vector3df(10, 10, 10));
	manip->setVertexColors(mesh, video::SColor(255, 0, 0, 255));
	node = sm->addMeshSceneNode(mesh, NULL, -1, core::vector3df(0, 0, 30));
	node->getMaterial(0).EmissiveColor = video::SColor(255, 0, 0, 30);
	mesh->drop();

	mesh = geom->createSphereMesh(5.0f, 32, 32);
	node = sm->addMeshSceneNode(mesh, NULL, -1, core::vector3df(0, 0, 50));
	node->getMaterial(0).EmissiveColor = video::SColor(255, 30, 30, 30);
	mesh->drop();

	mesh = geom->createConeMesh(5.0f, 10.0f, 32, video::SColor(255, 255, 0, 0), video::SColor(255, 255, 0, 0));
	node = sm->addMeshSceneNode(mesh, NULL, -1, core::vector3df(0, 0, 70));
	node->getMaterial(0).EmissiveColor = video::SColor(255, 30, 0, 0);
	mesh->drop();

	{
		vd->beginScene(true, true, video::SColor(255, 0, 0, 0));
		vd->setRenderTarget(rt);
		sm->drawAll();
		vd->setRenderTarget(NULL);
		vd->setTransform(video::ETS_WORLD, core::IdentityMatrix);
		vd->setTransform(video::ETS_VIEW, core::IdentityMatrix);
		vd->setTransform(video::ETS_PROJECTION, core::IdentityMatrix);
		rtMat.setTexture(0, rt);
		vd->setMaterial(rtMat);
		vd->drawIndexedTriangleList(vertices, 4, indices, 2);
		vd->endScene();
	}
	bool result = takeScreenshotAndCompareAgainstReference(vd, "-rttAndZBuffer.png");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}


// result should be two times the same blind text on the left side, and
// the fireball image (with a very small text inside) in the middle of the screen
// drivers that don't support image scaling will show a pink background instead
bool rttAndText(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice* device = createDevice(driverType, core::dimension2d<u32>(160, 120));
	if (!device)
		return true;

	video::IVideoDriver* driver = device->getVideoDriver();
	gui::IGUIEnvironment* guienv = device->getGUIEnvironment();

	if (!driver->queryFeature(video::EVDF_RENDER_TO_TARGET))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}
	logTestString("Testing driver %ls\n", driver->getName());

	//RTT
	video::ITexture* rt = driver->addRenderTargetTexture(core::dimension2d<u32>(256, 256), "rt");
	if (!rt)
	{
		device->closeDevice();
		device->run();
		device->drop();
		return false;
	}

	stabilizeScreenBackground(driver);

	driver->beginScene(true, true, video::SColor(255,255, 255, 255));
	driver->setRenderTarget(rt, true, true, video::SColor(255,255,0,255));
	driver->draw2DImage(driver->getTexture("../media/fireball.bmp"), core::recti(0, 0,rt->getSize().Width,rt->getSize().Height), core::recti(0,0,64,64));
	guienv->getBuiltInFont()->draw(L"OMGGG =!", core::rect<s32>(120, 100, 256, 256), video::SColor(255, 0, 0, 255));
	driver->setRenderTarget(0);
	driver->endScene();

	scene::ISceneManager* smgr = device->getSceneManager();

	scene::ISceneNode* cube = smgr->addCubeSceneNode(20);
	cube->setMaterialFlag(video::EMF_LIGHTING, false);
	cube->setMaterialTexture(0, rt); // set material of cube to render target

	smgr->addCameraSceneNode(0, core::vector3df(0, 0, -30));

	// create a long text to produce much difference in failing result pictures
	gui::IGUIStaticText* text = guienv->addStaticText(L"asdddddddoamgmoasmgom\nfoaomsodommogdd\nddddddddd", core::rect<s32>(10, 20, 100, 160));

	driver->beginScene(true, true, video::SColor(255,255, 255, 255));
	cube->setVisible(false);
	smgr->drawAll();
	guienv->drawAll();

	cube->setVisible(true);
	smgr->drawAll();
	video::SMaterial mat(cube->getMaterial(0));
	driver->setMaterial(mat);
	text->setRelativePosition(core::position2di(10,30));
	guienv->drawAll();

	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-rttAndText.png");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

static void Render(IrrlichtDevice* device, video::ITexture* rt, core::vector3df& pos1, 
				   core::vector3df& pos2, scene::IAnimatedMesh* sphereMesh, core::vector3df& pos3, core::vector3df& pos4)
{
	video::IVideoDriver* driver = device->getVideoDriver();
	driver->setRenderTarget(rt);
	device->getSceneManager()->drawAll();

	video::SMaterial mat;
	mat.ColorMaterial=video::ECM_NONE;
	mat.AmbientColor.set(255, 80, 80, 80);
	mat.DiffuseColor.set(255, 120, 30, 210);
	mat.SpecularColor.set(255,80,80,80);
	mat.Shininess = 8.f;	

	core::matrix4 m;
	m.setTranslation(pos1);
	driver->setTransform(video::ETS_WORLD, m);
	driver->setMaterial(mat);
	driver->drawMeshBuffer(sphereMesh->getMeshBuffer(0));

	m.setTranslation(pos2);
	mat.Shininess=0.f;
	driver->setTransform(video::ETS_WORLD, m);
	driver->setMaterial(mat);		
	driver->drawMeshBuffer(sphereMesh->getMeshBuffer(0));

	m.setTranslation(pos3);
	mat.Shininess=8.f;
	driver->setTransform(video::ETS_WORLD, m);
	driver->setMaterial(mat);
	driver->drawMeshBuffer(sphereMesh->getMeshBuffer(0));

	m.setTranslation(pos4);
	mat.Shininess=0.f;
	driver->setTransform(video::ETS_WORLD, m);
	driver->setMaterial(mat);
	driver->drawMeshBuffer(sphereMesh->getMeshBuffer(0));
}

bool rttAndAntiAliasing(video::E_DRIVER_TYPE driverType)
{
	SIrrlichtCreationParameters cp;
	cp.DriverType = driverType;
	cp.WindowSize = core::dimension2di(160, 120);
	cp.AntiAlias = 2;
	cp.Vsync = true;

	IrrlichtDevice* device = createDeviceEx(cp);
	if (!device)
		return true;

	video::IVideoDriver* driver = device->getVideoDriver();
	if ((driver->getDriverAttributes().getAttributeAsInt("AntiAlias")<2) ||
		(!driver->queryFeature(video::EVDF_RENDER_TO_TARGET)))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	stabilizeScreenBackground(driver);

	logTestString("Testing driver %ls\n", driver->getName());

	// sphere mesh
	scene::IAnimatedMesh* sphereMesh = device->getSceneManager()->addSphereMesh("atom", 1, 32, 32);

	// cam
	scene::ICameraSceneNode* cam = device->getSceneManager()->addCameraSceneNode(NULL, core::vector3df(0, 1, -5), core::vector3df(0, 0, 0));
	cam->setNearValue(0.01f);
	cam->setFarValue(100.f);
	cam->updateAbsolutePosition();
	device->getSceneManager()->setActiveCamera(cam);
	device->getSceneManager()->addLightSceneNode(0, core::vector3df(10,10,10));
	device->getSceneManager()->setAmbientLight(video::SColorf(0.3f,0.3f,0.3f));

	float radius = 3.f;
	core::vector3df pos1(-radius,0,0);
	core::vector3df pos2(radius,0,0);
	core::vector3df pos3(0,0,radius);
	core::vector3df pos4(0,0,-radius);
	core::matrix4 m;

	gui::IGUIStaticText* st = device->getGUIEnvironment()->addStaticText(L"", core::recti(0,0,200,20), false, false);
	st->setOverrideColor(video::SColor(255,255,255,0));

	core::dimension2du dim_txt = core::dimension2du(160/2, 120/2);

	video::ITexture* rt1 = device->getVideoDriver()->addRenderTargetTexture(dim_txt, "rt1", device->getColorFormat());
	video::ITexture* rt2 = device->getVideoDriver()->addRenderTargetTexture(dim_txt, "rt2", device->getColorFormat());
	video::ITexture* rt3 = device->getVideoDriver()->addRenderTargetTexture(dim_txt, "rt3", video::ECF_A8R8G8B8);
	video::ITexture* rt4 = device->getVideoDriver()->addRenderTargetTexture(dim_txt, "rt4", device->getColorFormat());

	device->getSceneManager()->setActiveCamera(cam);
	device->getVideoDriver()->beginScene();
#if 1
	st->setText(L"Texture Rendering");
	Render(device, rt1, pos1, pos2, sphereMesh, pos3, pos4);
	Render(device, rt2, pos1, pos2, sphereMesh, pos3, pos4);
	Render(device, rt3, pos1, pos2, sphereMesh, pos3, pos4);
	Render(device, rt4, pos1, pos2, sphereMesh, pos3, pos4);

	device->getVideoDriver()->setRenderTarget(0);
	device->getVideoDriver()->draw2DImage(rt1, core::position2di(0,0));
	device->getVideoDriver()->draw2DImage(rt2, core::position2di(80,0));
	device->getVideoDriver()->draw2DImage(rt3, core::position2di(0,60));
	device->getVideoDriver()->draw2DImage(rt4, core::position2di(80,60));
#else
	ITexture* rt0 = NULL;
	Render(device, rt0, pos1, pos2, sphereMesh, pos3, pos4);
#endif
	st->draw();
	device->getVideoDriver()->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-rttAndAntiAlias.png");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

bool rttFormats(video::E_DRIVER_TYPE driverType)
{
	SIrrlichtCreationParameters cp;
	cp.DriverType = driverType;
	cp.WindowSize = core::dimension2di(160, 120);

	IrrlichtDevice* device = createDeviceEx(cp);
	if (!device)
		return true;

	video::IVideoDriver* driver = device->getVideoDriver();

	logTestString("Testing driver %ls\n", driver->getName());

	video::ITexture* tex = 0;
	
	{
		tex = device->getVideoDriver()->addRenderTargetTexture(core::dimension2du(256,256), "rt", video::ECF_A1R5G5B5);
		if (tex)
		{
			if (tex->getColorFormat() != video::ECF_A1R5G5B5)
				logTestString("Format changed: ECF_A1R5G5B5 to %x\n", tex->getColorFormat());
			else
				logTestString("Format supported: ECF_A1R5G5B5\n");
			driver->removeTexture(tex);
			tex=0;
		}
		else
			logTestString("Format unsupported: ECF_A1R5G5B5\n");
	}
	{
		tex = device->getVideoDriver()->addRenderTargetTexture(core::dimension2du(256,256), "rt", video::ECF_R5G6B5);
		if (tex)
		{
			if (tex->getColorFormat() != video::ECF_R5G6B5)
				logTestString("Format changed: ECF_R5G6B5 to %x\n", tex->getColorFormat());
			else
				logTestString("Format supported: ECF_R5G6B5\n");
			driver->removeTexture(tex);
			tex=0;
		}
		else
			logTestString("Format unsupported: ECF_R5G6B5\n");
	}
	{
		tex = device->getVideoDriver()->addRenderTargetTexture(core::dimension2du(256,256), "rt", video::ECF_R8G8B8);
		if (tex)
		{
			if (tex->getColorFormat() != video::ECF_R8G8B8)
				logTestString("Format changed: ECF_R8G8B8 to %x\n", tex->getColorFormat());
			else
				logTestString("Format supported: ECF_R8G8B8\n");
			driver->removeTexture(tex);
			tex=0;
		}
		else
			logTestString("Format unsupported: ECF_R8G8B8\n");
	}
	{
		tex = device->getVideoDriver()->addRenderTargetTexture(core::dimension2du(256,256), "rt", video::ECF_A8R8G8B8);
		if (tex)
		{
			if (tex->getColorFormat() != video::ECF_A8R8G8B8)
				logTestString("Format changed: ECF_A8R8G8B8 to %x\n", tex->getColorFormat());
			else
				logTestString("Format supported: ECF_A8R8G8B8\n");
			driver->removeTexture(tex);
			tex=0;
		}
		else
			logTestString("Format unsupported: ECF_A8R8G8B8\n");
	}
	{
		tex = device->getVideoDriver()->addRenderTargetTexture(core::dimension2du(256,256), "rt", video::ECF_R16F);
		if (tex)
		{
			if (tex->getColorFormat() != video::ECF_R16F)
				logTestString("Format changed: ECF_R16F to %x\n", tex->getColorFormat());
			else
				logTestString("Format supported: ECF_R16F\n");
			driver->removeTexture(tex);
			tex=0;
		}
		else
			logTestString("Format unsupported: ECF_R16F\n");
	}
	{
		tex = device->getVideoDriver()->addRenderTargetTexture(core::dimension2du(256,256), "rt", video::ECF_G16R16F);
		if (tex)
		{
			if (tex->getColorFormat() != video::ECF_G16R16F)
				logTestString("Format changed: ECF_G16R16F to %x\n", tex->getColorFormat());
			else
				logTestString("Format supported: ECF_G16R16F\n");
			driver->removeTexture(tex);
			tex=0;
		}
		else
			logTestString("Format unsupported: ECF_G16R16F\n");
	}
	{
		tex = device->getVideoDriver()->addRenderTargetTexture(core::dimension2du(256,256), "rt", video::ECF_A16B16G16R16F);
		if (tex)
		{
			if (tex->getColorFormat() != video::ECF_A16B16G16R16F)
				logTestString("Format changed: ECF_A16B16G16R16F to %x\n", tex->getColorFormat());
			else
				logTestString("Format supported: ECF_A16B16G16R16F\n");
			driver->removeTexture(tex);
			tex=0;
		}
		else
			logTestString("Format unsupported: ECF_A16B16G16R16F\n");
	}
	{
		tex = device->getVideoDriver()->addRenderTargetTexture(core::dimension2du(256,256), "rt", video::ECF_R32F);
		if (tex)
		{
			if (tex->getColorFormat() != video::ECF_R32F)
				logTestString("Format changed: ECF_R32F to %x\n", tex->getColorFormat());
			else
				logTestString("Format supported: ECF_R32F\n");
			driver->removeTexture(tex);
			tex=0;
		}
		else
			logTestString("Format unsupported: ECF_R32F\n");
	}
	{
		tex = device->getVideoDriver()->addRenderTargetTexture(core::dimension2du(256,256), "rt", video::ECF_G32R32F);
		if (tex)
		{
			if (tex->getColorFormat() != video::ECF_G32R32F)
				logTestString("Format changed: ECF_G32R32F to %x\n", tex->getColorFormat());
			else
				logTestString("Format supported: ECF_G32R32F\n");
			driver->removeTexture(tex);
			tex=0;
		}
		else
			logTestString("Format unsupported: ECF_G32R32F\n");
	}
	{
		tex = device->getVideoDriver()->addRenderTargetTexture(core::dimension2du(256,256), "rt", video::ECF_A32B32G32R32F);
		if (tex)
		{
			if (tex->getColorFormat() != video::ECF_A32B32G32R32F)
				logTestString("Format changed: ECF_A32B32G32R32F to %x\n", tex->getColorFormat());
			else
				logTestString("Format supported: ECF_A32B32G32R32F\n");
			driver->removeTexture(tex);
			tex=0;
		}
		else
			logTestString("Format unsupported: ECF_A32B32G32R32F\n");
	}

	device->closeDevice();
	device->run();
	device->drop();

	return true;
}

bool renderTargetTexture(void)
{
	bool result = true;

	TestWithAllDrivers(testWith2DImage);

#if 0
	TestWithAllDrivers(rttAndZBuffer);
#endif

	TestWithAllDrivers(rttAndAntiAliasing);
	TestWithAllDrivers(rttAndText);

	logTestString("Test RTT format support\n");
	TestWithAllHWDrivers(rttFormats);

	return result;
}
