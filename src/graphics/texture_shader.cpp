//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
//            (C) 2015      Joerg Henrichs
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

#include "graphics/texture_shader.hpp"
#include "graphics/central_settings.hpp"

#include "config/user_config.hpp"

TextureShaderBase::BindFunction TextureShaderBase::m_all_bind_functions[] =
{ /* ST_NEAREST_FILTERED               */ &TextureShaderBase::bindTextureNearest,
  /* ST_TRILINEAR_ANISOTROPIC_FILTERED */ &TextureShaderBase::bindTextureTrilinearAnisotropic,
  /* ST_TRILINEAR_CUBEMAP              */ &TextureShaderBase::bindCubemapTrilinear,
  /* ST_BILINEAR_FILTERED              */ &TextureShaderBase::bindTextureBilinear,
  /* ST_SHADOW_SAMPLER                 */ &TextureShaderBase::bindTextureShadow,
  /* ST_TRILINEAR_CLAMPED_ARRAY2D      */ &TextureShaderBase::bindTrilinearClampedArrayTexture,
  /* ST_VOLUME_LINEAR_FILTERED         */ &TextureShaderBase::bindTextureVolume,
  /* ST_NEARED_CLAMPED_FILTERED        */ &TextureShaderBase::bindTextureNearestClamped,
  /* ST_BILINEAR_CLAMPED_FILTERED      */ &TextureShaderBase::bindTextureBilinearClamped,
  /* ST_SEMI_TRILINEAR                 */ &TextureShaderBase::bindTextureSemiTrilinear,
  /* ST_TEXTURE_BUFFER                 */ &TextureShaderBase::bindTextureBuffer
};

GLuint TextureShaderBase::m_all_texture_types[] =
{ /* ST_NEAREST_FILTERED               */ GL_TEXTURE_2D,
  /* ST_TRILINEAR_ANISOTROPIC_FILTERED */ GL_TEXTURE_2D,
  /* ST_TRILINEAR_CUBEMAP              */ GL_TEXTURE_CUBE_MAP,
  /* ST_BILINEAR_FILTERED              */ GL_TEXTURE_2D ,
  /* ST_SHADOW_SAMPLER                 */ GL_TEXTURE_2D_ARRAY,
  /* ST_TRILINEAR_CLAMPED_ARRAY2D      */ GL_TEXTURE_2D_ARRAY,
  /* ST_VOLUME_LINEAR_FILTERED         */ GL_TEXTURE_3D,
  /* ST_NEARED_CLAMPED_FILTERED        */ GL_TEXTURE_2D,
  /* ST_BILINEAR_CLAMPED_FILTERED      */ GL_TEXTURE_2D,
  /* ST_SEMI_TRILINEAR                 */ GL_TEXTURE_2D
#ifndef USE_GLES2
  /* ST_TEXTURE_BUFFER                */, GL_TEXTURE_BUFFER
#endif

};

// ----------------------------------------------------------------------------
void TextureShaderBase::bindTextureNearest(GLuint texture_unit, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (CVS->isEXTTextureFilterAnisotropicUsable())
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);

}   // bindTextureNearest

// ----------------------------------------------------------------------------
void TextureShaderBase::bindTextureTrilinearAnisotropic(GLuint tex_unit, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (CVS->isEXTTextureFilterAnisotropicUsable())
    {
        int aniso = UserConfigParams::m_anisotropic;
        if (aniso == 0) aniso = 1;
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                        (float)aniso);
    }
}   // bindTextureTrilinearAnisotropic

// ----------------------------------------------------------------------------
void TextureShaderBase::bindCubemapTrilinear(unsigned tex_unit, unsigned tex)
{
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (CVS->isEXTTextureFilterAnisotropicUsable())
    {
        int aniso = UserConfigParams::m_anisotropic;
        if (aniso == 0) aniso = 1;
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                        (float)aniso);
    }
}   // bindCubemapTrilinear

// ----------------------------------------------------------------------------
void TextureShaderBase::bindTextureNearestClamped(GLuint texture_unit,
                                                   GLuint tex_id)
{
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (CVS->isEXTTextureFilterAnisotropicUsable())
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}   // bindTextureNearestClamped

// ----------------------------------------------------------------------------
void TextureShaderBase::bindTextureBuffer(GLuint texture_unit, GLuint tex_id)
{
#ifndef USE_GLES2
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_BUFFER, tex_id);
#endif
}   // bindTextureBuffer

// ----------------------------------------------------------------------------
void TextureShaderBase::bindTextureBilinear(GLuint texture_unit, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (CVS->isEXTTextureFilterAnisotropicUsable())
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}   // bindTextureBilinear

