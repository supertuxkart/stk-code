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

class FontSettings
{
private:
    bool m_black_border;

    bool m_rtl;

    float m_scale;

    bool m_shadow;

    video::SColor m_shadow_color;
public:
    LEAK_CHECK()
    // ------------------------------------------------------------------------
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
    ~FontSettings() {}
    // ------------------------------------------------------------------------
    void setScale(float scale)                             { m_scale = scale; }
    // ------------------------------------------------------------------------
    float getScale() const                                  { return m_scale; }
    // ------------------------------------------------------------------------
    void setShadowColor(const video::SColor &col)     { m_shadow_color = col; }
    // ------------------------------------------------------------------------
    const video::SColor& getShadowColor() const      { return m_shadow_color; }
    // ------------------------------------------------------------------------
    bool useShadow() const                                 { return m_shadow; }
    // ------------------------------------------------------------------------
    void setShadow(bool shadow)                          { m_shadow = shadow; }
    // ------------------------------------------------------------------------
    void setBlackBorder(bool border)               { m_black_border = border; }
    // ------------------------------------------------------------------------
    bool useBlackBorder() const                      { return m_black_border; }
    // ------------------------------------------------------------------------
    void setRTL(bool rtl)                                      { m_rtl = rtl; }
    // ------------------------------------------------------------------------
    bool isRTL() const                                        { return m_rtl; }

};   // FontSettings

#endif
/* EOF */
