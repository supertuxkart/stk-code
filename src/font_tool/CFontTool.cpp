#include "CFontTool.h"
#include "IXMLWriter.h"
#include <iostream>
#include <fstream>

using namespace irr;

const int fontsizes[] = {4,6,8,9,10,11,12,14,16,18,20,22,24,26,28,36,48,56,68,72,0};

char bUsed[0x10000]={0};

inline u32 getTextureSizeFromSurfaceSize(u32 size)
{
	u32 ts = 0x01;
	while(ts < size)
		ts <<= 1;

	return ts;
}

bool LoadPoFiles(const char* sListFileName){
	char s[1024];
	std::ifstream fin(sListFileName);
	if(!fin){
		std::cout<<"Error: Can't open "<<sListFileName<<std::endl;
		return false;
	}
	std::cout<<"Opened list file "<<sListFileName<<std::endl;
	for(;;){
		fin.getline(s,1024);
		if(fin.eof()) break;
		std::ifstream fin2(s);
		if(!fin2){
			std::cout<<"Error: Can't open "<<s<<std::endl;
		}else{
			std::cout<<"Opened "<<s<<std::endl;
			//buggy code that convert UTF-8 to UCS-2
			for(;;){
				unsigned char c1=(unsigned char)fin2.get();
				unsigned short out=0;
				if(fin2.eof()) break;
				if(c1>=0xF0) continue;
				else if(c1>=0xE0){
					out=((c1&0xF)<<12)
						|((((unsigned char)fin2.get())&0x3F)<<6)
						|(((unsigned char)fin2.get())&0x3F);
				}else if(c1>=0xC0){
					out=((c1&0x1F)<<6)
						|(((unsigned char)fin2.get())&0x3F);
				}else{
					out=c1;
				}
				bUsed[out]=1;
			}
		}
	}
	return true;
}

