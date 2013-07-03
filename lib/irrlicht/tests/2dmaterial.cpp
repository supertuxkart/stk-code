#include "testUtils.h"
#include <map>
using namespace irr;

namespace
{
// don't use this code! It lacks many checks and is for testing
// purposes only!!!
// based on code and media from SuperTuxKart
class ScalableFont : public gui::IGUIFontBitmap
{
	float m_scale;
	struct TextureInfo
	{
		irr::core::stringc m_file_name;
		bool m_has_alpha;
		float m_scale;

		TextureInfo()
		{
			m_has_alpha = false;
			m_scale = 1.0f;
		}
	};

	std::map<int /* texture file ID */, TextureInfo> m_texture_files;

	void lazyLoadTexture(int texID)
	{
		const bool mipmap = Driver->getTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS);
		Driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);
		// load texture
		SpriteBank->setTexture(texID, Driver->getTexture( m_texture_files[texID].m_file_name ));
		// set previous mip-map+filter state
		Driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, mipmap);

		// couldn't load texture, abort.
		if (!SpriteBank->getTexture(texID))
		{
			return;
		}
		else
		{
			// colorkey texture rather than alpha channel?
			if (! m_texture_files[texID].m_has_alpha)
			{
				Driver->makeColorKeyTexture(SpriteBank->getTexture(texID), core::position2di(0,0));
			}
		}
	}
	void doReadXmlFile(io::IXMLReader* xml)
	{
		while (xml->read())
		{
			if (io::EXN_ELEMENT == xml->getNodeType())
			{
				if (core::stringw(L"include") == xml->getNodeName())
				{
					core::stringc filename = xml->getAttributeValue(L"file");
					io::IXMLReader* included = Environment->getFileSystem()->createXMLReader(filename.c_str());
					if (included != NULL)
					{
						doReadXmlFile(included);
						included->drop();
					}
				}
				else if (core::stringw(L"Texture") == xml->getNodeName())
				{
					// add a texture
					core::stringc filename = xml->getAttributeValue(L"filename");
					core::stringc fn = filename;
					u32 i = (u32)xml->getAttributeValueAsInt(L"index");

					float scale=1.0f;
					if (xml->getAttributeValue(L"scale"))
						scale = xml->getAttributeValueAsFloat(L"scale");
						//std::cout  << "scale = " << scale << std::endl;

					core::stringw alpha = xml->getAttributeValue(L"hasAlpha");

					//std::cout << "---- Adding font texture " << fn.c_str() << "; alpha=" << alpha.c_str() << std::endl;


					// make sure the sprite bank has enough textures in it
					while (i+1 > SpriteBank->getTextureCount())
					{
						SpriteBank->addTexture(NULL);
					}

					TextureInfo info;
					info.m_file_name   = fn;
					info.m_has_alpha   = (alpha == core::stringw("true"));
					info.m_scale = scale;

					m_texture_files[i] = info;
				}
				else if (core::stringw(L"c") == xml->getNodeName())
				{
					// adding a character to this font
					SFontArea a;
					gui::SGUISpriteFrame f;
					gui::SGUISprite s;
					core::rect<s32> rectangle;

					a.underhang = xml->getAttributeValueAsInt(L"u");
					a.overhang = xml->getAttributeValueAsInt(L"o");
					a.spriteno = SpriteBank->getSprites().size();
					s32 texno = xml->getAttributeValueAsInt(L"i");

					// parse rectangle
					core::stringc rectstr   = xml->getAttributeValue(L"r");
					wchar_t ch = xml->getAttributeValue(L"c")[0];

					const c8 *c = rectstr.c_str();
					s32 val;
					val = 0;
					while (*c >= '0' && *c <= '9')
					{
						val *= 10;
						val += *c - '0';
						c++;
					}
					rectangle.UpperLeftCorner.X = val;
					while (*c == L' ' || *c == L',') c++;

					val = 0;
					while (*c >= '0' && *c <= '9')
					{
						val *= 10;
						val += *c - '0';
						c++;
					}
					rectangle.UpperLeftCorner.Y = val;
					while (*c == L' ' || *c == L',') c++;

					val = 0;
					while (*c >= '0' && *c <= '9')
					{
						val *= 10;
						val += *c - '0';
						c++;
					}
					rectangle.LowerRightCorner.X = val;
					while (*c == L' ' || *c == L',') c++;

					val = 0;
					while (*c >= '0' && *c <= '9')
					{
						val *= 10;
						val += *c - '0';
						c++;
					}
					rectangle.LowerRightCorner.Y = val;

					CharacterMap[ch] = Areas.size();

					// make frame
					f.rectNumber = SpriteBank->getPositions().size();
					f.textureNumber = texno;

					// add frame to sprite
					s.Frames.push_back(f);
					s.frameTime = 0;

					// add rectangle to sprite bank
					SpriteBank->getPositions().push_back(rectangle);
					a.width = rectangle.getWidth();

					// add sprite to sprite bank
					SpriteBank->getSprites().push_back(s);

					// add character to font
					Areas.push_back(a);
				}
			}
		}
	}

