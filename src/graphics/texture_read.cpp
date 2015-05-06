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

#include "graphics/texture_read.hpp"

#include "config/user_config.hpp"

TextureReadBaseNew::BindFunction TextureReadBaseNew::m_all_bind_functions[] =
{ &TextureReadBaseNew::bindTextureNearest,
  &TextureReadBaseNew::bindTextureTrilinearAnisotropic };

GLuint TextureReadBaseNew::m_all_texture_types[] = { GL_TEXTURE_2D, GL_TEXTURE_2D };

// ----------------------------------------------------------------------------
GLuint TextureReadBaseNew::createSamplers(SamplerTypeNew sampler_type)
{
    switch (sampler_type)
    {
    case ST_NEAREST_FILTERED:
        return createNearestSampler();
    case ST_TRILINEAR_ANISOTROPIC_FILTERED:
        return createTrilinearSampler();

    default:
        assert(false);
        return 0;
    }   // switch
}   // createSamplers

// ----------------------------------------------------------------------------
void TextureReadBaseNew::bindTextureNearest(GLuint texture_unit, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}   // bindTextureNearest

// ----------------------------------------------------------------------------
void TextureReadBaseNew::bindTextureTrilinearAnisotropic(GLuint tex_unit, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int aniso = UserConfigParams::m_anisotropic;
    if (aniso == 0) aniso = 1;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)aniso);
}   // bindTextureTrilinearAnisotropic

// ----------------------------------------------------------------------------
GLuint TextureReadBaseNew::createNearestSampler()
{
#ifdef GL_VERSION_3_3
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
    return id;
#endif
}   // createNearestSampler

// ----------------------------------------------------------------------------
void TextureReadBaseNew::bindTextureNearestClamped(GLuint texture_unit,
                                                   GLuint tex_id)
{
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}   // bindTextureNearesClamped

// ----------------------------------------------------------------------------
void TextureReadBaseNew::bindTextureBilinear(GLuint texture_unit, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}   // bindTextureBilinear

// ----------------------------------------------------------------------------
void TextureReadBaseNew::bindTextureBilinearClamped(GLuint tex_unit, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}   // bindTextureBilinearClamped

// ----------------------------------------------------------------------------
void TextureReadBaseNew::bindTextureSemiTrilinear(GLuint tex_unit, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}   // bindTextureSemiTrilinear

// ----------------------------------------------------------------------------
GLuint TextureReadBaseNew::createTrilinearSampler()
{
#ifdef GL_VERSION_3_3
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int aniso = UserConfigParams::m_anisotropic;
    if (aniso == 0) aniso = 1;
    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)aniso);
    return id;
#endif
}   // createTrilinearSampler

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
