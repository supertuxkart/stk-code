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

#ifndef HEADER_FRAME_BUFFER_LAYER_HPP
#define HEADER_FRAME_BUFFER_LAYER_HPP

#include "graphics/frame_buffer.hpp"

class FrameBufferLayer : public FrameBuffer
{
private:
    std::vector<GLuint> m_fbo_layer;

public:
    LEAK_CHECK()
    // ------------------------------------------------------------------------
    FrameBufferLayer(const std::vector<GLuint> &rtts, unsigned w, unsigned h,
                     unsigned layer_count);
    // ------------------------------------------------------------------------
    FrameBufferLayer(const std::vector<GLuint> &rtts, GLuint depth_stencil,
                     unsigned w, unsigned h, unsigned layer_count);
    // ------------------------------------------------------------------------
    ~FrameBufferLayer()
    {
        if (!m_fbo_layer.empty())
        {
            glDeleteFramebuffers((int)m_fbo_layer.size(), m_fbo_layer.data());
        }
    }
    // ------------------------------------------------------------------------
    void bindLayer(unsigned i) const
    {
        assert(i < m_fbo_layer.size());
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_layer[i]);
        glViewport(0, 0, (int)m_width, (int)m_height);
        GLenum bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2 };
        glDrawBuffers((int)m_render_targets.size(), bufs);
    }
    // ------------------------------------------------------------------------
    void bindLayerDepthOnly(unsigned i) const
    {
        assert(i < m_fbo_layer.size());
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_layer[i]);
        glViewport(0, 0, (int)m_width, (int)m_height);
        GLenum bufs[] = { GL_NONE, GL_NONE, GL_NONE };
        glDrawBuffers((int)m_render_targets.size(), bufs);
    }
    // ------------------------------------------------------------------------
    unsigned getLayerCount() const     { return (unsigned)m_fbo_layer.size(); }

};

#endif

#endif