public:

	bool m_black_border;

	ScalableFont* m_fallback_font;
	float m_fallback_font_scale;
	int m_fallback_kerning_width;

	//! constructor
	ScalableFont(gui::IGUIEnvironment *env, const io::path& filename)
	: Driver(0), SpriteBank(0), Environment(env), WrongCharacter(0),
		MaxHeight(0), GlobalKerningWidth(0), GlobalKerningHeight(0)
	{
	#ifdef _DEBUG
		setDebugName("ScalableFont");
	#endif

		m_fallback_font = NULL;
		m_fallback_kerning_width = 0;
		m_fallback_font_scale = 1.0f;
		m_scale = 0.37f;
		m_black_border = false;

		if (Environment)
		{
			// don't grab environment, to avoid circular references
			Driver = Environment->getVideoDriver();

			SpriteBank = Environment->addEmptySpriteBank(filename);
			if (SpriteBank)
				SpriteBank->grab();
		}

		if (Driver)
			Driver->grab();

		setInvisibleCharacters ( L" " );

		io::IXMLReader* reader = env->getFileSystem()->createXMLReader(filename.c_str());
		if (reader)
		{
			load( reader );
			reader->drop();
		}
		assert_log(Areas.size() > 0);
	}

	//! destructor
	virtual ~ScalableFont()
	{
		if (Driver)
			Driver->drop();
		if (SpriteBank)
			SpriteBank->drop();
	}

	//! loads a font from an XML file
	bool load(io::IXMLReader* xml)
	{
		if (!SpriteBank)
			return false;

		doReadXmlFile(xml);

		// set bad character
		WrongCharacter = getAreaIDFromCharacter(L' ', NULL);

		setMaxHeight();

		for(wchar_t c='0'; c<='9'; c++)
		{
			SFontArea a = getAreaFromCharacter(c, NULL);
			if (a.overhang > m_max_digit_area.overhang ) m_max_digit_area.overhang  = a.overhang;
			if (a.underhang > m_max_digit_area.underhang) m_max_digit_area.underhang = a.underhang;
			if (a.width > m_max_digit_area.width) m_max_digit_area.width = a.width;
		}
		m_max_digit_area.overhang = 0;
		m_max_digit_area.underhang=0;
		return true;
	}
	//! draws an text and clips it to the specified rectangle if wanted
	virtual void draw(const core::stringw& text, const core::rect<s32>& position,
			video::SColor color, bool hcenter=false,
			bool vcenter=false, const core::rect<s32>* clip=0)
	{
		if (!Driver) return;

		core::position2d<s32> offset = position.UpperLeftCorner;
		core::dimension2d<s32> text_dimension;

		// When we use the "tab" hack, disable right-alignment, it messes up everything
//		bool has_tab = (text.findFirst(L'\t') != -1);
		// ---- collect character locations
		const unsigned int text_size = text.size();
		core::array<s32> indices(text_size);
		core::array<core::position2di> offsets(text_size);
		core::array<bool> fallback;
		fallback.set_used(text_size);

		for (u32 i = 0; i<text_size; i++)
		{
			wchar_t c = text[i];

			//hack: one tab character is supported, it moves the cursor to the middle of the area
			if (c == L'\t')
			{
				offset.X = position.UpperLeftCorner.X + position.getWidth()/2;
				continue;
			}

			if (c == L'\r' ||	// Windows breaks
				c == L'\n')	// Unix breaks
			{
				if (c==L'\r' && text[i+1]==L'\n')
					c = text[++i];
				offset.Y += (int)(MaxHeight*m_scale);
				offset.X  = position.UpperLeftCorner.X;
				if (hcenter)
					offset.X += (position.getWidth() - text_dimension.Width) >> 1;
				continue;
			}   // if lineBreak

			bool use_fallback_font = false;
			const SFontArea &area  = getAreaFromCharacter(c, &use_fallback_font);
			fallback[i] = use_fallback_font;
			offset.X += area.underhang;
			offsets.push_back(offset);
			// Invisible character. add something to the array anyway so that
			// indices from the various arrays remain in sync
			indices.push_back((Invisible.findFirst(c) < 0) ? (int)area.spriteno
						: -1);
			offset.X += getCharWidth(area, fallback[i]);
		}   // for i<text_size

		// ---- do the actual rendering
		const int indiceAmount = indices.size();
		core::array< gui::SGUISprite >& sprites = SpriteBank->getSprites();
		core::array< core::rect<s32> >& positions = SpriteBank->getPositions();
		core::array< gui::SGUISprite >* fallback_sprites;
		core::array< core::rect<s32> >* fallback_positions;
		if (m_fallback_font!=NULL)
		{
			fallback_sprites   = &m_fallback_font->SpriteBank->getSprites();
			fallback_positions = &m_fallback_font->SpriteBank->getPositions();
		}
		else
		{
			fallback_sprites   = NULL;
			fallback_positions = NULL;
		}

		video::IVideoDriver* driver = Environment->getVideoDriver();
		const int spriteAmount = sprites.size();
		for (int n=0; n<indiceAmount; n++)
		{
			const int spriteID = indices[n];
			if (!fallback[n] && (spriteID < 0 || spriteID >= spriteAmount))
				continue;
			if (indices[n] == -1)
				continue;

			//assert_log(sprites[spriteID].Frames.size() > 0);

			const int texID = (fallback[n] ?
					(*fallback_sprites)[spriteID].Frames[0].textureNumber :
					sprites[spriteID].Frames[0].textureNumber);

			core::rect<s32> source = (fallback[n] ?
						(*fallback_positions)[(*fallback_sprites)[spriteID].Frames[0].rectNumber] :
						positions[sprites[spriteID].Frames[0].rectNumber]);

			const TextureInfo& info = (fallback[n] ?
						(*(m_fallback_font->m_texture_files.find(texID))).second :
						(*(m_texture_files.find(texID))).second);
			float char_scale = info.m_scale;

			core::dimension2d<s32> size = source.getSize();

			float scale = (fallback[n] ? m_scale*m_fallback_font_scale : m_scale);
			size.Width  = (int)(size.Width  * scale * char_scale);
			size.Height = (int)(size.Height * scale * char_scale);

			// align vertically if character is smaller
			int y_shift = (size.Height < MaxHeight*m_scale ? (int)((MaxHeight*m_scale - size.Height)/2.0f) : 0);

			core::rect<s32> dest(offsets[n] + core::position2di(0, y_shift), size);

			video::SColor colors[] = {color, color, color, color};

			video::ITexture* texture = (fallback[n] ?
										m_fallback_font->SpriteBank->getTexture(texID) :
										SpriteBank->getTexture(texID) );

			if (texture == NULL)
			{
				// perform lazy loading

				if (fallback[n])
				{
					m_fallback_font->lazyLoadTexture(texID);
					texture = m_fallback_font->SpriteBank->getTexture(texID);
				}
				else
				{
					lazyLoadTexture(texID);
					texture = SpriteBank->getTexture(texID);
				}

				if (texture == NULL)
				{
					continue; // no such character
				}
			}

			if (m_black_border)
			{
				// draw black border
				video::SColor black(color.getAlpha(),0,0,0);
				video::SColor black_colors[] = {black, black, black, black};

				for (int x_delta=-2; x_delta<=2; x_delta++)
				{
					for (int y_delta=-2; y_delta<=2; y_delta++)
					{
						if (x_delta == 0 || y_delta == 0) continue;
						driver->draw2DImage(texture,
											dest + core::position2d<s32>(x_delta, y_delta),
											source,
											clip,
											black_colors, true);
					}
				}
			}

			if (fallback[n])
			{
				// draw text over
				static video::SColor orange(color.getAlpha(), 255, 100, 0);
				static video::SColor yellow(color.getAlpha(), 255, 220, 15);
				video::SColor title_colors[] = {yellow, orange, orange, yellow};
				driver->draw2DImage(texture,
									dest,
									source,
									clip,
									title_colors, true);
			}
			else
			{
				driver->draw2DImage(texture,
									dest,
									source,
									clip,
									colors, true);

			}
		}
	}

	//! returns the dimension of a text
	virtual core::dimension2d<u32> getDimension(const wchar_t* text) const
	{
		assert_log(Areas.size() > 0);

		core::dimension2d<u32> dim(0, 0);
		core::dimension2d<u32> thisLine(0, (int)(MaxHeight*m_scale));

		for (const wchar_t* p = text; *p; ++p)
		{
			if (*p == L'\r' || // Windows breaks
				*p == L'\n') // Unix breaks
			{
				if (*p==L'\r' && p[1] == L'\n') // Windows breaks
					++p;
				dim.Height += thisLine.Height;
				if (dim.Width < thisLine.Width)
					dim.Width = thisLine.Width;
				thisLine.Width = 0;
				continue;
			}

			bool fallback = false;
			const SFontArea &area = getAreaFromCharacter(*p, &fallback);

			thisLine.Width += area.underhang;

			thisLine.Width += getCharWidth(area, fallback);
		}

		dim.Height += thisLine.Height;
		if (dim.Width < thisLine.Width) dim.Width = thisLine.Width;

		// std::cout << "ScalableFont::getDimension returns : " << dim.Width << ", " << dim.Height << " --> ";

		dim.Width  = (int)(dim.Width + 0.9f); // round up
		dim.Height = (int)(dim.Height + 0.9f);

		//std::cout << dim.Width << ", " << dim.Height << std::endl;

		return dim;
	}
	//! Calculates the index of the character in the text which is on a specific position.
	virtual s32 getCharacterFromPos(const wchar_t* text, s32 pixel_x) const
	{
		s32 x = 0;
		s32 idx = 0;

		while (text[idx])
		{
			const SFontArea& a = Areas[getAreaIDFromCharacter(text[idx], NULL)];

			x += a.width + a.overhang + a.underhang + GlobalKerningWidth;

			if (x >= pixel_x)
				return idx;

			++idx;
		}

		return -1;
	}
	//! Returns the type of this font
	virtual gui::EGUI_FONT_TYPE getType() const { return gui::EGFT_BITMAP; }

	//! set an Pixel Offset on Drawing ( scale position on width )
	virtual void setKerningWidth (s32 kerning)
	{
		GlobalKerningWidth = kerning;
	}
	virtual void setKerningHeight (s32 kerning)
	{
		GlobalKerningHeight = kerning;
	}
	//! set an Pixel Offset on Drawing ( scale position on width )
	virtual s32 getKerningWidth(const wchar_t* thisLetter=0, const wchar_t* previousLetter=0) const
	{
		s32 ret = GlobalKerningWidth;

		if (thisLetter)
		{
			ret += Areas[getAreaIDFromCharacter(*thisLetter, NULL)].overhang;

			if (previousLetter)
			{
				ret += Areas[getAreaIDFromCharacter(*previousLetter, NULL)].underhang;
			}
		}

		return ret;
	}
	virtual s32 getKerningHeight() const
	{
		return GlobalKerningHeight;
	}

	//! gets the sprite bank
	virtual gui::IGUISpriteBank* getSpriteBank() const
	{
		return SpriteBank;
	}

	//! returns the sprite number from a given character
	virtual u32 getSpriteNoFromChar(const wchar_t *c) const
	{
		return Areas[getAreaIDFromCharacter(*c, NULL)].spriteno;
	}

	virtual void setInvisibleCharacters( const wchar_t *s )
	{
		Invisible = s;
	}

