#include "testUtils.h"

using namespace irr;

namespace
{

// this test renders random point clouds using different primitives on top
// tests the primitives type support in general and can hint to differences
// between the drivers
bool testWithDriver(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device =
		createDevice(driverType, core::dimension2du(160, 120));
	if (!device)
		return true;
	
	scene::ISceneManager* smgr = device->getSceneManager();
	video::IVideoDriver* driver = device->getVideoDriver();

	stabilizeScreenBackground(driver);

	logTestString("Testing driver %ls\n", driver->getName());

	smgr->addCameraSceneNode(0, core::vector3df(128,128,-100), core::vector3df(128,128,128));

	scene::SMeshBuffer Buffer;

	Buffer.Material.Wireframe = false;
	Buffer.Material.Lighting = false;
	Buffer.Material.FogEnable = false;
	Buffer.Material.BackfaceCulling = false;
	Buffer.Material.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA;

	device->getRandomizer()->reset();
	const u32 points=256;
	Buffer.Vertices.reallocate(points);
	for (u32 i=0; i<points; ++i)
	{
		f32 x = (f32)(1+device->getRandomizer()->rand()%points);
		f32 y = (f32)(1+device->getRandomizer()->rand()%points);
		f32 z = (f32)(1+device->getRandomizer()->rand()%points);
		video::SColor color(255, device->getRandomizer()->rand()%255, device->getRandomizer()->rand()%255, device->getRandomizer()->rand()%255);
		Buffer.Vertices.push_back( video::S3DVertex(x,y,z,0,1,0,color,0,0) );
	}
	Buffer.recalculateBoundingBox();
	for (u32 i=0; i<Buffer.Vertices.size(); ++i)
	{
		Buffer.Indices.push_back(i);
	}

	bool result = true;
	for (u32 Type=scene::EPT_POINTS; Type <= scene::EPT_POINT_SPRITES; ++Type)
	{
		driver->beginScene(true, true, video::SColor(255,100,101,140));
		smgr->drawAll();
		u32 primCount = 0;
		switch (Type)
		{
			case scene::EPT_POINTS: primCount = Buffer.Indices.size(); break;
			case scene::EPT_LINE_STRIP: primCount = Buffer.Indices.size()-1; break;
			case scene::EPT_LINE_LOOP: primCount = Buffer.Indices.size()-1; break;
			case scene::EPT_LINES: primCount = Buffer.Indices.size()/2; break;
			case scene::EPT_TRIANGLE_STRIP: primCount = Buffer.Indices.size()-2; break;
			case scene::EPT_TRIANGLE_FAN: primCount = Buffer.Indices.size()-2; break;
			case scene::EPT_TRIANGLES: primCount = Buffer.Indices.size()/3; break;
			case scene::EPT_QUAD_STRIP: primCount = (Buffer.Indices.size()-2)/4; break;
			case scene::EPT_QUADS: primCount = Buffer.Indices.size()/4; break;
			case scene::EPT_POLYGON: primCount = Buffer.Indices.size()-1; break;
			case scene::EPT_POINT_SPRITES: primCount = Buffer.Indices.size(); break;
			default: break;
		}
 
		// TODO: mode is buggy, but required for skybox. So driver supports it, but would core dump here.
		if (driverType==video::EDT_BURNINGSVIDEO && Type==scene::EPT_TRIANGLE_FAN)
			continue;
		driver->setMaterial(Buffer.Material);
		driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
		driver->drawVertexPrimitiveList(Buffer.getVertices(),
					Buffer.getVertexCount(), Buffer.getIndices(), primCount,
					video::EVT_STANDARD, (scene::E_PRIMITIVE_TYPE)Type,
					video::EIT_16BIT);
		driver->endScene();
		core::stringc name = "-drawVPL_";
		// we use character enumeration as we have more than 9 types
		name.append(Type-scene::EPT_POINTS+'a');
		name.append(".png");
		result &= takeScreenshotAndCompareAgainstReference(driver, name.c_str());
	}

	device->closeDevice();
	device->run();
	device->drop();

	return result ;
}
}

bool drawVertexPrimitive(void)
{
	bool result = true;
	TestWithAllDrivers(testWithDriver);
	return result;
}
