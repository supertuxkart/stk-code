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

#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"

#include <ISceneManager.h>
#include <algorithm>
#include <vector>

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

    float m_min_saturation;

    bool m_transparent;

    std::vector<int> m_colorizable_parts;

public:
    RenderInfo(float hue = 0.0f, float min_saturation = 0.0f,
               bool transparent = false)
    {
        m_hue = hue;
        m_min_saturation = min_saturation;
        m_transparent = transparent;
    }
    // ------------------------------------------------------------------------
    void setColorizableParts(irr::scene::IMesh* m)
    {
        for (int i = 0; i < int(m->getMeshBufferCount()); i++)
        {
            scene::IMeshBuffer* mb = m->getMeshBuffer(i);
            Material* material = material_manager->getMaterialFor(mb
                ->getMaterial().getTexture(0), mb);
            if (material->isColorizable())
                m_colorizable_parts.push_back(i);
        }
    }
    // ------------------------------------------------------------------------
    void setHue(float hue)                                    { m_hue = hue; }
    // ------------------------------------------------------------------------
    void setMinSaturation(float min_saturation)
                                        { m_min_saturation = min_saturation; }
    // ------------------------------------------------------------------------
    void setTransparent(bool transparent)     { m_transparent = transparent; }
    // ------------------------------------------------------------------------
    float getHue() const                                     { return m_hue; }
    // ------------------------------------------------------------------------
    float getMinSaturation() const                { return m_min_saturation; }
    // ------------------------------------------------------------------------
    bool isTransparent() const                       { return m_transparent; }
    // ------------------------------------------------------------------------
    bool isColorizable(int mesh_buffer_index) const
    {
        return m_colorizable_parts.empty() ||
            std::find(m_colorizable_parts.begin(), m_colorizable_parts.end(),
            mesh_buffer_index) != m_colorizable_parts.end();
    }

};   // RenderInfo

#endif

/* EOF */
