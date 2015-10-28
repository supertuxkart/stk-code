// Copyright (C) 2002-2015 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_GUI_FONT_H_INCLUDED__
#define __C_GUI_FONT_H_INCLUDED__

#include "utils/leak_check.hpp"

#ifdef ENABLE_FREETYPE
#include "guiengine/get_font_properties.hpp"
#endif // ENABLE_FREETYPE

#include "IrrCompileConfig.h"
#include "IGUIFontBitmap.h"
#include "irrString.h"
#include "irrMap.h"
#include "IXMLReader.h"
#include "IReadFile.h"
#include "irrArray.h"


#include <map>
#include <string>

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

#ifndef ENABLE_FREETYPE
    struct TextureInfo
    {
        irr::core::stringc m_file_name;
        bool m_has_alpha;
        float m_scale;
        bool m_exclude_from_max_height_calculation;

        TextureInfo()
        {
            m_has_alpha = false;
            m_scale = 1.0f;
        }
    };

    std::map<int /* texture file ID */, TextureInfo> m_texture_files;

    void doReadXmlFile(io::IXMLReader* xml);
#endif // ENABLE_FREETYPE

    bool m_is_hollow_copy;
    bool m_rtl;

public:

    LEAK_CHECK()

    bool m_black_border;

#ifdef ENABLE_FREETYPE
    TTFLoadingType m_type;
    FontUse        m_font_use;
    u32            m_dpi;
#endif // ENABLE_FREETYPE

    ScalableFont* m_fallback_font;
    float         m_fallback_font_scale;
    int           m_fallback_kerning_width;

    //! constructor
#ifdef ENABLE_FREETYPE
    ScalableFont(IGUIEnvironment* env, TTFLoadingType type);
#else
    ScalableFont(IGUIEnvironment* env, const std::string &filename);
#endif // ENABLE_FREETYPE

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

    //! destructor
    virtual ~ScalableFont();

#ifdef ENABLE_FREETYPE
    //! loads a font from a TTF file
    bool loadTTF();
#else
    //! loads a font from an XML file
    bool load(io::IXMLReader* xml);

    void lazyLoadTexture(int texID);
#endif // ENABLE_FREETYPE

    //! draws an text and clips it to the specified rectangle if wanted
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

    //! returns the dimension of a text
    virtual core::dimension2d<u32> getDimension(const wchar_t* text) const;

    //! Calculates the index of the character in the text which is on a specific position.
    virtual s32 getCharacterFromPos(const wchar_t* text, s32 pixel_x) const;

    //! Returns the type of this font
    virtual EGUI_FONT_TYPE getType() const { return EGFT_BITMAP; }

    //! set an Pixel Offset on Drawing ( scale position on width )
    virtual void setKerningWidth (s32 kerning);
    virtual void setKerningHeight (s32 kerning);

    //! set an Pixel Offset on Drawing ( scale position on width )
    virtual s32 getKerningWidth(const wchar_t* thisLetter=0, const wchar_t* previousLetter=0) const;
    virtual s32 getKerningHeight() const;

    /** Sets if digits are to be mono-spaced. */
    void    setMonospaceDigits(bool mono) {m_mono_space_digits = mono; }
    bool    getMonospaceDigits() const { return m_mono_space_digits;   }
    void    setShadow(const irr::video::SColor &col);
    void    disableShadow() {m_shadow = false;}

    //! gets the sprite bank
    virtual IGUISpriteBank* getSpriteBank() const;

    //! returns the sprite number from a given character
    virtual u32 getSpriteNoFromChar(const wchar_t *c) const;

    virtual void setInvisibleCharacters( const wchar_t *s );

    void setScale(const float scale);
    float getScale() const { return m_scale; }

    void updateRTL();

#ifdef ENABLE_FREETYPE
    //! re-create fonts when language is changed
    void recreateFromLanguage();

    //! lazy load new characters discovered in normal font
    bool lazyLoadChar();

    //! force create a new texture (glyph) page in a font
    void forceNewPage();
#endif // ENABLE_FREETYPE

private:

#ifdef ENABLE_FREETYPE
    struct SFontArea
    {
        SFontArea() : width(0), spriteno(0), offsety(0), offsety_bt(0), bearingx(0) {}
        s32             width;
        u32             spriteno;
        s32             offsety;
        s32             offsety_bt;
        s32             bearingx;
    };
#else
    struct SFontArea
    {
        SFontArea() : underhang(0), overhang(0), width(0), spriteno(0) {}
        s32             underhang;
        s32             overhang;
        s32             width;
        u32             spriteno;
    };
#endif // ENABLE_FREETYPE

    int getCharWidth(const SFontArea& area, const bool fallback) const;
    s32 getAreaIDFromCharacter(const wchar_t c, bool* fallback_font) const;
    const SFontArea &getAreaFromCharacter(const wchar_t c, bool* fallback_font) const;
    void setMaxHeight();

    core::array<SFontArea>      Areas;
    /** The maximum values of all digits, used in monospace_digits. */
    mutable SFontArea           m_max_digit_area;
    std::map<wchar_t, s32>      CharacterMap;
    video::IVideoDriver*        Driver;
    IGUISpriteBank*         SpriteBank;
    IGUIEnvironment*        Environment;
    u32             WrongCharacter;
    s32             MaxHeight;
    s32             GlobalKerningWidth, GlobalKerningHeight;
#ifdef ENABLE_FREETYPE
    s32                     GlyphMaxHeight;
    video::ITexture*        LastNormalPage;
#endif // ENABLE_FREETYPE

    core::stringw Invisible;
};

} // end namespace gui
} // end namespace irr


#endif // __C_GUI_FONT_H_INCLUDED__

