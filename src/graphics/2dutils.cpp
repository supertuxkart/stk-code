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

#ifndef SERVER_ONLY
#include "graphics/2dutils.hpp"

#include "graphics/central_settings.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/shader.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "graphics/texture_shader.hpp"
#include "utils/cpp2011.hpp"

#include <IVideoDriver.h>
#include <cmath>

// ============================================================================
class Primitive2DList : public TextureShader<Primitive2DList, 1, core::vector2df>
{
public:
    Primitive2DList()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "primitive2dlist.vert",
                            GL_FRAGMENT_SHADER, "transparent.frag");
        assignUniforms("fullscreen");
        assignSamplerNames(0, "tex", ST_BILINEAR_FILTERED);
    }   // Primitive2DList
};   //Primitive2DList

// ============================================================================
class UniformColoredTextureRectShader : public TextureShader<UniformColoredTextureRectShader,1,
                                                      core::vector2df, core::vector2df,
                                                      core::vector2df, core::vector2df,
                                                      video::SColor>
{
public:
    UniformColoredTextureRectShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "texturedquad.vert",
                    GL_FRAGMENT_SHADER, "uniformcolortexturedquad.frag");

        assignUniforms("center", "size", "texcenter", "texsize", "color");

        assignSamplerNames(0, "tex", ST_BILINEAR_FILTERED);
    }   // UniformColoredTextureRectShader
};   // UniformColoredTextureRectShader

// ============================================================================
class TextureRectShader : public TextureShader<TextureRectShader, 1,
                                             core::vector2df, core::vector2df,
                                             core::vector2df, core::vector2df>
{
public:
    TextureRectShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "texturedquad.vert",
                            GL_FRAGMENT_SHADER, "texturedquad.frag");
        assignUniforms("center", "size", "texcenter", "texsize");

        assignSamplerNames(0, "tex", ST_BILINEAR_CLAMPED_FILTERED);
    }   // TextureRectShader
};   // TextureRectShader

// ============================================================================
class ColoredRectShader : public Shader<ColoredRectShader, core::vector2df,
                                        core::vector2df, video::SColor>
{
public:
    ColoredRectShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "coloredquad.vert",
                            GL_FRAGMENT_SHADER, "coloredquad.frag");
        assignUniforms("center", "size", "color");
    }   // ColoredRectShader
};   // ColoredRectShader

// ============================================================================

class ColoredTextureRectShader : public TextureShader<ColoredTextureRectShader, 1,
                                               core::vector2df, core::vector2df,
                                               core::vector2df, core::vector2df>
{
public:
    GLuint m_color_vbo;
    GLuint m_vao;

    ColoredTextureRectShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "colortexturedquad.vert",
                            GL_FRAGMENT_SHADER, "colortexturedquad.frag");
        assignUniforms("center", "size", "texcenter", "texsize");

        assignSamplerNames(0, "tex", ST_BILINEAR_FILTERED);

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, SharedGPUObjects::getQuadBuffer());
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 
                              (GLvoid *)(2 * sizeof(float)));
        glBindVertexArray(0);
        const uint8_t quad_color[] = {   0,   0,   0, 255,
                                        255,   0,   0, 255,
                                          0, 255,   0, 255,
                                          0,   0, 255, 255 };
        glGenBuffers(1, &m_color_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_color_vbo);
        glBufferData(GL_ARRAY_BUFFER, 16, quad_color, GL_DYNAMIC_DRAW);

    }   // ColoredTextureRectShader
};   // ColoredTextureRectShader

// ============================================================================
static void drawTexColoredQuad(const video::ITexture *texture,
                               const video::SColor *col, float width,
                               float height, float center_pos_x,
                               float center_pos_y, float tex_center_pos_x,
                               float tex_center_pos_y, float tex_width,
                               float tex_height)
{
    glBindVertexArray(ColoredTextureRectShader::getInstance()->m_vao);
    glBindBuffer(GL_ARRAY_BUFFER,
                 ColoredTextureRectShader::getInstance()->m_color_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 16, col);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4 , 0);

    ColoredTextureRectShader::getInstance()->use();
    glBindVertexArray(ColoredTextureRectShader::getInstance()->m_vao);
    ColoredTextureRectShader::getInstance()
        ->setTextureUnits(texture->getTextureHandler());
    ColoredTextureRectShader::getInstance()
        ->setUniforms(core::vector2df(center_pos_x, center_pos_y),
                      core::vector2df(width, height),
                      core::vector2df(tex_center_pos_x, tex_center_pos_y),
                      core::vector2df(tex_width, tex_height));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    glGetError();
}   // drawTexColoredQuad

