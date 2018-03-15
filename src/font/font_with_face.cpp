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

#include "font/face_ttf.hpp"
#include "font/font_manager.hpp"
#include "font/font_settings.hpp"
#include "graphics/2dutils.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/stk_texture.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/skin.hpp"
#include "utils/string_utils.hpp"

#include <array>

// ----------------------------------------------------------------------------
/** Constructor. It will initialize the \ref m_spritebank and TTF files to use.
 *  \param name The name of face, used by irrlicht to distinguish spritebank.
 *  \param ttf \ref FaceTTF for this face to use.
 */
FontWithFace::FontWithFace(const std::string& name, FaceTTF* ttf)
{
    m_spritebank = irr_driver->getGUI()->addEmptySpriteBank(name.c_str());

    assert(m_spritebank != NULL);
    m_spritebank->grab();

    m_fallback_font = NULL;
    m_fallback_font_scale = 1.0f;
    m_glyph_max_height = 0;
    m_face_ttf = ttf;

}   // FontWithFace
// ----------------------------------------------------------------------------
/** Destructor. Clears the glyph page and sprite bank.
 */
FontWithFace::~FontWithFace()
{
    for (unsigned int i = 0; i < m_spritebank->getTextureCount(); i++)
    {
        STKTexManager::getInstance()->removeTexture(
            static_cast<STKTexture*>(m_spritebank->getTexture(i)));
    }
    m_spritebank->drop();
    m_spritebank = NULL;

    // To be deleted by font_manager
    m_face_ttf = NULL;

}   // ~FontWithFace

// ----------------------------------------------------------------------------
/** Initialize the font structure, but don't load glyph here.
 */
void FontWithFace::init()
{
    setDPI();
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

    reset();
}   // init

// ----------------------------------------------------------------------------
/** Clear all the loaded characters, sub-class can do pre-loading of characters
 *  after this.
 */
void FontWithFace::reset()
{
    m_new_char_holder.clear();
    m_character_area_map.clear();
    m_character_glyph_info_map.clear();
    for (unsigned int i = 0; i < m_spritebank->getTextureCount(); i++)
    {
        STKTexManager::getInstance()->removeTexture(
            static_cast<STKTexture*>(m_spritebank->getTexture(i)));
    }
    m_spritebank->clear();
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
    unsigned int font_number = 0;
    unsigned int glyph_index = 0;
    while (font_number < m_face_ttf->getTotalFaces())
    {
        glyph_index = FT_Get_Char_Index(m_face_ttf->getFace(font_number), c);
        if (glyph_index > 0) break;
        font_number++;
    }
    m_character_glyph_info_map[c] = GlyphInfo(font_number, glyph_index);
}   // loadGlyphInfo

// ----------------------------------------------------------------------------
/** Create a new glyph page by filling it with transparent content.
 */
void FontWithFace::createNewGlyphPage()
{
#ifndef SERVER_ONLY
    uint8_t* data = new uint8_t[getGlyphPageSize() * getGlyphPageSize() *
    (CVS->isARBTextureSwizzleUsable() ? 1 : 4)]();
#else
    uint8_t* data = NULL;
#endif
    m_current_height = 0;
    m_used_width = 0;
    m_used_height = 0;
    STKTexture* stkt = new STKTexture(data, typeid(*this).name() +
        StringUtils::toString(m_spritebank->getTextureCount()),
        getGlyphPageSize(),
#ifndef SERVER_ONLY
        CVS->isARBTextureSwizzleUsable()
#else
        false
#endif
        );
    m_spritebank->addTexture(STKTexManager::getInstance()->addTexture(stkt));
}   // createNewGlyphPage

// ----------------------------------------------------------------------------
/** Render a glyph for a character into bitmap and save it into the glyph page.
 *  \param c The character to be loaded.
 *  \param c \ref GlyphInfo for the character.
 */
void FontWithFace::insertGlyph(wchar_t c, const GlyphInfo& gi)
{
    assert(gi.glyph_index > 0);
    assert(gi.font_number < m_face_ttf->getTotalFaces());
    FT_Face cur_face = m_face_ttf->getFace(gi.font_number);
    FT_GlyphSlot slot = cur_face->glyph;

    // Same face may be shared across the different FontWithFace,
    // so reset dpi each time
    font_manager->checkFTError(FT_Set_Pixel_Sizes(cur_face, 0, getDPI()),
        "setting DPI");

    font_manager->checkFTError(FT_Load_Glyph(cur_face, gi.glyph_index,
        FT_LOAD_DEFAULT), "loading a glyph");

    font_manager->checkFTError(shapeOutline(&(slot->outline)),
        "shaping outline");

    font_manager->checkFTError(FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL),
        "rendering a glyph to bitmap");

    // Convert to an anti-aliased bitmap
    FT_Bitmap* bits = &(slot->bitmap);
    core::dimension2du texture_size(bits->width + 1, bits->rows + 1);
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

    const unsigned int cur_tex = m_spritebank->getTextureCount() -1;
