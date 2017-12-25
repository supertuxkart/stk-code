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

#include <assert.h>
#include <vector>

namespace irr
{
    namespace scene { class IMesh; }
}

enum KartRenderType: unsigned int
{
    KRT_DEFAULT,
    KRT_RED,
    KRT_ORANGE,
    KRT_YELLOW,
    KRT_GREEN,
    KRT_PALE_BLUE,
    KRT_BLUE,
    KRT_PURPLE,
    KRT_TRANSPARENT,
};

/**
  * \ingroup graphics
  */
class RenderInfo : public NoCopy
{
private:
    float m_static_hue;

    bool m_transparent;

public:
    // ------------------------------------------------------------------------
    RenderInfo(float hue = 0.0f, bool transparent = false)
    {
        m_static_hue = hue;
        m_transparent = transparent;
    }
    // ------------------------------------------------------------------------
    ~RenderInfo() {}
    // ------------------------------------------------------------------------
    void setHue(float hue)                             { m_static_hue = hue; }
    // ------------------------------------------------------------------------
    void setTransparent(bool transparent)     { m_transparent = transparent; }
    // ------------------------------------------------------------------------
    float getHue() const                              { return m_static_hue; }
    // ------------------------------------------------------------------------
    bool isTransparent() const                       { return m_transparent; }
    // ------------------------------------------------------------------------
    void setKartModelRenderInfo(KartRenderType krt)
    {
        setHue(krt == KRT_RED ? 1.0f :
            krt == KRT_ORANGE ? 0.06f :
            krt == KRT_YELLOW ? 0.17f :
            krt == KRT_GREEN ? 0.35f :
            krt == KRT_PALE_BLUE ? 0.5f :
            krt == KRT_BLUE ? 0.66f :
            krt == KRT_PURPLE ? 0.8f : 0.0f);
        setTransparent(krt == KRT_TRANSPARENT ? true : false);
    }

};   // RenderInfo

#endif

/* EOF */