// ----------------------------------------------------------------------------
static void drawTexQuad(GLuint texture, float width, float height,
                        float center_pos_x, float center_pos_y, 
                        float tex_center_pos_x, float tex_center_pos_y,
                        float tex_width, float tex_height)
{
    TextureRectShader::getInstance()->use();
    glBindVertexArray(SharedGPUObjects::getUI_VAO());

    TextureRectShader::getInstance()->setTextureUnits(texture);
    TextureRectShader::getInstance()->setUniforms(
                    core::vector2df(center_pos_x, center_pos_y), 
                    core::vector2df(width, height),
                    core::vector2df(tex_center_pos_x, tex_center_pos_y),
                    core::vector2df(tex_width, tex_height));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGetError();
}   // drawTexQuad


// ----------------------------------------------------------------------------
static void getSize(unsigned texture_width, unsigned texture_height,
                    bool textureisRTT, const core::rect<s32>& destRect,
                    const core::rect<s32>& sourceRect,
                    float &width, float &height,
                    float &center_pos_x, float &center_pos_y,
                    float &tex_width, float &tex_height,
                    float &tex_center_pos_x, float &tex_center_pos_y   )
{
    core::dimension2d<u32> frame_size = irr_driver->getActualScreenSize();
    const int screen_w = frame_size.Width;
    const int screen_h =  frame_size.Height;
    center_pos_x = float(destRect.UpperLeftCorner.X + destRect.LowerRightCorner.X);
    center_pos_x /= screen_w;
    center_pos_x -= 1.;
    center_pos_y = float(destRect.UpperLeftCorner.Y + destRect.LowerRightCorner.Y);
    center_pos_y /= screen_h;
    center_pos_y = float(1.f - center_pos_y);
    width = float(destRect.LowerRightCorner.X - destRect.UpperLeftCorner.X);
    width /= screen_w;
    height = float(destRect.LowerRightCorner.Y - destRect.UpperLeftCorner.Y);
    height /= screen_h;

    tex_center_pos_x = float(sourceRect.UpperLeftCorner.X + sourceRect.LowerRightCorner.X);
    tex_center_pos_x /= texture_width * 2.f;
    tex_center_pos_y = float(sourceRect.UpperLeftCorner.Y + sourceRect.LowerRightCorner.Y);
    tex_center_pos_y /= texture_height * 2.f;
    tex_width = float(sourceRect.LowerRightCorner.X - sourceRect.UpperLeftCorner.X);
    tex_width /= texture_width * 2.f;
    tex_height = float(sourceRect.LowerRightCorner.Y - sourceRect.UpperLeftCorner.Y);
    tex_height /= texture_height * 2.f;

    if (textureisRTT)
        tex_height = -tex_height;

    const f32 invW = 1.f / static_cast<f32>(texture_width);
    const f32 invH = 1.f / static_cast<f32>(texture_height);
    const core::rect<f32> tcoords(sourceRect.UpperLeftCorner.X  * invW,
                                  sourceRect.UpperLeftCorner.Y  * invH,
                                  sourceRect.LowerRightCorner.X * invW,
                                  sourceRect.LowerRightCorner.Y * invH);
}   // getSize

