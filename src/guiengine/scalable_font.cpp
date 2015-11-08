//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2002-2012 Nikolaus Gebhardt
//  Copyright (C) 2015 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "guiengine/scalable_font.hpp"

#include "graphics/2dutils.hpp"
#include "guiengine/engine.hpp"
#include "io/file_manager.hpp"
#include "utils/translation.hpp"

#include <IAttributes.h>
#include <IGUIEnvironment.h>
#include <IGUISpriteBank.h>
#include <IVideoDriver.h>

#include <clocale>
#include <cmath>
#include <cwctype>

using namespace GUIEngine;

namespace irr
{
namespace gui
{

// ----------------------------------------------------------------------------

ScalableFont::ScalableFont(IGUIEnvironment *env, TTFLoadingType type)
            : m_video_driver(0), m_spritebank(0), m_gui_env(env),
              m_max_height(0), m_global_kerning_width(0), m_global_kerning_height(0)
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
    m_type                   = type;
    m_font_use               = (FontUse)0;
    m_shadow                 = false;
    m_mono_space_digits      = false;
    m_rtl                    = translations->isRTLLanguage();

    if (m_gui_env)
    {
        // don't grab environment, to avoid circular references
        m_video_driver = m_gui_env->getVideoDriver();

        m_spritebank = m_gui_env->addEmptySpriteBank((std::to_string(type)).c_str());
        if (m_spritebank)
            m_spritebank->grab();
    }

    if (m_video_driver)
        m_video_driver->grab();

    setInvisibleCharacters(L" ");

    if (!loadTTF())
    {
        Log::fatal("ScalableFont", "Loading TTF font failed");
    }