private:

	struct SFontArea
	{
		SFontArea() : underhang(0), overhang(0), width(0), spriteno(0) {}
		s32 underhang;
		s32 overhang;
		s32 width;
		u32 spriteno;
	};

	int getCharWidth(const SFontArea& area, const bool fallback) const
	{
		core::array< gui::SGUISprite >& sprites = SpriteBank->getSprites();
		core::array< gui::SGUISprite >* fallback_sprites = (m_fallback_font != NULL ?
													&m_fallback_font->SpriteBank->getSprites() :
													NULL);

		const int texID = (fallback ?
				(*fallback_sprites)[area.spriteno].Frames[0].textureNumber :
				sprites[area.spriteno].Frames[0].textureNumber);

		const TextureInfo& info = (fallback ?
					(*(m_fallback_font->m_texture_files.find(texID))).second :
					(*(m_texture_files.find(texID))).second);
		const float char_scale = info.m_scale;

		//std::cout << "area.spriteno=" << area.spriteno << ", char_scale=" << char_scale << std::endl;

		if (fallback)
			return (int)(((area.width + area.overhang)*m_fallback_font_scale + m_fallback_kerning_width) * m_scale * char_scale);
		else
			return (int)((area.width + area.overhang + GlobalKerningWidth) * m_scale * char_scale);
	}
	s32 getAreaIDFromCharacter(const wchar_t c, bool* fallback_font) const
	{
		std::map<wchar_t, s32>::const_iterator n = CharacterMap.find(c);
		if (n != CharacterMap.end())
		{
			if (fallback_font != NULL)
				*fallback_font = false;
			return (*n).second;
		}
		else if (m_fallback_font != NULL && fallback_font != NULL)
		{
			*fallback_font = true;
			return m_fallback_font->getAreaIDFromCharacter(c, NULL);
		}
		else
		{
			// std::cout << "The font does not have this character : <" << (int)c << ">" << std::endl;
			if (fallback_font != NULL)
				*fallback_font = false;
			return WrongCharacter;
		}
	}
	const SFontArea &getAreaFromCharacter(const wchar_t c, bool* fallback_font) const
	{
		const int area_id = getAreaIDFromCharacter(c, fallback_font);
		const bool use_fallback_font = (fallback_font && *fallback_font);

		// Note: fallback_font can be NULL
		return ( use_fallback_font ? m_fallback_font->Areas[area_id] : Areas[area_id]);
	}   // getAreaFromCharacter
	void setMaxHeight()
	{
		// FIXME: should consider per-texture scaling
		MaxHeight = 0;
		s32 t;

		core::array< core::rect<s32> >& p = SpriteBank->getPositions();

		for (u32 i=0; i<p.size(); ++i)
		{
			t = p[i].getHeight();
			if (t>MaxHeight)
				MaxHeight = t;
		}
	}
	core::array<SFontArea>	Areas;
	/** The maximum values of all digits, used in monospace_digits. */
	mutable SFontArea	m_max_digit_area;
	std::map<wchar_t, s32>	CharacterMap;
	video::IVideoDriver*	Driver;
	gui::IGUISpriteBank*	SpriteBank;
	gui::IGUIEnvironment*	Environment;
	u32			WrongCharacter;
	s32			MaxHeight;
	s32			GlobalKerningWidth, GlobalKerningHeight;

	core::stringw Invisible;
};
}