#ifndef SERVER_ONLY
    if (bits->buffer != NULL)
    {
        video::ITexture* tex = m_spritebank->getTexture(cur_tex);
        glBindTexture(GL_TEXTURE_2D, tex->getOpenGLTextureName());
        assert(bits->pixel_mode == FT_PIXEL_MODE_GRAY);
        if (CVS->isARBTextureSwizzleUsable())
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, m_used_width, m_used_height,
                bits->width, bits->rows, GL_RED, GL_UNSIGNED_BYTE,
                bits->buffer);
        }
        else
        {
            const unsigned int size = bits->width * bits->rows;
            uint8_t* image_data = new uint8_t[size * 4];
            memset(image_data, 255, size * 4);
            for (unsigned int i = 0; i < size; i++)
                image_data[4 * i + 3] = bits->buffer[i];
            glTexSubImage2D(GL_TEXTURE_2D, 0, m_used_width, m_used_height,
                bits->width, bits->rows, GL_RGBA, GL_UNSIGNED_BYTE,
                image_data);
            delete[] image_data;
        }
        if (tex->hasMipMaps())
            glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
#endif

    // Store the rectangle of current glyph
    gui::SGUISpriteFrame f;
    gui::SGUISprite s;
    core::rect<s32> rectangle(m_used_width, m_used_height,
        m_used_width + bits->width, m_used_height + bits->rows);
    f.rectNumber = m_spritebank->getPositions().size();
    f.textureNumber = cur_tex;

    // Add frame to sprite
    s.Frames.push_back(f);
    s.frameTime = 0;
    m_spritebank->getPositions().push_back(rectangle);
    m_spritebank->getSprites().push_back(s);

    // Save glyph metrics
    FontArea a;
    a.advance_x = cur_face->glyph->advance.x / BEARING;
    a.bearing_x = cur_face->glyph->metrics.horiBearingX / BEARING;
    const int cur_height = (cur_face->glyph->metrics.height / BEARING);
    const int cur_offset_y = cur_height -
        (cur_face->glyph->metrics.horiBearingY / BEARING);
    a.offset_y = m_glyph_max_height - cur_height + cur_offset_y;
    a.offset_y_bt = -cur_offset_y;
    a.spriteno = f.rectNumber;
    m_character_area_map[c] = a;

    // Store used area
    m_used_width += texture_size.Width;
    if (m_current_height < texture_size.Height)
        m_current_height = texture_size.Height;
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
        insertGlyph(c, gi);
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
    for (unsigned int i = 0; i < m_spritebank->getTextureCount(); i++)
    {
        video::ITexture* tex = m_spritebank->getTexture(i);
        core::dimension2d<u32> size = tex->getSize();
        video::ECOLOR_FORMAT col_format = tex->getColorFormat();
        void* data = tex->lock();
        video::IImage* image = irr_driver->getVideoDriver()
            ->createImageFromData(col_format, size, data,
            true/*ownForeignMemory*/);
        tex->unlock();
        irr_driver->getVideoDriver()->writeImageToFile(image, std::string
            (name + "_" + StringUtils::toString(i) + ".png").c_str());
        image->drop();
    }
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
    const int screen_width = irr_driver->getFrameSize().Width;
    const int screen_height = irr_driver->getFrameSize().Height;
    float scale = std::max(0, screen_width - 640) / 564.0f;

    // attempt to compensate for small screens
    if (screen_width < 1200)
        scale = std::max(0, screen_width - 640) / 750.0f;
    if (screen_width < 900 || screen_height < 700)
        scale = std::min(scale, 0.05f);

    m_face_dpi = unsigned((getScalingFactorOne() + 0.2f * scale) *
        getScalingFactorTwo());

}   // setDPI

// ----------------------------------------------------------------------------
/** Return the \ref FontArea about a character.
 *  \param c The character to get.
 *  \param[out] fallback_font Whether fallback font is used.
 */
