//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef HEADER_RTTS_HPP
#define HEADER_RTTS_HPP

#include "utils/leak_check.hpp"
#include <cassert>

class FrameBuffer;
class FrameBufferLayer;

enum TypeFBO
{
    FBO_COLORS,
    FBO_NORMAL_AND_DEPTHS,
    FBO_SP,
    FBO_RGBA_1,
    FBO_RGBA_2,
    FBO_COMBINED_DIFFUSE_SPECULAR,
    FBO_TMP1_WITH_DS,
    FBO_HALF1_R,
    FBO_HALF1,
    FBO_HALF2,

    FBO_RGBA_3, // MLAA

    FBO_SSAO, // SSAO
    FBO_LINEAR_DEPTH, // SSAO
    FBO_HALF2_R, // SSAO

    FBO_QUARTER1, // Glow || God Ray
    FBO_QUARTER2, // God Ray

    FBO_BLOOM_1024, // The reset is for bloom only (may be optimized layer)
    FBO_BLOOM_512,
    FBO_TMP_512,
    FBO_LENS_512,

    FBO_BLOOM_256,
    FBO_TMP_256,
    FBO_LENS_256,

    FBO_BLOOM_128,
    FBO_TMP_128,
    FBO_LENS_128,
    FBO_COUNT
};

enum TypeRTT : unsigned int
{
    RTT_COLOR = 0,
    RTT_NORMAL_AND_DEPTH,
    RTT_SP_DIFF_COLOR, // RGBA
    RTT_RGBA_2,
    RTT_DIFFUSE,
    RTT_SPECULAR,
    RTT_TMP1,
    RTT_HALF1,
    RTT_HALF1_R,
    RTT_HALF2,

    RTT_RGBA_3,

    RTT_SSAO,
    RTT_LINEAR_DEPTH,
    RTT_HALF2_R,

    RTT_QUARTER1,
    RTT_QUARTER2,

    RTT_BLOOM_1024,
    RTT_BLOOM_512,
    RTT_TMP_512,
    RTT_LENS_512,
    RTT_BLOOM_256,
    RTT_TMP_256,
    RTT_LENS_256,
    RTT_BLOOM_128,
    RTT_TMP_128,
    RTT_LENS_128,

    RTT_COUNT
};

class RTT
{
public:
    RTT(unsigned int width, unsigned int height, float rtt_scale = 1.0f,
        bool use_default_fbo_only = false);
    ~RTT();

    unsigned int getWidth () const { return m_width ; }
    unsigned int getHeight() const { return m_height; }

    FrameBufferLayer* getShadowFrameBuffer() { return m_shadow_fbo; }
    unsigned getDepthStencilTexture() const
    {
        assert(m_depth_stencil_tex != 0);
        return m_depth_stencil_tex;
    }
    unsigned getRenderTarget(enum TypeRTT target) const
    {
        assert(m_render_target_textures[target] != 0);
        return m_render_target_textures[target];
    }
    FrameBuffer& getFBO(enum TypeFBO fbo)
    {
        assert(m_frame_buffers[fbo] != NULL);
        return *m_frame_buffers[fbo];
    }

private:
    unsigned m_render_target_textures[RTT_COUNT] = {};
    FrameBuffer* m_frame_buffers[FBO_COUNT] = {};
    unsigned m_depth_stencil_tex = 0;

    unsigned int m_width;
    unsigned int m_height;

    unsigned m_shadow_depth_tex = 0;
    FrameBufferLayer* m_shadow_fbo;

    LEAK_CHECK();
};

#endif

