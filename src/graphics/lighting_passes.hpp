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

#ifndef HEADER_LIGHTING_PASSES_HPP
#define HEADER_LIGHTING_PASSES_HPP

#include "graphics/gl_headers.hpp"
#include <vector3d.h>

namespace irr
{
    namespace scene
    {
        class ICameraSceneNode;
    }
    namespace video
    {
        class SColorf;
    }
}

class FrameBuffer;
class PostProcessing;
class ShadowMatrices;

using namespace irr;

class LightingPasses
{
private:
    unsigned m_point_light_count;

    void renderEnvMap(GLuint normal_depth_texture,
                      GLuint depth_stencil_texture,
                      GLuint specular_probe,
                      GLuint albedo_buffer);

    /** Generate diffuse and specular map */
    void         renderSunlight(const core::vector3df &direction,
                                const video::SColorf &col,
                                GLuint normal_depth_texture,
                                GLuint depth_stencil_texture);

public:
    LightingPasses(): m_point_light_count(0){}
    
    void updateLightsInfo(irr::scene::ICameraSceneNode * const camnode,
                          float dt);
    void renderLights(  bool has_shadow,
                        GLuint normal_depth_texture,
                        GLuint depth_stencil_texture,
                        GLuint albedo_texture,
                        const FrameBuffer* shadow_framebuffer,
                        GLuint specular_probe);
    void renderLightsScatter(GLuint depth_stencil_texture,
                             const FrameBuffer& half1_framebuffer,
                             const FrameBuffer& half2_framebuffer,
                             const PostProcessing* post_processing);
};

#endif //HEADER_LIGHTING_PASSES_HPP
