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


// ----------------------------------------------------------------------------
void swapPixels(char *old_img, char *new_img, unsigned stride, unsigned old_i,
                unsigned old_j, unsigned new_i, unsigned new_j)
{
    new_img[4 * (stride * new_i + new_j)] = old_img[4 * (stride * old_i + old_j)];
    new_img[4 * (stride * new_i + new_j) + 1] = old_img[4 * (stride * old_i + old_j) + 1];
    new_img[4 * (stride * new_i + new_j) + 2] = old_img[4 * (stride * old_i + old_j) + 2];
    new_img[4 * (stride * new_i + new_j) + 3] = old_img[4 * (stride * old_i + old_j) + 3];
}   // swapPixels


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
GLuint Skybox::generateCubeMapFromTextures(video::IVideoDriver *video_driver)
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

        video::IImage* image = video_driver
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


void Skybox::generateDiffuseCoefficients(video::IVideoDriver *video_driver,
                                         const video::SColor &ambient)
{
    const unsigned texture_permutation[] = { 2, 3, 0, 1, 5, 4 };

    unsigned sh_w = 0, sh_h = 0;
    unsigned char *sh_rgba[6];

    if (m_spherical_harmonics_textures.size() == 6)
    {

        for (unsigned i = 0; i < 6; i++)
        {
            sh_w = std::max(sh_w, m_spherical_harmonics_textures[i]->getSize().Width);
            sh_h = std::max(sh_h, m_spherical_harmonics_textures[i]->getSize().Height);
        }

        for (unsigned i = 0; i < 6; i++)
            sh_rgba[i] = new unsigned char[sh_w * sh_h * 4];
        for (unsigned i = 0; i < 6; i++)
        {
            unsigned idx = texture_permutation[i];

            video::IImage* image = video_driver->createImageFromData(
                m_spherical_harmonics_textures[idx]->getColorFormat(),
                m_spherical_harmonics_textures[idx]->getSize(),
                m_spherical_harmonics_textures[idx]->lock(),
                false
                );
            m_spherical_harmonics_textures[idx]->unlock();

            image->copyToScaling(sh_rgba[i], sh_w, sh_h);
            delete image;
        }

    }
    else
    {
        sh_w = 16;
        sh_h = 16;

        for (unsigned i = 0; i < 6; i++)
        {
            sh_rgba[i] = new unsigned char[sh_w * sh_h * 4];

            for (unsigned j = 0; j < sh_w * sh_h * 4; j += 4)
            {
                sh_rgba[i][j] = ambient.getBlue();
                sh_rgba[i][j + 1] = ambient.getGreen();
                sh_rgba[i][j + 2] = ambient.getRed();
                sh_rgba[i][j + 3] = 255;
            }
        }
    }

    // Convert to float texture
    Color *FloatTexCube[6];
    for (unsigned i = 0; i < 6; i++)
    {
        FloatTexCube[i] = new Color[sh_w * sh_h];
        for (unsigned j = 0; j < sh_w * sh_h; j++)
        {
            FloatTexCube[i][j].Blue = powf(float(0xFF & sh_rgba[i][4 * j]) / 255.f, 2.2f);
            FloatTexCube[i][j].Green = powf(float(0xFF & sh_rgba[i][4 * j + 1]) / 255.f, 2.2f);
            FloatTexCube[i][j].Red = powf(float(0xFF & sh_rgba[i][4 * j + 2]) / 255.f, 2.2f);
        }
    }

    generateSphericalHarmonics(FloatTexCube, sh_w, m_blue_SH_coeff, m_green_SH_coeff, m_red_SH_coeff);
    Log::debug("Skybox", "Blue_SH: %f, %f, %f, %f, %f, %f, %f, %f, %f",
                m_blue_SH_coeff[0], m_blue_SH_coeff[1], m_blue_SH_coeff[2],
                m_blue_SH_coeff[3], m_blue_SH_coeff[4], m_blue_SH_coeff[5],
                m_blue_SH_coeff[6], m_blue_SH_coeff[7], m_blue_SH_coeff[8]);
    Log::debug("Skybox", "Green_SH: %f, %f, %f, %f, %f, %f, %f, %f, %f",
                m_green_SH_coeff[0], m_green_SH_coeff[1], m_green_SH_coeff[2],
                m_green_SH_coeff[3], m_green_SH_coeff[4], m_green_SH_coeff[5],
                m_green_SH_coeff[6], m_green_SH_coeff[7], m_green_SH_coeff[8]);
    Log::debug("Skybox", "Red_SH: %f, %f, %f, %f, %f, %f, %f, %f, %f",
                m_red_SH_coeff[0], m_red_SH_coeff[1], m_red_SH_coeff[2],
                m_red_SH_coeff[3], m_red_SH_coeff[4], m_red_SH_coeff[5],
                m_red_SH_coeff[6], m_red_SH_coeff[7], m_red_SH_coeff[8]);

    for (unsigned i = 0; i < 6; i++)
    {
        delete[] sh_rgba[i];
        delete[] FloatTexCube[i];
    }

    if (m_spherical_harmonics_textures.size() != 6)
    {
        // Diffuse env map is x 0.25, compensate
        for (unsigned i = 0; i < 9; i++)
        {
            m_blue_SH_coeff[i] *= 4;
            m_green_SH_coeff[i] *= 4;
            m_red_SH_coeff[i] *= 4;
        }
    }
}   // generateDiffuseCoefficients




Skybox::Skybox(video::IVideoDriver *video_driver,
               const std::vector<video::ITexture *> &skybox_textures,
               const std::vector<video::ITexture *> &spherical_harmonics_textures,
               const video::SColor &ambient)
{
    m_skybox_textures = skybox_textures;
    m_spherical_harmonics_textures = spherical_harmonics_textures;
    
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    generateDiffuseCoefficients(video_driver, ambient);
    if (!skybox_textures.empty())
    {
        m_cube_map = generateCubeMapFromTextures(video_driver);
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


