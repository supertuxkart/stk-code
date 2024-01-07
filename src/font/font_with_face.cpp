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

#include "font/font_with_face.hpp"

#include "config/user_config.hpp"
#include "font/face_ttf.hpp"
#include "font/font_drawer.hpp"
#include "font/font_manager.hpp"
#include "font/font_settings.hpp"
#include "graphics/2dutils.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/skin.hpp"
#include "utils/string_utils.hpp"
#include "utils/utf8.h"

#include "GlyphLayout.h"
#include "IVideoDriver.h"
#include <array>
#ifndef SERVER_ONLY
#include <ge_texture.hpp>
#endif

#include "../lib/irrlicht/source/Irrlicht/CGUISpriteBank.h"

#ifndef SERVER_ONLY
extern "C"
{
    #include <mipmap/img.h>
    #include <mipmap/imgresize.h>
}
#endif

// ----------------------------------------------------------------------------
/** Constructor. It will initialize the \ref m_spritebank and TTF files to use.
 *  \param name The name of face, used by irrlicht to distinguish spritebank.
 *  \param ttf \ref FaceTTF for this face to use.
 */
FontWithFace::FontWithFace(const std::string& name)
{
    m_spritebank = new irr::gui::CGUISpriteBank(irr_driver->getGUI());
    m_fallback_font = NULL;
    m_fallback_font_scale = 1.0f;
    m_glyph_max_height = 0;
    m_face_ttf = new FaceTTF();
    m_face_dpi = 40;
    m_inverse_shaping = 1.0f;
}   // FontWithFace

// ----------------------------------------------------------------------------
/** Destructor. Clears the glyph page and sprite bank.
 */
FontWithFace::~FontWithFace()
{
    for (unsigned int i = 0; i < m_spritebank->getTextureCount(); i++)
    {
        STKTexManager::getInstance()->removeTexture(
            m_spritebank->getTexture(i));
    }
    m_spritebank->drop();

    delete m_face_ttf;
}   // ~FontWithFace

// ----------------------------------------------------------------------------
/** Initialize the font structure, but don't load glyph here.
 */
void FontWithFace::init()
{
    m_glyph_max_height = 0;
    setDPI();
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
    {
        reset();
        return;
    }

    // Get the max height for this face
    assert(m_face_ttf->getTotalFaces() > 0);
    FT_Face cur_face = m_face_ttf->getFace(0);
    font_manager->checkFTError(FT_Set_Pixel_Sizes(cur_face, 0, getDPI()),
        "setting DPI");

    for (int i = 32; i < 128; i++)
    {
        // Test all basic latin characters
        const int idx = FT_Get_Char_Index(cur_face, (wchar_t)i);
        if (idx == 0) continue;
        font_manager->checkFTError(FT_Load_Glyph(cur_face, idx,
            FT_LOAD_DEFAULT), "setting max height");

        const int height = cur_face->glyph->metrics.height / BEARING;
        if (height > m_glyph_max_height)
            m_glyph_max_height = height;
    }
#endif
    reset();
}   // init

// ----------------------------------------------------------------------------
/** Clear all the loaded characters, sub-class can do pre-loading of characters
 *  after this.
 */
void FontWithFace::reset()
{
    m_new_char_holder.clear();
    m_character_glyph_info_map.clear();
    for (unsigned int i = 0; i < m_spritebank->getTextureCount(); i++)
    {
        STKTexManager::getInstance()->removeTexture(
            m_spritebank->getTexture(i));
    }
    m_spritebank->clear();
    m_face_ttf->reset();
    createNewGlyphPage();
}   // reset

// ----------------------------------------------------------------------------
/** Convert a character to a glyph index in one of the font in \ref m_face_ttf,
 *  it will find the first TTF that supports this character, if the final
 *  glyph_index is 0, this means such character is not supported by all TTFs in
 *  \ref m_face_ttf.
 *  \param c The character to be loaded.
 */
void FontWithFace::loadGlyphInfo(wchar_t c)
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;

    unsigned int font_number = 0;
    unsigned int glyph_index = 0;
    m_face_ttf->getFontAndGlyphFromChar(c, &font_number, &glyph_index);
    m_character_glyph_info_map[c] = GlyphInfo(font_number, glyph_index);
#endif
}   // loadGlyphInfo

// ----------------------------------------------------------------------------
/** Create a new glyph page by filling it with transparent content.
 */
void FontWithFace::createNewGlyphPage()
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;
    m_current_height = 0;
    m_used_width = 0;
    m_used_height = 0;
    video::ITexture* font_texture = GE::createFontTexture(typeid(*this).name() +
        StringUtils::toString(m_spritebank->getTextureCount()),
        getGlyphPageSize(), !useColorGlyphPage());
    m_spritebank->addTexture(STKTexManager::getInstance()->addTexture(
        font_texture));
