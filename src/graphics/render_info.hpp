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
    KRT_BLUE,
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

    std::vector<float> m_dynamic_hue;

public:
    LEAK_CHECK();
    // ------------------------------------------------------------------------
    RenderInfo(float hue = 0.0f, bool transparent = false);
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
        setHue(krt == KRT_BLUE ? 0.66f : krt == KRT_RED ? 1.0f : 0.0f);
        setTransparent(krt == KRT_TRANSPARENT ? true : false);
    }
    // ------------------------------------------------------------------------
    /** Returns true if this render info is static. ie affect all material
      * using the same hue. (like the kart colorization in soccer game)
      */
    bool isStatic() const                    { return m_dynamic_hue.empty(); }
    // ------------------------------------------------------------------------
    unsigned int getNumberOfHue() const 
    {
        return (unsigned int)m_dynamic_hue.size(); 
    }   // getNumberOfHue
    // ------------------------------------------------------------------------
    float getDynamicHue(unsigned int hue) const
    {
        assert(hue < m_dynamic_hue.size());
        return m_dynamic_hue[hue];
    }
    // ------------------------------------------------------------------------
    void setDynamicHue(irr::scene::IMesh* mesh);

};   // RenderInfo

#endif

/* EOF */