    assert(m_areas.size() > 0);
}

// ----------------------------------------------------------------------------

ScalableFont::~ScalableFont()
{
    if (!m_is_hollow_copy)
    {
        if (m_video_driver)
            m_video_driver->drop();
        if (m_spritebank)
            m_spritebank->drop();
    }
}

// ----------------------------------------------------------------------------

bool ScalableFont::loadTTF()
{
    if (!m_spritebank)
    {
        Log::error("ScalableFont::loadTTF", "SpriteBank is NULL!!");
        return false;
    }

    switch(m_type)
    {
        case T_NORMAL:
            m_font_use = F_DEFAULT;
            break;
        case T_DIGIT:
            m_font_use = F_DIGIT;
            break;
        case T_BOLD:
            m_font_use = F_BOLD;
            break;
    }

    GUIEngine::GlyphPageCreator* gp_creator = GUIEngine::getGlyphPageCreator();

    //Initialize glyph slot
    FT_GlyphSlot slot;
    FT_Error     err;

    std::vector <s32> offset;
    std::vector <s32> bearingx;
    std::vector <s32> advance;
    std::vector <s32> height;

    std::set<wchar_t>::iterator it;
    s32 curr_maxheight = 0;
    s32 t;
    u32 texno = 0;
    m_spritebank->addTexture(NULL);
    gp_creator->clearGlyphPage();
    gp_creator->createNewGlyphPage();

    GUIEngine::FTEnvironment* ft_env = GUIEngine::getFreetype();
    std::set<wchar_t> preload_char = getPreloadCharacters(m_type);

    it = preload_char.begin();
    while (it != preload_char.end())
    {
        SGUISpriteFrame f;
        SGUISprite s;
        core::rect<s32> rectangle;

        int idx;
        if (m_type == T_BOLD) //Lite-Fontconfig for stk, this one is specifically for bold title
        {
            int count = F_BOLD;
            while (count < GUIEngine::F_COUNT)
            {
                m_font_use = (FontUse)count;
                FT_Face curr_face = ft_env->getFace(m_font_use);

                idx = FT_Get_Char_Index(curr_face, *it);
                if (idx > 0) break;
                count++;
            }
        }
        else
        {
            FT_Face curr_face = ft_env->getFace(m_font_use);
            idx = FT_Get_Char_Index(curr_face, *it);
        }

        FT_Face curr_face = ft_env->getFace(m_font_use);
        slot = curr_face->glyph;

        if (idx)
        {
            // Load glyph image into the slot (erase previous one)
            err = FT_Load_Glyph(curr_face, idx,
                  FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL);
            if (err)
                Log::error("ScalableFont::loadTTF", "Can't load a single glyph.");

            // Store vertical offset on line.
            s32 curr_offset = (curr_face->glyph->metrics.height >> 6) - (curr_face->glyph->metrics.horiBearingY >> 6);
            offset.push_back(curr_offset);

            // This is to be used later.
            t = curr_face->glyph->metrics.height >> 6;
            height.push_back(t);
            if (t > curr_maxheight)
                curr_maxheight = t;

            // Store horizontal padding (bearingX).
            s32 curr_bearingx = curr_face->glyph->metrics.horiBearingX >> 6;
            bearingx.push_back(curr_bearingx);

            // Store total width on horizontal line.
            s32 curr_advance = curr_face->glyph->advance.x >> 6;
            advance.push_back(curr_advance);

            // Convert to an anti-aliased bitmap
            FT_Bitmap bits = slot->bitmap;

            m_character_map[*it] = m_spritebank->getSprites().size();

            if (!gp_creator->checkEnoughSpace(bits))
            // Glyph page is full, save current one and reset the current page
            {
#ifdef FONT_DEBUG
                gp_creator->dumpGlyphPage(((std::to_string(m_type)) + "_" + (std::to_string(texno))).c_str());
#endif
                m_spritebank->setTexture(texno, m_video_driver->addTexture("Glyph_page", gp_creator->getPage()));
                gp_creator->clearGlyphPage();
                m_spritebank->addTexture(NULL);
                gp_creator->createNewGlyphPage();
                texno++;
            }

            // Inserting now
            if (gp_creator->insertGlyph(bits, rectangle))
            {
                f.rectNumber = m_spritebank->getPositions().size();
                f.textureNumber = texno;

                // add frame to sprite
                s.Frames.push_back(f);
                s.frameTime = 0;

                m_spritebank->getPositions().push_back(rectangle);
                m_spritebank->getSprites().push_back(s);
            }
            else
                return false;
        }

        // Check for glyph page which can fit all characters
        if (it == --preload_char.end())
        {
#ifdef FONT_DEBUG
            gp_creator->dumpGlyphPage(((std::to_string(m_type)) + "_" + (std::to_string(texno))).c_str());
#endif
            if (m_type == T_NORMAL)
            {
                m_last_normal_page = m_video_driver->addTexture("Glyph_page", gp_creator->getPage());
                m_spritebank->setTexture(texno, m_last_normal_page);
            }
            else
                m_spritebank->setTexture(texno, m_video_driver->addTexture("Glyph_page", gp_creator->getPage()));
        }

        if (*it == (wchar_t)32 && m_spritebank->getPositions().size() == 1)
            continue; //Preventing getAreaIDFromCharacter of whitespace == 0, which make space disappear
        else ++it;
    }

    //Fix unused glyphs....
    if (m_type == T_NORMAL || T_BOLD)
    {
        m_character_map[(wchar_t)9]   = getAreaIDFromCharacter((wchar_t)160, NULL); //Use non-breaking space glyph to tab.
        m_character_map[(wchar_t)173] = 0; //Don't need a glyph for the soft hypen, as it only print when not having enough space.
                                           //And then it will convert to a "-".

        if (m_type == T_NORMAL)
        {
            m_character_map[(wchar_t)8204]  = 0; //They are zero width chars found in Arabic.
            m_character_map[(wchar_t)65279] = 0;
        }
    }

    if (m_type == T_BOLD)
    {
        setlocale(LC_ALL, "en_US.UTF8");
        for  (it = preload_char.begin(); it != preload_char.end(); ++it)
        {
            if (iswupper((wchar_t)*it) && *it < 640)
                m_character_map[towlower((wchar_t)*it)] = getAreaIDFromCharacter(*it, NULL);
        }
    }

    for (unsigned int n = 0 ; n < m_spritebank->getSprites().size(); ++n)
    {
        //Storing now
        SFontArea a;
        a.spriteno      = n;
        a.offsety       = curr_maxheight - height.at(n)
                        + offset.at(n); //Compute the correct offset as ttf texture image is cropped against the glyph fully.

        a.offsety_bt    = -offset.at(n); //FIXME
                                         //Specific offset for billboard text as billboard text seems to be drawn bottom-up,
                                         //as the offset in calculated based on the fact that the characters are drawn all
                                         //at the bottom line, so no addition is required, but if we can make draw2dimage draw
                                         //characters close to the bottom line too, than only one offsety is needed.

        if (!n) //Skip width-less characters
            a.bearingx  = 0;
        else
            a.bearingx  = bearingx.at(n);
        if (!n) //Skip width-less characters
            a.width     = 0;
        else
            a.width     = advance.at(n);
        // add character to font
        m_areas.push_back(a);
    }

    //Reserve 10 for normal font new characters added, 40 for digit font to display separately
    //Consider fallback font (bold) too
    m_max_height = (int)((curr_maxheight + (m_type == T_DIGIT ? 40 : 10) +
                   (m_type == T_BOLD ? 20 : 0))*m_scale);

    m_glyph_max_height = curr_maxheight;

    for(wchar_t c='0'; c<='9'; c++)
    {
        SFontArea a = getAreaFromCharacter(c, NULL);
        m_max_digit_area.width     = a.width;
        m_max_digit_area.offsety   = a.offsety;
        m_max_digit_area.bearingx  = a.bearingx;
    }

    switch (m_type)
    {
        case T_NORMAL:
            Log::info("ScalableFont::loadTTF", "Created %d glyphs "
                       "supporting %d characters for normal font %s using %d glyph page(s)."
                       , m_areas.size(), m_character_map.size(), ft_env->getFace(m_font_use)->family_name, m_spritebank->getTextureCount());
        break;
        case T_DIGIT:
            Log::info("ScalableFont::loadTTF", "Created %d glyphs "
                       "supporting %d characters for high-res digits font %s using %d glyph page(s)."
                       , m_areas.size(), m_character_map.size(), ft_env->getFace(m_font_use)->family_name, m_spritebank->getTextureCount());
        break;
        case T_BOLD:
            Log::info("ScalableFont::loadTTF", "Created %d glyphs "
                       "supporting %d characters for bold title font %s using %d glyph page(s)."
                       , m_areas.size(), m_character_map.size(), ft_env->getFace(m_font_use)->family_name, m_spritebank->getTextureCount());
        break;
    }

    return true;
}

// ----------------------------------------------------------------------------

bool ScalableFont::lazyLoadChar()
{
    //Mainly copy from loadTTF(), so removing unnecessary comments
    if (m_type != T_NORMAL) return false; //Make sure only insert char inside normal font

    FT_GlyphSlot slot;
    FT_Error     err;

    GUIEngine::FTEnvironment* ft_env = GUIEngine::getFreetype();
    GUIEngine::GlyphPageCreator* gp_creator = GUIEngine::getGlyphPageCreator();

    s32 height;
    s32 bearingx;
    s32 curr_offset;
    s32 width;
    u32 texno = m_spritebank->getTextureCount() - 1;
    core::stringw lazy_load = gp_creator->getNewChar();
    for (u32 i = 0; i < lazy_load.size(); ++i)
    {
        SGUISpriteFrame f;
        SGUISprite s;
        core::rect<s32> rectangle;

        //Lite-Fontconfig for stk
        int idx;
        int font = F_DEFAULT;
        while (font <= F_LAST_REGULAR_FONT)
        {
            m_font_use = (FontUse)font;

            idx = FT_Get_Char_Index(ft_env->getFace(m_font_use), lazy_load[i]);
            if (idx > 0) break;

            font++;
        }

        FT_Face curr_face = ft_env->getFace(m_font_use);

        slot = curr_face->glyph;

        if (idx)
        {
            err = FT_Load_Glyph(curr_face, idx,
                  FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL);
            if (err)
                Log::error("ScalableFont::loadTTF", "Can't load a single glyph.");

            curr_offset = (curr_face->glyph->metrics.height >> 6) - (curr_face->glyph->metrics.horiBearingY >> 6);
            height = curr_face->glyph->metrics.height >> 6;
            bearingx = curr_face->glyph->metrics.horiBearingX >> 6;
            width = curr_face->glyph->advance.x >> 6;
            FT_Bitmap bits = slot->bitmap;
            m_character_map[lazy_load[i]] = m_spritebank->getSprites().size();

            if (!gp_creator->checkEnoughSpace(bits))
            {
#ifdef FONT_DEBUG
                gp_creator->dumpGlyphPage(((std::to_string(m_type)) + "_" + (std::to_string(texno))).c_str());
#endif
                m_spritebank->setTexture(texno, m_video_driver->addTexture("Glyph_page", gp_creator->getPage()));
                gp_creator->clearGlyphPage();
                m_spritebank->addTexture(NULL);
                gp_creator->createNewGlyphPage();
                texno++;
            }

            if (gp_creator->insertGlyph(bits, rectangle))
            {
                f.rectNumber = m_spritebank->getPositions().size();
                f.textureNumber = texno;
                s.Frames.push_back(f);
                s.frameTime = 0;
                m_spritebank->getPositions().push_back(rectangle);
                m_spritebank->getSprites().push_back(s);
            }
            else
                return false;

            SFontArea a;
            a.spriteno   = m_spritebank->getSprites().size() - 1;
            a.offsety    = m_glyph_max_height - height + curr_offset;
            a.offsety_bt = -curr_offset;
            a.bearingx   = bearingx;
            a.width      = width;
            m_areas.push_back(a);
        }
        else
            m_character_map[lazy_load[i]] = 1; //Set any wrong characters to 1 (space), preventing it from loading again
    }

#ifdef FONT_DEBUG
    gp_creator->dumpGlyphPage(((std::to_string(m_type)) + "_" + (std::to_string(texno))).c_str());
#endif
    gp_creator->clearNewCharHolder(); //Clear the Newly characters in creator after they are loaded
    m_video_driver->removeTexture(m_last_normal_page); //Remove old texture
    m_last_normal_page = m_video_driver->addTexture("Glyph_page", gp_creator->getPage());
    m_spritebank->setTexture(texno, m_last_normal_page);

    if (!m_is_hollow_copy)
    {
        GUIEngine::cleanHollowCopyFont();
        GUIEngine::reloadHollowCopyFont(GUIEngine::getFont());
    }

    Log::debug("ScalableFont::lazyLoadChar", "New characters drawn by %s inserted, there are %d glyphs "
              "supporting %d characters for normal font using %d glyph page(s) now."
              , ft_env->getFace(m_font_use)->family_name, m_areas.size(), m_character_map.size(), m_spritebank->getTextureCount());

    return true;
}

// ----------------------------------------------------------------------------

void ScalableFont::forceNewPage()
{
    GUIEngine::GlyphPageCreator* gp_creator = GUIEngine::getGlyphPageCreator();

    m_spritebank->setTexture(m_spritebank->getTextureCount() - 1, m_video_driver->addTexture("Glyph_page", gp_creator->getPage()));
    gp_creator->clearGlyphPage();
    m_spritebank->addTexture(NULL);
    gp_creator->createNewGlyphPage();
}

// ----------------------------------------------------------------------------

std::set<wchar_t> ScalableFont::getPreloadCharacters(const GUIEngine::TTFLoadingType type)
{
    std::set<wchar_t> preload_char;

    switch (type)
    {
        case T_NORMAL:
            for (u32 i = 32; i < 128; ++i)
                preload_char.insert((wchar_t)i); //Include basic Latin

            preload_char.insert((wchar_t)160);   //Non-breaking space
            preload_char.insert((wchar_t)215);   //Used on resolution selection screen (X).
            break;
        case T_DIGIT:
            preload_char.insert((wchar_t)32);    //Space

            for (u32 i = 47; i < 59; ++i)
                preload_char.insert((wchar_t)i); //Include chars used by timer and laps count only
            break;
        case T_BOLD:
            preload_char = translations->getCurrentAllChar(); //Loading unique characters

            for (u32 i = 65; i < 256; ++i)
                preload_char.insert((wchar_t)i); //Include basic Latin too, starting from A (char code 65)

            setlocale(LC_ALL, "en_US.UTF8");
            std::set<wchar_t>::iterator it = preload_char.begin();

            while (it != preload_char.end())
            {
                //Only use all capital letter for bold char with latin (<640 of char code).
                //Remove all characters (>char code 8191) not used by the title
                if (((iswlower((wchar_t)*it) || !iswalpha((wchar_t)*it)) && *it < 640) || *it > 8191)
                    it = preload_char.erase(it);
                else
                    ++it;
            }

            //Final hack to make stk display title properly
            for (u32 i = 32; i < 65; ++i)
                preload_char.insert((wchar_t)i); //Include basic symbol (from space (char code 32) to @(char code 64))

            preload_char.insert((wchar_t)160);   //Non-breaking space

            //Remove Ordinal indicator (char code 170 and 186)
            preload_char.erase((wchar_t)170);
            preload_char.erase((wchar_t)186);

            preload_char.erase((wchar_t)304);    //Remove Capital I-dotted (char code 304) with using "I" altogether.
            break;
    }

    return preload_char;
}

// ----------------------------------------------------------------------------

void ScalableFont::recreateFromLanguage()
{
    //Clean previous font data
    m_spritebank->clear();
    m_areas.clear();
    m_character_map.clear();
    m_max_height = 0;
    m_global_kerning_width = 0;
    m_global_kerning_height = 0;

    //Set to default scale to reload font
    m_scale = 1;
    //Reload
    if (!loadTTF())
    {
        Log::fatal("ScalableFont", "Recreation of TTF font failed");
    }
}

// ----------------------------------------------------------------------------

void ScalableFont::updateRTL()
{
    m_rtl = translations->isRTLLanguage();
}

// ----------------------------------------------------------------------------

void ScalableFont::setShadow(const irr::video::SColor &col)
{
    m_shadow = true;
    m_shadow_color = col;
}

// ----------------------------------------------------------------------------

void ScalableFont::setScale(const float scale)
{
    m_scale = scale;
}

// ----------------------------------------------------------------------------

void ScalableFont::setKerningWidth(s32 kerning)
{
    m_global_kerning_width = kerning;
}

// ----------------------------------------------------------------------------

s32 ScalableFont::getKerningWidth(const wchar_t* thisLetter, const wchar_t* previousLetter) const
{
    s32 ret = m_global_kerning_width;

    return ret;
}

// ----------------------------------------------------------------------------

void ScalableFont::setKerningHeight(s32 kerning)
{
    m_global_kerning_height = kerning;
}

// ----------------------------------------------------------------------------

s32 ScalableFont::getKerningHeight () const
{
    return m_global_kerning_height;
}

// ----------------------------------------------------------------------------

u32 ScalableFont::getSpriteNoFromChar(const wchar_t *c) const
{
    return m_areas[getAreaIDFromCharacter(*c, NULL)].spriteno;
}

// ----------------------------------------------------------------------------

s32 ScalableFont::getAreaIDFromCharacter(const wchar_t c, bool* fallback_font) const
{
    std::map<wchar_t, s32>::const_iterator n = m_character_map.find(c);
    if (n != m_character_map.end())
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
        return 1; //The first preload character in all type of fonts is space, which is 1
    }
}

