#ifndef __IRR_FONT_TOOL_INCLUDED__
#define __IRR_FONT_TOOL_INCLUDED__


#include "irrlicht.h"

#if defined(_IRR_WINDOWS_)
	#ifdef _MBCS
		#undef _MBCS
	#endif

	#define UNICODE
	#define _WIN32_WINNT 0x0500	
	#include "windows.h"
#else
	#ifdef _IRR_COMPILE_WITH_X11_
	#include <X11/Xlib.h>
	#endif
	#include <X11/Xft/Xft.h>
	#include <set>
#endif

bool LoadPoFiles(const char* sListFileName);

namespace irr {
	class CFontTool : public irr::IReferenceCounted
	{
	public:
		CFontTool(irr::IrrlichtDevice* device);
		~CFontTool();

		virtual bool makeBitmapFont(u32 fontIndex, u32 charsetIndex, 
				s32 fontSize, u32 texturewidth, u32 textureHeight,
				bool bold, bool italic, bool aa, bool alpha,
				bool usedOnly=false,bool exclideLatin=false);

		virtual bool saveBitmapFont(const c8* filename, const c8* format);

		virtual void selectCharSet(u32 currentCharSet);

		struct SFontArea
		{
			SFontArea() : rectangle(), underhang(0), overhang(0), sourceimage(0) {}
			core::rect<s32> rectangle;
			s32		underhang;
			s32		overhang;
			u32		sourceimage;
		};

	/*	struct SFontMap
		{
			SFontMap() :  areas(), start(0), count(0) {}
			core::array< SFontArea >	areas;
			s32				start;
			s32				count;
		};*/

		core::array<core::stringw>	FontNames;
		core::array<core::stringw>	CharSets;
		//core::array<SFontMap>		Mappings;
		core::array<SFontArea>		Areas;
		core::map<wchar_t, u32>		CharMap;

		core::array<video::ITexture*>	currentTextures;
		core::array<video::IImage*>	currentImages;
		const int			*FontSizes;
		IrrlichtDevice			*Device;

		bool UseAlphaChannel;

		// windows
		#ifdef _IRR_WINDOWS_
		HDC dc;
		#endif

	};
}
#endif // __IRR_FONT_TOOL_INCLUDED__