// The actual bug that was behind this issue was the combination of
// 2d rendering and mipmaps. The issue was reproduced using the special
// draw2dimage version, hence the name.
static bool draw2DImage4c(video::E_DRIVER_TYPE type)
{
	IrrlichtDevice *device = createDevice(type, core::dimension2d<u32>(240, 120));

	if (!device)
		return true; // could not create selected driver.

	video::IVideoDriver* driver = device->getVideoDriver();

	if (!driver->queryFeature(video::EVDF_BILINEAR_FILTER))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	stabilizeScreenBackground(driver);

	logTestString("Testing driver %ls\n", driver->getName());

	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS,true);
	driver->setTextureCreationFlag(video::ETCF_OPTIMIZED_FOR_QUALITY,true);

	video::ITexture* images = driver->getTexture("../media/2ddemo.png");
	driver->makeColorKeyTexture(images, core::position2d<s32>(0,0));

	core::rect<s32> imp1(349,15,385,78);
	core::rect<s32> imp2(387,15,423,78);

	// font cannot handle loading from sub-dirs
	io::path cwd = device->getFileSystem()->getWorkingDirectory();
	device->getFileSystem()->changeWorkingDirectoryTo("media");

	ScalableFont* font = new ScalableFont(device->getGUIEnvironment(), "title_font.xml");
	font->m_fallback_font_scale = 4.0f;
	font->m_fallback_kerning_width = 15;
	font->setKerningWidth(-18);
	font->m_black_border = true;

	/*
	Prepare a nicely filtering 2d render mode for special cases.
	*/
	driver->getMaterial2D().UseMipMaps = true;
	driver->getMaterial2D().TextureLayer[0].BilinearFilter = true;

	{
		driver->beginScene(true, true, video::SColor(255,120,102,136));

		driver->enableMaterial2D();

		// draw fire & dragons background world
		driver->draw2DImage(images, core::position2di(),
							core::rect<s32>(0,0,342,224), 0,
							video::SColor(255,255,255,255), true);

		// draw flying imp
		driver->draw2DImage(images, core::position2d<s32>(114,75),
							imp1, 0, video::SColor(255,255,255,255), true);

		// draw second flying imp
		driver->draw2DImage(images, core::position2d<s32>(220,55),
							imp2, 0, video::SColor(255,255,255,255), true);

		driver->draw2DImage(images, core::rect<s32>(10,10,108,48),
							core::rect<s32>(354,87,442,118));

		video::SColor colors[] = {0xff00ffff, 0xff00ffff, 0xffffff00, 0xffffff00};
		driver->draw2DImage(images, core::recti(10,50,108,88),
			core::recti(354,87,442,118), 0, colors, true);

		font->draw( L"WXYZsSdDrRjJbB", core::rect<s32>(30,20,300,300),
				video::SColor(255,255,255,255) );

		driver->enableMaterial2D(false);

		driver->draw2DImage(images, core::recti(10,90,108,128),
			core::recti(354,87,442,118), 0, colors, true);

		font->draw( L"WXYZsSdDrRjJbB", core::rect<s32>(30,60,300,400),
				video::SColor(255,255,255,255) );

		driver->endScene();
	}
	font->drop();
	device->getFileSystem()->changeWorkingDirectoryTo(cwd);

	// don't go under 99% as the difference is not very large
	bool result = takeScreenshotAndCompareAgainstReference(driver, "-draw2DImage4cFilter.png");

	device->closeDevice();
	device->run();
	device->drop();
	return result;
}

