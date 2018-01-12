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

#ifndef HEADER_RENDER_INFO_HPP
#define HEADER_RENDER_INFO_HPP

#include "utils/no_copy.hpp"

/**
  * \ingroup graphics
  */
class RenderInfo : public NoCopy
{
private:
    float m_hue;

    bool m_transparent;

public:
    // ------------------------------------------------------------------------
    RenderInfo(float hue = 0.0f, bool transparent = false)
    {
        m_hue = hue;
        m_transparent = transparent;
    }
    // ------------------------------------------------------------------------
    void setHue(float hue)                                    { m_hue = hue; }
    // ------------------------------------------------------------------------
    void setTransparent(bool transparent)     { m_transparent = transparent; }
    // ------------------------------------------------------------------------
    float getHue() const                                     { return m_hue; }
    // ------------------------------------------------------------------------
    bool isTransparent() const                       { return m_transparent; }

};   // RenderInfo

#endif

/* EOF */
