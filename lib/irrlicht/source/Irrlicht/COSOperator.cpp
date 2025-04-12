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
#ifdef _IRR_COMPILE_WITH_OSX_DEVICE_
#include "MacOSX/OSXClipboard.h"
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
#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
void COSOperator::copyToClipboard(const wchar_t* text) const
{
	if (wcslen(text)==0)
		return;

// Windows version
#if defined(_IRR_XBOX_PLATFORM_)
#elif defined(_IRR_WINDOWS_API_)
	if (!OpenClipboard(NULL) || text == 0)
		return;

	EmptyClipboard();

	HGLOBAL clipbuffer;
	wchar_t * buffer;

	clipbuffer = GlobalAlloc(GMEM_DDESHARE, wcslen(text)*sizeof(wchar_t) + sizeof(wchar_t));
	buffer = (wchar_t*)GlobalLock(clipbuffer);

	wcscpy(buffer, text);

	GlobalUnlock(clipbuffer);
	SetClipboardData(CF_UNICODETEXT, clipbuffer); //Windwos converts between CF_UNICODETEXT and CF_TEXT automatically.
	CloseClipboard();

#else

#endif
}
#else
void COSOperator::copyToClipboard(const c8* text) const
{
	if (strlen(text)==0)
		return;

// Windows version
#if defined(_IRR_XBOX_PLATFORM_)
#elif defined(_IRR_WINDOWS_API_)
	if (!OpenClipboard(NULL) || text == 0)
		return;

	EmptyClipboard();

	HGLOBAL clipbuffer;
	char * buffer;

	clipbuffer = GlobalAlloc(GMEM_DDESHARE, strlen(text)+1);
	buffer = (char*)GlobalLock(clipbuffer);

	strcpy(buffer, text);

	GlobalUnlock(clipbuffer);
	SetClipboardData(CF_TEXT, clipbuffer);
	CloseClipboard();

// MacOSX version
#elif defined(_IRR_COMPILE_WITH_OSX_DEVICE_)

	OSXCopyToClipboard(text);
	
#elif defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
	SDL_SetClipboardText(text);
#endif
}
#endif


//! gets text from the clipboard
//! \return Returns 0 if no string is in there.
#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
const wchar_t* COSOperator::getTextFromClipboard() const
{
#if defined(_IRR_XBOX_PLATFORM_)
		return 0;
#elif defined(_IRR_WINDOWS_API_)
	if (!OpenClipboard(NULL))
		return 0;

	wchar_t * buffer = 0;

	HANDLE hData = GetClipboardData( CF_UNICODETEXT ); //Windwos converts between CF_UNICODETEXT and CF_TEXT automatically.
	buffer = (wchar_t*)GlobalLock( hData );
	GlobalUnlock( hData );
	CloseClipboard();
	return buffer;

#else

	return 0;
#endif
}
#else
const c8* COSOperator::getTextFromClipboard() const
{
#if defined(_IRR_XBOX_PLATFORM_)
		return 0;
#elif defined(_IRR_WINDOWS_API_)
	if (!OpenClipboard(NULL))
		return 0;

	char * buffer = 0;

	HANDLE hData = GetClipboardData( CF_TEXT );
	buffer = (char*)GlobalLock( hData );
	GlobalUnlock( hData );
	CloseClipboard();
	return buffer;

#elif defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
	return (OSXCopyFromClipboard());
	
	

#elif defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
	return SDL_GetClipboardText();
#else

	return 0;
#endif
}
#endif


} // end namespace

