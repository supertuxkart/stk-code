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

#include "graphics/render_info.hpp"

#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"

#include <ISceneManager.h>

// ----------------------------------------------------------------------------
RenderInfo::RenderInfo(float hue, bool transparent)
{
    m_static_hue = hue;
    m_transparent = transparent;
}   // RenderInfo

// ----------------------------------------------------------------------------
void RenderInfo::setDynamicHue(irr::scene::IMesh* mesh)
{
    unsigned int n = mesh->getMeshBufferCount();
    for (unsigned int i = 0; i < n; i++)
    {
        scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
        Material* m = material_manager->getMaterialFor(mb
            ->getMaterial().getTexture(0), mb);
        m_dynamic_hue.push_back(m->getRandomHue());
    }
}   // setDynamicHue
