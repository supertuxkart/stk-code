// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "COSOperator.h"

#ifdef _IRR_WINDOWS_API_
#ifndef _IRR_XBOX_PLATFORM_
#include <windows.h>
#endif
#else
#include <string.h>
#include <unistd.h>
#if !defined(_IRR_SOLARIS_PLATFORM_) && !defined(__CYGWIN__) && !defined(__HAIKU__)
#include <sys/param.h>
#include <sys/types.h>
#endif
#endif

#include <cassert>

#include "IrrlichtDevice.h"
#if defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
#include "SDL_clipboard.h"
#endif

namespace irr
{

// constructor  linux
   COSOperator::COSOperator(const core::stringc& osVersion, IrrlichtDevice* /*device*/)
: OperatingSystem(osVersion)
{
}


// constructor
COSOperator::COSOperator(const core::stringc& osVersion) 
: OperatingSystem(osVersion)
{
	#ifdef _DEBUG
	setDebugName("COSOperator");
	#endif
}


//! returns the current operating system version as string.
const core::stringc& COSOperator::getOperatingSystemVersion() const
{
	return OperatingSystem;
}

//! copies text to the clipboard
void COSOperator::copyToClipboard(const c8* text) const
{
	if (strlen(text)==0)
		return;

#if defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
	SDL_SetClipboardText(text);
#endif
	return;
}

//! gets text from the clipboard. result must be freed using SDL_free()
//! \return Returns an empty string on failure or 0 in absence of SDL device.
const c8* COSOperator::getTextFromClipboard() const
{
#if defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
	return SDL_GetClipboardText();
#else
	return 0;
#endif
}


} // end namespace