#endif
}   // createNewGlyphPage

// ----------------------------------------------------------------------------
/** Render a glyph for a character into bitmap and save it into the glyph page.
 *  \param font_number Font number in \ref FaceTTF ttf list
 *  \param glyph_index Glyph index in ttf
 */
void FontWithFace::insertGlyph(unsigned font_number, unsigned glyph_index)
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;

    assert(glyph_index > 0);
    assert(font_number < m_face_ttf->getTotalFaces());
    FT_Face cur_face = m_face_ttf->getFace(font_number);
    FT_GlyphSlot slot = cur_face->glyph;

    if (FT_HAS_COLOR(cur_face) && cur_face->num_fixed_sizes != 0)
    {
        font_manager->checkFTError(FT_Load_Glyph(cur_face, glyph_index,
            FT_LOAD_DEFAULT | FT_LOAD_COLOR), "loading a glyph");
    }
    else
    {
        // Same face may be shared across the different FontWithFace,
        // so reset dpi each time
        font_manager->checkFTError(FT_Set_Pixel_Sizes(cur_face, 0, getDPI()),
            "setting DPI");

        unsigned flag = FT_HAS_COLOR(cur_face) ?
            (FT_LOAD_DEFAULT | FT_LOAD_COLOR) : FT_LOAD_DEFAULT;
        font_manager->checkFTError(FT_Load_Glyph(cur_face, glyph_index,
            flag), "loading a glyph");

        font_manager->checkFTError(shapeOutline(&(slot->outline)),
            "shaping outline");

        font_manager->checkFTError(FT_Render_Glyph(slot,
            FT_RENDER_MODE_NORMAL), "rendering a glyph to bitmap");
    }

    // Convert to an anti-aliased bitmap
    FT_Bitmap* bits = &(slot->bitmap);
    float scale_ratio = 1.0f;
    unsigned cur_glyph_width = bits->width;
    unsigned cur_glyph_height = bits->rows;
    if (bits->pixel_mode == FT_PIXEL_MODE_BGRA)
    {
        scale_ratio =
                (float)getDPI() / (float)font_manager->getShapingDPI();
        cur_glyph_width = (unsigned)(bits->width * scale_ratio);
        cur_glyph_height = (unsigned)(bits->rows * scale_ratio);
    }
    core::dimension2du texture_size(cur_glyph_width + 1, cur_glyph_height + 1);
    if ((m_used_width + texture_size.Width > getGlyphPageSize() &&
        m_used_height + m_current_height + texture_size.Height >
        getGlyphPageSize())                                     ||
        m_used_height + texture_size.Height > getGlyphPageSize())
    {
        // Add a new glyph page if current one is full
        createNewGlyphPage();
    }

    // Determine the linebreak location
    if (m_used_width + texture_size.Width > getGlyphPageSize())
    {
        m_used_width  = 0;
        m_used_height += m_current_height;
        m_current_height = 0;
    }

    const unsigned int cur_tex = m_spritebank->getTextureCount() - 1;
    if (bits->buffer != NULL && !GUIEngine::isNoGraphics())
    {
        video::ITexture* tex = m_spritebank->getTexture(cur_tex);
        if (bits->pixel_mode == FT_PIXEL_MODE_GRAY)
        {
            tex->updateTexture(bits->buffer, video::ECF_R8, bits->width,
                bits->rows, m_used_width, m_used_height);
        }
        else if (bits->pixel_mode == FT_PIXEL_MODE_BGRA)
        {
            assert(useColorGlyphPage());
            // Scale it to normal font dpi
            video::IImage* unscaled = irr_driver->getVideoDriver()
                ->createImageFromData(video::ECF_A8R8G8B8,
                { bits->width, bits->rows },
                bits->buffer, true/*ownForeignMemory*/, false/*deleteMemory*/);
            assert(unscaled);
            video::IImage* scaled = irr_driver
                ->getVideoDriver()->createImage(video::ECF_A8R8G8B8,
                { cur_glyph_width , cur_glyph_height});
            assert(scaled);
            if (cur_glyph_width == bits->width &&
                cur_glyph_height == bits->rows)
            {
                unscaled->copyTo(scaled);
            }
            else if (cur_glyph_width >= bits->width ||
                cur_glyph_height >= bits->rows)
            {
                unscaled->copyToScaling(scaled);
            }
            else
            {
                imReduceOptions options;
                imReduceSetOptions(&options, IM_REDUCE_FILTER_LINEAR/*filter*/,
                    3/*hopcount*/, 16.0f/*alpha*/, 1.0f/*amplifynormal*/,
                    0.0f/*normalsustainfactor*/);
                int ret = imReduceImageKaiserData((unsigned char*)scaled->lock(),
                    (unsigned char*)unscaled->lock(), bits->width, bits->rows, 4,
                    bits->width * 4, cur_glyph_width , cur_glyph_height,
                    &options);
                if (ret != 1)
                {
                    Log::error("FontWithFace",
                        "Error reduce bitmap font size.");
                }
            }
            uint8_t* scaled_data = (uint8_t*)scaled->lock();
            tex->updateTexture(scaled_data, video::ECF_A8R8G8B8,
                cur_glyph_width, cur_glyph_height, m_used_width,
                m_used_height);
            unscaled->drop();
            scaled->drop();
        }
        else
        {
            assert(false && "Invalid pixel mode");
        }
    }

    // Store the rectangle of current glyph
    gui::SGUISpriteFrame f;
    gui::SGUISprite s;
    core::rect<s32> rectangle(m_used_width, m_used_height,
        m_used_width + cur_glyph_width, m_used_height + cur_glyph_height);
    f.rectNumber = m_spritebank->getPositions().size();
    f.textureNumber = cur_tex;

    // Add frame to sprite
    s.Frames.push_back(f);
    s.frameTime = 0;
    m_spritebank->getPositions().push_back(rectangle);
    m_spritebank->getSprites().push_back(s);

    // Save glyph metrics
    FontArea a;
    a.advance_x = (int)
        (cur_face->glyph->advance.x / BEARING * scale_ratio);
    a.bearing_x = (int)
        (cur_face->glyph->metrics.horiBearingX / BEARING * scale_ratio);
    const int cur_height =
        (int)(cur_face->glyph->metrics.height / BEARING * scale_ratio);
    const int cur_offset_y = cur_height -
        (int)(cur_face->glyph->metrics.horiBearingY / BEARING * scale_ratio);
    a.offset_y = m_glyph_max_height - cur_height + cur_offset_y;
    a.offset_y_bt = -cur_offset_y;
    a.spriteno = f.rectNumber;
    m_face_ttf->insertFontArea(a, font_number, glyph_index);

    // Store used area
    m_used_width += texture_size.Width;
    if (m_current_height < texture_size.Height)
        m_current_height = texture_size.Height;