// ----------------------------------------------------------------------------
void TextureShaderBase::bindTextureBilinearClamped(GLuint tex_unit, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (CVS->isEXTTextureFilterAnisotropicUsable())
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}   // bindTextureBilinearClamped

// ----------------------------------------------------------------------------
void TextureShaderBase::bindTextureSemiTrilinear(GLuint tex_unit, GLuint tex_id)
{
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (CVS->isEXTTextureFilterAnisotropicUsable())
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}   // bindTextureSemiTrilinear

// ----------------------------------------------------------------------------
void TextureShaderBase::bindTextureShadow(GLuint tex_unit, GLuint tex_id)
{
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex_id);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
}   // bindTextureShadow

// ----------------------------------------------------------------------------
void TextureShaderBase::bindTrilinearClampedArrayTexture(unsigned tex_unit,
                                                          unsigned tex_id)
{
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex_id);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (CVS->isEXTTextureFilterAnisotropicUsable())
    {
        int aniso = UserConfigParams::m_anisotropic;
        if (aniso == 0) aniso = 1;
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                        (float)aniso);
    }
}   // bindTrilinearClampedArrayTexture

// ----------------------------------------------------------------------------
void TextureShaderBase::bindTextureVolume(unsigned tex_unit, unsigned tex_id)
{
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(GL_TEXTURE_3D, tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (CVS->isEXTTextureFilterAnisotropicUsable())
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}   // bindTextureVolume

// ----------------------------------------------------------------------------

GLuint TextureShaderBase::createSamplers(SamplerTypeNew sampler_type)
{
    switch (sampler_type)
    {
    case ST_NEAREST_FILTERED:
        return createNearestSampler();
    case ST_TRILINEAR_ANISOTROPIC_FILTERED:
        return createTrilinearSampler();
    case ST_TRILINEAR_CUBEMAP:
        return createTrilinearSampler();
    case ST_BILINEAR_FILTERED:
        return createBilinearSampler();
    case ST_SHADOW_SAMPLER:
        return createShadowSampler();
    case ST_TRILINEAR_CLAMPED_ARRAY2D:
        return createTrilinearClampedArray();
    case ST_VOLUME_LINEAR_FILTERED:
        return createBilinearSampler();
    case ST_NEARED_CLAMPED_FILTERED:
    {
        // WHAT'S THAT??????????????????
        unsigned id;
        glGenSamplers(1, &id);
        glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (CVS->isEXTTextureFilterAnisotropicUsable())
            glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
        return createNearestSampler();
    }
    case ST_BILINEAR_CLAMPED_FILTERED:
        return createBilinearClampedSampler();
    case ST_SEMI_TRILINEAR:
        return createSemiTrilinearSampler();
#ifndef USE_GLES2
    case ST_TEXTURE_BUFFER:
        return 0;
#endif
    default:
        assert(false);
        return 0;
    }   // switch
}   // createSamplers

// ----------------------------------------------------------------------------
GLuint TextureShaderBase::createNearestSampler()
{
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (CVS->isEXTTextureFilterAnisotropicUsable())
        glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
    return id;
}   // createNearestSampler

// ----------------------------------------------------------------------------
GLuint TextureShaderBase::createTrilinearSampler()
{
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (CVS->isEXTTextureFilterAnisotropicUsable())
    {
        int aniso = UserConfigParams::m_anisotropic;
        if (aniso == 0) aniso = 1;
        glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)aniso);
    }
    return id;
}   // createTrilinearSampler

// ----------------------------------------------------------------------------
GLuint TextureShaderBase::createBilinearSampler()
{
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (CVS->isEXTTextureFilterAnisotropicUsable())
        glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
    return id;
}   // createBilinearSampler
// ----------------------------------------------------------------------------
GLuint TextureShaderBase::createShadowSampler()
{
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameterf(id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glSamplerParameterf(id, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    return id;
}   // createShadowSampler

// ----------------------------------------------------------------------------
GLuint TextureShaderBase::createBilinearClampedSampler()
{
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (CVS->isEXTTextureFilterAnisotropicUsable())
        glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
    return id;
}   // createBilinearClampedSampler


// ----------------------------------------------------------------------------
GLuint TextureShaderBase::createTrilinearClampedArray()
{
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (CVS->isEXTTextureFilterAnisotropicUsable())
    {
        int aniso = UserConfigParams::m_anisotropic;
        if (aniso == 0) aniso = 1;
        glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)aniso);
    }
    return id;
}   // createTrilinearClampedArray
// ----------------------------------------------------------------------------
GLuint TextureShaderBase::createSemiTrilinearSampler()
{
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (CVS->isEXTTextureFilterAnisotropicUsable())
        glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
    return id;
}   // createSemiTrilinearSampler

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#endif   // !SERVER_ONLY