const FontWithFace::FontArea&
    FontWithFace::getAreaFromCharacter(const wchar_t c,
                                       bool* fallback_font) const
{
    std::map<wchar_t, FontArea>::const_iterator n =
        m_character_area_map.find(c);
    if (n != m_character_area_map.end())
    {
        if (fallback_font != NULL)
            *fallback_font = false;
        return n->second;
    }
    else if (m_fallback_font != NULL && fallback_font != NULL)
    {
        *fallback_font = true;
        return m_fallback_font->getAreaFromCharacter(c, NULL);
    }

    // Not found, return the first font area, which is a white-space
    if (fallback_font != NULL)
        *fallback_font = false;
    return m_character_area_map.begin()->second;

}   // getAreaFromCharacter

// ----------------------------------------------------------------------------
/** Get the dimension of text with support to different \ref FontSettings,
 *  it will also do checking for missing characters in font and lazy load them.
 *  \param text The text to be calculated.
 *  \param font_settings \ref FontSettings to use.
 *  \return The dimension of text
 */
core::dimension2d<u32> FontWithFace::getDimension(const wchar_t* text,
                                            FontSettings* font_settings)
{
    const float scale = font_settings ? font_settings->getScale() : 1.0f;
    // Test if lazy load char is needed
    insertCharacters(text);
    updateCharactersList();

    assert(m_character_area_map.size() > 0);
    core::dimension2d<float> dim(0.0f, 0.0f);
    core::dimension2d<float> this_line(0.0f, m_font_max_height * scale);

    for (const wchar_t* p = text; *p; ++p)
    {
        if (*p == L'\r'  ||      // Windows breaks
            *p == L'\n'      )   // Unix breaks
        {
            if (*p==L'\r' && p[1] == L'\n') // Windows breaks
                ++p;
            dim.Height += this_line.Height;
            if (dim.Width < this_line.Width)
                dim.Width = this_line.Width;
            this_line.Width = 0;
            continue;
        }

        bool fallback = false;
        const FontArea &area = getAreaFromCharacter(*p, &fallback);

        this_line.Width += getCharWidth(area, fallback, scale);
    }

    dim.Height += this_line.Height;
    if (dim.Width < this_line.Width)
        dim.Width = this_line.Width;

    core::dimension2d<u32> ret_dim(0, 0);
    ret_dim.Width  = (u32)(dim.Width + 0.9f); // round up
    ret_dim.Height = (u32)(dim.Height + 0.9f);

    return ret_dim;
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
    const float scale = font_settings ? font_settings->getScale() : 1.0f;
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

    return -1;
}   // getCharacterFromPos

// ----------------------------------------------------------------------------
/** Render text and clip it to the specified rectangle if wanted, it will also
 *  do checking for missing characters in font and lazy load them.
 *  \param text The text to be rendering.
 *  \param position The position to be rendering.
 *  \param color The color used when rendering.
 *  \param hcenter If rendered horizontally center.
 *  \param vcenter If rendered vertically center.
 *  \param clip If clipping is needed.
 *  \param font_settings \ref FontSettings to use.
 *  \param char_collector \ref FontCharCollector to render billboard text.
 */