#endif
}   // insertGlyph

// ----------------------------------------------------------------------------
/** Update the supported characters for this font if required.
 */
void FontWithFace::updateCharactersList()
{
    if (m_fallback_font != NULL)
        m_fallback_font->updateCharactersList();

    if (m_new_char_holder.empty()) return;
    for (const wchar_t& c : m_new_char_holder)
    {
        const GlyphInfo& gi = getGlyphInfo(c);
        insertGlyph(gi.font_number, gi.glyph_index);
    }
    m_new_char_holder.clear();

}   // updateCharactersList

// ----------------------------------------------------------------------------
/** Write the current glyph page in png inside current running directory.
 *  Mainly for debug use.
 *  \param name The file name.
 */
void FontWithFace::dumpGlyphPage(const std::string& name)
{
#ifndef SERVER_ONLY
    for (unsigned int i = 0; i < m_spritebank->getTextureCount(); i++)
    {
        video::ITexture* tex = m_spritebank->getTexture(i);
        core::dimension2d<u32> size = tex->getSize();
        video::ECOLOR_FORMAT col_format = tex->getColorFormat();
        void* data = tex->lock(video::ETLM_READ_ONLY);
        if (!data)
            continue;
        video::IImage* image = irr_driver->getVideoDriver()
            ->createImageFromData(col_format, size, data,
            false/*ownForeignMemory*/);
        irr_driver->getVideoDriver()->writeImageToFile(image, std::string
            (name + "_" + StringUtils::toString(i) + ".png").c_str());
        image->drop();
        tex->unlock();
    }
#endif
}   // dumpGlyphPage

// ----------------------------------------------------------------------------
/** Write the current glyph page in png inside current running directory.
 *  Useful in gdb without parameter.
 */
void FontWithFace::dumpGlyphPage()
{
    dumpGlyphPage("face");
}   // dumpGlyphPage

// ----------------------------------------------------------------------------
/** Set the face dpi which is resolution-dependent.
 *  Normal text will range from 0.8, in 640x* resolutions (won't scale below
 *  that) to 1.0, in 1024x* resolutions, and linearly up.
 *  Bold text will range from 0.2, in 640x* resolutions (won't scale below
 *  that) to 0.4, in 1024x* resolutions, and linearly up.
 */
