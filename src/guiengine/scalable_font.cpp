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

#include "guiengine/scalable_font.hpp"

#include "font/font_manager.hpp"
#include "font/font_settings.hpp"
#include "font/font_with_face.hpp"

namespace irr
{
namespace gui
{
// ----------------------------------------------------------------------------
ScalableFont::ScalableFont(FontWithFace* face)
{
    m_face = face;
    m_font_settings = new FontSettings();
}   // ScalableFont

// ----------------------------------------------------------------------------
ScalableFont::~ScalableFont()
{
    delete m_font_settings;
}   // ~ScalableFont

// ----------------------------------------------------------------------------
void ScalableFont::setShadow(const irr::video::SColor &col)
{
    m_font_settings->setShadow(true);
    m_font_settings->setShadowColor(col);
}   // setShadow

// ----------------------------------------------------------------------------
void ScalableFont::disableShadow()
{
    m_font_settings->setShadow(false);
}   // disableShadow

// ----------------------------------------------------------------------------
void ScalableFont::setBlackBorder(bool enabled)
{
    m_font_settings->setBlackBorder(enabled);
}   // setBlackBorder

// ----------------------------------------------------------------------------
void ScalableFont::setColoredBorder(const irr::video::SColor &col)
{
    m_font_settings->setColoredBorder(true);
    m_font_settings->setBorderColor(col);
}   // setColoredBorder

// ----------------------------------------------------------------------------
void ScalableFont::setThinBorder(bool thin)
{
    m_font_settings->setThinBorder(thin);
}   // setThinBorder

// ----------------------------------------------------------------------------
void ScalableFont::disableColoredBorder()
{
    m_font_settings->setColoredBorder(false);
}   // setShadow

// ----------------------------------------------------------------------------
void ScalableFont::setScale(float scale)
{
    m_font_settings->setScale(scale);
}   // setScale

// ----------------------------------------------------------------------------
float ScalableFont::getScale() const
{
    return m_font_settings->getScale();
}   // getScale

// ----------------------------------------------------------------------------
core::dimension2d<u32> ScalableFont::getDimension(const wchar_t* text) const
{
    return m_face->getDimension(text, m_font_settings);
}   // getDimension

// ----------------------------------------------------------------------------
void ScalableFont::draw(const core::stringw& text,
                        const core::rect<s32>& position, video::SColor color,
                        bool hcenter, bool vcenter,
                        const core::rect<s32>* clip)
{
    m_face->drawText(text, position, color, hcenter, vcenter, clip,
        m_font_settings);
}   // draw

// ----------------------------------------------------------------------------
void ScalableFont::draw(const std::vector<GlyphLayout>& gls,
                        const core::rect<s32>& position, video::SColor color,
                        bool hcenter, bool vcenter,
                        const core::rect<s32>* clip)
{
    m_face->render(gls, position, color, hcenter, vcenter, clip,
        m_font_settings);
}   // draw

// ----------------------------------------------------------------------------
void ScalableFont::draw(const core::stringw& text,
                        const core::rect<s32>& position,
                        const video::SColor& color, bool hcenter, bool vcenter,
                        const core::rect<s32>* clip, bool ignoreRTL)
{
    m_face->drawText(text, position, color, hcenter, vcenter, clip,
        m_font_settings);
}   // draw

// ----------------------------------------------------------------------------
void ScalableFont::drawQuick(const core::stringw& text,
                             const core::rect<s32>& position,
                             const video::SColor color, bool hcenter,
                             bool vcenter, const core::rect<s32>* clip)
{
    m_face->drawTextQuick(text, position, color, hcenter, vcenter, clip,
        m_font_settings);
}   // draw

// ----------------------------------------------------------------------------
s32 ScalableFont::getCharacterFromPos(const wchar_t* text, s32 pixel_x) const
{
    return m_face->getCharacterFromPos(text, pixel_x, m_font_settings);
}   // getCharacterFromPos

// ----------------------------------------------------------------------------
IGUISpriteBank* ScalableFont::getSpriteBank() const
{
    return m_face->getSpriteBank();
}   // getSpriteBank

// ----------------------------------------------------------------------------
s32 ScalableFont::getHeightPerLine() const
{
    return m_face->getFontMaxHeight()
         * m_face->getNativeScalingFactor()
         * m_font_settings->getScale();
}   // getHeightPerLine

// ----------------------------------------------------------------------------
/** Convert text to glyph layouts for fast rendering with optional caching
 *  enabled.
 */
void ScalableFont::initGlyphLayouts(const core::stringw& text,
                                    std::vector<GlyphLayout>& gls,
                                    u32 shape_flag)
{
#ifndef SERVER_ONLY
    font_manager->initGlyphLayouts(text, gls, shape_flag);
#endif
}   // initGlyphLayouts

// ----------------------------------------------------------------------------
f32 ScalableFont::getInverseShaping() const
{
#ifndef SERVER_ONLY
    return m_face->getInverseShaping();
#else
    return 1.0f;
#endif
}   // getShapingScale

// ----------------------------------------------------------------------------
s32 ScalableFont::getFaceFontMaxHeight() const
{
#ifndef SERVER_ONLY
    return m_face->getFontMaxHeight();
#else
    return 1;
#endif
}   // getFaceFontMaxHeight

// ----------------------------------------------------------------------------
s32 ScalableFont::getFaceGlyphMaxHeight() const
{
#ifndef SERVER_ONLY
    return m_face->getGlyphMaxHeight();
#else
    return 1;
#endif
}   // getFaceGlyphMaxHeight

} // end namespace gui
} // end namespace irr
