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

#include "utils/leak_check.hpp"

#include <algorithm>
#include <vector>

namespace irr
{
    namespace scene { class IMesh; }
}

/**
  * \ingroup graphics
  */
class RenderInfo
{
public:
    enum KartRenderType
    {
        KRT_DEFAULT,
        KRT_RED,
        KRT_BLUE,
        KRT_TRANSPARENT,
    };

private:
    float m_hue;

    bool m_transparent;

    std::vector<int> m_colorizable_parts;

public:
    LEAK_CHECK();
    // ------------------------------------------------------------------------
    RenderInfo(float hue = 0.0f, bool transparent = false);
    // ------------------------------------------------------------------------
    ~RenderInfo() {}
    // ------------------------------------------------------------------------
    void setColorizableParts(irr::scene::IMesh* m);
    // ------------------------------------------------------------------------
    void setHue(float hue)                                    { m_hue = hue; }
    // ------------------------------------------------------------------------
    void setTransparent(bool transparent)     { m_transparent = transparent; }
    // ------------------------------------------------------------------------
    float getHue() const                                     { return m_hue; }
    // ------------------------------------------------------------------------
    bool isTransparent() const                       { return m_transparent; }
    // ------------------------------------------------------------------------
    bool isColorizable(int mesh_buffer_index) const
    {
        return m_colorizable_parts.empty() ||
            std::find(m_colorizable_parts.begin(), m_colorizable_parts.end(),
            mesh_buffer_index) != m_colorizable_parts.end();
    }
    // ------------------------------------------------------------------------
    void setKartModelRenderInfo(KartRenderType krt)
    {
        setHue(krt == RenderInfo::KRT_BLUE ? 0.66f :
            krt == RenderInfo::KRT_RED ? 1.0f : 0.0f);

        setTransparent(krt == RenderInfo::KRT_TRANSPARENT ? true : false);
    }
    // ------------------------------------------------------------------------
    void setRenderInfo(const RenderInfo* other)            { *this = *other; }

};   // RenderInfo

#endif

/* EOF */