void FontWithFace::setDPI()
{
    float scale = std::min(irr_driver->getActualScreenSize().Height / 720.0f,
                             irr_driver->getActualScreenSize().Width  / 900.0f);
    int factorTwo = getScalingFactorTwo();
    
    if (UserConfigParams::m_font_size < 0)
    {
        UserConfigParams::m_font_size = 0;
    }
    else if (UserConfigParams::m_font_size > 6)
    {
        UserConfigParams::m_font_size = 6;
    }
    
    factorTwo += UserConfigParams::m_font_size * 5 - 10;
    m_face_dpi = int(factorTwo * getScalingFactorOne() * scale);
#ifndef SERVER_ONLY
    if (!disableTextShaping())
    {
        m_inverse_shaping = (1.0f / (float)font_manager->getShapingDPI()) *
            float(m_face_dpi);
    }
#endif
}   // setDPI

// ----------------------------------------------------------------------------
/* Get the question mark glyph to show unsupported charaters. */
const FontArea* FontWithFace::getUnknownFontArea() const
{
#ifdef SERVER_ONLY
    static FontArea area;
    return &area;
#else
    std::map<wchar_t, GlyphInfo>::const_iterator n =
        m_character_glyph_info_map.find(L'?');
    assert(n != m_character_glyph_info_map.end());
    const FontArea* area = m_face_ttf->getFontArea(n->second.font_number,
        n->second.glyph_index);
    assert(area != NULL);
    return area;
#endif
}   // getUnknownFontArea

// ----------------------------------------------------------------------------
/** Return the \ref FontArea about a character.
 *  \param c The character to get.
 *  \param[out] fallback_font Whether fallback font is used.
 */
const FontArea& FontWithFace::getAreaFromCharacter(const wchar_t c,
                                                   bool* fallback_font) const
{
    std::map<wchar_t, GlyphInfo>::const_iterator n =
        m_character_glyph_info_map.find(c);
    // Not found, return the first font area, which is a white-space
    if (n == m_character_glyph_info_map.end())
        return *getUnknownFontArea();

#ifndef SERVER_ONLY
    const FontArea* area = m_face_ttf->getFontArea(n->second.font_number,
        n->second.glyph_index);
    if (area != NULL)
    {
        if (fallback_font != NULL)
            *fallback_font = false;
        return *area;
    }
    else if (m_fallback_font != NULL && fallback_font != NULL)
    {
        *fallback_font = true;
        return m_fallback_font->getAreaFromCharacter(c, NULL);
    }

    // Not found, return the first font area, which is a white-space
    if (fallback_font != NULL)
        *fallback_font = false;
#endif

    return *getUnknownFontArea();
}   // getAreaFromCharacter

// ----------------------------------------------------------------------------
/** Get the dimension of text with support to different \ref FontSettings,
 *  it will also do checking for missing characters in font and lazy load them.
 *  \param text The text to be calculated.
 *  \param font_settings \ref FontSettings to use.
 *  \return The dimension of text
 */
core::dimension2d<u32> FontWithFace::getDimension(const core::stringw& text,
                                                  FontSettings* font_settings)
{
#ifdef SERVER_ONLY
    return core::dimension2d<u32>(1, 1);
#else
    if (GUIEngine::isNoGraphics())
        return core::dimension2d<u32>(1, 1);

    const float scale = (font_settings ? font_settings->getScale() : 1.0f)
                     * getNativeScalingFactor();
    if (disableTextShaping())
    {
        return gui::getGlyphLayoutsDimension(text2GlyphsWithoutShaping(text),
            m_font_max_height * scale, 1.0f/*inverse shaping*/, scale);
    }

    auto& gls = font_manager->getCachedLayouts(text);
    if (gls.empty() && !text.empty())
        font_manager->shape(StringUtils::wideToUtf32(text), gls);

    return gui::getGlyphLayoutsDimension(gls,
        m_font_max_height * scale, m_inverse_shaping, scale);
#endif
}   // getDimension
                                  
// ----------------------------------------------------------------------------
/** Calculate the index of the character in the text on a specific position.
 *  \param text The text to be calculated.
 *  \param pixel_x The specific position.
 *  \param font_settings \ref FontSettings to use.
 *  \return The index of the character, -1 means no character in such position.
 */
int FontWithFace::getCharacterFromPos(const wchar_t* text, int pixel_x,
                                      FontSettings* font_settings) const
{
#ifndef SERVER_ONLY
    const float scale = (font_settings ? font_settings->getScale() : 1.0f)
                     * getNativeScalingFactor();
    float x = 0;
    int idx = 0;

    while (text[idx])
    {
        bool use_fallback_font = false;
        const FontArea &a  = getAreaFromCharacter(text[idx],
            &use_fallback_font);

        x += getCharWidth(a, use_fallback_font, scale);

        if (x >= float(pixel_x))
            return idx;

        ++idx;
    }
#endif
    return -1;
}   // getCharacterFromPos