// ----------------------------------------------------------------------------
void draw2DImage(const video::ITexture* texture,
                 const core::rect<s32>& destRect,
                 const core::rect<s32>& sourceRect,
                 const core::rect<s32>* clip_rect,
                 const video::SColor &colors, bool use_alpha_channel_of_texture)
{
    if (!CVS->isGLSL())
    {
        video::SColor duplicatedArray[4] = { colors, colors, colors, colors };
        draw2DImage(texture, destRect, sourceRect, clip_rect, duplicatedArray,
                    use_alpha_channel_of_texture);
        return;
    }

    float width, height, center_pos_x, center_pos_y;
    float tex_width, tex_height, tex_center_pos_x, tex_center_pos_y;

    getSize(texture->getSize().Width, texture->getSize().Height,
            texture->isRenderTarget(), destRect, sourceRect, width, height,
            center_pos_x, center_pos_y, tex_width, tex_height,
            tex_center_pos_x, tex_center_pos_y);

    if (use_alpha_channel_of_texture)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    if (clip_rect)
    {
        if (!clip_rect->isValid())
            return;

        glEnable(GL_SCISSOR_TEST);
        const core::dimension2d<u32>& render_target_size =
                           irr_driver->getActualScreenSize();
        glScissor(clip_rect->UpperLeftCorner.X,
                  (s32)render_target_size.Height - clip_rect->LowerRightCorner.Y,
                  clip_rect->getWidth(), clip_rect->getHeight());
    }

    UniformColoredTextureRectShader::getInstance()->use();
    glBindVertexArray(SharedGPUObjects::getUI_VAO());
    UniformColoredTextureRectShader::getInstance()
        ->setTextureUnits(texture->getTextureHandler());

    UniformColoredTextureRectShader::getInstance()
        ->setUniforms(core::vector2df(center_pos_x, center_pos_y),
                      core::vector2df(width, height),
                      core::vector2df(tex_center_pos_x, tex_center_pos_y),
                      core::vector2df(tex_width, tex_height),
                      colors);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (clip_rect)
        glDisable(GL_SCISSOR_TEST);
    glUseProgram(0);

    glGetError();
}   // draw2DImage

// ----------------------------------------------------------------------------
void draw2DImageFromRTT(GLuint texture, size_t texture_w, size_t texture_h,
                        const core::rect<s32>& destRect,
                        const core::rect<s32>& sourceRect,
                        const core::rect<s32>* clip_rect,
                        const video::SColor &colors,
                        bool use_alpha_channel_of_texture)
{
    if (use_alpha_channel_of_texture)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    float width, height,
        center_pos_x, center_pos_y,
        tex_width, tex_height,
        tex_center_pos_x, tex_center_pos_y;

    getSize((int)texture_w, (int)texture_h, true,
        destRect, sourceRect, width, height, center_pos_x, center_pos_y,
        tex_width, tex_height, tex_center_pos_x, tex_center_pos_y);

    UniformColoredTextureRectShader::getInstance()->use();
    glBindVertexArray(SharedGPUObjects::getUI_VAO());

    UniformColoredTextureRectShader::getInstance()->setTextureUnits(texture);
    UniformColoredTextureRectShader::getInstance()
        ->setUniforms(core::vector2df(center_pos_x, center_pos_y), 
                      core::vector2df(width, height),
                      core::vector2df(tex_center_pos_x, tex_center_pos_y),
                      core::vector2df(tex_width, tex_height), colors        );

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}   // draw2DImageFromRTT

// ----------------------------------------------------------------------------
void draw2DImage(const video::ITexture* texture,
                 const core::rect<s32>& destRect,
                 const core::rect<s32>& sourceRect,
                 const core::rect<s32>* clip_rect,
                 const video::SColor* const colors,
                 bool use_alpha_channel_of_texture)
{
    if (!CVS->isGLSL())
    {
        irr_driver->getVideoDriver()->draw2DImage(texture, destRect, sourceRect,
            clip_rect, colors, use_alpha_channel_of_texture);
        return;
    }

    float width, height, center_pos_x, center_pos_y, tex_width, tex_height;
    float tex_center_pos_x, tex_center_pos_y;

    getSize(texture->getSize().Width, texture->getSize().Height,
            texture->isRenderTarget(), destRect, sourceRect, width, height, 
            center_pos_x, center_pos_y, tex_width, tex_height,
            tex_center_pos_x, tex_center_pos_y);

    if (use_alpha_channel_of_texture)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    if (clip_rect)
    {
        if (!clip_rect->isValid())
            return;

        glEnable(GL_SCISSOR_TEST);
        const core::dimension2d<u32>& render_target_size =
                            irr_driver->getActualScreenSize();
        glScissor(clip_rect->UpperLeftCorner.X,
                  (s32)render_target_size.Height - clip_rect->LowerRightCorner.Y,
                  clip_rect->getWidth(), clip_rect->getHeight());
    }
    if (colors)
    {
        drawTexColoredQuad(texture, colors, width, height, center_pos_x,
                           center_pos_y, tex_center_pos_x, tex_center_pos_y,
                           tex_width, tex_height);
    }
    else
    {
        drawTexQuad(texture->getTextureHandler(), width, height,
                    center_pos_x, center_pos_y, tex_center_pos_x,
                    tex_center_pos_y, tex_width, tex_height);
    }
    if (clip_rect)
        glDisable(GL_SCISSOR_TEST);
    glUseProgram(0);

    glGetError();
}   // draw2DImage