// windows specific
#ifdef _IRR_WINDOWS_

	const DWORD charsets[] = { ANSI_CHARSET, DEFAULT_CHARSET, OEM_CHARSET, BALTIC_CHARSET, GB2312_CHARSET, CHINESEBIG5_CHARSET,
								EASTEUROPE_CHARSET, GREEK_CHARSET, HANGUL_CHARSET, MAC_CHARSET, RUSSIAN_CHARSET,
								SHIFTJIS_CHARSET, SYMBOL_CHARSET, TURKISH_CHARSET, VIETNAMESE_CHARSET, JOHAB_CHARSET,
								ARABIC_CHARSET, HEBREW_CHARSET, THAI_CHARSET, 0};

	const wchar_t *setnames[] = {L"ANSI", L"All Available", L"OEM", L"Baltic", L"Chinese Simplified", L"Chinese Traditional",
								L"Eastern European", L"Greek", L"Hangul", L"Macintosh", L"Russian",
								L"Japanese", L"Symbol", L"Turkish", L"Vietnamese", L"Johab",
								L"Arabic", L"Hebrew", L"Thai", 0};

	// callback for adding font names
	int CALLBACK EnumFontFamExProc( ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme,
					DWORD FontType, LPARAM lParam)
	{
		CFontTool* t = (CFontTool*) lParam;
		t->FontNames.push_back( core::stringw(lpelfe->elfFullName));
		return 1;
	}

	//
	// Constructor
	//

	CFontTool::CFontTool(IrrlichtDevice* device) : FontSizes(fontsizes),
			Device(device), UseAlphaChannel(false),
			// win specific
			dc(0)
	{
		// init display context
		dc = CreateDC(L"DISPLAY", L"DISPLAY", 0 ,0 );

		// populate list of available character set names
		for (int i=0; setnames[i] != 0; ++i)
			CharSets.push_back( core::stringw(setnames[i]));

		selectCharSet(0);
	}

	void CFontTool::selectCharSet(u32 currentCharSet)
	{
		if ( currentCharSet >= CharSets.size() )
			return;

		LOGFONTW lf;
		lf.lfFaceName[0] = L'\0';
		lf.lfCharSet = (BYTE) charsets[currentCharSet];
		// HRESULT hr; // no error checking(!)

		// clear font list
		FontNames.clear();

		// create list of available fonts
		EnumFontFamiliesExW( dc, (LPLOGFONTW) &lf, (FONTENUMPROCW) EnumFontFamExProc, (LPARAM) this, 0);
	}

	bool CFontTool::makeBitmapFont(u32 fontIndex, u32 charsetIndex, s32 fontSize, u32 textureWidth, u32 textureHeight, bool bold, bool italic, bool aa, bool alpha,bool usedOnly,bool excludeLatin)
	{
		if (fontIndex >= FontNames.size() || charsetIndex >= CharSets.size() )
			return false;

		UseAlphaChannel = alpha;
		u32 currentImage = 0;

		// create the font
		HFONT font = CreateFontW(
			-MulDiv(fontSize, GetDeviceCaps(dc, LOGPIXELSY), 72), 0,
			0,0,
			bold ? FW_BOLD : 0,
			italic, 0,0, charsets[charsetIndex], 0,0,
			aa ? ANTIALIASED_QUALITY : 0,
			0, FontNames[fontIndex].c_str() );

		if (!font)
			return false;

		SelectObject(dc, font);
		SetTextAlign (dc,TA_LEFT | TA_TOP | TA_NOUPDATECP);

		// get rid of the current textures/images
		for (u32 i=0; i<currentTextures.size(); ++i)
			currentTextures[i]->drop();
		currentTextures.clear();

		for (u32 i=0; i<currentImages.size(); ++i)
			currentImages[i]->drop();
		currentImages.clear();

		// clear current image mappings
		CharMap.clear();
		// clear array
		Areas.clear();

		// get information about this font's unicode ranges.
		s32 size = GetFontUnicodeRanges( dc, 0);
		c8 *buf = new c8[size];
		LPGLYPHSET glyphs = (LPGLYPHSET)buf;

		GetFontUnicodeRanges( dc, glyphs);

		// s32 TotalCharCount = glyphs->cGlyphsSupported;

		s32 currentx=0, currenty=0, maxy=0;

		for (u32 range=0; range < glyphs->cRanges; range++)
		{
			WCRANGE* current = &glyphs->ranges[range];

			//maxy=0;

			// loop through each glyph and write its size and position
			for (s32 ch=current->wcLow; ch< current->wcLow + current->cGlyphs; ch++)
			{
				wchar_t currentchar = ch;

				/* if ( IsDBCSLeadByte((BYTE) ch))
					continue; // surragate pairs unsupported */
				if(excludeLatin && ch>=0 && ch<0x100) continue;
				if(usedOnly && !bUsed[(unsigned short)ch]) continue;

				// get the dimensions
				SIZE size;
				ABC abc;
				GetTextExtentPoint32W(dc, &currentchar, 1, &size);
				SFontArea fa;
				fa.underhang = 0;
				fa.overhang  = 0;

				if (GetCharABCWidthsW(dc, currentchar, currentchar, &abc)) // for unicode fonts, get overhang, underhang, width
				{
					size.cx = abc.abcB;
					fa.underhang  = abc.abcA;
					fa.overhang   = abc.abcC;

					if (abc.abcB-abc.abcA+abc.abcC<1)
						continue; // nothing of width 0
				}
				if (size.cy < 1)
					continue;

				//GetGlyphOutline(dc, currentchar, GGO_METRICS, &gm, 0, 0, 0);

				//size.cx++; size.cy++;

				// wrap around?
				if (currentx + size.cx > (s32) textureWidth)
				{
					currenty += maxy;
					currentx = 0;
					if ((u32)(currenty + maxy) > textureHeight)
					{
						currentImage++; // increase Image count
						currenty=0;
					}
					maxy = 0;
				}
				// add this char dimension to the current map

				fa.rectangle = core::rect<s32>(currentx, currenty, currentx + size.cx, currenty + size.cy);
				fa.sourceimage = currentImage;

				CharMap.insert(currentchar, Areas.size());
				Areas.push_back( fa );

				currentx += size.cx +1;

				if (size.cy+1 > maxy)
					maxy = size.cy+1;
			}
		}
		currenty += maxy;

		u32 lastTextureHeight = getTextureSizeFromSurfaceSize(currenty);

		// delete the glyph set
		delete [] buf;

		currentImages.set_used(currentImage+1);
		currentTextures.set_used(currentImage+1);

		for (currentImage=0; currentImage < currentImages.size(); ++currentImage)
		{
			core::stringc logmsg = "Creating image ";
			logmsg += (s32) (currentImage+1);
			logmsg += " of ";
			logmsg += (s32) currentImages.size();
			Device->getLogger()->log(logmsg.c_str());
			// no need for a huge final texture
			u32 texHeight = textureHeight;
			if (currentImage == currentImages.size()-1 )
				texHeight = lastTextureHeight;

			// make a new bitmap
			HBITMAP bmp = CreateCompatibleBitmap(dc, textureWidth, texHeight);
			HDC bmpdc = CreateCompatibleDC(dc);

			LOGBRUSH lbrush;
			lbrush.lbColor = RGB(0,0,0);
			lbrush.lbHatch = 0;
			lbrush.lbStyle = BS_SOLID;

			HBRUSH brush = CreateBrushIndirect(&lbrush);
			HPEN pen = CreatePen(PS_NULL, 0, 0);

			HGDIOBJ oldbmp = SelectObject(bmpdc, bmp);
			HGDIOBJ oldbmppen = SelectObject(bmpdc, pen);
			HGDIOBJ oldbmpbrush = SelectObject(bmpdc, brush);
			HGDIOBJ oldbmpfont = SelectObject(bmpdc, font);

			SetTextColor(bmpdc, RGB(255,255,255));

			Rectangle(bmpdc, 0,0,textureWidth,texHeight);
			SetBkMode(bmpdc, TRANSPARENT);

			// draw the letters...

			// iterate through the tree
			core::map<wchar_t, u32>::Iterator it = CharMap.getIterator();
			while (!it.atEnd())
			{
				s32 currentArea = (*it).getValue();
				wchar_t wch = (*it).getKey();
				// sloppy but I couldnt be bothered rewriting it
				if (Areas[currentArea].sourceimage == currentImage)
				{
					// draw letter
					s32 sx = Areas[currentArea].rectangle.UpperLeftCorner.X - Areas[currentArea].underhang;
					TextOutW(bmpdc, sx, Areas[currentArea].rectangle.UpperLeftCorner.Y, &wch, 1);

					// if ascii font...
					//SetPixel(bmpdc, Areas[currentArea].rectangle.UpperLeftCorner.X, Areas[currentArea].rectangle.UpperLeftCorner.Y, RGB(255,255,0));// left upper corner mark
				}
				it++;
			}

			// copy the font bitmap into a new irrlicht image
			BITMAP b;
			PBITMAPINFO pbmi;
			WORD cClrBits;
			u32 cformat;

			// Retrieve the bitmap color format, width, and height.
			GetObject(bmp, sizeof(BITMAP), (LPSTR)&b);

			// Convert the color format to a count of bits.
			cClrBits = (WORD)(b.bmPlanes * b.bmBitsPixel);

			if (cClrBits <= 8) // we're not supporting these
				cformat = -1;
			else if (cClrBits <= 16)
				cformat = video::ECF_A1R5G5B5;
			else if (cClrBits <= 24)
				cformat = video::ECF_R8G8B8;
			else
				cformat = video::ECF_A8R8G8B8;

			pbmi = (PBITMAPINFO) LocalAlloc(LPTR,
						sizeof(BITMAPINFOHEADER));

			// Initialize the fields in the BITMAPINFO structure.

			pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			pbmi->bmiHeader.biWidth = b.bmWidth;
			pbmi->bmiHeader.biHeight = b.bmHeight;
			pbmi->bmiHeader.biPlanes = b.bmPlanes;
			pbmi->bmiHeader.biBitCount = b.bmBitsPixel;

			// If the bitmap is not compressed, set the BI_RGB flag.
			pbmi->bmiHeader.biCompression = BI_RGB;

			// Compute the number of bytes in the array of color
			// indices and store the result in biSizeImage.
			// For Windows NT, the width must be DWORD aligned unless
			// the bitmap is RLE compressed. This example shows this.
			// For Windows 95/98/Me, the width must be WORD aligned unless the
			// bitmap is RLE compressed.
			pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
							* pbmi->bmiHeader.biHeight;
			// Set biClrImportant to 0, indicating that all of the
			// device colors are important.
			pbmi->bmiHeader.biClrImportant = 0;

			LPBYTE lpBits; // memory pointer

			PBITMAPINFOHEADER pbih = (PBITMAPINFOHEADER) pbmi;
			lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

			GetDIBits(dc, bmp, 0, (WORD) pbih->biHeight, lpBits, pbmi, DIB_RGB_COLORS);

			// DEBUG- copy to clipboard
			//OpenClipboard(hWnd);
			//EmptyClipboard();
			//SetClipboardData(CF_BITMAP, bmp);
			//CloseClipboard();

			// flip bitmap
			s32 rowsize = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8;
			c8 *row = new c8[rowsize];
			for (s32 i=0; i < (pbih->biHeight/2); ++i)
			{
				// grab a row
				memcpy(row, lpBits + (rowsize * i), rowsize);
				// swap row
				memcpy(lpBits + (rowsize * i), lpBits + ((pbih->biHeight-1 -i) * rowsize ) , rowsize);
				memcpy(lpBits + ((pbih->biHeight-1 -i) * rowsize ), row , rowsize);
			}

			bool ret = false;

			if (cformat == video::ECF_A8R8G8B8)
			{
				// in this case the font should have an alpha channel, but since windows doesn't draw one
				// we have to set one manually by going through all the pixels.. *sigh*

				u8* m;
				for (m = lpBits; m < lpBits + pbih->biSizeImage; m+=4)
				{
					if (UseAlphaChannel)
					{
						if (m[0] > 0) // pixel has colour
						{
							m[3]=m[0];  // set alpha
							m[0]=m[1]=m[2] = 255; // everything else is full
						}
					}
					else
						m[3]=255; // all pixels are full alpha
				}

			}
			else if (cformat == video::ECF_A1R5G5B5)
			{
				u8* m;
				for (m = lpBits; m < lpBits + pbih->biSizeImage; m+=2)
				{
					WORD *p = (WORD*)m;
					if (m[0] > 0 || !UseAlphaChannel) // alpha should be set
						*p |= 0x8000; // set alpha bit
				}
			}
			else
			{
				cformat = -1;
			}

			// make a texture from the image
			if (cformat != -1)
			{
				// turn mip-mapping off
				bool b = Device->getVideoDriver()->getTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS);
				currentImages[currentImage] = Device->getVideoDriver()->createImageFromData((video::ECOLOR_FORMAT)cformat, core::dimension2d<u32>(textureWidth,texHeight), (void*)lpBits);
				Device->getVideoDriver()->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS,b);
			}
			else
			{
				Device->getLogger()->log("Couldn't create font, your pixel format is unsupported.");
			}

			// free memory and windows resources
			// sloppy I know, but I only intended to create one image at first.
			delete [] row;
			LocalFree(pbmi);
			GlobalFree(lpBits);
			DeleteDC(bmpdc);
			DeleteObject(brush);
			DeleteObject(pen);
			DeleteObject(bmp);

			if (currentImages[currentImage])
			{
				// add texture
				currentTextures[currentImage] = Device->getVideoDriver()->addTexture("GUIFontImage",currentImages[currentImage]);
				currentTextures[currentImage]->grab();
			}
			else
			{
				Device->getLogger()->log("Something went wrong, aborting.");
				// drop all images
				DeleteObject(font);
				return false;
			}
		} // looping through each texture
		DeleteObject(font);
		return true;
	}

