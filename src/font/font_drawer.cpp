//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2020 SuperTuxKart-Team
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

#include "font/font_drawer.hpp"
#include "graphics/2dutils.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/texture_shader.hpp"

#include <IVideoDriver.h>
#include <S3DVertex.h>
#include <SMaterial.h>
#include <irrArray.h>
#include <map>
#include <memory>

// ============================================================================
class FontDrawerShader : public TextureShader<FontDrawerShader, 1, core::vector2df>
{
public:
    GLuint m_vao, m_vbo, m_ibo;
    FontDrawerShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "primitive2dlist.vert",
                            GL_FRAGMENT_SHADER, "colortexturedquad.frag");
        assignUniforms("fullscreen");
        assignSamplerNames(0, "tex", ST_BILINEAR_FILTERED);
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ibo);
    }   // FontDrawerShader
    ~FontDrawerShader()
    {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ibo);
    }
};   //FontDrawerShader

// ============================================================================
std::unique_ptr<core::rect<s32> > g_clip;
// ============================================================================
std::map<video::ITexture*, std::vector<uint8_t> > g_glyphs;
// ============================================================================
bool g_batching = false;
// ----------------------------------------------------------------------------
void FontDrawer::startBatching()
{
    g_batching = true;
}   // startBatching

// ----------------------------------------------------------------------------
bool FontDrawer::isBatching()
{
    return g_batching;
}   // isBatching

// ----------------------------------------------------------------------------
void FontDrawer::endBatching()
{
    g_batching = false;
    draw();
}   // endBatching

// ----------------------------------------------------------------------------
void FontDrawer::addGlyph(video::ITexture* texture,
                          const core::rect<float>& dest_rect,
                          const core::rect<s32>& source_rect,
                          const core::rect<s32>* clip_rect,
                          const video::SColor* color)
{
    if (clip_rect)
        g_clip.reset(new core::rect<s32>(*clip_rect));
    else
        g_clip.reset();
    const float tex_h = (float)texture->getSize().Height;
    const float tex_w = (float)texture->getSize().Width;

    if (g_glyphs.find(texture) == g_glyphs.end())
        texture->grab();

    auto& glyph_data = g_glyphs[texture];
    size_t stride = sizeof(video::S3DVertex);
    if (CVS->isGLSL())
        stride -= 16;
    size_t old_size = glyph_data.size();
    glyph_data.resize(old_size + stride * 4);
    uint8_t* glyph_ptr = &glyph_data[old_size];

    auto copy_glyph = [stride](const video::S3DVertex& v, uint8_t* glyph_ptr)
    {
        if (CVS->isGLSL())
        {
            size_t pos_size = sizeof(float) * 2;
            memcpy(glyph_ptr, &v.Pos.X, pos_size);
            memcpy(glyph_ptr + pos_size, &v.Color, stride - pos_size);
        }
        else
            memcpy(glyph_ptr, &v.Pos.X, stride);
    };

    video::S3DVertex glyph = video::S3DVertex(
        dest_rect.UpperLeftCorner.X, dest_rect.LowerRightCorner.Y, 0, 0, 0, 0,
        color[1], source_rect.UpperLeftCorner.X / tex_w,
        source_rect.LowerRightCorner.Y / tex_h);
    copy_glyph(glyph, glyph_ptr);
    glyph = video::S3DVertex(
        dest_rect.UpperLeftCorner.X, dest_rect.UpperLeftCorner.Y, 0, 0, 0, 0,
        color[0], source_rect.UpperLeftCorner.X / tex_w,
        source_rect.UpperLeftCorner.Y / tex_h);
    copy_glyph(glyph, glyph_ptr + stride);
    glyph = video::S3DVertex(
        dest_rect.LowerRightCorner.X, dest_rect.UpperLeftCorner.Y, 0, 0, 0, 0,
        color[2], source_rect.LowerRightCorner.X / tex_w,
        source_rect.UpperLeftCorner.Y / tex_h);
    copy_glyph(glyph, glyph_ptr + stride * 2);
    glyph = video::S3DVertex(
        dest_rect.LowerRightCorner.X, dest_rect.LowerRightCorner.Y, 0, 0, 0, 0,
        color[3], source_rect.LowerRightCorner.X / tex_w,
        source_rect.LowerRightCorner.Y / tex_h);
    copy_glyph(glyph, glyph_ptr + stride * 3);
}   // addGlyph

