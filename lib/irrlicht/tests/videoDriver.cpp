// Copyright (C) 2008-2012 Colin MacDonald, Christian Stehno
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;

/** Test various things in video drivers. */
bool testVideoDriver(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device =
		createDevice(driverType, dimension2d<u32>(160, 120));

	if (!device)
		return true;

	video::IVideoDriver* driver = device->getVideoDriver();
	logTestString("Testing driver %ls\n", driver->getName());
	logTestString("MaxTextures: %d\n", driver->getDriverAttributes().getAttributeAsInt("MaxTextures"));
	logTestString("MaxSupportedTextures: %d\n", driver->getDriverAttributes().getAttributeAsInt("MaxSupportedTextures"));
	logTestString("MaxLights: %d\n", driver->getDriverAttributes().getAttributeAsInt("MaxLights"));
	logTestString("MaxAnisotropy: %d\n", driver->getDriverAttributes().getAttributeAsInt("MaxAnisotropy"));
	logTestString("MaxUserClipPlanes: %d\n", driver->getDriverAttributes().getAttributeAsInt("MaxUserClipPlanes"));
	logTestString("MaxAuxBuffers: %d\n", driver->getDriverAttributes().getAttributeAsInt("MaxAuxBuffers"));
	logTestString("MaxMultipleRenderTargets: %d\n", driver->getDriverAttributes().getAttributeAsInt("MaxMultipleRenderTargets"));
	logTestString("MaxIndices: %d\n", driver->getDriverAttributes().getAttributeAsInt("MaxIndices"));
	logTestString("MaxTextureSize: %d\n", driver->getDriverAttributes().getAttributeAsInt("MaxTextureSize"));
	logTestString("MaxGeometryVerticesOut: %d\n", driver->getDriverAttributes().getAttributeAsInt("MaxGeometryVerticesOut"));
	logTestString("Version: %d\n", driver->getDriverAttributes().getAttributeAsInt("Version"));
	logTestString("ShaderLanguageVersion: %d\n\n", driver->getDriverAttributes().getAttributeAsInt("ShaderLanguageVersion"));

	device->closeDevice();
	device->run();
	device->drop();
	return true;
}

bool videoDriver()
{
	bool result = true;
	TestWithAllDrivers(testVideoDriver);
	return result;
}