// ----------------------------------------------------------------------------

const ScalableFont::SFontArea &ScalableFont::getAreaFromCharacter(const wchar_t c,
                                                    bool* fallback_font) const
{
    const int area_id = getAreaIDFromCharacter(c, fallback_font);

    if (m_mono_space_digits && ((c>=L'0' && c<=L'9') || c==L' '))
    {
        const SFontArea &area = (fallback_font && *fallback_font)
                              ? m_fallback_font->m_areas[area_id]
                              : m_areas[area_id];
        m_max_digit_area.spriteno = area.spriteno;
        return m_max_digit_area;
    }

    const bool use_fallback_font = (fallback_font && *fallback_font);

    if (use_fallback_font)
    {
        assert(area_id < (int)m_fallback_font->m_areas.size());
    }
    else
    {
        assert(area_id < (int)m_areas.size());
    }

    // Note: fallback_font can be NULL
    return (use_fallback_font ? m_fallback_font->m_areas[area_id] : m_areas[area_id]);
}   // getAreaFromCharacter

// ----------------------------------------------------------------------------

bool ScalableFont::hasThisChar(const wchar_t c) const
{
    std::map<wchar_t, s32>::const_iterator n = m_character_map.find(c);
    if (n != m_character_map.end())
        return true;
    return false;
}