void FontWithFace::render(const core::stringw& text,
                          const core::rect<s32>& position,
                          const video::SColor& color, bool hcenter,
                          bool vcenter, const core::rect<s32>* clip,
                          FontSettings* font_settings,
                          FontCharCollector* char_collector)
{
#ifndef SERVER_ONLY
    const bool black_border = font_settings ?
        font_settings->useBlackBorder() : false;
    const bool rtl = font_settings ? font_settings->isRTL() : false;
    const float scale = font_settings ? font_settings->getScale() : 1.0f;
    const float shadow = font_settings ? font_settings->useShadow() : false;

    if (shadow)
    {
        assert(font_settings);
        // Avoid infinite recursion
        font_settings->setShadow(false);

        core::rect<s32> shadowpos = position;
        shadowpos.LowerRightCorner.X += 2;
        shadowpos.LowerRightCorner.Y += 2;
        render(text, shadowpos, font_settings->getShadowColor(), hcenter,
            vcenter, clip, font_settings);

        // Set back
        font_settings->setShadow(true);
    }

    core::position2d<float> offset(float(position.UpperLeftCorner.X),
        float(position.UpperLeftCorner.Y));
    core::dimension2d<s32> text_dimension;

    if (rtl || hcenter || vcenter || clip)
    {
        text_dimension = getDimension(text.c_str(), font_settings);

        if (hcenter)
            offset.X += (position.getWidth() - text_dimension.Width) / 2;
        else if (rtl)
            offset.X += (position.getWidth() - text_dimension.Width);

        if (vcenter)
            offset.Y += (position.getHeight() - text_dimension.Height) / 2;
        if (clip)
        {
            core::rect<s32> clippedRect(core::position2d<s32>
                (s32(offset.X), s32(offset.Y)), text_dimension);
            clippedRect.clipAgainst(*clip);
            if (!clippedRect.isValid()) return;
        }
    }

    // Collect character locations
    const unsigned int text_size = text.size();
    core::array<s32> indices(text_size);
    core::array<core::position2d<float>> offsets(text_size);
    std::vector<bool> fallback(text_size);

    // Test again if lazy load char is needed,
    // as some text isn't drawn with getDimension
    insertCharacters(text.c_str());
    updateCharactersList();

    for (u32 i = 0; i < text_size; i++)
    {
        wchar_t c = text[i];

        if (c == L'\r' ||          // Windows breaks
            c == L'\n'    )        // Unix breaks
        {
            if (c==L'\r' && text[i+1]==L'\n')
                c = text[++i];
            offset.Y += m_font_max_height * scale;
            offset.X  = float(position.UpperLeftCorner.X);
            if (hcenter)
                offset.X += (position.getWidth() - text_dimension.Width) >> 1;
            continue;
        }   // if lineBreak

        bool use_fallback_font = false;
        const FontArea &area   = getAreaFromCharacter(c, &use_fallback_font);
        fallback[i]            = use_fallback_font;
        if (char_collector == NULL)
        {
            float glyph_offset_x = area.bearing_x *
                (fallback[i] ? m_fallback_font_scale : scale);
            float glyph_offset_y = area.offset_y *
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
            float glyph_offset_x = area.bearing_x *
                (fallback[i] ? m_fallback_font_scale : scale);
            float glyph_offset_y = area.offset_y_bt *
                (fallback[i] ? m_fallback_font_scale : scale);
            offset.X += glyph_offset_x;
            offset.Y += glyph_offset_y;
            offsets.push_back(offset);
            offset.X -= glyph_offset_x;
            offset.Y -= glyph_offset_y;
        }

        indices.push_back(area.spriteno);
        offset.X += getCharWidth(area, fallback[i], scale);
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

    if ((black_border || isBold()) && char_collector == NULL)
    {
        // Draw black border first, to make it behind the real character
        // which make script language display better
        video::SColor black(color.getAlpha(),0,0,0);
        for (int n = 0; n < indice_amount; n++)
        {
            const int sprite_id = indices[n];
            if (!fallback[n] && (sprite_id < 0 || sprite_id >= sprite_amount))
                continue;
            if (indices[n] == -1) continue;

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

            for (int x_delta = -2; x_delta <= 2; x_delta++)
            {
                for (int y_delta = -2; y_delta <= 2; y_delta++)
                {
                    if (x_delta == 0 || y_delta == 0) continue;
                    draw2DImage(texture, dest + core::position2d<float>
                        (float(x_delta), float(y_delta)), source, clip,
                        black, true);
                }
            }
        }
    }

    for (int n = 0; n < indice_amount; n++)
    {
        const int sprite_id = indices[n];
        if (!fallback[n] && (sprite_id < 0 || sprite_id >= sprite_amount))
            continue;
        if (indices[n] == -1) continue;

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

        if (fallback[n] || isBold())
        {
            video::SColor top = GUIEngine::getSkin()->getColor("font::top");
            video::SColor bottom = GUIEngine::getSkin()
                ->getColor("font::bottom");
            top.setAlpha(color.getAlpha());
            bottom.setAlpha(color.getAlpha());

            std::array<video::SColor, 4> title_colors;
            if (CVS->isGLSL())
            {
                title_colors = { { top, bottom, top, bottom } };
            }
            else
            {
                title_colors = { { bottom, top, top, bottom } };
            }
            if (char_collector != NULL)
            {
                char_collector->collectChar(texture, dest, source,
                    title_colors.data());
            }
            else
            {
                draw2DImage(texture, dest, source, clip, title_colors.data(),
                    true);
            }
        }
        else
        {
            if (char_collector != NULL)
            {
                video::SColor colors[] = {color, color, color, color};
                char_collector->collectChar(texture, dest, source, colors);
            }
            else
            {
                draw2DImage(texture, dest, source, clip, color, true);
            }
        }
    }
#endif
}   // render