#else

	CFontTool::CFontTool(IrrlichtDevice *device) : FontSizes(fontsizes), Device(device), UseAlphaChannel(false)
	{
		if (!XftInitFtLibrary())
		{
			core::stringc logmsg = "XFT not found\n";
			Device->getLogger()->log(logmsg.c_str());
			exit(EXIT_FAILURE);
		}

		/* Get a list of the font foundries, storing them in a set to sort */
		std::set<core::stringw> foundries;
		Display* display = (Display*)Device->getVideoDriver()->getExposedVideoData().OpenGLLinux.X11Display;
		XftFontSet* fonts = XftListFonts(display, DefaultScreen(display), 0, XFT_FOUNDRY, 0);
		for (int i = 0; i < fonts->nfont; i++)
		{
			char *foundry;
			XftPatternGetString(fonts->fonts[i], XFT_FOUNDRY, 0, &foundry);
			core::stringw tmp(foundry);
			foundries.insert(tmp);
		}
		XftFontSetDestroy(fonts);

		/* Copy the sorted list into the array */
		CharSets.clear();
		for (std::set<core::stringw>::iterator i = foundries.begin(); i != foundries.end(); i++)
			CharSets.push_back((*i).c_str());
		selectCharSet(0);
	}

	/* Note: There must be some trick for using strings as pattern parameters to XftListFonts because
	no matter how I specify a string, I end up with an intermittent segfault.  Since XftFontList is
	just calling FcFontList, that's what I'll do too since that works OK */
	void CFontTool::selectCharSet(u32 currentCharSet)
	{
		/* Get a list of the font families, storing them in a set to sort */
		char foundry[256];
		sprintf(&foundry[0],"%ls",CharSets[currentCharSet].c_str());
		std::set<core::stringw> families;
		XftPattern *pattern = FcPatternCreate();
		XftPatternAddString(pattern, FC_FOUNDRY, &foundry[0]);
		XftObjectSet *objectset = FcObjectSetCreate();
		XftObjectSetAdd(objectset, XFT_FOUNDRY);
		XftObjectSetAdd(objectset, XFT_FAMILY);
		FcFontSet *fonts = FcFontList(NULL, pattern, objectset);

		for (int i = 0; i < fonts->nfont; i++)
		{
			char* ptr;
			XftPatternGetString(fonts->fonts[i], XFT_FAMILY, 0, &ptr);
			core::stringw family(ptr);
			families.insert(family);
		}
		XftPatternDestroy(pattern);
		FcObjectSetDestroy(objectset);

		/* Copy the sorted list into the array */
		FontNames.clear();
		for (std::set<core::stringw>::iterator i = families.begin(); i != families.end(); i++)
			FontNames.push_back((*i).c_str());
	}

	bool CFontTool::makeBitmapFont(u32 fontIndex, u32 charsetIndex, s32 fontSize, u32 textureWidth, u32 textureHeight, bool bold, bool italic, bool aa, bool alpha,bool usedOnly,bool excludeLatin)
	{
		if (fontIndex >= FontNames.size() || charsetIndex >= CharSets.size() )
			return false;

		Display *display = (Display*) Device->getVideoDriver()->getExposedVideoData().OpenGLLinux.X11Display;
		u32 screen = DefaultScreen(display);
		Window win = RootWindow(display, screen);
		Visual *visual = DefaultVisual(display, screen);
		UseAlphaChannel = alpha;
		u32 currentImage = 0;

		XftResult result;
		XftPattern *request = XftPatternCreate();
		char foundry[256], family[256];
		sprintf(&foundry[0],"%ls",CharSets[charsetIndex].c_str());
		sprintf(&family[0],"%ls",FontNames[fontIndex].c_str());
		XftPatternAddString(request, XFT_FOUNDRY, &foundry[0]);
		XftPatternAddString(request, XFT_FAMILY, &family[0]);
		XftPatternAddInteger(request, XFT_PIXEL_SIZE, fontSize);
		XftPatternAddInteger(request, XFT_WEIGHT, bold ? XFT_WEIGHT_BLACK : XFT_WEIGHT_LIGHT);
		XftPatternAddInteger(request, XFT_SLANT, italic ? XFT_SLANT_ITALIC : XFT_SLANT_ROMAN);
		XftPatternAddBool(request, XFT_ANTIALIAS, aa);

		/* Find the closest font that matches the user choices and open it and check if the returned
		font has anti aliasing enabled by default, even if it wasn't requested */
		FcBool aaEnabled;
		XftPattern *found = XftFontMatch(display, DefaultScreen(display), request, &result);
		XftPatternGetBool(found, XFT_ANTIALIAS, 0, &aaEnabled);
		aa = aaEnabled;
		XftFont *font = XftFontOpenPattern(display, found);

		// get rid of the current textures/images
		for (u32 i=0; i<currentTextures.size(); ++i)
			currentTextures[i]->drop();
		currentTextures.clear();
		for (u32 i=0; i<currentImages.size(); ++i)
			currentImages[i]->drop();
		currentImages.clear();
		CharMap.clear();
		Areas.clear();

		/* Calculate the max height of the font.  Annoyingly, it seems that the height property of the font
		is the maximum height of any single character, but a string of characters, aligned along their
		baselines, can exceed this figure.  Because I don't know any better way of doing it, I'm going to
		have to use the brute force method.

		Note: There will be a certain number of charters in a font, however they may not be grouped
		consecutively, and could in fact be spread out with many gaps */
		u32 maxY = 0;
		u32 charsFound = 0;
		for (FT_UInt charCode = 0; charsFound < FcCharSetCount(font->charset); charCode++)
		{
			if (!XftCharExists(display, font, charCode))
				continue;

			charsFound++;

			XGlyphInfo extents;
			XftTextExtents32(display, font, &charCode, 1, &extents);
			if ((extents.xOff <= 0) && (extents.height <= 0))
				continue;

			/* Calculate the width and height, adding 1 extra pixel if anti aliasing is enabled */
			u32 chWidth = extents.xOff + (aa ? 1 : 0);
			u32 chHeight = (font->ascent - extents.y + extents.height) +  (aa ? 1 : 0);
			if (chHeight > maxY)
				maxY = chHeight;

			/* Store the character details here */
			SFontArea fontArea;
			fontArea.rectangle = core::rect<s32>(0, 0, chWidth, chHeight);
			CharMap.insert(charCode, Areas.size());
			Areas.push_back(fontArea);
		}
		core::stringc logmsg = "Found ";
		logmsg += (s32) (CharMap.size() + 1);
		logmsg += " characters";
		Device->getLogger()->log(logmsg.c_str());

		/* Get the size of the chars and allocate them a position on a texture.  If the next character that
		is added would be outside the width or height of the texture, then a new texture is added */
		u32 currentX = 0, currentY = 0, rowY = 0;
		for (core::map<wchar_t, u32>::Iterator it = CharMap.getIterator(); !it.atEnd(); it++)
		{
			s32 currentArea = (*it).getValue();
			SFontArea *fontArea = &Areas[currentArea];
			u32 chWidth = fontArea->rectangle.LowerRightCorner.X;
			u32 chHeight = fontArea->rectangle.LowerRightCorner.Y;

			/* If the width of this char will exceed the textureWidth then start a new row */
			if ((currentX + chWidth) > textureWidth)
			{
				currentY += rowY;
				currentX = 0;

				/* If the new row added to the texture exceeds the textureHeight then start a new texture */
				if ((currentY + rowY) > textureHeight)
				{
					currentImage++;
					currentY = 0;
				}
				rowY = 0;
			}

			/* Update the area with the current x and y and texture */
			fontArea->rectangle = core::rect<s32>(currentX, currentY, currentX + chWidth, currentY + chHeight);
			fontArea->sourceimage = currentImage;
			currentX += chWidth + 1;
			if (chHeight + 1 > rowY)
				rowY = chHeight + 1;
		}

		/* The last row of chars and the last texture weren't accounted for in the loop, so add them here */
		currentY += rowY;
		u32 lastTextureHeight = getTextureSizeFromSurfaceSize(currentY);
		currentImages.set_used(currentImage + 1);
		currentTextures.set_used(currentImage + 1);

		/* Initialise colours */
		XftColor colFore, colBack;
		XRenderColor xFore = {0xffff, 0xffff, 0xffff, 0xffff};
		XRenderColor xBack = {0x0000, 0x0000, 0x0000, 0xffff};
		XftColorAllocValue(display, DefaultVisual(display, screen), DefaultColormap(display, screen), &xFore, &colFore);
		XftColorAllocValue(display, DefaultVisual(display, screen), DefaultColormap(display, screen), &xBack, &colBack);

		/* Create a pixmap that is large enough to hold any character in the font */
		Pixmap pixmap = XCreatePixmap(display, win, textureWidth, maxY, DefaultDepth(display, screen));
		XftDraw *draw = XftDrawCreate(display, pixmap, visual, DefaultColormap(display, screen));

		/* Render the chars */
		for (currentImage = 0; currentImage < currentImages.size(); ++currentImage)
		{
			core::stringc logmsg = "Creating image ";
			logmsg += (s32) (currentImage+1);
			logmsg += " of ";
			logmsg += (s32) currentImages.size();
			Device->getLogger()->log(logmsg.c_str());

			/* The last texture that is saved is vertically shrunk to fit the characters drawn on it */
			u32 texHeight = textureHeight;
			if (currentImage == currentImages.size() - 1)
				texHeight = lastTextureHeight;

			/* The texture that holds this "page" of characters */
			currentImages[currentImage] = Device->getVideoDriver()->createImage(video::ECF_A8R8G8B8, core::dimension2du(textureWidth, texHeight));
			currentImages[currentImage]->fill(video::SColor(alpha ? 0 : 255,0,0,0));

			for (core::map<wchar_t, u32>::Iterator it = CharMap.getIterator(); !it.atEnd(); it++)
			{
				FcChar32 wch = (*it).getKey();
				s32 currentArea = (*it).getValue();
				if (Areas[currentArea].sourceimage == currentImage)
				{
					SFontArea *fontArea = &Areas[currentArea];
					u32 chWidth = fontArea->rectangle.LowerRightCorner.X - fontArea->rectangle.UpperLeftCorner.X;
					u32 chHeight = fontArea->rectangle.LowerRightCorner.Y - fontArea->rectangle.UpperLeftCorner.Y;

					/* Draw the glyph onto the pixmap */
					XGlyphInfo extents;
					XftDrawRect(draw, &colBack, 0, 0, chWidth, chHeight);
					XftTextExtents32(display, font, &wch, 1, &extents);
					XftDrawString32(draw, &colFore, font, extents.x, extents.y, &wch, 1);

					/* Convert the pixmap into an image, then copy it onto the Irrlicht texture, pixel by pixel.
					There's bound to be a faster way, but this is adequate */
					u32 xDest = fontArea->rectangle.UpperLeftCorner.X;
					u32 yDest = fontArea->rectangle.UpperLeftCorner.Y + font->ascent - extents.y;
					XImage *image = XGetImage(display, pixmap, 0, 0, chWidth, chHeight, 0xffffff, XYPixmap);
					if (image)
					{
						for (u32 ySrc = 0; ySrc < chHeight; ySrc++)
							for (u32 xSrc = 0; xSrc < chWidth; xSrc++)
							{
								/* Get the pixel colour and break it down into rgb components */
								u32 col = XGetPixel(image, xSrc, ySrc);
								u32 a = 255;
								u32 r = col & visual->red_mask;
								u32 g = col & visual->green_mask;
								u32 b = col & visual->blue_mask;
								while (r > 0xff) r >>= 8;
								while (g > 0xff) g >>= 8;
								while (b > 0xff) b >>= 8;

								/* To make the background transparent, set the colour to 100% white and the alpha to
								the average of the three rgb colour components to maintain the anti-aliasing */
								if (alpha)
								{
									a = (r + g + b) / 3;
									r = 255;
									g = 255;
									b = 255;
								}
								currentImages[currentImage]->setPixel(xDest + xSrc,yDest + ySrc,video::SColor(a,r,g,b));
							}
						image->f.destroy_image(image);
					}
				}
			}

			/* Add the texture to the list */
			currentTextures[currentImage] = Device->getVideoDriver()->addTexture("GUIFontImage",currentImages[currentImage]);
			currentTextures[currentImage]->grab();
		}

		XftColorFree (display, visual, DefaultColormap(display, screen), &colFore);
		XftColorFree (display, visual, DefaultColormap(display, screen), &colBack);
		XftFontClose(display,font);
		XftPatternDestroy(request);
		XftDrawDestroy(draw);
		XFreePixmap(display, pixmap);
		return true;
	}
