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

#include "font/font_manager.hpp"
#include "font/font_settings.hpp"
#include "utils/cpp2011.hpp"

#include <algorithm>
#include <map>
#include <set>

const int BEARING = 64;

class FaceTTF;

class FontWithFace : public NoCopy
{
public:
    class FontCharCollector
    {
    public:
        virtual void collectChar(video::ITexture* texture,
                                 const core::rect<float>& destRect,
                                 const core::rect<s32>& sourceRect,
                                 const video::SColor* const colors) = 0;
    };

    struct FontArea
    {
        FontArea() : advance_x(0), bearing_x(0) ,offset_y(0), offset_y_bt(0),
                     spriteno(0) {}
        int advance_x;
        int bearing_x;
        int offset_y;
        int offset_y_bt;
        int spriteno;
    };

protected:
    int                  m_font_max_height;

    int                  m_glyph_max_height;

    // ------------------------------------------------------------------------
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
    void setFallbackFont(FontWithFace* face)        { m_fallback_font = face; }
    // ------------------------------------------------------------------------
    void setFallbackFontScale(float scale)   { m_fallback_font_scale = scale; }

private:
    struct GlyphInfo
    {
        unsigned int font_number;
        unsigned int glyph_index;
        GlyphInfo(unsigned int first = 0, unsigned int second = 0)
        {
            font_number = first;
            glyph_index = second;
        }
    };

    FaceTTF*                     m_face_ttf;

    FontWithFace*                m_fallback_font;
    float                        m_fallback_font_scale;

    /** A temporary holder stored new char to be inserted. */
    std::set<wchar_t>            m_new_char_holder;

    gui::IGUISpriteBank*         m_spritebank;

    /** A full glyph page for this font. */
    video::IImage*               m_page;

    unsigned int                 m_temp_height;
    unsigned int                 m_used_width;
    unsigned int                 m_used_height;
    unsigned int                 m_face_dpi;

    std::map<wchar_t, FontArea>  m_character_area_map;
    std::map<wchar_t, GlyphInfo> m_character_glyph_info_map;

    // ------------------------------------------------------------------------
    float getCharWidth(const FontArea& area, bool fallback, float scale) const
    {
        if (fallback)
            return area.advance_x * m_fallback_font_scale;
        else
            return area.advance_x * scale;
    }
    // ------------------------------------------------------------------------
    bool loadedChar(wchar_t c) const
    {
        std::map<wchar_t, GlyphInfo>::const_iterator n =
            m_character_glyph_info_map.find(c);
        if (n != m_character_glyph_info_map.end())
            return true;
        return false;
    }
    // ------------------------------------------------------------------------
    const GlyphInfo& getGlyphInfo(wchar_t c) const
    {
        std::map<wchar_t, GlyphInfo>::const_iterator n =
            m_character_glyph_info_map.find(c);
        // Make sure we always find GlyphInfo
        assert(n != m_character_glyph_info_map.end());
        return n->second;
    }
    // ------------------------------------------------------------------------
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
    void addLazyLoadChar(wchar_t c)            { m_new_char_holder.insert(c); }
    // ------------------------------------------------------------------------
    void insertGlyph(wchar_t c, const GlyphInfo& gi);
    // ------------------------------------------------------------------------
    void setDPI();
    // ------------------------------------------------------------------------
    virtual bool supportLazyLoadChar() const = 0;
    // ------------------------------------------------------------------------
    virtual unsigned int getGlyphPageSize() const = 0;
    // ------------------------------------------------------------------------
    virtual float getScalingFactorOne() const = 0;
    // ------------------------------------------------------------------------
    virtual unsigned int getScalingFactorTwo() const = 0;

public:
    LEAK_CHECK();
    // ------------------------------------------------------------------------
    FontWithFace(const std::string& name, FaceTTF* ttf);
    // ------------------------------------------------------------------------
    virtual ~FontWithFace();
    // ------------------------------------------------------------------------
    virtual void init();
    // ------------------------------------------------------------------------
    virtual void reset();
    // ------------------------------------------------------------------------
    core::dimension2d<u32> getDimension(const wchar_t* text,
                                  FontSettings* font_settings = NULL);
    // ------------------------------------------------------------------------
    int getCharacterFromPos(const wchar_t* text, int pixel_x,
                            FontSettings* font_settings = NULL) const;
    // ------------------------------------------------------------------------
    void render(const core::stringw& text, const core::rect<s32>& position,
                const video::SColor& color, bool hcenter, bool vcenter,
                const core::rect<s32>* clip,
                FontSettings* font_settings,
                FontCharCollector* char_collector = NULL);
    // ------------------------------------------------------------------------
    /** Write the current glyph page in png inside current running directory.
     *  Mainly for debug use.
     *  \param name The file name.
     */
    void dumpGlyphPage(const std::string& name);
    // ------------------------------------------------------------------------
    /** Write the current glyph page in png inside current running directory.
     *  Useful in gdb without parameter.
     */
    void dumpGlyphPage();
    // ------------------------------------------------------------------------
    gui::IGUISpriteBank* getSpriteBank() const         { return m_spritebank; }
    // ------------------------------------------------------------------------
    const FontArea& getAreaFromCharacter(const wchar_t c,
                                         bool* fallback_font) const;
    // ------------------------------------------------------------------------
    unsigned int getDPI() const                          { return m_face_dpi; }

};   // FontWithFace

#endif
/* EOF */
