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


void COSOperator::copyToClipboard(const char* text) const{
#if defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
	SDL_SetClipboardText(text);
	return;
//! copies text to the clipboard
#else
	if (strlen(text)==0)
		return;

// Windows version
#if defined(_IRR_XBOX_PLATFORM_)
#elif defined(_IRR_WINDOWS_API_)
	if (!OpenClipboard(NULL) || text == 0)
		return;

	EmptyClipboard();

	HGLOBAL clipbuffer;

	int widelen = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, text, -1, nullptr, 0);
	if(widelen > 0)
	{
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, widelen*sizeof(wchar_t));
		wchar_t* buffer = (wchar_t*)GlobalLock(clipbuffer);
		MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, text, -1, buffer, widelen);
		SetClipboardData(CF_UNICODETEXT, clipbuffer);
	}
	else
	{	// if MultiByteToWideChar fails, fallback to the old behaviour.
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, strlen(text)+1);
		char* buffer = (char*)GlobalLock(clipbuffer);
		strcpy(buffer, text);
		SetClipboardData(CF_TEXT, clipbuffer);
	}

	GlobalUnlock(clipbuffer);
	CloseClipboard();

// MacOSX version
#elif defined(_IRR_COMPILE_WITH_OSX_DEVICE_)

	OSXCopyToClipboard(text);
#endif
#endif
}

//! gets text from the clipboard
//! \return Returns 0 if no string is in there.
const char* COSOperator::getTextFromClipboard() const
{
#if defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
	return SDL_GetClipboardText();
#elif defined(_IRR_XBOX_PLATFORM_)
		return 0;
#elif defined(_IRR_WINDOWS_API_)
	if (!OpenClipboard(NULL))
		return 0;

	wchar_t * widebuffer = 0;
	char buffer[1445];

	HANDLE hData = GetClipboardData( CF_UNICODETEXT ); //Windwos converts between CF_UNICODETEXT and CF_TEXT automatically.
	widebuffer = (wchar_t*)GlobalLock( hData );
	
	int widelen = wcslen(widebuffer)+1;
	widelen = (widelen < 360 ? widelen : 360); // limit to max user message size.
	int bufferlen = WideCharToMultiByte(CP_UTF8, WC_DEFAULTCHAR, widebuffer, widelen, nullptr, 0, NULL, NULL);

	if(bufferlen > 0)
	{
		bufferlen = (bufferlen < 1440 ? bufferlen : 1440);
		WideCharToMultiByte(CP_UTF8, WC_DEFAULTCHAR, widebuffer, widelen, buffer, 1444, NULL, NULL);
		strcpy(buffer+bufferlen, "\0\0\0");
	}
	char * result = buffer;

	GlobalUnlock( hData );
	CloseClipboard();
	return result;

#elif defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
	return (OSXCopyFromClipboard());
#else
	return 0;
#endif
}


} // end namespace

