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

#ifndef HEADER_FRAME_BUFFER_HPP
#define HEADER_FRAME_BUFFER_HPP

#include "graphics/gl_headers.hpp"
#include "graphics/irr_driver.hpp"
#include "utils/log.hpp"
#include "utils/leak_check.hpp"
#include "utils/no_copy.hpp"

#include <cassert>
#include <vector>

class FrameBuffer : public NoCopy
{
protected:
    GLuint m_fbo = 0;

    std::vector<GLuint> m_render_targets;

    GLuint m_depth_texture = 0;

    unsigned m_width = 0;

    unsigned m_height = 0;

public:
    LEAK_CHECK()
    // ------------------------------------------------------------------------
    FrameBuffer() {}
    // ------------------------------------------------------------------------
    FrameBuffer(const std::vector<GLuint> &rtts, unsigned w, unsigned h)
    {
        m_render_targets = rtts;
        m_width = w;
        m_height = h;

        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        for (unsigned i = 0; i < rtts.size(); i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                GL_TEXTURE_2D, rtts[i], 0);
        }
        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) ==
            GL_FRAMEBUFFER_COMPLETE_EXT);
    }
    // ------------------------------------------------------------------------
    FrameBuffer(const std::vector<GLuint> &rtts, GLuint depth_stencil,
                unsigned w, unsigned h)
    {
        m_render_targets = rtts;
        m_depth_texture = depth_stencil;
        m_width = w;
        m_height = h;

        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        for (unsigned i = 0; i < rtts.size(); i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                GL_TEXTURE_2D, rtts[i], 0);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
            GL_TEXTURE_2D, depth_stencil, 0);
        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) ==
            GL_FRAMEBUFFER_COMPLETE_EXT);
    }
    // ------------------------------------------------------------------------
    ~FrameBuffer()
    {
        if (m_fbo != 0)
        {
            glDeleteFramebuffers(1, &m_fbo);
        }
    }
    // ------------------------------------------------------------------------
    void bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, (int)m_width, (int)m_height);
        GLenum bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2 };
        glDrawBuffers((int)m_render_targets.size(), bufs);
    }
    // ------------------------------------------------------------------------
    void bindDepthOnly() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, (int)m_width, (int)m_height);
        GLenum bufs[] = { GL_NONE, GL_NONE, GL_NONE };
        glDrawBuffers((int)m_render_targets.size(), bufs);
    }
    // ------------------------------------------------------------------------
    const std::vector<GLuint>& getRTT() const      { return m_render_targets; }
    // ------------------------------------------------------------------------
    GLuint getDepthTexture() const
    {
        assert(m_depth_texture != 0);
        return m_depth_texture;
    }
    // ------------------------------------------------------------------------
    unsigned int getWidth() const                           { return m_width; }
    // ------------------------------------------------------------------------
    unsigned int getHeight() const                         { return m_height; }
    // ------------------------------------------------------------------------
    static void blit(const FrameBuffer &src, const FrameBuffer &dst, 
                     GLbitfield mask = GL_COLOR_BUFFER_BIT,
                     GLenum filter = GL_NEAREST)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, src.m_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst.m_fbo);
        glBlitFramebuffer(0, 0, (int)src.m_width, (int)src.m_height, 0, 0,
                          (int)dst.m_width, (int)dst.m_height, mask, filter);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    // ------------------------------------------------------------------------
    void blitToDefault(size_t x0, size_t y0, size_t x1, size_t y1)
    {
        if (m_fbo == 0)
        {
            Log::warn("FrameBuffer", "Don't blit layered framebuffer");
            return;
        }
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, irr_driver->getDefaultFramebuffer());
        glBlitFramebuffer(0, 0, (int)m_width, (int)m_height, (int)x0, (int)y0,
            (int)x1, (int)y1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, irr_driver->getDefaultFramebuffer());
    }

};


#endif

#endif
