// Copyright (C) 2008-2012 Christian Stehno, Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;

//! Tests rendering MRTs
static bool testWithDriver(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice (driverType, core::dimension2d < u32 > (220, 80));
	if (!device)
		return true; // No error if device does not exist

	video::IVideoDriver* driver = device->getVideoDriver();
	// We need at least GLSL 1.10 or HLSL 1.1
//	if (driver->getDriverAttributes().getAttributeAsInt("ShaderLanguageVersion")<=100)
//		return true;

	stabilizeScreenBackground(driver);

	logTestString("Testing driver %ls\n", driver->getName());

	const char* const ps1="struct PS_INPUT\n {\n float4 Position      : POSITION0;\n };\n\n struct PS_OUTPUT\n {\n float4   Color      : COLOR0;\n float4   Normal      : COLOR1;\n float4   Depth      : COLOR2;\n };\n PS_OUTPUT pixelMain( PS_INPUT Input )\n {\n PS_OUTPUT Output;\n Output.Color = float4(1.0,1.0,1.0,1.0);\n Output.Normal = float4(0.0,1.0,0.0,1.0);\n Output.Depth = float4(0.0,0.0,1.0,1.0);\n return Output;\n }";
	const char* const ps2="void main(void)\n {\n gl_FragData[0] = vec4(1.0,1.0,1.0,1.0);\n gl_FragData[1] = vec4(0.0,1.0,0.0,1.0);\n gl_FragData[2] = vec4(0.0,0.0,1.0,1.0);\n }";

	// variable
	video::ITexture* gbuffer[3];
	core::array<video::IRenderTarget> gbufferlist;
	const core::dimension2du texsize(64,64);
	bool result=true;
	s32 newMaterialType = -1;

	if (device->getVideoDriver()->getDriverAttributes().getAttributeAsInt("MaxMultipleRenderTargets") > 2)
	{
		// allocate buffer
		gbuffer[0] = driver->addRenderTargetTexture(texsize, "rta", video::ECF_A8R8G8B8);
		gbuffer[1] = driver->addRenderTargetTexture(texsize, "rtb", video::ECF_A8R8G8B8);
		gbuffer[2] = driver->addRenderTargetTexture(texsize, "rtc", video::ECF_A8R8G8B8);
		for( u32 i = 0; i < 3; ++i )
		   gbufferlist.push_back( video::IRenderTarget(gbuffer[i]) );

		video::IGPUProgrammingServices* gpu = driver->getGPUProgrammingServices();

		if (gpu)
		{
			newMaterialType = gpu->addHighLevelShaderMaterial(
				0, "vertexMain", video::EVST_VS_1_1,
				driverType==video::EDT_DIRECT3D9?ps1:ps2, "pixelMain", video::EPST_PS_1_1);
		}
	}

	// shader creation succeeded
	if (newMaterialType!=-1)
	{
		scene::ISceneNode* node = device->getSceneManager()->addCubeSceneNode();
		node->setMaterialType((video::E_MATERIAL_TYPE)newMaterialType);
		device->getSceneManager()->addCameraSceneNode(0, core::vector3df(0,0,-10));

		driver->beginScene (true, true, video::SColor (255, 200, 200, 200));
		// render
		driver->setRenderTarget( gbufferlist );
		device->getSceneManager()->drawAll();
		driver->setRenderTarget(0);

		// draw debug rt
		driver->draw2DImage(gbuffer[0], core::position2d<s32>(0,0));
		driver->draw2DImage(gbuffer[1], core::position2d<s32>(64,0));
		driver->draw2DImage(gbuffer[2], core::position2d<s32>(128,0)); 

		driver->endScene();

		result = takeScreenshotAndCompareAgainstReference(driver, "-mrt.png");

		driver->beginScene (true, true, video::SColor (255, 200, 200, 200));
		// render
		device->getSceneManager()->getActiveCamera()->setPosition(core::vector3df(0,0,-15));
		driver->setRenderTarget( gbufferlist );
		device->getSceneManager()->drawAll();
		driver->setRenderTarget(0);

		// draw debug rt
		driver->draw2DImage(gbuffer[0], core::position2d<s32>(0,0));
		driver->draw2DImage(gbuffer[1], core::position2d<s32>(64,0));
		driver->draw2DImage(gbuffer[2], core::position2d<s32>(128,0)); 

		driver->endScene();

		result &= takeScreenshotAndCompareAgainstReference(driver, "-mrt2.png");
	}

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}


bool mrt(void)
{
	bool result = true;

	TestWithAllHWDrivers(testWithDriver);

	return result;
}
