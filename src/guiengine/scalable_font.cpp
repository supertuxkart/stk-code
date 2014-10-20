// Copyright (C) 2002-2013 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "guiengine/scalable_font.hpp"

#include <IGUIEnvironment.h>
#include <IXMLReader.h>
#include <IReadFile.h>
#include <IVideoDriver.h>
#include <IGUISpriteBank.h>

#include "guiengine/engine.hpp"
#include "io/file_manager.hpp"
#include "utils/translation.hpp"
#include "graphics/2dutils.hpp"

namespace irr
{
namespace gui
{

//! constructor
ScalableFont::ScalableFont(IGUIEnvironment *env, const std::string &filename)
            : Driver(0), SpriteBank(0), Environment(env), WrongCharacter(0),
              MaxHeight(0), GlobalKerningWidth(0), GlobalKerningHeight(0)
{
#ifdef _DEBUG
    setDebugName("ScalableFont");
#endif

    m_fallback_font          = NULL;
    m_fallback_kerning_width = 0;
    m_fallback_font_scale    = 1.0f;
    m_scale                  = 1.0f;
    m_is_hollow_copy         = false;
    m_black_border           = false;
    m_shadow                 = false;
    m_mono_space_digits      = false;
    m_rtl                    = translations->isRTLLanguage();

    if (Environment)
    {
        // don't grab environment, to avoid circular references
        Driver = Environment->getVideoDriver();

        SpriteBank = Environment->addEmptySpriteBank(io::path(filename.c_str()));
        if (SpriteBank)
            SpriteBank->grab();
    }

    if (Driver)
        Driver->grab();

    setInvisibleCharacters ( L" " );

    io::IXMLReader* reader = file_manager->createXMLReader(filename.c_str());
    if (!load( reader ))
    {
        Log::fatal("ScalableFont", "Loading font failed");
    }
    reader->drop();

    assert(Areas.size() > 0);
}


//! destructor
ScalableFont::~ScalableFont()
{
    if (!m_is_hollow_copy)
    {
        if (Driver)     Driver->drop();
        if (SpriteBank) SpriteBank->drop();
    }
}

void ScalableFont::updateRTL()
{
    m_rtl = translations->isRTLLanguage();
}

void ScalableFont::setShadow(const irr::video::SColor &col)
{
    m_shadow = true;
    m_shadow_color = col;
}

void ScalableFont::doReadXmlFile(io::IXMLReader* xml)
{
    int trim_top = 0;
    int trim_bottom = 0;

    while (xml->read())
    {
        if (io::EXN_ELEMENT == xml->getNodeType())
        {
            if (core::stringw(L"include") == xml->getNodeName())
            {
                core::stringc filename = xml->getAttributeValue(L"file");
                /*
                const wchar_t* iflangis = xml->getAttributeValue(L"iflanguage");

                Log::info("ScalableFont", "langcode = %s", translations->getCurrentLanguageCode().c_str());

                if (iflangis != NULL &&
                    core::stringc(iflangis) != translations->getCurrentLanguageCode().c_str())
                {
                    continue;
                }
                */

                io::IXMLReader* included = file_manager->createXMLReader(
                    file_manager->getAsset(FileManager::FONT, filename.c_str()));
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
                core::stringc fn = file_manager->getAsset(FileManager::FONT,
                                                          filename.c_str()).c_str();
                u32 i = (u32)xml->getAttributeValueAsInt(L"index");

                float scale=1.0f;
                if(xml->getAttributeValue(L"scale"))
                {
                    scale = xml->getAttributeValueAsFloat(L"scale");
                    //Log::info("ScalableFont", "scale = %f", scale);
                }

                bool excludeFromMaxHeightCalculation = false;
                if (xml->getAttributeValue(L"excludeFromMaxHeightCalculation"))
                    excludeFromMaxHeightCalculation = (core::stringc(xml->getAttributeValue(L"excludeFromMaxHeightCalculation")) == "true");

                core::stringw alpha = xml->getAttributeValue(L"hasAlpha");

                //Log::info("ScalableFont", "Adding font texture %s; alpha = %s", fn.c_str(), alpha.c_str());


                // make sure the sprite bank has enough textures in it
                while (i+1 > SpriteBank->getTextureCount())
                {
                    SpriteBank->addTexture(NULL);
                }

                TextureInfo info;
                info.m_file_name   = fn;
                info.m_has_alpha   = (alpha == core::stringw("true"));
                info.m_scale       = scale;
                info.m_exclude_from_max_height_calculation = excludeFromMaxHeightCalculation;


#ifdef DEBUG
                if (m_texture_files.find(i) != m_texture_files.end())
                {
                    Log::warn("ScalableFont", "Font conflict, two images have texture %i.", i);
                }
#endif

                m_texture_files[i] = info;
            }
            else if (core::stringw(L"font") == xml->getNodeName())
            {
                trim_top = (u32)xml->getAttributeValueAsInt(L"trim_top"); // returns 0 if no such attribute
                trim_bottom = (u32)xml->getAttributeValueAsInt(L"trim_bottom");
            }
            else if (core::stringw(L"c") == xml->getNodeName())
            {
                // adding a character to this font
                SFontArea a;
                SGUISpriteFrame f;
                SGUISprite s;
                core::rect<s32> rectangle;

                a.underhang     = xml->getAttributeValueAsInt(L"u");
                a.overhang      = xml->getAttributeValueAsInt(L"o");
                a.spriteno      = SpriteBank->getSprites().size();
                s32 texno       = xml->getAttributeValueAsInt(L"i");

                // parse rectangle
                core::stringc rectstr   = xml->getAttributeValue(L"r");
                wchar_t ch      = xml->getAttributeValue(L"c")[0];

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
                rectangle.UpperLeftCorner.Y = val + trim_top;
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
                rectangle.LowerRightCorner.Y = val - trim_bottom;

                CharacterMap[ch] = Areas.size();

                //Log::info("ScalableFont", "Inserting character '%d' with area %d", (int)ch, Areas.size());

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

//! loads a font file from xml
bool ScalableFont::load(io::IXMLReader* xml)
{
    if (!SpriteBank)
    {
        Log::error("ScalableFont::load", "SpriteBank is NULL!!");
        return false;
    }

    doReadXmlFile(xml);

    // set bad character
    WrongCharacter = getAreaIDFromCharacter(L' ', NULL);

    setMaxHeight();

    for(wchar_t c='0'; c<='9'; c++)
    {
        SFontArea a = getAreaFromCharacter(c, NULL);
        if(a.overhang  > m_max_digit_area.overhang ) m_max_digit_area.overhang  = a.overhang;
        if(a.underhang > m_max_digit_area.underhang) m_max_digit_area.underhang = a.underhang;
        if(a.width     > m_max_digit_area.width    ) m_max_digit_area.width     = a.width;
    }
    m_max_digit_area.overhang = 0;m_max_digit_area.underhang=0;
    return true;
}
void ScalableFont::setScale(const float scale)
{
    m_scale = scale;
}

void ScalableFont::setMaxHeight()
{
    MaxHeight = 0;
    s32 t;

    core::array< core::rect<s32> >& p  = SpriteBank->getPositions();
    core::array< SGUISprite >& sprites = SpriteBank->getSprites();

    for (u32 i=0; i<p.size(); ++i)
    {
        t = p[i].getHeight();

        // FIXME: consider fallback fonts
        int texID = sprites[i].Frames[0].textureNumber;

        const TextureInfo& info = (*(m_texture_files.find(texID))).second;
        if (info.m_exclude_from_max_height_calculation) continue;

        float char_scale = info.m_scale;
        t = (int)(t*char_scale);

        if (t>MaxHeight)
            MaxHeight = t;
    }

    MaxHeight = (int)(MaxHeight*m_scale);
}



//! set an Pixel Offset on Drawing ( scale position on width )
void ScalableFont::setKerningWidth(s32 kerning)
{
    GlobalKerningWidth = kerning;
}


//! set an Pixel Offset on Drawing ( scale position on width )
s32 ScalableFont::getKerningWidth(const wchar_t* thisLetter, const wchar_t* previousLetter) const
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


//! set an Pixel Offset on Drawing ( scale position on height )
void ScalableFont::setKerningHeight(s32 kerning)
{
    GlobalKerningHeight = kerning;
}


//! set an Pixel Offset on Drawing ( scale position on height )
s32 ScalableFont::getKerningHeight () const
{
    return GlobalKerningHeight;
}


//! returns the sprite number from a given character
u32 ScalableFont::getSpriteNoFromChar(const wchar_t *c) const
{
    return Areas[getAreaIDFromCharacter(*c, NULL)].spriteno;
}


s32 ScalableFont::getAreaIDFromCharacter(const wchar_t c, bool* fallback_font) const
{
    std::map<wchar_t, s32>::const_iterator n = CharacterMap.find(c);
    if (n != CharacterMap.end())
    {
        if (fallback_font != NULL) *fallback_font = false;
        //Log::info("ScalableFont", "Character %d found in font", (int)c);
        return (*n).second;
    }
    else if (m_fallback_font != NULL && fallback_font != NULL)
    {
        //Log::warn("ScalableFont", "Font does not have this character: <%d>, try fallback font", (int)c);
        *fallback_font = true;
        return m_fallback_font->getAreaIDFromCharacter(c, NULL);
    }
    else
    {
        //Log::warn("ScalableFont", "The font does not have this character: <%d>", (int)c);
        if (fallback_font != NULL) *fallback_font = false;
        return WrongCharacter;
    }
}

const ScalableFont::SFontArea &ScalableFont::getAreaFromCharacter(const wchar_t c,
                                                    bool* fallback_font) const
{
    const int area_id = getAreaIDFromCharacter(c, fallback_font);


    if(m_mono_space_digits && ( (c>=L'0' && c<=L'9') || c==L' ' ) )
    {
        const SFontArea &area = (fallback_font && *fallback_font)
                              ? m_fallback_font->Areas[area_id]
                              : Areas[area_id];
        m_max_digit_area.spriteno = area.spriteno;
        return m_max_digit_area;
    }

    const bool use_fallback_font = (fallback_font && *fallback_font);

    if (use_fallback_font)
    {
        assert(area_id < (int)m_fallback_font->Areas.size());
    }
    else
    {
        assert(area_id < (int)Areas.size());
    }

    // Note: fallback_font can be NULL
    return ( use_fallback_font ? m_fallback_font->Areas[area_id] : Areas[area_id]);
}   // getAreaFromCharacter


void ScalableFont::setInvisibleCharacters( const wchar_t *s )
{
    Invisible = s;
}


//! returns the dimension of text
core::dimension2d<u32> ScalableFont::getDimension(const wchar_t* text) const
{
    assert(Areas.size() > 0);

    core::dimension2d<u32> dim(0, 0);
    core::dimension2d<u32> thisLine(0, (int)(MaxHeight*m_scale));

    for (const wchar_t* p = text; *p; ++p)
    {
        if (*p == L'\r'  ||      // Windows breaks
            *p == L'\n'      )   // Unix breaks
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

   //Log::info("ScalableFont", "ScalableFont::getDimension returns: %d, %d", dim.Width, dim.Height);

    dim.Width  = (int)(dim.Width + 0.9f); // round up
    dim.Height = (int)(dim.Height + 0.9f);

    //Log::info("ScalableFont", "After: %d, %d", dim.Width, dim.Height);

    return dim;
}

void ScalableFont::draw(const core::stringw& text,
    const core::rect<s32>& position, video::SColor color,
    bool hcenter, bool vcenter,
    const core::rect<s32>* clip)
{
    doDraw(text, position, color, hcenter, vcenter, clip, NULL);
}

void ScalableFont::draw(const core::stringw& text,
                        const core::rect<s32>& position, video::SColor color,
                        bool hcenter, bool vcenter,
                        const core::rect<s32>* clip, bool ignoreRTL)
{
    bool previousRTL = m_rtl;
    if (ignoreRTL) m_rtl = false;

    doDraw(text, position, color, hcenter, vcenter, clip, NULL);

    if (ignoreRTL) m_rtl = previousRTL;
}

//! draws some text and clips it to the specified rectangle if wanted
void ScalableFont::doDraw(const core::stringw& text,
                          const core::rect<s32>& position, video::SColor color,
                          bool hcenter, bool vcenter,
                          const core::rect<s32>* clip,
                          FontCharCollector* charCollector)
{
    if (!Driver) return;

    if (m_shadow)
    {
        m_shadow = false; // avoid infinite recursion

        core::rect<s32> shadowpos = position;
        shadowpos.LowerRightCorner.X += 2;
        shadowpos.LowerRightCorner.Y += 2;

        draw(text, shadowpos, m_shadow_color, hcenter, vcenter, clip);

        m_shadow = true; // set back
    }

    core::position2d<s32> offset = position.UpperLeftCorner;
    core::dimension2d<s32> text_dimension;

    if (m_rtl || hcenter || vcenter || clip)
    {
        text_dimension = getDimension(text.c_str());

        if (hcenter)    offset.X += (position.getWidth() - text_dimension.Width) / 2;
        else if (m_rtl) offset.X += (position.getWidth() - text_dimension.Width);

        if (vcenter)    offset.Y += (position.getHeight() - text_dimension.Height) / 2;
        if (clip)
        {
            core::rect<s32> clippedRect(offset, text_dimension);
            clippedRect.clipAgainst(*clip);
            if (!clippedRect.isValid()) return;
        }
    }

    // ---- collect character locations
    const unsigned int text_size = text.size();
    core::array<s32>               indices(text_size);
    core::array<core::position2di> offsets(text_size);
    std::vector<bool>              fallback(text_size);

    for (u32 i = 0; i<text_size; i++)
    {
        wchar_t c = text[i];

        if (c == L'\r' ||          // Windows breaks
            c == L'\n'    )        // Unix breaks
        {
            if(c==L'\r' && text[i+1]==L'\n') c = text[++i];
            offset.Y += (int)(MaxHeight*m_scale);
            offset.X  = position.UpperLeftCorner.X;
            if (hcenter)
                offset.X += (position.getWidth() - text_dimension.Width) >> 1;
            continue;
        }   // if lineBreak

        bool use_fallback_font = false;
        const SFontArea &area  = getAreaFromCharacter(c, &use_fallback_font);
        fallback[i]            = use_fallback_font;
        offset.X              += area.underhang;
        offsets.push_back(offset);
        // Invisible character. add something to the array anyway so that
        // indices from the various arrays remain in sync
        indices.push_back( Invisible.findFirst(c) < 0  ? area.spriteno
                                                       : -1            );
        offset.X += getCharWidth(area, fallback[i]);
    }   // for i<text_size

    // ---- do the actual rendering
    const int indiceAmount                    = indices.size();
    core::array< SGUISprite >& sprites        = SpriteBank->getSprites();
    core::array< core::rect<s32> >& positions = SpriteBank->getPositions();
    core::array< SGUISprite >* fallback_sprites;
    core::array< core::rect<s32> >* fallback_positions;
    if(m_fallback_font!=NULL)
    {
        fallback_sprites   = &m_fallback_font->SpriteBank->getSprites();
        fallback_positions = &m_fallback_font->SpriteBank->getPositions();
    }
    else
    {
        fallback_sprites   = NULL;
        fallback_positions = NULL;
    }

    const int spriteAmount      = sprites.size();
    for (int n=0; n<indiceAmount; n++)
    {
        const int spriteID = indices[n];
        if (!fallback[n] && (spriteID < 0 || spriteID >= spriteAmount)) continue;
        if (indices[n] == -1) continue;

        //assert(sprites[spriteID].Frames.size() > 0);

        const int texID = (fallback[n] ?
                           (*fallback_sprites)[spriteID].Frames[0].textureNumber :
                           sprites[spriteID].Frames[0].textureNumber);

        core::rect<s32> source = (fallback[n] ?
                                  (*fallback_positions)[(*fallback_sprites)[spriteID].Frames[0].rectNumber] :
                                  positions[sprites[spriteID].Frames[0].rectNumber]);

        const TextureInfo& info = (fallback[n] ?
                                   (*(m_fallback_font->m_texture_files.find(texID))).second :
                                   (*(m_texture_files.find(texID))).second
                                   );
        float char_scale = info.m_scale;

        core::dimension2d<s32> size = source.getSize();

        float scale = (fallback[n] ? m_scale*m_fallback_font_scale : m_scale);
        size.Width  = (int)(size.Width  * scale * char_scale);
        size.Height = (int)(size.Height * scale * char_scale);

        // align vertically if character is smaller
        int y_shift = (size.Height < MaxHeight*m_scale ? (int)((MaxHeight*m_scale - size.Height)/2.0f) : 0);

        core::rect<s32> dest(offsets[n] + core::position2di(0, y_shift), size);

        video::ITexture* texture = (fallback[n] ?
                                    m_fallback_font->SpriteBank->getTexture(texID) :
                                    SpriteBank->getTexture(texID) );

        /*
        if (fallback[n])
        {
            Log::info("ScalableFont", "Using fallback font %s; source area is %d, %d; size %d, %d; dest = %d, %d",
                core::stringc(texture->getName()).c_str(), source.UpperLeftCorner.X, source.UpperLeftCorner.Y,
                source.getWidth(), source.getHeight(), offsets[n].X, offsets[n].Y);
        }
        */

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
                Log::warn("ScalableFont", "Character not found in current font");
                continue; // no such character
            }
        }

        if (m_black_border && charCollector == NULL)
        {
            // draw black border
            video::SColor black(color.getAlpha(),0,0,0);

            for (int x_delta=-2; x_delta<=2; x_delta++)
            {
                for (int y_delta=-2; y_delta<=2; y_delta++)
                {
                    if (x_delta == 0 || y_delta == 0) continue;
                    draw2DImage(texture,
                                dest + core::position2d<s32>(x_delta, y_delta),
                                source,
                                clip,
                                black, true);
                }
            }
        }

        if (fallback[n])
        {
            // TODO: don't hardcode colors?
            static video::SColor orange(color.getAlpha(), 255, 100, 0);
            static video::SColor yellow(color.getAlpha(), 255, 220, 15);
            video::SColor title_colors[] = {yellow, orange, orange, yellow};

            if (charCollector != NULL)
            {
                charCollector->collectChar(texture,
                    dest,
                    source,
                    title_colors);
            }
            else
            {
                draw2DImage(texture,
                    dest,
                    source,
                    clip,
                    title_colors, true);
            }
        }
        else
        {
            if (charCollector != NULL)
            {
                video::SColor colors[] = { color, color, color, color };
                charCollector->collectChar(texture,
                    dest,
                    source,
                    colors);
            }
            else
            {
                draw2DImage(texture,
                    dest,
                    source,
                    clip,
                    color, true);
            }
#ifdef FONT_DEBUG
            video::IVideoDriver* driver = GUIEngine::getDriver();
            driver->draw2DLine(core::position2d<s32>(dest.UpperLeftCorner.X,  dest.UpperLeftCorner.Y),
                               core::position2d<s32>(dest.UpperLeftCorner.X,  dest.LowerRightCorner.Y),
                               video::SColor(255, 255,0,0));
            driver->draw2DLine(core::position2d<s32>(dest.LowerRightCorner.X, dest.LowerRightCorner.Y),
                               core::position2d<s32>(dest.LowerRightCorner.X, dest.UpperLeftCorner.Y),
                               video::SColor(255, 255,0,0));
            driver->draw2DLine(core::position2d<s32>(dest.LowerRightCorner.X, dest.LowerRightCorner.Y),
                               core::position2d<s32>(dest.UpperLeftCorner.X,  dest.LowerRightCorner.Y),
                               video::SColor(255, 255,0,0));
            driver->draw2DLine(core::position2d<s32>(dest.UpperLeftCorner.X,  dest.UpperLeftCorner.Y),
                               core::position2d<s32>(dest.LowerRightCorner.X, dest.UpperLeftCorner.Y),
                               video::SColor(255, 255,0,0));
#endif
        }
    }
}


void ScalableFont::lazyLoadTexture(int texID)
{
    Driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

    // load texture
    assert(m_texture_files[texID].m_file_name.size() > 0);
    SpriteBank->setTexture(texID, Driver->getTexture( m_texture_files[texID].m_file_name ));

    // set previous mip-map+filter state
    //Driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, mipmap);

    // couldn't load texture, abort.
    if (!SpriteBank->getTexture(texID))
    {
        Log::error("ScalableFont::lazyLoadTexture", "Unable to load all textures in the font");
        _IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
        return;
    }
    else
    {
        assert(m_texture_files[texID].m_file_name.size() > 0);

        // colorkey texture rather than alpha channel?
        if (! m_texture_files[texID].m_has_alpha)
        {
            Driver->makeColorKeyTexture(SpriteBank->getTexture(texID), core::position2di(0,0));
        }
    }
}

int ScalableFont::getCharWidth(const SFontArea& area, const bool fallback) const
{
    core::array< SGUISprite >& sprites = SpriteBank->getSprites();
    core::array< SGUISprite >* fallback_sprites = (m_fallback_font != NULL ?
                                                   &m_fallback_font->SpriteBank->getSprites() :
                                                   NULL);

    const int texID = (fallback ?
                       (*fallback_sprites)[area.spriteno].Frames[0].textureNumber :
                       sprites[area.spriteno].Frames[0].textureNumber);

    const TextureInfo& info = (fallback ?
                               (*(m_fallback_font->m_texture_files.find(texID))).second :
                                (*(m_texture_files.find(texID))).second
                               );
    assert(info.m_file_name.size() > 0);
    const float char_scale = info.m_scale;

    //Log::info("ScalableFont", "area.spriteno = %d, char_scale = %f", area.spriteno, char_scale);

    if (fallback) return (int)(((area.width + area.overhang)*m_fallback_font_scale + m_fallback_kerning_width) * m_scale * char_scale);
    else          return (int)((area.width + area.overhang + GlobalKerningWidth) * m_scale * char_scale);
}


//! Calculates the index of the character in the text which is on a specific position.
s32 ScalableFont::getCharacterFromPos(const wchar_t* text, s32 pixel_x) const
{
    s32 x = 0;
    s32 idx = 0;

    while (text[idx])
    {
        bool use_fallback_font = false;
        const SFontArea &a  = getAreaFromCharacter(text[idx], &use_fallback_font);

        x += getCharWidth(a, use_fallback_font) + a.overhang + a.underhang + GlobalKerningWidth;

        if (x >= pixel_x)
            return idx;

        ++idx;
    }

    return -1;
}


IGUISpriteBank* ScalableFont::getSpriteBank() const
{
    return SpriteBank;
}

} // end namespace gui
} // end namespace irr