// ----------------------------------------------------------------------------

void ScalableFont::setInvisibleCharacters( const wchar_t *s )
{
    m_invisible = s;
}

// ----------------------------------------------------------------------------

core::dimension2d<u32> ScalableFont::getDimension(const wchar_t* text) const
{
    GUIEngine::GlyphPageCreator* gp_creator = GUIEngine::getGlyphPageCreator();

    if (m_type == T_NORMAL || T_BOLD) //lazy load char
    {
        for (const wchar_t* p = text; *p; ++p)
        {
            if (*p == L'\r' ||  *p == L'\n' || *p == L' ' || *p < 32) continue;
            if (!GUIEngine::getFont()->hasThisChar(*p))
                gp_creator->insertChar(*p);
        }

        if (gp_creator->getNewChar().size() > 0 && !m_is_hollow_copy && m_scale == 1)
        {
            Log::debug("ScalableFont::getDimension", "New character(s) %s discoverd, perform lazy loading",
                         StringUtils::wide_to_utf8(gp_creator->getNewChar().c_str()).c_str());

            if (!GUIEngine::getFont()->lazyLoadChar())
                Log::error("ScalableFont::lazyLoadChar", "Can't insert new char into glyph pages.");
        }
    }

    assert(m_areas.size() > 0);

    core::dimension2d<u32> dim(0, 0);
    core::dimension2d<u32> thisLine(0, (int)(m_max_height*m_scale));

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

// ----------------------------------------------------------------------------

void ScalableFont::draw(const core::stringw& text,
    const core::rect<s32>& position, video::SColor color,
    bool hcenter, bool vcenter,
    const core::rect<s32>* clip)
{
    doDraw(text, position, color, hcenter, vcenter, clip, NULL);
}

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

void ScalableFont::doDraw(const core::stringw& text,
                          const core::rect<s32>& position, video::SColor color,
                          bool hcenter, bool vcenter,
                          const core::rect<s32>* clip,
                          FontCharCollector* charCollector)
{
    if (!m_video_driver) return;

    GUIEngine::GlyphPageCreator* gp_creator = GUIEngine::getGlyphPageCreator();

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

    if (m_type == T_NORMAL || T_BOLD) //lazy load char, have to do this again
    {                                 //because some text isn't drawn with getDimension
        for (u32 i = 0; i<text_size; i++)
        {
            wchar_t c = text[i];
            if (c == L'\r' ||  c == L'\n' || c == L' ' || c < 32) continue;
            if (!GUIEngine::getFont()->hasThisChar(c))
                gp_creator->insertChar(c);

            if (charCollector != NULL && m_type == T_NORMAL && m_spritebank->getSprites()
                [GUIEngine::getFont()->getSpriteNoFromChar(&c)].Frames[0].textureNumber
                == m_spritebank->getTextureCount() - 1) //Prevent overwriting texture used by billboard text
            {
                 Log::debug("ScalableFont::doDraw", "Character used by billboard text is in the last glyph page of normal font."
                              " Create a new glyph page for new characters inserted later to prevent it from being removed.");
                 GUIEngine::getFont()->forceNewPage();
            }
        }

        if (gp_creator->getNewChar().size() > 0 && !m_is_hollow_copy && m_scale == 1)
        {
            Log::debug("ScalableFont::doDraw", "New character(s) %s discoverd, perform lazy loading",
                         StringUtils::wide_to_utf8(gp_creator->getNewChar().c_str()).c_str());

            if (!GUIEngine::getFont()->lazyLoadChar())
                Log::error("ScalableFont::lazyLoadChar", "Can't insert new char into glyph pages.");
        }
    }

    for (u32 i = 0; i<text_size; i++)
    {
        wchar_t c = text[i];

        if (c == L'\r' ||          // Windows breaks
            c == L'\n'    )        // Unix breaks
        {
            if(c==L'\r' && text[i+1]==L'\n') c = text[++i];
            offset.Y += (int)(m_max_height*m_scale);
            offset.X  = position.UpperLeftCorner.X;
            if (hcenter)
                offset.X += (position.getWidth() - text_dimension.Width) >> 1;
            continue;
        }   // if lineBreak

        bool use_fallback_font = false;
        const SFontArea &area  = getAreaFromCharacter(c, &use_fallback_font);
        fallback[i]            = use_fallback_font;
        if (charCollector == NULL)
        {
            //Try to use ceil to make offset calculate correctly when m_scale is smaller than 1
            s32 glyph_offset_x = ceil((float) area.bearingx*
                                 (fallback[i] ? m_scale*m_fallback_font_scale : m_scale));
            s32 glyph_offset_y = ceil((float) area.offsety*
                                 (fallback[i] ? m_scale*m_fallback_font_scale : m_scale));
            offset.X += glyph_offset_x;
            offset.Y += glyph_offset_y + floor(m_type == T_DIGIT ? 20*m_scale : 0); //Additional offset for digit text
            offsets.push_back(offset);
            offset.X -= glyph_offset_x;
            offset.Y -= glyph_offset_y + floor(m_type == T_DIGIT ? 20*m_scale : 0);
        }
        else //Billboard text specific
        {
            s32 glyph_offset_x = ceil((float) area.bearingx*
                                 (fallback[i] ? m_scale*m_fallback_font_scale : m_scale));
            s32 glyph_offset_y = ceil((float) area.offsety_bt*
                                 (fallback[i] ? m_scale*m_fallback_font_scale : m_scale));
            offset.X += glyph_offset_x;
            offset.Y += glyph_offset_y + floor(m_type == T_DIGIT ? 20*m_scale : 0); //Additional offset for digit text
            offsets.push_back(offset);
            offset.X -= glyph_offset_x;
            offset.Y -= glyph_offset_y + floor(m_type == T_DIGIT ? 20*m_scale : 0);
        }
        // Invisible character. add something to the array anyway so that
        // indices from the various arrays remain in sync
        indices.push_back(m_invisible.findFirst(c) < 0  ? area.spriteno : -1);
        offset.X += getCharWidth(area, fallback[i]);
    }   // for i<text_size

    // ---- do the actual rendering
    const int indiceAmount                    = indices.size();
    core::array< SGUISprite >& sprites        = m_spritebank->getSprites();
    core::array< core::rect<s32> >& positions = m_spritebank->getPositions();
    core::array< SGUISprite >* fallback_sprites;
    core::array< core::rect<s32> >* fallback_positions;
    if (m_fallback_font != NULL)
    {
        fallback_sprites   = &m_fallback_font->m_spritebank->getSprites();
        fallback_positions = &m_fallback_font->m_spritebank->getPositions();
    }
    else
    {
        fallback_sprites   = NULL;
        fallback_positions = NULL;
    }

    const int spriteAmount      = sprites.size();

    if (m_black_border && charCollector == NULL)
    { //Draw black border first, to make it behind the real character
      //which make script language display better
        video::SColor black(color.getAlpha(),0,0,0);
        for (int n = 0; n < indiceAmount; n++)
        {
            const int spriteID = indices[n];
            if (!fallback[n] && (spriteID < 0 || spriteID >= spriteAmount)) continue;
            if (indices[n] == -1) continue;

            const int texID = (fallback[n] ?
                               (*fallback_sprites)[spriteID].Frames[0].textureNumber :
                               sprites[spriteID].Frames[0].textureNumber);

            core::rect<s32> source = (fallback[n] ?
                                      (*fallback_positions)[(*fallback_sprites)[spriteID].Frames[0].rectNumber] :
                                      positions[sprites[spriteID].Frames[0].rectNumber]);

            core::dimension2d<s32> size = source.getSize();

            float scale = (fallback[n] ? m_scale*m_fallback_font_scale : m_scale);
            size.Width  = (int)(size.Width  * scale);
            size.Height = (int)(size.Height * scale);

            core::rect<s32> dest(offsets[n], size);

            video::ITexture* texture = (fallback[n] ?
                                        m_fallback_font->m_spritebank->getTexture(texID) :
                                        m_spritebank->getTexture(texID) );

            for (int x_delta = -2; x_delta <= 2; x_delta++)
            {
                for (int y_delta = -2; y_delta <= 2; y_delta++)
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
    }

    for (int n = 0; n < indiceAmount; n++)
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

        core::dimension2d<s32> size = source.getSize();

        float scale = (fallback[n] ? m_scale*m_fallback_font_scale : m_scale);
        size.Width  = (int)(size.Width  * scale);
        size.Height = (int)(size.Height * scale);

        core::rect<s32> dest(offsets[n], size);

        video::ITexture* texture = (fallback[n] ?
                                    m_fallback_font->m_spritebank->getTexture(texID) :
                                    m_spritebank->getTexture(texID) );

        /*
        if (fallback[n])
        {
            Log::info("ScalableFont", "Using fallback font %s; source area is %d, %d; size %d, %d; dest = %d, %d",
                core::stringc(texture->getName()).c_str(), source.UpperLeftCorner.X, source.UpperLeftCorner.Y,
                source.getWidth(), source.getHeight(), offsets[n].X, offsets[n].Y);
        }
        */
#ifdef FONT_DEBUG
        GL32_draw2DRectangle(video::SColor(255, 255,0,0), dest,clip);
#endif

        if (fallback[n] || m_type == T_BOLD)
        {
            // TODO: don't hardcode colors?
            video::SColor orange(color.getAlpha(), 255, 100, 0);
            video::SColor yellow(color.getAlpha(), 255, 220, 15);
            video::SColor title_colors[] = {orange, yellow, orange, yellow};

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
        }
    }
}

// ----------------------------------------------------------------------------

s32 ScalableFont::getCharWidth(const SFontArea& area, const bool fallback) const
{
    if (fallback) return (int)((area.width*m_fallback_font_scale + m_fallback_kerning_width) * m_scale);
    else          return (int)((area.width + m_global_kerning_width) * m_scale);
}

// ----------------------------------------------------------------------------

s32 ScalableFont::getCharacterFromPos(const wchar_t* text, s32 pixel_x) const
{
    s32 x = 0;
    s32 idx = 0;

    while (text[idx])
    {
        bool use_fallback_font = false;
        const SFontArea &a  = getAreaFromCharacter(text[idx], &use_fallback_font);

        x += getCharWidth(a, use_fallback_font) + m_global_kerning_width;

        if (x >= pixel_x)
            return idx;

        ++idx;
    }

    return -1;
}

// ----------------------------------------------------------------------------

IGUISpriteBank* ScalableFont::getSpriteBank() const
{
    return m_spritebank;
}

} // end namespace gui
} // end namespace irr
