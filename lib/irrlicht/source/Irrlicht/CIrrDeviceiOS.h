// Copyright (C) 2002-2008 Nikolaus Gebhardt
// Copyright (C) 2008 Redshift Software, Inc.
// Copyright (C) 2012-2015 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_IRR_DEVICE_IOS_H_INCLUDED__
#define __C_IRR_DEVICE_IOS_H_INCLUDED__

#include "IrrCompileConfig.h"

#include <string>

namespace irr
{
namespace CIrrDeviceiOS
{
std::string getSystemLanguageCode();
void openURLiOS(const char* url);
void debugPrint(const char* line);
}
};

#endif
