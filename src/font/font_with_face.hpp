//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#ifndef HEADER_FONT_WITH_FACE_HPP
#define HEADER_FONT_WITH_FACE_HPP

#include "utils/cpp2011.hpp"
#include "utils/leak_check.hpp"
#include "utils/no_copy.hpp"

#include <algorithm>
#include <cassert>
#include <map>
#include <set>
#include <string>

#ifndef SERVER_ONLY
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#endif

#include <GlyphLayout.h>
#include <dimension2d.h>
#include <irrString.h>
#include <rect.h>

using namespace irr;

const int BEARING = 64;

class FaceTTF;
class FontSettings;
struct FontArea;

namespace irr
{
    namespace video
    {
        class ITexture;
        class SColor;
    }
    namespace gui
    {
        class IGUISpriteBank;
    }
}

/** An abstract class which contains functions which convert vector fonts into
 *  bitmap and render them in STK. To make STK draw characters with different
 *  render option (like scaling, shadow) using a same FontWithFace, you need
 *  to wrap this with \ref irr::gui::ScalableFont and configure the
 *  \ref FontSettings for it.
 *  \ingroup font
 */
class FontWithFace : public NoCopy
{
public:
    /** A class for \ref STKTextBillboard to get font info to render billboard
     *  text. */
    class FontCharCollector
    {
    public:
        /** Collect the character info for billboard text.
         *  \param texture The texture of the character.
         *  \param destRect The destination rectangle
         *  \param sourceRect The source rectangle in the glyph page
         *  \param colors The color to render it. */
        virtual void collectChar(video::ITexture* texture,
                                 const core::rect<float>& destRect,
                                 const core::rect<s32>& sourceRect,
                                 const video::SColor* const colors) = 0;
    };

protected:
    /** Used in vertical dimension calculation. */
    int m_font_max_height;

    /** Used in top side bearing calculation. */
    int m_glyph_max_height;

    // ------------------------------------------------------------------------
    /** Check characters to see if they are loaded in font, if not load them.
     *  For font that doesn't need lazy loading, nothing will be done.
     *  \param in_ptr Characters to check.
     *  \param first_load If true, it will ignore \ref supportLazyLoadChar,
     *  which is called in \ref reset. */
    void insertCharacters(const wchar_t* in_ptr, bool first_load = false)
    {
        if (!supportLazyLoadChar() && !first_load) return;

        for (const wchar_t* p = in_ptr; *p; ++p)
        {
            if (*p == L'\r' ||  *p == L'\n' || *p < (wchar_t)32)
                continue;
            if (!loadedChar(*p))
            {
                loadGlyphInfo(*p);
                if (supportChar(*p))
                    addLazyLoadChar(*p);
                else if (m_fallback_font != NULL)
                {
                    if (!m_fallback_font->loadedChar(*p))
                    {
                        m_fallback_font->loadGlyphInfo(*p);
                        if (m_fallback_font->supportChar(*p))
                            m_fallback_font->addLazyLoadChar(*p);
                    }
                }
            }
        }
    }
    // ------------------------------------------------------------------------
    void updateCharactersList();
    // ------------------------------------------------------------------------
    /** Set the fallback font for this font, so if some character is missing in
     *  this font, it will use that fallback font to try rendering it.
     *  \param face A \ref FontWithFace font. */
    void setFallbackFont(FontWithFace* face)        { m_fallback_font = face; }
    // ------------------------------------------------------------------------
    /** Set the scaling of fallback font.
     *  \param scale The scaling to set. */
    void setFallbackFontScale(float scale)   { m_fallback_font_scale = scale; }

private:
    /** Mapping of glyph index to a TTF in \ref FaceTTF. */
    struct GlyphInfo
    {
        GlyphInfo(unsigned int font_num = 0, unsigned int glyph_idx = 0) :
            font_number(font_num), glyph_index(glyph_idx) {}
        /** Index to a TTF in \ref FaceTTF. */
        unsigned int font_number;
        /** Glyph index in the TTF, 0 means no such glyph. */
        unsigned int glyph_index;
    };

