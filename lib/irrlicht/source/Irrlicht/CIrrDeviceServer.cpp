// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef SERVER_ONLY

#include "CIrrDeviceServer.h"
#include "MobileCursorControl.h"
#include "SIrrCreationParameters.h"

namespace irr
{

//! constructor
CIrrDeviceServer::CIrrDeviceServer(const SIrrlichtCreationParameters& param)
	: CIrrDeviceStub(param)
{
	#ifdef _DEBUG
	setDebugName("CIrrDeviceServer");
	#endif

	Operator = 0;
	CursorControl = new gui::MobileCursorControl();
	VideoDriver = video::createNullDriver(FileSystem, CreationParams.WindowSize);
	createGUIAndScene();
}

} // end namespace irr

#endif // SERVER_ONLY

