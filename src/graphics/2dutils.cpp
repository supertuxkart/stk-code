#include "2dutils.hpp"
#include "glwrap.hpp"
#include "utils/cpp2011.hpp"

#include "../../lib/irrlicht/source/Irrlicht/COpenGLTexture.h"

static void drawTexColoredQuad(const video::ITexture *texture, const video::SColor *col, float width, float height,
    float center_pos_x, float center_pos_y, float tex_center_pos_x, float tex_center_pos_y,
    float tex_width, float tex_height)
{
    unsigned colors[] = {
        col[0].getRed(), col[0].getGreen(), col[0].getBlue(), col[0].getAlpha(),
        col[1].getRed(), col[1].getGreen(), col[1].getBlue(), col[1].getAlpha(),
        col[2].getRed(), col[2].getGreen(), col[2].getBlue(), col[2].getAlpha(),
        col[3].getRed(), col[3].getGreen(), col[3].getBlue(), col[3].getAlpha(),
    };

    glBindBuffer(GL_ARRAY_BUFFER, UIShader::ColoredTextureRectShader::getInstance()->colorvbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 16 * sizeof(unsigned), colors);

    glUseProgram(UIShader::ColoredTextureRectShader::getInstance()->Program);
    glBindVertexArray(UIShader::ColoredTextureRectShader::getInstance()->vao);

    UIShader::ColoredTextureRectShader::getInstance()->SetTextureUnits(static_cast<const irr::video::COpenGLTexture*>(texture)->getOpenGLTextureName());
    UIShader::ColoredTextureRectShader::getInstance()->setUniforms(
        core::vector2df(center_pos_x, center_pos_y), core::vector2df(width, height),
        core::vector2df(tex_center_pos_x, tex_center_pos_y), core::vector2df(tex_width, tex_height));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGetError();
}

static
void drawTexQuad(GLuint texture, float width, float height,
float center_pos_x, float center_pos_y, float tex_center_pos_x, float tex_center_pos_y,
float tex_width, float tex_height)
{
    glUseProgram(UIShader::TextureRectShader::getInstance()->Program);
    glBindVertexArray(SharedObject::UIVAO);

    UIShader::TextureRectShader::getInstance()->SetTextureUnits(texture);
    UIShader::TextureRectShader::getInstance()->setUniforms(
        core::vector2df(center_pos_x, center_pos_y), core::vector2df(width, height),
        core::vector2df(tex_center_pos_x, tex_center_pos_y),
        core::vector2df(tex_width, tex_height));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGetError();
}

static void
getSize(unsigned texture_width, unsigned texture_height, bool textureisRTT,
const core::rect<s32>& destRect,
const core::rect<s32>& sourceRect,
float &width, float &height,
float &center_pos_x, float &center_pos_y,
float &tex_width, float &tex_height,
float &tex_center_pos_x, float &tex_center_pos_y
)
{
    core::dimension2d<u32> frame_size =
        irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
    const int screen_w = frame_size.Width;
    const int screen_h = frame_size.Height;
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
    const core::rect<f32> tcoords(
        sourceRect.UpperLeftCorner.X * invW,
        sourceRect.UpperLeftCorner.Y * invH,
        sourceRect.LowerRightCorner.X * invW,
        sourceRect.LowerRightCorner.Y *invH);
}

void draw2DImage(const video::ITexture* texture, const core::rect<s32>& destRect,
    const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
    const video::SColor &colors, bool useAlphaChannelOfTexture)
{
    if (!irr_driver->isGLSL()) {
        video::SColor duplicatedArray[4] = {
            colors, colors, colors, colors
        };
        draw2DImage(texture, destRect, sourceRect, clipRect, duplicatedArray, useAlphaChannelOfTexture);
        return;
    }

    float width, height,
        center_pos_x, center_pos_y,
        tex_width, tex_height,
        tex_center_pos_x, tex_center_pos_y;

    getSize(texture->getOriginalSize().Width, texture->getOriginalSize().Height, texture->isRenderTarget(),
        destRect, sourceRect, width, height, center_pos_x, center_pos_y,
        tex_width, tex_height, tex_center_pos_x, tex_center_pos_y);

    if (useAlphaChannelOfTexture)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    if (clipRect)
    {
        if (!clipRect->isValid())
            return;

        glEnable(GL_SCISSOR_TEST);
        const core::dimension2d<u32>& renderTargetSize = irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
        glScissor(clipRect->UpperLeftCorner.X, renderTargetSize.Height - clipRect->LowerRightCorner.Y,
            clipRect->getWidth(), clipRect->getHeight());
    }

    glUseProgram(UIShader::UniformColoredTextureRectShader::getInstance()->Program);
    glBindVertexArray(SharedObject::UIVAO);

    UIShader::UniformColoredTextureRectShader::getInstance()->SetTextureUnits(static_cast<const irr::video::COpenGLTexture*>(texture)->getOpenGLTextureName());
    UIShader::UniformColoredTextureRectShader::getInstance()->setUniforms(
        core::vector2df(center_pos_x, center_pos_y), core::vector2df(width, height), core::vector2df(tex_center_pos_x, tex_center_pos_y), core::vector2df(tex_width, tex_height), colors);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (clipRect)
        glDisable(GL_SCISSOR_TEST);
    glUseProgram(0);

    glGetError();
}