    /** \ref FaceTTF to load glyph from. */
    FaceTTF*                     m_face_ttf;

    /** Fallback font to use if some character isn't supported by this font. */
    FontWithFace*                m_fallback_font;

    /** Scaling for fallback font. */
    float                        m_fallback_font_scale;

    /** A temporary holder to store new characters to be inserted. */
    std::set<wchar_t>            m_new_char_holder;

    /** Sprite bank to store each glyph. */
    gui::IGUISpriteBank*         m_spritebank;

    /** The current max height at current drawing line in glyph page. */
    unsigned int                 m_current_height;

    /** The used width in glyph page. */
    unsigned int                 m_used_width;

    /** The used height in glyph page. */
    unsigned int                 m_used_height;

    /** The dpi of this font. */
    unsigned int                 m_face_dpi;

    /** Used to undo the scale on text shaping, only need to take care of
     *  width. */
    float                        m_inverse_shaping;
    /** Store a list of loaded and tested character to a \ref GlyphInfo. */
    std::map<wchar_t, GlyphInfo> m_character_glyph_info_map;

    // ------------------------------------------------------------------------
    float getCharWidth(const FontArea& area, bool fallback, float scale) const;
    // ------------------------------------------------------------------------
    /** Test if a character has already been tried to be loaded.
     *  \param c Character to test.
     *  \return True if tested. */
    bool loadedChar(wchar_t c) const
    {
        std::map<wchar_t, GlyphInfo>::const_iterator n =
            m_character_glyph_info_map.find(c);
        if (n != m_character_glyph_info_map.end())
            return true;
        return false;
    }
    // ------------------------------------------------------------------------
    /** Get the \ref GlyphInfo from \ref m_character_glyph_info_map about a
     *  character.
     *  \param c Character to get.
     *  \return \ref GlyphInfo of this character. */
    const GlyphInfo& getGlyphInfo(wchar_t c) const
    {
        std::map<wchar_t, GlyphInfo>::const_iterator n =
            m_character_glyph_info_map.find(c);
        // Make sure we always find GlyphInfo
        assert(n != m_character_glyph_info_map.end());
        return n->second;
    }
    // ------------------------------------------------------------------------
    /** Tells whether a character is supported by all TTFs in \ref m_face_ttf
     *  which is determined by \ref GlyphInfo of this character.
     *  \param c Character to test.
     *  \return True if it's supported. */
    bool supportChar(wchar_t c)
    {
        std::map<wchar_t, GlyphInfo>::const_iterator n =
            m_character_glyph_info_map.find(c);
        if (n != m_character_glyph_info_map.end())
        {
            return n->second.glyph_index > 0;
        }
        return false;
    }
    // ------------------------------------------------------------------------
    void loadGlyphInfo(wchar_t c);
    // ------------------------------------------------------------------------
    void createNewGlyphPage();
    // ------------------------------------------------------------------------
    /** Add a character into \ref m_new_char_holder for lazy loading later. */
    void addLazyLoadChar(wchar_t c)            { m_new_char_holder.insert(c); }
    // ------------------------------------------------------------------------
    /** Override it if sub-class should not do lazy loading characters. */
    virtual bool supportLazyLoadChar() const                   { return true; }
    // ------------------------------------------------------------------------
    /** Defined by sub-class about the texture size of glyph page, it should be
     *  a power of two. */
    virtual unsigned int getGlyphPageSize() const = 0;
    // ------------------------------------------------------------------------
    /** Defined by sub-class about the scaling factor 1. */
    virtual float getScalingFactorOne() const = 0;
    // ------------------------------------------------------------------------
    /** Defined by sub-class about the scaling factor 2. */
    virtual unsigned int getScalingFactorTwo() const = 0;
    // ------------------------------------------------------------------------
    /** Override it if sub-class has bold outline. */
    virtual bool isBold() const                               { return false; }
    // ------------------------------------------------------------------------
    const FontArea* getUnknownFontArea() const;
    // ------------------------------------------------------------------------
    std::vector<gui::GlyphLayout> text2GlyphsWithoutShaping(
                                                       const core::stringw& t);
    // ------------------------------------------------------------------------
#ifndef SERVER_ONLY
    /** Override it if any outline shaping is needed to be done before
     *  rendering the glyph into bitmap.
     *  \return A FT_Error value if needed. */
    virtual int shapeOutline(FT_Outline* outline) const           { return 0; }
#endif

public:
    LEAK_CHECK()
    // ------------------------------------------------------------------------
    FontWithFace(const std::string& name);
    // ------------------------------------------------------------------------
    virtual ~FontWithFace();
    // ------------------------------------------------------------------------
    virtual void init();
    // ------------------------------------------------------------------------
    virtual void reset();
    // ------------------------------------------------------------------------
    virtual core::dimension2d<u32> getDimension(const core::stringw& text,
                                           FontSettings* font_settings = NULL);
    // ------------------------------------------------------------------------
    int getCharacterFromPos(const wchar_t* text, int pixel_x,
                            FontSettings* font_settings = NULL) const;
    // ------------------------------------------------------------------------
    void render(const std::vector<gui::GlyphLayout>& gl,
                const core::rect<s32>& position, const video::SColor& color,
                bool hcenter, bool vcenter, const core::rect<s32>* clip,
                FontSettings* font_settings,
                FontCharCollector* char_collector = NULL);
    // ------------------------------------------------------------------------
    virtual void drawText(const core::stringw& text,
                          const core::rect<s32>& position,
                          const video::SColor& color, bool hcenter,
                          bool vcenter, const core::rect<s32>* clip,
                          FontSettings* font_settings,
                          FontCharCollector* char_collector = NULL);
    // ------------------------------------------------------------------------
    void drawTextQuick(const core::stringw& text,
                       const core::rect<s32>& position,
                       const video::SColor& color, bool hcenter, bool vcenter,
                       const core::rect<s32>* clip,
                       FontSettings* font_settings,
                       FontCharCollector* char_collector = NULL);
    // ------------------------------------------------------------------------
    void dumpGlyphPage(const std::string& name);
    // ------------------------------------------------------------------------
    void dumpGlyphPage();
    // ------------------------------------------------------------------------
    /** Return the sprite bank. */
    gui::IGUISpriteBank* getSpriteBank() const         { return m_spritebank; }
    // ------------------------------------------------------------------------
    const FontArea& getAreaFromCharacter(const wchar_t c,
                                         bool* fallback_font) const;
    // ------------------------------------------------------------------------
    /** Return the dpi of this face. */
    unsigned int getDPI() const                          { return m_face_dpi; }
    // ------------------------------------------------------------------------
    FaceTTF* getFaceTTF() const                          { return m_face_ttf; }
    // ------------------------------------------------------------------------
    void insertGlyph(unsigned font_number, unsigned glyph_index);
    // ------------------------------------------------------------------------
    int getFontMaxHeight() const                  { return m_font_max_height; }
    // ------------------------------------------------------------------------
    int getGlyphMaxHeight() const                { return m_glyph_max_height; }
    // ------------------------------------------------------------------------
    virtual bool disableTextShaping() const                   { return false; }
    // ------------------------------------------------------------------------
    float getInverseShaping() const               { return m_inverse_shaping; }
    // ------------------------------------------------------------------------
    virtual bool useColorGlyphPage() const                    { return false; }
    // ------------------------------------------------------------------------
    /** Defined by sub-class about the native scaling factor, to provide */
    /** a texture with higher resolution when the scale is > 1.0f */
    virtual float getNativeScalingFactor() const               { return 1.0f; }
    // ------------------------------------------------------------------------
    void setDPI();
};   // FontWithFace

#endif
/* EOF */
