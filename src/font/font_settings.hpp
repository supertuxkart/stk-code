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

#ifndef HEADER_FONT_SETTINGS_HPP
#define HEADER_FONT_SETTINGS_HPP

#include "utils/leak_check.hpp"

#include "SColor.h"

using namespace irr;

/** This class stores settings when rendering fonts, used when instantiating
 *  \ref irr::gui::ScalableFont.
 *  \ingroup font
 */
class FontSettings
{
private:
    /** True if black border will be drawn when rendering. */
    bool m_black_border;

    /** True if a custom colored border will be drawn when rendering.
      * If both a black border and a colored border are set to be used,
      * the colored border will take priority. */
    bool m_colored_border;

    /* True if the border to draw should be thin */
    bool m_thin_border;

    /** Scaling when rendering. */
    float m_scale;

    /** True if shadow will be drawn when rendering. */
    bool m_shadow;

    /** Save the color of shadow when rendering. */
    video::SColor m_shadow_color;

    /** Used when m_colored_border is true */
    video::SColor m_border_color;

public:
    LEAK_CHECK()
    // ------------------------------------------------------------------------
    /** Constructor. It will initialize all members with default values if no
     *  parameter is given. */
    FontSettings(bool black_border = false, bool colored_border = false,
                 bool thin_border = false, float scale = 1.0f,
                 bool shadow = false,
                 const video::SColor& shadow_color = video::SColor(0, 0, 0, 0),
                 const video::SColor& border_color = video::SColor(0, 0, 0, 0))
    {
        m_black_border = black_border;
        m_colored_border = colored_border;
        m_thin_border = thin_border;
        m_scale = scale;
        m_shadow = shadow;
        m_shadow_color = shadow_color;
        m_border_color = border_color;
    }
    // ------------------------------------------------------------------------
    /** Set the scaling.
     *  \param scale Scaling to be set. */
    void setScale(float scale)                             { m_scale = scale; }
    // ------------------------------------------------------------------------
    /** Return the scaling. */
    float getScale() const                                  { return m_scale; }
    // ------------------------------------------------------------------------
    /** Set the color of shadow.
     *  \param col The color of shadow to be set. */
    void setShadowColor(const video::SColor &col)     { m_shadow_color = col; }
    // ------------------------------------------------------------------------
    /** Return the color of shadow. */
    const video::SColor& getShadowColor() const      { return m_shadow_color; }
    // ------------------------------------------------------------------------
    /** Return if shadow is enabled. */
    bool useShadow() const                                 { return m_shadow; }
    // ------------------------------------------------------------------------
    /** Set whether shadow is enabled.
     *  \param shadow If it's enabled. */
    void setShadow(bool shadow)                          { m_shadow = shadow; }
    // ------------------------------------------------------------------------
    /** Set whether black border is enabled.
     *  \param border If it's enabled. */
    void setBlackBorder(bool border)               { m_black_border = border; }
    // ------------------------------------------------------------------------
    /** Set whether a custom colored border is enabled.
     *  \param border If it's enabled. */
    void setColoredBorder(bool border )          { m_colored_border = border; }
    // ------------------------------------------------------------------------
    /** Set whether the text outline should be thin or not. */
    void setThinBorder(bool thin)                     { m_thin_border = thin; }
    // ------------------------------------------------------------------------
    /** Set the color of border (used when a non-black border is requested).
     *  \param col The color of border to be set. */
    void setBorderColor(const video::SColor &col)     { m_border_color = col; }
    // ------------------------------------------------------------------------
    /** Return the color of the border.. */
    const video::SColor& getBorderColor() const      { return m_border_color; }
    // ------------------------------------------------------------------------
    /** Return if black border is enabled. */
    bool useBlackBorder() const                      { return m_black_border; }
    // ------------------------------------------------------------------------
    /** Return if black border is enabled. */
    bool useColoredBorder() const                  { return m_colored_border; }
    // ------------------------------------------------------------------------
    /** Return if the border should be thin or not. */
    bool useThinBorder() const                        { return m_thin_border; }

};   // FontSettings

#endif
/* EOF */
