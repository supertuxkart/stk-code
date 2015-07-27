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


#include "graphics/skybox.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/IBL.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/shaders.hpp"


#include <algorithm> 
#include <cassert>

using namespace irr;


class SkyboxShader : public TextureShader<SkyboxShader,1>
{
private:
    GLuint m_vao;

public:
    SkyboxShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "sky.vert",
                            GL_FRAGMENT_SHADER, "sky.frag");
        assignUniforms();
        assignSamplerNames(0, "tex", ST_TRILINEAR_CUBEMAP);

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, SharedGPUObjects::getSkyTriVBO());
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glBindVertexArray(0);
    }   // SkyboxShader
    // ------------------------------------------------------------------------
    void bindVertexArray()
    {
        glBindVertexArray(m_vao);
    }   // bindVertexArray
};   // SkyboxShader


namespace {
    // ----------------------------------------------------------------------------
    void swapPixels(char *old_img, char *new_img, unsigned stride, unsigned old_i,
                    unsigned old_j, unsigned new_i, unsigned new_j)
    {
        new_img[4 * (stride * new_i + new_j)] = old_img[4 * (stride * old_i + old_j)];
        new_img[4 * (stride * new_i + new_j) + 1] = old_img[4 * (stride * old_i + old_j) + 1];
        new_img[4 * (stride * new_i + new_j) + 2] = old_img[4 * (stride * old_i + old_j) + 2];
        new_img[4 * (stride * new_i + new_j) + 3] = old_img[4 * (stride * old_i + old_j) + 3];
    }   // swapPixels
}

// ----------------------------------------------------------------------------
/** Generate an opengl cubemap texture from 6 2d textures.
Out of legacy the sequence of textures maps to :
- 1st texture maps to GL_TEXTURE_CUBE_MAP_POSITIVE_Y
- 2nd texture maps to GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
- 3rd texture maps to GL_TEXTURE_CUBE_MAP_POSITIVE_X
- 4th texture maps to GL_TEXTURE_CUBE_MAP_NEGATIVE_X
- 5th texture maps to GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
- 6th texture maps to GL_TEXTURE_CUBE_MAP_POSITIVE_Z
*  \param textures sequence of 6 textures.
*/
GLuint Skybox::generateCubeMapFromTextures()
{
    assert(m_skybox_textures.size() == 6);

    GLuint result;
    glGenTextures(1, &result);

    unsigned size = 0;
    for (unsigned i = 0; i < 6; i++)
    {
        size = std::max(size, m_skybox_textures[i]->getSize().Width);
        size = std::max(size, m_skybox_textures[i]->getSize().Height);
    }

    const unsigned texture_permutation[] = { 2, 3, 0, 1, 5, 4 };
    char *rgba[6];
    for (unsigned i = 0; i < 6; i++)
        rgba[i] = new char[size * size * 4];
    for (unsigned i = 0; i < 6; i++)
    {
        unsigned idx = texture_permutation[i];

        video::IImage* image = irr_driver->getVideoDriver()
            ->createImageFromData(m_skybox_textures[idx]->getColorFormat(),
                                  m_skybox_textures[idx]->getSize(),
                                  m_skybox_textures[idx]->lock(), false   );
        m_skybox_textures[idx]->unlock();

        image->copyToScaling(rgba[i], size, size);
        image->drop();

        if (i == 2 || i == 3)
        {
            char *tmp = new char[size * size * 4];
            memcpy(tmp, rgba[i], size * size * 4);
            for (unsigned x = 0; x < size; x++)
            {
                for (unsigned y = 0; y < size; y++)
                {
                    swapPixels(tmp, rgba[i], size, x, y, (size - y - 1), x);
                }
            }
            delete[] tmp;
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, result);
        if (CVS->isTextureCompressionEnabled())
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                         GL_COMPRESSED_SRGB_ALPHA, size, size, 0, GL_BGRA,
                         GL_UNSIGNED_BYTE, (GLvoid*)rgba[i]);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                         GL_SRGB_ALPHA, size, size, 0, GL_BGRA,
                         GL_UNSIGNED_BYTE, (GLvoid*)rgba[i]);
        }
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    for (unsigned i = 0; i < 6; i++)
        delete[] rgba[i];
    return result;
}   // generateCubeMapFromTextures






Skybox::Skybox(const std::vector<video::ITexture *> &skybox_textures)
{
    m_skybox_textures = skybox_textures;
    
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    if (!skybox_textures.empty())
    {
        m_cube_map = generateCubeMapFromTextures();
        m_specular_probe = generateSpecularCubemap(m_cube_map);
    }
    
    
}

Skybox::~Skybox()
{
    //TODOskybox
}

// ----------------------------------------------------------------------------
void Skybox::render(const scene::ICameraSceneNode *camera) const
{
    if (m_skybox_textures.empty())
        return;
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    assert(m_skybox_textures.size() == 6);

    glDisable(GL_BLEND);

    SkyboxShader::getInstance()->use();
    SkyboxShader::getInstance()->bindVertexArray();
    SkyboxShader::getInstance()->setUniforms();

    SkyboxShader::getInstance()->setTextureUnits(m_cube_map);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}   // renderSkybox