#endif

	CFontTool::~CFontTool()
	{
#ifdef _IRR_WINDOWS_
		// destroy display context
		if (dc)
			DeleteDC(dc);
#endif

		// drop textures+images
		for (u32 i=0; i<currentTextures.size(); ++i)
			currentTextures[i]->drop();
		currentTextures.clear();

		for (u32 i=0; i<currentImages.size(); ++i)
			currentImages[i]->drop();
		currentImages.clear();
	}

bool CFontTool::saveBitmapFont(const c8 *filename, const c8* format)
{
	if (currentImages.size() == 0)
	{
		Device->getLogger()->log("No image data to write, aborting.");
		return false;
	}

	core::stringc fn = filename;
	core::stringc imagename = filename;
	fn += ".xml";

	io::IXMLWriter *writer = Device->getFileSystem()->createXMLWriter(fn.c_str());

	// header and line breaks
	writer->writeXMLHeader();
	writer->writeLineBreak();

	// write information
	writer->writeElement(L"font", false, L"type", L"bitmap");
	writer->writeLineBreak();
	writer->writeLineBreak();

	// write images and link to them
	for (u32 i=0; i<currentImages.size(); ++i)
	{
		imagename = filename;
		imagename += (s32)i;
		imagename += ".";
		imagename += format;
		Device->getVideoDriver()->writeImageToFile(currentImages[i],imagename.c_str());

		writer->writeElement(L"Texture", true,
				L"index", core::stringw(i).c_str(),
				L"filename", core::stringw(imagename.c_str()).c_str(),
				L"hasAlpha", UseAlphaChannel ? L"true" : L"false");
		writer->writeLineBreak();
	}

	writer->writeLineBreak();

	// write each character
	core::map<wchar_t, u32>::Iterator it = CharMap.getIterator();
	while (!it.atEnd())
	{
		SFontArea &fa = Areas[(*it).getValue()];

		wchar_t c[2];
		c[0] = (*it).getKey();
		c[1] = L'\0';
		core::stringw area, under, over, image;
		area  = core::stringw(fa.rectangle.UpperLeftCorner.X);
		area += L", ";
		area += fa.rectangle.UpperLeftCorner.Y;
		area += L", ";
		area += fa.rectangle.LowerRightCorner.X;
		area += L", ";
		area += fa.rectangle.LowerRightCorner.Y;

		core::array<core::stringw> names;
		core::array<core::stringw> values;
		names.clear();
		values.clear();
		// char
		names.push_back(core::stringw(L"c"));
		values.push_back(core::stringw(c));
		// image number
		if (fa.sourceimage != 0)
		{
			image = core::stringw(fa.sourceimage);
			names.push_back(core::stringw(L"i"));
			values.push_back(image);
		}
		// rectangle
		names.push_back(core::stringw(L"r"));
		values.push_back(area);

		if (fa.underhang != 0)
		{
			under = core::stringw(fa.underhang);
			names.push_back(core::stringw(L"u"));
			values.push_back(under);
		}
		if (fa.overhang != 0)
		{
			over = core::stringw(fa.overhang);
			names.push_back(core::stringw(L"o"));
			values.push_back(over);
		}
		writer->writeElement(L"c", true, names, values);

		writer->writeLineBreak();
		it++;
	}

	writer->writeClosingTag(L"font");

	writer->drop();

	Device->getLogger()->log("Bitmap font saved.");

	return true;
}