// ----------------------------------------------------------------------------
void draw2DImageRotationColor(video::ITexture* texture,
                              const irr::core::rect<irr::s32>& destRect,
                              const irr::core::rect<irr::s32>& sourceRect,
                              const irr::core::rect<irr::s32>* clipRect,
                              float rotation, const irr::video::SColor& color)
{
    const core::dimension2d<u32>& ss = texture->getSize();
    core::rect<f32> tcoords;
    tcoords.UpperLeftCorner.X = (f32)sourceRect.UpperLeftCorner.X / (f32)ss.Width;
    tcoords.UpperLeftCorner.Y = (f32)sourceRect.UpperLeftCorner.Y / (f32)ss.Height;
    tcoords.LowerRightCorner.X = (f32)sourceRect.LowerRightCorner.X / (f32)ss.Width;
    tcoords.LowerRightCorner.Y = (f32)sourceRect.LowerRightCorner.Y / (f32)ss.Height;

    video::S3DVertex vtx[4];
    vtx[0] = video::S3DVertex((f32)destRect.UpperLeftCorner.X, (f32)destRect.UpperLeftCorner.Y, 0.0f,
        0.0f, 0.0f, 0.0f, color,
        tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y);
    vtx[1] = video::S3DVertex((f32)destRect.LowerRightCorner.X, (f32)destRect.UpperLeftCorner.Y, 0.0f,
        0.0f, 0.0f, 0.0f, color,
        tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y);
    vtx[2] = video::S3DVertex((f32)destRect.LowerRightCorner.X, (f32)destRect.LowerRightCorner.Y, 0.0f,
        0.0f, 0.0f, 0.0f, color,
        tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y);
    vtx[3] = video::S3DVertex((f32)destRect.UpperLeftCorner.X, (f32)destRect.LowerRightCorner.Y, 0.0f,
        0.0f, 0.0f, 0.0f, color,
        tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y);

    core::vector2df center(
        (vtx[0].Pos.X + vtx[1].Pos.X + vtx[2].Pos.X + vtx[3].Pos.X) / 4.0f,
        (vtx[0].Pos.Y + vtx[1].Pos.Y + vtx[2].Pos.Y + vtx[3].Pos.Y) / 4.0f);

    float cos_rotation = cos(rotation);
    float sin_rotation = sin(rotation);
    for (int i = 0; i < 4; i++)
    {
        core::vector2df orig_pos(vtx[i].Pos.X, vtx[i].Pos.Y);
        vtx[i].Pos.X = center.X + (orig_pos.X - center.X) * cos_rotation - (orig_pos.Y - center.Y) * sin_rotation;
        vtx[i].Pos.Y = center.Y + (orig_pos.X - center.X) * sin_rotation + (orig_pos.Y - center.Y) * cos_rotation;
    }

    u16 indices[6] = { 0, 1, 2, 0, 2, 3 };
    if (!CVS->isGLSL())
    {
        video::SMaterial m;
        m.setTexture(0, texture);
        if (irr_driver->getVideoDriver()->getDriverType() == video::EDT_OGLES2)
        {
            m.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
        }
        else
        {
            // For vertex color
            m.MaterialType = video::EMT_ONETEXTURE_BLEND;
            m.MaterialTypeParam =
                pack_textureBlendFunc(video::EBF_SRC_ALPHA,
                video::EBF_ONE_MINUS_SRC_ALPHA,
                video::EMFN_MODULATE_1X,
                video::EAS_TEXTURE | video::EAS_VERTEX_COLOR);
        }
        irr_driver->getVideoDriver()->setMaterial(m);
    }
    else
    {
        indices[0] = 2;
        indices[2] = 0;
        indices[3] = 3;
        indices[5] = 0;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    draw2DVertexPrimitiveList(texture, vtx, 4, indices, 2, video::EVT_STANDARD,
        scene::EPT_TRIANGLES, video::EIT_16BIT);
}   // draw2DImageRotationColor

// ----------------------------------------------------------------------------
void draw2DVertexPrimitiveList(video::ITexture *tex, const void* vertices,
                               u32 vertexCount, const void* indexList, 
                               u32 primitiveCount, video::E_VERTEX_TYPE vType,
                               scene::E_PRIMITIVE_TYPE pType,
                               video::E_INDEX_TYPE iType)
{
    if (!CVS->isGLSL())
    {
        irr_driver->getVideoDriver()
            ->draw2DVertexPrimitiveList(vertices, vertexCount, indexList,
                                        primitiveCount, vType, pType, iType);
        return;
    }

    GLuint tmpvao, tmpvbo, tmpibo;
    primitiveCount += 2;
    glGenVertexArrays(1, &tmpvao);
    glBindVertexArray(tmpvao);
    glGenBuffers(1, &tmpvbo);
    glBindBuffer(GL_ARRAY_BUFFER, tmpvbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * getVertexPitchFromType(vType),
                 vertices, GL_STREAM_DRAW);
    glGenBuffers(1, &tmpibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tmpibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, primitiveCount * sizeof(u16),
                 indexList, GL_STREAM_DRAW);

    VertexUtils::bindVertexArrayAttrib(vType);

    Primitive2DList::getInstance()->use();
    Primitive2DList::getInstance()->setUniforms(
        core::vector2df(float(irr_driver->getActualScreenSize().Width),
        float(irr_driver->getActualScreenSize().Height)));
    Primitive2DList::getInstance()->setTextureUnits(tex->getTextureHandler());
    glDrawElements(GL_TRIANGLE_FAN, primitiveCount, GL_UNSIGNED_SHORT, 0);

    glDeleteVertexArrays(1, &tmpvao);
    glDeleteBuffers(1, &tmpvbo);
    glDeleteBuffers(1, &tmpibo);

}   // draw2DVertexPrimitiveList

// ----------------------------------------------------------------------------
void GL32_draw2DRectangle(video::SColor color, const core::rect<s32>& position,
                          const core::rect<s32>* clip)
{

    if (!CVS->isGLSL())
    {
        irr_driver->getVideoDriver()->draw2DRectangle(color, position, clip);
        return;
    }

    core::dimension2d<u32> frame_size = irr_driver->getActualScreenSize();
    const int screen_w = frame_size.Width;
    const int screen_h = frame_size.Height;
    float center_pos_x = float(position.UpperLeftCorner.X + position.LowerRightCorner.X);
    center_pos_x /= screen_w;
    center_pos_x -= 1;
    float center_pos_y = float(position.UpperLeftCorner.Y + position.LowerRightCorner.Y);
    center_pos_y /= screen_h;
    center_pos_y = 1 - center_pos_y;
    float width = float(position.LowerRightCorner.X - position.UpperLeftCorner.X);
    width /= screen_w;
    float height = float(position.LowerRightCorner.Y - position.UpperLeftCorner.Y);
    height /= screen_h;

    if (color.getAlpha() < 255)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    if (clip)
    {
        if (!clip->isValid())
            return;

        glEnable(GL_SCISSOR_TEST);
        const core::dimension2d<u32>& render_target_size = 
                                            irr_driver->getActualScreenSize();
        glScissor(clip->UpperLeftCorner.X,
                  (s32)render_target_size.Height - clip->LowerRightCorner.Y,
                  clip->getWidth(), clip->getHeight());
    }

    ColoredRectShader::getInstance()->use();
    glBindVertexArray(SharedGPUObjects::getUI_VAO());
    ColoredRectShader::getInstance()
        ->setUniforms(core::vector2df(center_pos_x, center_pos_y),
                      core::vector2df(width, height), color        );

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    if (clip)
        glDisable(GL_SCISSOR_TEST);
    glUseProgram(0);

    glGetError();
}   // GL32_draw2DRectangle

void preloadShaders()
{
    Primitive2DList::getInstance();
    UniformColoredTextureRectShader::getInstance();
    TextureRectShader::getInstance();
    ColoredRectShader::getInstance();
    ColoredTextureRectShader::getInstance();
}   // preloadShaders

#endif   // !SERVER_ONLY

