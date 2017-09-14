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

#include <irrlicht.h>

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

    /** If true, characters will have right alignment when rendering, for RTL
     *  language. */
    bool m_rtl;

    /** Scaling when rendering. */
    float m_scale;

    /** True if shadow will be drawn when rendering. */
    bool m_shadow;

    /** Save the color of shadow when rendering. */
    video::SColor m_shadow_color;

public:
    LEAK_CHECK()
    // ------------------------------------------------------------------------
    /** Constructor. It will initialize all members with default values if no
     *  parameter is given. */
    FontSettings(bool black_border = false, bool rtl = false,
                 float scale = 1.0f, bool shadow = false,
                 const video::SColor& color = video::SColor(0, 0, 0, 0))
    {
        m_black_border = black_border;
        m_rtl = rtl;
        m_scale = scale;
        m_shadow = shadow;
        m_shadow_color = color;
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
    /** Return if black border is enabled. */
    bool useBlackBorder() const                      { return m_black_border; }
    // ------------------------------------------------------------------------
    /** Set right text alignment for RTL language.
     *  \param rtl If it's enabled. */
    void setRTL(bool rtl)                                      { m_rtl = rtl; }
    // ------------------------------------------------------------------------
    /** Return if right text alignment for RTL language is enabled. */
    bool isRTL() const                                        { return m_rtl; }

};   // FontSettings

#endif
/* EOF */