// ----------------------------------------------------------------------------
/** Render text and clip it to the specified rectangle if wanted, it will also
 *  do checking for missing characters in font and lazy load them.
 *  \param gl GlyphLayout rendered by libraqm to be rendering.
 *  \param position The position to be rendering.
 *  \param color The color used when rendering.
 *  \param hcenter If rendered horizontally center.
 *  \param vcenter If rendered vertically center.
 *  \param clip If clipping is needed.
 *  \param font_settings \ref FontSettings to use.
 *  \param char_collector \ref FontCharCollector to render billboard text.
 */
#undef DEBUG_NEWLINE
void FontWithFace::render(const std::vector<gui::GlyphLayout>& gl,
                          const core::rect<s32>& position,
                          const video::SColor& color, bool hcenter,
                          bool vcenter, const core::rect<s32>* clip,
                          FontSettings* font_settings,
                          FontCharCollector* char_collector)
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics() || gl.empty())
        return;

    const bool black_border = font_settings ?
        font_settings->useBlackBorder() : false;
    const bool colored_border = font_settings ?
        font_settings->useColoredBorder() : false;
    const float scale = (font_settings ? font_settings->getScale() : 1.0f)
                     * getNativeScalingFactor();
    const float shadow = font_settings ? font_settings->useShadow() : false;

    if (shadow)
    {
        assert(font_settings);
        // Avoid infinite recursion
        font_settings->setShadow(false);

        core::rect<s32> shadowpos = position;
        shadowpos.LowerRightCorner.X += 2;
        shadowpos.LowerRightCorner.Y += 2;
        render(gl, shadowpos, font_settings->getShadowColor(), hcenter,
            vcenter, clip, font_settings);

        // Set back
        font_settings->setShadow(true);
    }

    core::position2d<float> offset;
    f32 next_line_height = 0.0f;
    std::vector<f32> width_per_line;
    if (!gui::getDrawOffset(position, hcenter, vcenter, gl, m_inverse_shaping,
        m_font_max_height, m_glyph_max_height, scale, clip, &offset,
        &next_line_height, &width_per_line))
        return;

    // Collect character locations
    const unsigned int text_size = gl.size();
    std::vector<std::pair<int, int> > indices;
    core::array<core::position2d<float>> offsets(text_size);
    std::vector<bool> fallback(text_size);
    core::array<core::position2d<float>> gld_offsets;
    gui::GlyphLayoutDraw df_used = gui::GLD_NONE;

    // Check if the line is RTL
    bool rtl = (gl[0].flags & gui::GLF_RTL_LINE) != 0;
    if (!hcenter && rtl)
        offset.X += (s32)(position.getWidth() - width_per_line[0]);

    unsigned cur_line = 0;
    bool line_changed = false;
    for (unsigned i = 0; i < gl.size(); i++)
    {
        const gui::GlyphLayout& glyph_layout = gl[i];
        if ((glyph_layout.flags & gui::GLF_NEWLINE) != 0)
        {
#ifdef DEBUG_NEWLINE
            if ((glyph_layout.flags & gui::GLF_BREAKTEXT_NEWLINE) != 0)
            {
                GL32_draw2DRectangle(video::SColor(255, 255, 50, 50),
                    core::recti(offset.X, offset.Y, offset.X + 3,
                    offset.Y + next_line_height));
            }
            else
            {
                GL32_draw2DRectangle(video::SColor(255, 50, 50, 255),
                    core::recti(offset.X, offset.Y, offset.X + 3,
                    offset.Y + next_line_height));
            }
#endif
            offset.X = float(position.UpperLeftCorner.X);
            offset.Y += (s32)next_line_height;
            cur_line++;
            line_changed = true;
            continue;
        }
        if (line_changed)
        {
            line_changed = false;
            rtl = (glyph_layout.flags & gui::GLF_RTL_LINE) != 0;
            if (hcenter)
            {
                offset.X += (s32)(
                    (position.getWidth() - width_per_line.at(cur_line)) / 2.f);
            }
            else if (rtl)
            {
                offset.X +=
                    (s32)(position.getWidth() - width_per_line.at(cur_line));
            }
        }

        bool use_fallback_font = false;
        const FontArea* area = NULL;
        if (glyph_layout.index == 0)
            area = getUnknownFontArea();
        if (area == NULL)
        {
            if (m_face_ttf->enabledForFont(glyph_layout.face_idx))
            {
                area = m_face_ttf->getFontArea(
                    glyph_layout.face_idx, glyph_layout.index);
                if (area == NULL)
                {
                    insertGlyph(glyph_layout.face_idx, glyph_layout.index);
                    area = m_face_ttf->getFontArea(
                        glyph_layout.face_idx, glyph_layout.index);
                }
            }
            else if (m_fallback_font && m_fallback_font
                ->m_face_ttf->enabledForFont(glyph_layout.face_idx))
            {
                use_fallback_font = true;
                area = m_fallback_font->m_face_ttf->getFontArea(
                    glyph_layout.face_idx, glyph_layout.index);
                if (area == NULL)
                {
                    m_fallback_font->insertGlyph(glyph_layout.face_idx,
                        glyph_layout.index);
                    area = m_fallback_font->m_face_ttf->getFontArea(
                        glyph_layout.face_idx, glyph_layout.index);
                }
            }
        }
        fallback[i]            = use_fallback_font;
        if (char_collector == NULL)
        {
            float glyph_offset_x = (int)(area->bearing_x +
                glyph_layout.x_offset * m_inverse_shaping) *
                (fallback[i] ? m_fallback_font_scale : scale);
            float glyph_offset_y = (int)(area->offset_y -
                glyph_layout.y_offset * m_inverse_shaping) *
                (fallback[i] ? m_fallback_font_scale : scale);
            offset.X += glyph_offset_x;
            offset.Y += glyph_offset_y;
            offsets.push_back(offset);
            offset.X -= glyph_offset_x;
            offset.Y -= glyph_offset_y;
        }
        else
        {
            // Billboard text specific, use offset_y_bt instead
            float glyph_offset_x = (int)(area->bearing_x +
                glyph_layout.x_offset * m_inverse_shaping) *
                (fallback[i] ? m_fallback_font_scale : scale);
            float glyph_offset_y = (int)(area->offset_y_bt +
                glyph_layout.y_offset * m_inverse_shaping) *
                (fallback[i] ? m_fallback_font_scale : scale);
            offset.X += glyph_offset_x;
            offset.Y += glyph_offset_y;
            offsets.push_back(offset);
            offset.X -= glyph_offset_x;
            offset.Y -= glyph_offset_y;
        }

        indices.emplace_back(area->spriteno, glyph_layout.flags);
        if ((glyph_layout.flags & gui::GLF_QUICK_DRAW) != 0)
        {
            offset.X += glyph_layout.x_advance * scale;
        }
        else
        {
            int width = (int)(glyph_layout.x_advance * m_inverse_shaping);
            if (char_collector == NULL)
            {
                float each_size = width * scale /
                    (float)glyph_layout.cluster.size();
                float start = offset.X;
                for (unsigned df = 0; df < glyph_layout.draw_flags.size();
                     df++)
                {
                    if (glyph_layout.draw_flags[df] != gui::GLD_NONE)
                    {
                        if (df_used == gui::GLD_NONE)
                        {
                            if (glyph_layout.draw_flags[df] & gui::GLD_MARKED)
                                df_used = gui::GLD_MARKED;
                            else if (glyph_layout.draw_flags[df] &
                                gui::GLD_COMPOSING)
                                df_used = gui::GLD_COMPOSING;
                        }
                        gld_offsets.push_back({start, offset.Y});
                        gld_offsets.push_back({start + each_size, offset.Y});
                    }
                    start += each_size;
                }
            }
            offset.X += width * scale;
        }
    }   // for i < text_size

    // Do the actual rendering
    const int indice_amount                 = indices.size();
    core::array<gui::SGUISprite>& sprites   = m_spritebank->getSprites();
    core::array<core::rect<s32>>& positions = m_spritebank->getPositions();
    core::array<gui::SGUISprite>* fallback_sprites;
    core::array<core::rect<s32>>* fallback_positions;
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

    const int sprite_amount = sprites.size();

    if ((black_border || colored_border || isBold()) && char_collector == NULL)
    {
        // Draw black border first, to make it behind the real character
        // which make script language display better
        video::SColor custom_color = font_settings->getBorderColor();
        video::SColor black(color.getAlpha(),0,0,0);
        video::SColor border_color = (colored_border) ? custom_color : black;

        for (int n = 0; n < indice_amount; n++)
        {
            const int sprite_id = indices[n].first;
            if (!fallback[n] && (sprite_id < 0 || sprite_id >= sprite_amount))
                continue;
            if (indices[n].first == -1) continue;

            const int tex_id = (fallback[n] ?
                (*fallback_sprites)[sprite_id].Frames[0].textureNumber :
                sprites[sprite_id].Frames[0].textureNumber);

            core::rect<s32> source = (fallback[n] ? (*fallback_positions)
                [(*fallback_sprites)[sprite_id].Frames[0].rectNumber] :
                positions[sprites[sprite_id].Frames[0].rectNumber]);

            core::dimension2d<float> size(0.0f, 0.0f);

            float cur_scale = (fallback[n] ? m_fallback_font_scale : scale);
            size.Width  = source.getSize().Width  * cur_scale;
            size.Height = source.getSize().Height * cur_scale;

            core::rect<float> dest(offsets[n], size);

            video::ITexture* texture = (fallback[n] ?
                m_fallback_font->m_spritebank->getTexture(tex_id) :
                m_spritebank->getTexture(tex_id));

            bool thin_border = font_settings ?
                font_settings->useThinBorder() : false;

            if (fallback[n])
                thin_border = true;
            int thickness = (thin_border) ? 1 : 2;

            for (int x_delta = -thickness; x_delta <= thickness; x_delta++)
            {
                for (int y_delta = -thickness; y_delta <= thickness; y_delta++)
                {
                    if (x_delta == 0 || y_delta == 0) continue;
                    FontDrawer::addGlyph(texture, dest + core::position2d<float>
                        (float(x_delta), float(y_delta)), source, clip,
                        border_color);
                }
            }
        }
    }

    std::array<video::SColor, 4> white =
        { {
              video::SColor(-1), video::SColor(-1),
              video::SColor(-1), video::SColor(-1)
        } };
    video::SColor top = GUIEngine::getSkin()->getColor("font::top");
    video::SColor bottom = GUIEngine::getSkin()->getColor("font::bottom");
    top.setAlpha(color.getAlpha());
    bottom.setAlpha(color.getAlpha());

    std::array<video::SColor, 4> title_colors =
        { { bottom, top , bottom, top } };

    video::SColor text_marked = GUIEngine::getSkin()->getColor(
        "text_field::background_marked");
    video::SColor text_neutral = GUIEngine::getSkin()->getColor(
        "text::neutral");

    for (int n = 0; n < indice_amount; n++)
    {
        const int sprite_id = indices[n].first;
        if (!fallback[n] && (sprite_id < 0 || sprite_id >= sprite_amount))
            continue;
        if (indices[n].first == -1) continue;

        const int tex_id = (fallback[n] ?
            (*fallback_sprites)[sprite_id].Frames[0].textureNumber :
            sprites[sprite_id].Frames[0].textureNumber);

        core::rect<s32> source = (fallback[n] ?
            (*fallback_positions)[(*fallback_sprites)[sprite_id].Frames[0]
            .rectNumber] : positions[sprites[sprite_id].Frames[0].rectNumber]);

        core::dimension2d<float> size(0.0f, 0.0f);

        float cur_scale = (fallback[n] ? m_fallback_font_scale : scale);
        size.Width  = source.getSize().Width  * cur_scale;
        size.Height = source.getSize().Height * cur_scale;

        core::rect<float> dest(offsets[n], size);

        video::ITexture* texture = (fallback[n] ?
            m_fallback_font->m_spritebank->getTexture(tex_id) :
            m_spritebank->getTexture(tex_id));

        const bool is_colored = (indices[n].second & gui::GLF_COLORED) != 0;
        const bool is_url = (indices[n].second & gui::GLF_URL) != 0;
        if (isBold())
        {
            if (char_collector != NULL)
            {
                char_collector->collectChar(texture, dest, source,
                    is_colored ? white.data() : title_colors.data());
            }
            else
            {
                FontDrawer::addGlyph(texture, dest, source, clip,
                    is_colored ? white.data() : title_colors.data());
            }
        }
        else
        {
            if (char_collector != NULL)
            {
                std::array<video::SColor, 4> single_color =
                    { {color, color, color, color} };
                char_collector->collectChar(texture, dest, source,
                    is_colored ? white.data() : single_color.data());
            }
            else
            {
                video::SColor single_color = color;
                if (is_url)
                    single_color = text_marked;
                else if (is_colored)
                    single_color = video::SColor(-1);
                FontDrawer::addGlyph(texture, dest, source, clip,
                    single_color);
            }
        }
    }
    FontDrawer::draw();

    for (unsigned i = 0; i < gld_offsets.size(); i += 2)
    {
        if (df_used == gui::GLD_MARKED)
        {
            core::rect<s32> gld((s32)gld_offsets[i].X, (s32)gld_offsets[i].Y,
                (s32)gld_offsets[i + 1].X,
                (s32)(gld_offsets[i + 1].Y + m_font_max_height * scale));
            GL32_draw2DRectangle(text_marked, gld, clip);
        }
        else if (df_used == gui::GLD_COMPOSING)
        {
            float line1 = m_font_max_height * scale * 0.88f;
            float line2 = m_font_max_height * scale * 0.92f;
            core::rect<s32> gld((s32)gld_offsets[i].X,
                (s32)gld_offsets[i].Y + line1, (s32)gld_offsets[i + 1].X,
                (s32)(gld_offsets[i + 1].Y + line2));
            GL32_draw2DRectangle(text_neutral, gld, clip);
        }
    }
