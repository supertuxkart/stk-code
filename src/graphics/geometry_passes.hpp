//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 SuperTuxKart-Team
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

#ifndef HEADER_GEOMETRY_PASSES_HPP
#define HEADER_GEOMETRY_PASSES_HPP

#include "graphics/glwrap.hpp"
#include "graphics/shadow_matrices.hpp"
#include <ITexture.h>

class GeometryPasses
{
private:

    irr::core::vector3df m_wind_dir;
    irr::video::ITexture *m_displace_tex;


public:
    GeometryPasses();
    
    void renderSolidFirstPass();
    void renderSolidSecondPass( unsigned render_target_diffuse,
                                unsigned render_target_specular,
                                unsigned render_target_half_red);
    void renderNormalsVisualisation();
    void renderTransparent(unsigned render_target);
    void renderShadows(const ShadowMatrices& shadow_matrices,
                       const FrameBuffer& shadow_framebuffer);
    void renderReflectiveShadowMap(const ShadowMatrices& shadow_matrices,
                                   const FrameBuffer& reflective_shadow_map_framebuffer);


};

#endif //HEADER_GEOMETRY_PASSES_HPP