// ----------------------------------------------------------------------------
void FontDrawer::draw()
{
    if (g_batching || g_glyphs.empty())
        return;

    if (g_clip && !g_clip->isValid())
    {
        for (auto it = g_glyphs.begin(); it != g_glyphs.end();)
        {
            it->first->drop();
            it = g_glyphs.erase(it);
        }
        return;
    }

    if (CVS->isGLSL())
    {
        FontDrawerShader::getInstance()->use();
        FontDrawerShader::getInstance()->setUniforms(
            core::vector2df(float(irr_driver->getActualScreenSize().Width),
            float(irr_driver->getActualScreenSize().Height)));
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (g_clip)
        irr_driver->getVideoDriver()->enableScissorTest(*g_clip);
    else
        irr_driver->getVideoDriver()->disableScissorTest();

    for (auto& glyph : g_glyphs)
    {
        std::vector<uint16_t> indices;
        std::vector<uint32_t> indices_32;
        u32 idx = 0;
        size_t stride = sizeof(video::S3DVertex);
        if (CVS->isGLSL())
            stride -= 16;
        for (unsigned i = 0; i < glyph.second.size() / stride; i += 4)
        {
            if (idx >= 65536)
            {
                if (indices_32.empty())
                {
                    for (auto& index : indices)
                        indices_32.push_back(index);
                }
                indices_32.push_back(idx++);
                indices_32.push_back(idx++);
                indices_32.push_back(idx++);
                indices_32.push_back(indices_32[indices_32.size() - 3]);
                indices_32.push_back(indices_32[indices_32.size() - 2]);
                indices_32.push_back(idx++);
            }
            else
            {
                indices.push_back(idx++);
                indices.push_back(idx++);
                indices.push_back(idx++);
                indices.push_back(indices[indices.size() - 3]);
                indices.push_back(indices[indices.size() - 2]);
                indices.push_back(idx++);
            }
        }
        void* idx_data = indices_32.empty() ?
            (void*)indices.data() : (void*)indices_32.data();
        size_t idx_count = indices_32.empty() ? indices.size() : indices_32.size();
        size_t idx_size = indices_32.empty() ?
            idx_count * sizeof(u16) : idx_count * sizeof(u32);
        if (CVS->isGLSL())
        {
            // Unfortunately we need to re-define the vao each time otherwise
            // if texture is changed no glyph is drawn
            glBindVertexArray(FontDrawerShader::getInstance()->m_vao);
            glBindBuffer(GL_ARRAY_BUFFER,
                FontDrawerShader::getInstance()->m_vbo);
            glBufferData(GL_ARRAY_BUFFER,
                glyph.second.size(), glyph.second.data(), GL_STREAM_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                FontDrawerShader::getInstance()->m_ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx_size, idx_data,
                GL_STREAM_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, 0);
            glDisableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (GLvoid*)8);
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)12);
            FontDrawerShader::getInstance()->setTextureUnits(
                glyph.first->getTextureHandler());
            glDrawElements(GL_TRIANGLES, idx_count,
                indices_32.empty() ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        else
        {
            video::SMaterial m;
            m.setTexture(0, glyph.first);
            m.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
            irr_driver->getVideoDriver()->setMaterial(m);
            irr_driver->getVideoDriver()->draw2DVertexPrimitiveList(
                glyph.second.data(), glyph.second.size() / stride,
                idx_data, idx_count / 3, video::EVT_STANDARD,
                scene::EPT_TRIANGLES,
                indices_32.empty() ? video::EIT_16BIT : video::EIT_32BIT);
        }
    }

    if (g_clip)
    {
        irr_driver->getVideoDriver()->disableScissorTest();
        g_clip.reset();
    }

    if (CVS->isGLSL())
    {
        glUseProgram(0);
    }

    for (auto it = g_glyphs.begin(); it != g_glyphs.end();)
    {
        it->first->drop();
        it = g_glyphs.erase(it);
    }
}   // draw

#endif