#endif
}   // render

// ----------------------------------------------------------------------------
/** Return a character width.
 *  \param area \ref FontArea to get glyph metrics.
 *  \param fallback If fallback font is used.
 *  \param scale The scaling of the character.
 *  \return The calculated width with suitable scaling. */
float FontWithFace::getCharWidth(const FontArea& area, bool fallback,
                                 float scale) const
{
    if (fallback)
        return area.advance_x * m_fallback_font_scale;
    else
        return area.advance_x * scale;
}   // getCharWidth

// ----------------------------------------------------------------------------
/* Cached version of render to make drawing as fast as possible. */
void FontWithFace::drawText(const core::stringw& text,
                            const core::rect<s32>& position,
                            const video::SColor& color, bool hcenter,
                            bool vcenter, const core::rect<s32>* clip,
                            FontSettings* font_settings,
                            FontCharCollector* char_collector)

{
#ifndef SERVER_ONLY
    if (text.empty() || GUIEngine::isNoGraphics())
        return;

    if (disableTextShaping())
    {
        render(text2GlyphsWithoutShaping(text), position, color, hcenter,
            vcenter, clip, font_settings, char_collector);
        return;
    }

    auto& gls = font_manager->getCachedLayouts(text);
    if (gls.empty() && !text.empty())
        font_manager->shape(StringUtils::wideToUtf32(text), gls);

    render(gls, position, color, hcenter, vcenter, clip,
        font_settings, char_collector);
#endif
}   // drawText

