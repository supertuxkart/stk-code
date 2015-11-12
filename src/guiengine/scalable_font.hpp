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

#ifndef HEADER_SCALABLE_FONT_HPP
#define HEADER_SCALABLE_FONT_HPP

#include "guiengine/ft_environment.hpp"
#include "utils/leak_check.hpp"

#include <map>
#include <string>
#include <set>

namespace irr
{

namespace video
{
    class IVideoDriver;
    class IImage;
    class ITexture;
}

namespace gui
{

    class IGUIEnvironment;

class FontCharCollector
{
public:

    virtual void collectChar(video::ITexture* texture, const core::rect<s32>& destRect,
        const core::rect<s32>& sourceRect, const video::SColor* const colors) = 0;
};

class ScalableFont : public IGUIFontBitmap
{
    float m_scale;
    bool m_shadow;
    /** True if digits should be mono spaced. */

    bool m_mono_space_digits;
    irr::video::SColor m_shadow_color;

    bool m_is_hollow_copy;
    bool m_rtl;

public:

    LEAK_CHECK()

    bool m_black_border;

    ScalableFont* m_fallback_font;
    float         m_fallback_font_scale;
    int           m_fallback_kerning_width;

    ScalableFont(IGUIEnvironment* env, GUIEngine::TTFLoadingType type);
    virtual ~ScalableFont();

    /** Creates a hollow copy of this font; i.e. the underlying font data is the *same* for
      * both fonts. The advantage of doing this is that you can change "view" parameters
      * in the copy, for example kerning or scale.
      * The object returned by this method is 'new'ed and must be deleted. Deleting it will
      * not delete the data of the original. Do *not* delete the original as long as this
      * hollow copy is still used, since they share their data (and the original is the one
      * that "owns" it).
      */
    ScalableFont* getHollowCopy() const
    {
        ScalableFont* out = new ScalableFont(*this);
        out->m_is_hollow_copy = true;
        out->setReferenceCount(1);
        return out;
    }

    /** loads a font from a TTF file */
    bool loadTTF();

    /** lazy load new characters discovered in normal font */
    bool lazyLoadChar();

    /** draws an text and clips it to the specified rectangle if wanted */
    virtual void draw(const core::stringw& text, const core::rect<s32>& position,
        video::SColor color, bool hcenter = false,
        bool vcenter = false, const core::rect<s32>* clip = 0);

    void draw(const core::stringw& text, const core::rect<s32>& position,
        video::SColor color, bool hcenter,
        bool vcenter, const core::rect<s32>* clip, bool ignoreRTL);

    void doDraw(const core::stringw& text, const core::rect<s32>& position,
              video::SColor color, bool hcenter,
              bool vcenter, const core::rect<s32>* clip,
              FontCharCollector* charCollector = NULL);

    /** returns the dimension of a text */
    virtual core::dimension2d<u32> getDimension(const wchar_t* text) const;

    /** Calculates the index of the character in the text which is on a specific position. */
    virtual s32 getCharacterFromPos(const wchar_t* text, s32 pixel_x) const;

    /** Returns the type of this font */
    virtual EGUI_FONT_TYPE getType() const { return EGFT_BITMAP; }

    /** set an Pixel Offset on Drawing ( scale position on width ) */
    virtual void setKerningWidth (s32 kerning);
    virtual void setKerningHeight (s32 kerning);

    /** set an Pixel Offset on Drawing ( scale position on width ) */
    virtual s32 getKerningWidth(const wchar_t* thisLetter=0, const wchar_t* previousLetter=0) const;
    virtual s32 getKerningHeight() const;

    /** Sets if digits are to be mono-spaced. */
    void    setMonospaceDigits(bool mono) {m_mono_space_digits = mono; }
    bool    getMonospaceDigits() const { return m_mono_space_digits;   }
    void    setShadow(const irr::video::SColor &col);
    void    disableShadow() {m_shadow = false;}

    /** gets the sprite bank */
    virtual IGUISpriteBank* getSpriteBank() const;

    /** returns the sprite number from a given character */
    virtual u32 getSpriteNoFromChar(const wchar_t *c) const;

    virtual void setInvisibleCharacters( const wchar_t *s );

    /** test whether current font has this character regardless of fallback font */
    virtual bool hasThisChar(const wchar_t c) const;

    void setScale(const float scale);
    float getScale() const { return m_scale; }

    void updateRTL();

    /** re-create fonts when language is changed */
    void recreateFromLanguage();

    /** force create a new texture (glyph) page in a font */
    void forceNewPage();

private:

    struct SFontArea
    {
        SFontArea() : width(0), spriteno(0), offsety(0), offsety_bt(0), bearingx(0) {}
        s32             width;
        u32             spriteno;
        s32             offsety;
        s32             offsety_bt;
        s32             bearingx;
    };

    s32 getCharWidth(const SFontArea& area, const bool fallback) const;
    s32 getAreaIDFromCharacter(const wchar_t c, bool* fallback_font) const;
    const SFontArea &getAreaFromCharacter(const wchar_t c, bool* fallback_font) const;
    /** get characters to be pre-loaded base on font type */
    std::set<wchar_t> getPreloadCharacters(const GUIEngine::TTFLoadingType);

    GUIEngine::TTFLoadingType m_type;
    GUIEngine::FontUse        m_font_use;
    video::IVideoDriver      *m_video_driver;
    IGUISpriteBank           *m_spritebank;
    IGUIEnvironment          *m_gui_env;
    video::ITexture          *m_last_normal_page;

    core::array<SFontArea>   m_areas;
    /** The maximum values of all digits, used in monospace_digits. */
    mutable SFontArea        m_max_digit_area;
    std::map<wchar_t, s32>   m_character_map;

    s32                      m_max_height;
    s32                      m_global_kerning_width;
    s32                      m_global_kerning_height;
    s32                      m_glyph_max_height;

    core::stringw            m_invisible;
};

} // end namespace gui
} // end namespace irr


#endif // HEADER_SCALABLE_FONT_HPP

