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

#ifndef HEADER_SCALABLE_FONT_HPP
#define HEADER_SCALABLE_FONT_HPP

#include "utils/leak_check.hpp"

#include <IGUIFontBitmap.h>

class FontSettings;
class FontWithFace;

namespace irr
{
namespace gui
{

class ScalableFont : public IGUIFontBitmap
{
private:
    FontWithFace* m_face;

    FontSettings* m_font_settings;

public:
    LEAK_CHECK()
    // ------------------------------------------------------------------------
    ScalableFont(FontWithFace* face);
    // ------------------------------------------------------------------------
    virtual ~ScalableFont();
    // ------------------------------------------------------------------------
    FontSettings* getFontSettings()                 { return m_font_settings; }
    // ------------------------------------------------------------------------
    const FontSettings* getFontSettings() const     { return m_font_settings; }
    // ------------------------------------------------------------------------
    virtual void setScale(float scale);
    // ------------------------------------------------------------------------
    virtual float getScale() const;
    // ------------------------------------------------------------------------
    void setShadow(const irr::video::SColor &col);
    // ------------------------------------------------------------------------
    void disableShadow();
    // ------------------------------------------------------------------------
    void setBlackBorder(bool enabled);
    // ------------------------------------------------------------------------
    void setColoredBorder(const irr::video::SColor &col);
    // ------------------------------------------------------------------------
    void setThinBorder(bool thin);
    // ------------------------------------------------------------------------
    void disableColoredBorder();
    // ------------------------------------------------------------------------
    void draw(const core::stringw& text, const core::rect<s32>& position,
              const video::SColor& color, bool hcenter,
              bool vcenter, const core::rect<s32>* clip,
              bool ignoreRTL);
    // ------------------------------------------------------------------------
    /** draws an text and clips it to the specified rectangle if wanted */
    virtual void draw(const core::stringw& text,
                      const core::rect<s32>& position,
                      video::SColor color, bool hcenter = false,
                      bool vcenter = false, const core::rect<s32>* clip = 0);
    // ------------------------------------------------------------------------
    virtual void drawQuick(const core::stringw& text,
                           const core::rect<s32>& position,
                           video::SColor color, bool hcenter = false,
                           bool vcenter = false,
                           const core::rect<s32>* clip = 0);
    // ------------------------------------------------------------------------
    virtual void draw(const std::vector<GlyphLayout>& gls,
                      const core::rect<s32>& position,
                      video::SColor color, bool hcenter = false,
                      bool vcenter = false, const core::rect<s32>* clip = 0);
    // ------------------------------------------------------------------------
   virtual void initGlyphLayouts(const core::stringw& text,
                                 std::vector<GlyphLayout>& gls,
                                 u32 shape_flag = 0);
    // ------------------------------------------------------------------------
    /** returns the dimension of a text */
    virtual core::dimension2d<u32> getDimension(const wchar_t* text) const;
    // ------------------------------------------------------------------------
    virtual s32 getHeightPerLine() const;
    // ------------------------------------------------------------------------
    /** Calculates the index of the character in the text which is on a
     * specific position. */
    virtual s32 getCharacterFromPos(const wchar_t* text, s32 pixel_x) const;
    // ------------------------------------------------------------------------
    /** Returns the type of this font */
    virtual EGUI_FONT_TYPE getType() const              { return EGFT_BITMAP; }
    // ------------------------------------------------------------------------
    /** gets the sprite bank */
    virtual IGUISpriteBank* getSpriteBank() const;
    // ------------------------------------------------------------------------
    /** returns the sprite number from a given character, unused in STK */
    virtual u32 getSpriteNoFromChar(const wchar_t *c) const       { return 0; }
    // ------------------------------------------------------------------------
    // Below is not used:
    /** set an Pixel Offset on Drawing ( scale position on width ) */
    virtual void setKerningWidth (s32 kerning) {}
    // ------------------------------------------------------------------------
    virtual void setKerningHeight (s32 kerning) {}
    // ------------------------------------------------------------------------
    /** set an Pixel Offset on Drawing ( scale position on width ) */
    virtual s32 getKerningWidth(const wchar_t* thisLetter=0,
                                const wchar_t* previousLetter=0) const
                                                                  { return 0; }
    // ------------------------------------------------------------------------
    virtual s32 getKerningHeight() const                          { return 0; }
    // ------------------------------------------------------------------------
    virtual void setInvisibleCharacters( const wchar_t *s ) {}
    // ------------------------------------------------------------------------
    virtual f32 getInverseShaping() const;
    // ------------------------------------------------------------------------
    virtual s32 getFaceFontMaxHeight() const;
    // ------------------------------------------------------------------------
    virtual s32 getFaceGlyphMaxHeight() const;
};

} // end namespace gui
} // end namespace irr

#endif // HEADER_SCALABLE_FONT_HPP