// ----------------------------------------------------------------------------
/* No text shaping and bidi operation of drawText. */
void FontWithFace::drawTextQuick(const core::stringw& text,
                                 const core::rect<s32>& position,
                                 const video::SColor& color, bool hcenter,
                                 bool vcenter, const core::rect<s32>* clip,
                                 FontSettings* font_settings,
                                 FontCharCollector* char_collector)
{
#ifndef SERVER_ONLY
    if (text.empty() || GUIEngine::isNoGraphics())
        return;

    render(text2GlyphsWithoutShaping(text), position, color, hcenter,
        vcenter, clip, font_settings, char_collector);
#endif
}   // drawTextQuick

// ----------------------------------------------------------------------------
/** Convert text to drawable GlyphLayout without text shaping, used in digit
 *  font or debugging message. */
std::vector<gui::GlyphLayout> FontWithFace::
    text2GlyphsWithoutShaping(const core::stringw& t)
{
    std::vector<gui::GlyphLayout> layouts;
#ifndef SERVER_ONLY
    for (unsigned i = 0; i < t.size(); i++)
    {
        wchar_t c = t[i];
        gui::GlyphLayout gl = { 0 };
        if (c == L'\r' ||          // Windows breaks
            c == L'\n'    )        // Unix breaks
        {
            if (c == L'\r' && i != t.size() - 1 && t[i + 1] == L'\n')
                i++;
            gl.flags = gui::GLF_NEWLINE;
            layouts.push_back(gl);
            continue;
        }
        auto ret = m_character_glyph_info_map.find(c);
        if (ret == m_character_glyph_info_map.end())
        {
            unsigned font = 0;
            unsigned glyph = 0;
            if (!m_face_ttf->getFontAndGlyphFromChar(c, &font, &glyph))
            {
                m_character_glyph_info_map[c] = GlyphInfo(font, glyph);
                continue;
            }
            m_character_glyph_info_map[c] = GlyphInfo(font, glyph);
            ret = m_character_glyph_info_map.find(c);
            insertGlyph(font, glyph);
        }
        const FontArea* area = m_face_ttf->getFontArea
            (ret->second.font_number, ret->second.glyph_index);
        if (area == NULL)
            continue;
        gl.index = ret->second.glyph_index;
        gl.x_advance = area->advance_x;
        gl.face_idx = ret->second.font_number;
        gl.flags = gui::GLF_QUICK_DRAW;
        layouts.push_back(gl);
    }
#endif
    return layouts;
}   // text2GlyphsWithoutShaping