// This test renders a 3d scene and a gui on top of it. The GUI is
// filtered via 2dmaterial (blurred).
// TODO: Works only for OpenGL right now
static bool addBlend2d(video::E_DRIVER_TYPE type)
{
	SIrrlichtCreationParameters params;
	params.AntiAlias = 0;
	params.Bits = 32;
	params.WindowSize = core::dimension2d<u32>(160, 120);
	params.DriverType = type;

	IrrlichtDevice *device = createDeviceEx(params);

	if (!device)
		return true; // in case the driver type does not exist

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();

	if (!driver->queryFeature(video::EVDF_BILINEAR_FILTER))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	logTestString("Testing driver %ls\n", driver->getName());

	scene::IAnimatedMesh* mesh = smgr->getMesh("../media/sydney.md2");
	if (!mesh)
	{
		device->closeDevice();
		device->run();
		device->drop();
		return false;
	}

	stabilizeScreenBackground(driver);

	scene::IAnimatedMeshSceneNode* node = smgr->addAnimatedMeshSceneNode( mesh );

	if (node)
	{
		node->setMaterialFlag(video::EMF_LIGHTING, false);
		node->setMD2Animation(scene::EMAT_STAND);
		node->setMaterialTexture( 0, driver->getTexture("../media/sydney.bmp") );
	}

	smgr->addCameraSceneNode(0, core::vector3df(0,30,-40), core::vector3df(0,5,0));

	gui::IGUIEnvironment* env = device->getGUIEnvironment();
	{
		// create the toolbox window
		gui::IGUIWindow* wnd = env->addWindow(core::rect<s32>(0,0,800,480),
			false, L"Toolset", 0, 100);

		// create tab control and tabs
		gui::IGUITabControl* tab = env->addTabControl(
			core::rect<s32>(2,20,800-602,480-7), wnd, true, true);

		gui::IGUITab* t1 = tab->addTab(L"Config");

		// add some edit boxes and a button to tab one
		env->addImage(driver->getTexture("../media/tools.png"), core::vector2d<s32>(10,20), true, t1);
		env->addStaticText(L"X:", core::rect<s32>(22,48,40,66), false, false, t1);
		env->addEditBox(L"1.0", core::rect<s32>(40,46,130,66), true, t1, 201);

		// quick scale buttons
		env->addButton(core::rect<s32>(65,20,95,40), t1, 102, L"* 10");
		env->addButton(core::rect<s32>(100,20,130,40), t1, 103, L"* 0.1");
	}

	video::SMaterial& material2D = driver->getMaterial2D();
	for (unsigned int n=0; n<video::MATERIAL_MAX_TEXTURES; n++)
	{
		material2D.TextureLayer[n].BilinearFilter = true;
		material2D.TextureLayer[n].TextureWrapU = video::ETC_CLAMP_TO_EDGE;
		material2D.TextureLayer[n].TextureWrapV = video::ETC_CLAMP_TO_EDGE;
	}
	material2D.AntiAliasing=video::EAAM_FULL_BASIC;

	driver->beginScene(true, true, video::SColor(255,100,101,140));
	smgr->drawAll();
	driver->enableMaterial2D();
	env->drawAll();
	driver->enableMaterial2D(false);
	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-addBlend2D.png", 98.2f);

	device->closeDevice();
	device->run();
	device->drop();
	return result;
}

