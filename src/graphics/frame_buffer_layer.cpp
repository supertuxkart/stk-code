//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#ifndef SERVER_ONLY
#include "graphics/frame_buffer_layer.hpp"
#include "graphics/central_settings.hpp"

// ----------------------------------------------------------------------------
FrameBufferLayer::FrameBufferLayer(const std::vector<GLuint> &rtts, unsigned w,
                                   unsigned h, unsigned layer_count)
                : FrameBuffer()
{
    m_render_targets = rtts;
    m_width = w;
    m_height = h;

    m_fbo_layer.resize(layer_count);
    glGenFramebuffers(layer_count, m_fbo_layer.data());
    for (unsigned layer = 0; layer < layer_count; layer++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_layer[layer]);
        for (unsigned i = 0; i < rtts.size(); i++)
        {
            glFramebufferTextureLayer(GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0 + i, rtts[i], 0, layer);
        }
        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) ==
            GL_FRAMEBUFFER_COMPLETE_EXT);
    }
}   // FrameBufferLayer

// ----------------------------------------------------------------------------
FrameBufferLayer::FrameBufferLayer(const std::vector<GLuint> &rtts,
                                   GLuint depth_stencil, unsigned w,
                                   unsigned h, unsigned layer_count)
                : FrameBuffer()
{
    m_render_targets = rtts;
    m_depth_texture = depth_stencil;
    m_width = w;
    m_height = h;

    m_fbo_layer.resize(layer_count);
    glGenFramebuffers(layer_count, m_fbo_layer.data());
    for (unsigned layer = 0; layer < layer_count; layer++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_layer[layer]);
        for (unsigned i = 0; i < rtts.size(); i++)
        {
            glFramebufferTextureLayer(GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0 + i, rtts[i], 0, layer);
        }
        glFramebufferTextureLayer(GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT, depth_stencil, 0, layer);
        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) ==
            GL_FRAMEBUFFER_COMPLETE_EXT);
    }
}   // FrameBufferLayer

#endif