void draw2DImageFromRTT(GLuint texture, size_t texture_w, size_t texture_h,
    const core::rect<s32>& destRect,
    const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
    const video::SColor &colors, bool useAlphaChannelOfTexture)
{
    if (useAlphaChannelOfTexture)
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

    glUseProgram(UIShader::UniformColoredTextureRectShader::getInstance()->Program);
    glBindVertexArray(SharedObject::UIVAO);

    UIShader::UniformColoredTextureRectShader::getInstance()->SetTextureUnits(texture);
    UIShader::UniformColoredTextureRectShader::getInstance()->setUniforms(
        core::vector2df(center_pos_x, center_pos_y), core::vector2df(width, height),
        core::vector2df(tex_center_pos_x, tex_center_pos_y), core::vector2df(tex_width, tex_height),
        colors);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw2DImage(const video::ITexture* texture, const core::rect<s32>& destRect,
    const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
    const video::SColor* const colors, bool useAlphaChannelOfTexture)
{
    if (!irr_driver->isGLSL())
    {
        irr_driver->getVideoDriver()->draw2DImage(texture, destRect, sourceRect, clipRect, colors, useAlphaChannelOfTexture);
        return;
    }

    float width, height,
        center_pos_x, center_pos_y,
        tex_width, tex_height,
        tex_center_pos_x, tex_center_pos_y;

    getSize(texture->getOriginalSize().Width, texture->getOriginalSize().Height, texture->isRenderTarget(),
        destRect, sourceRect, width, height, center_pos_x, center_pos_y,
        tex_width, tex_height, tex_center_pos_x, tex_center_pos_y);

    if (useAlphaChannelOfTexture)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    if (clipRect)
    {
        if (!clipRect->isValid())
            return;

        glEnable(GL_SCISSOR_TEST);
        const core::dimension2d<u32>& renderTargetSize = irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
        glScissor(clipRect->UpperLeftCorner.X, renderTargetSize.Height - clipRect->LowerRightCorner.Y,
            clipRect->getWidth(), clipRect->getHeight());
    }
    if (colors)
        drawTexColoredQuad(texture, colors, width, height, center_pos_x, center_pos_y,
        tex_center_pos_x, tex_center_pos_y, tex_width, tex_height);
    else
        drawTexQuad(static_cast<const irr::video::COpenGLTexture*>(texture)->getOpenGLTextureName(), width, height, center_pos_x, center_pos_y,
        tex_center_pos_x, tex_center_pos_y, tex_width, tex_height);
    if (clipRect)
        glDisable(GL_SCISSOR_TEST);
    glUseProgram(0);

    glGetError();
}

void draw2DVertexPrimitiveList(video::ITexture *tex, const void* vertices,
    u32 vertexCount, const void* indexList, u32 primitiveCount,
    video::E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType, video::E_INDEX_TYPE iType)
{
    if (!irr_driver->isGLSL())
    {
        irr_driver->getVideoDriver()->draw2DVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount, vType, pType, iType);
        return;
    }
    GLuint tmpvao, tmpvbo, tmpibo;
    primitiveCount += 2;
    glGenVertexArrays(1, &tmpvao);
    glBindVertexArray(tmpvao);
    glGenBuffers(1, &tmpvbo);
    glBindBuffer(GL_ARRAY_BUFFER, tmpvbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * getVertexPitchFromType(vType), vertices, GL_STREAM_DRAW);
    glGenBuffers(1, &tmpibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tmpibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, primitiveCount * sizeof(u16), indexList, GL_STREAM_DRAW);

    VertexUtils::bindVertexArrayAttrib(vType);

    glUseProgram(UIShader::Primitive2DList::getInstance()->Program);
    UIShader::Primitive2DList::getInstance()->setUniforms();
    const video::SOverrideMaterial &m = irr_driver->getVideoDriver()->getOverrideMaterial();
    compressTexture(tex, false);
    UIShader::Primitive2DList::getInstance()->SetTextureUnits(getTextureGLuint(tex));
    glDrawElements(GL_TRIANGLE_FAN, primitiveCount, GL_UNSIGNED_SHORT, 0);

    glDeleteVertexArrays(1, &tmpvao);
    glDeleteBuffers(1, &tmpvbo);
    glDeleteBuffers(1, &tmpibo);

}

void GL32_draw2DRectangle(video::SColor color, const core::rect<s32>& position,
    const core::rect<s32>* clip)
{

    if (!irr_driver->isGLSL())
    {
        irr_driver->getVideoDriver()->draw2DRectangle(color, position, clip);
        return;
    }

    core::dimension2d<u32> frame_size =
        irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
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
        const core::dimension2d<u32>& renderTargetSize = irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
        glScissor(clip->UpperLeftCorner.X, renderTargetSize.Height - clip->LowerRightCorner.Y,
            clip->getWidth(), clip->getHeight());
    }

    glUseProgram(UIShader::ColoredRectShader::getInstance()->Program);
    glBindVertexArray(SharedObject::UIVAO);
    UIShader::ColoredRectShader::getInstance()->setUniforms(core::vector2df(center_pos_x, center_pos_y), core::vector2df(width, height), color);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    if (clip)
        glDisable(GL_SCISSOR_TEST);
    glUseProgram(0);

    glGetError();
}