// This test renders 4 times the same image. Two via IGUIImage, two via draw2DImage
// 3 of the 4 images are filtered via 2dmaterial and bilinear filter, only the one
// at the bottom left is not.
static bool moreFilterTests(video::E_DRIVER_TYPE type)
{
	IrrlichtDevice* device = irr::createDevice(type, core::dimension2du(160,120));
	if (!device)
		return true;

	video::IVideoDriver* driver = device->getVideoDriver();
	gui::IGUIEnvironment* gui = device->getGUIEnvironment();

	if (!driver->queryFeature(video::EVDF_BILINEAR_FILTER))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	stabilizeScreenBackground(driver);

	logTestString("Testing driver %ls\n", driver->getName());

	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
	video::ITexture* tex = driver->getTexture("../media/irrlichtlogo.jpg");
	gui::IGUIImage* image = gui->addImage(core::recti(0,0,64,64));
	image->setScaleImage(true);
	image->setImage(tex);
	image->setUseAlphaChannel(true);
	driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
	driver->getMaterial2D().TextureLayer[0].TrilinearFilter=true;

	{
		driver->beginScene(true, true, irr::video::SColor(255,255,255,255));

		// all three logos should be with filtering
		driver->enableMaterial2D();

		driver->getMaterial2D().setTexture(0, 0);
		driver->draw2DImage(tex, irr::core::rect<irr::s32>(64, 64, 128, 128), irr::core::rect<irr::s32>(0, 0, 88, 31));

		driver->getMaterial2D().setTexture(0, tex);
		driver->draw2DImage(tex, irr::core::rect<irr::s32>(64, 0, 128, 64), irr::core::rect<irr::s32>(0, 0, 88, 31));

		gui->drawAll();

		// the next gui image should be without filter
		driver->enableMaterial2D(false);
		image->setRelativePosition(core::recti(0,64,64,128));
		gui->drawAll();

		driver->endScene();
	}

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-2dmatFilter.png");

	device->closeDevice();
	device->run();
	device->drop();
	return result;
}

bool twodmaterial()
{
	bool result = true;
	TestWithAllDrivers(addBlend2d);
	TestWithAllDrivers(moreFilterTests);
	TestWithAllDrivers(draw2DImage4c);
	return result;
}
