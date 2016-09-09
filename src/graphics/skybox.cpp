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


class SpecularIBLGenerator : public TextureShader<SpecularIBLGenerator, 1,
                                                  core::matrix4, float >
{
public:
    GLuint m_tu_samples;
    SpecularIBLGenerator()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER,   "screenquad.vert",
                            GL_FRAGMENT_SHADER, "importance_sampling_specular.frag");
        assignUniforms("PermutationMatrix", "ViewportSize");
        m_tu_samples = 1;
        assignSamplerNames(0, "tex", ST_TRILINEAR_CUBEMAP);
        assignTextureUnit(m_tu_samples, "samples");
    }

};   // SpecularIBLGenerator


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

    // ----------------------------------------------------------------------------
    // From http://http.developer.nvidia.com/GPUGems3/gpugems3_ch20.html
    /** Returns the index-th pair from Hammersley set of pseudo random set.
        Hammersley set is a uniform distribution between 0 and 1 for 2 components.
        We use the natural indexation on the set to avoid storing the whole set.
        \param index of the pair
        \param size of the set. */
    std::pair<float, float> getHammersleySequence(int index, int samples)
    {
        float InvertedBinaryRepresentation = 0.;
        for (size_t i = 0; i < 32; i++)
        {
            InvertedBinaryRepresentation += ((index >> i) & 0x1)
                                          * powf(.5, (float) (i + 1.));
        }
        return std::make_pair(float(index) / float(samples),
                              InvertedBinaryRepresentation);
    }   // HammersleySequence


    // ----------------------------------------------------------------------------
    /** Returns a pseudo random (theta, phi) generated from a probability density
    *  function modeled after Phong function.
    *  \param a pseudo random float pair from a uniform density function between
    *         0 and 1.
    *   \param exponent from the Phong formula.
    */
    std::pair<float, float> getImportanceSamplingPhong(std::pair<float, float> Seeds,
                                                       float exponent)
    {
        return std::make_pair(acosf(powf(Seeds.first, 1.f / (exponent + 1.f))),
                              2.f * 3.14f * Seeds.second);
    }   // getImportanceSamplingPhong

    // ----------------------------------------------------------------------------
    static core::matrix4 getPermutationMatrix(size_t indexX, float valX,
                                              size_t indexY, float valY,
                                              size_t indexZ, float valZ)
    {
        core::matrix4 result_mat;
        float *M = result_mat.pointer();
        memset(M, 0, 16 * sizeof(float));
        assert(indexX < 4);
        assert(indexY < 4);
        assert(indexZ < 4);
        M[indexX] = valX;
        M[4 + indexY] = valY;
        M[8 + indexZ] = valZ;
        return result_mat;
    }   // getPermutationMatrix

}  //namespace

// ----------------------------------------------------------------------------
/** Generate an opengl cubemap texture from 6 2d textures */
void Skybox::generateCubeMapFromTextures()
{
    assert(m_skybox_textures.size() == 6);

    glGenTextures(1, &m_cube_map);

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

#if defined(USE_GLES2)
        for (unsigned int j = 0; j < size * size; j++)
        {
            char tmp_val = rgba[i][j*4];
            rgba[i][j*4] = rgba[i][j*4 + 2];
            rgba[i][j*4 + 2] = tmp_val;
        }
#endif

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

        glBindTexture(GL_TEXTURE_CUBE_MAP, m_cube_map);
#if !defined(USE_GLES2)
        GLint internal_format = CVS->isTextureCompressionEnabled() ?
                                    GL_COMPRESSED_SRGB_ALPHA : GL_SRGB_ALPHA;
        GLint format = GL_BGRA;
#else
        GLint internal_format = GL_RGBA8;
        GLint format = GL_RGBA;
#endif

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                     internal_format, size, size, 0, format,
                     GL_UNSIGNED_BYTE, (GLvoid*)rgba[i]);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    for (unsigned i = 0; i < 6; i++)
        delete[] rgba[i];
}   // generateCubeMapFromTextures


// ----------------------------------------------------------------------------
void Skybox::generateSpecularCubemap()
{
    glGenTextures(1, &m_specular_probe);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_specular_probe);
    size_t cubemap_size = 256;
    for (int i = 0; i < 6; i++)
    {
#if !defined(USE_GLES2)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F,
                     cubemap_size, cubemap_size, 0, GL_BGRA, GL_FLOAT, 0);
#else
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8,
                     cubemap_size, cubemap_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#endif
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    if (!CVS->isDefferedEnabled())
        return;

#if !defined(USE_GLES2)

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, cubemap_size, cubemap_size);
    GLenum bufs[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, bufs);
    SpecularIBLGenerator::getInstance()->use();

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    core::matrix4 M[6] = {
        getPermutationMatrix(2, -1., 1, -1., 0, 1.),
        getPermutationMatrix(2, 1., 1, -1., 0, -1.),
        getPermutationMatrix(0, 1., 2, 1., 1, 1.),
        getPermutationMatrix(0, 1., 2, -1., 1, -1.),
        getPermutationMatrix(0, 1., 1, -1., 2, 1.),
        getPermutationMatrix(0, -1., 1, -1., 2, -1.),
    };

    for (unsigned level = 0; level < 8; level++)
    {
        // Blinn Phong can be approximated by Phong with 4x the specular
        // coefficient
        // See http://seblagarde.wordpress.com/2012/03/29/relationship-between-phong-and-blinn-lighting-model/
        // NOTE : Removed because it makes too sharp reflexion
        float roughness = (8 - level) * pow(2.f, 10.f) / 8.f;
        float viewportSize = float(1 << (8 - level));

        float *tmp = new float[2048];
        for (unsigned i = 0; i < 1024; i++)
        {
            std::pair<float, float> sample =
                getImportanceSamplingPhong(getHammersleySequence(i, 1024),
                                        roughness);
            tmp[2 * i] = sample.first;
            tmp[2 * i + 1] = sample.second;
        }

        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0 +
                        SpecularIBLGenerator::getInstance()->m_tu_samples);
        GLuint sampleTex, sampleBuffer;
        glGenBuffers(1, &sampleBuffer);
        glBindBuffer(GL_TEXTURE_BUFFER, sampleBuffer);
        glBufferData(GL_TEXTURE_BUFFER, 2048 * sizeof(float), tmp,
                     GL_STATIC_DRAW);
        glGenTextures(1, &sampleTex);
        glBindTexture(GL_TEXTURE_BUFFER, sampleTex);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, sampleBuffer);
        glBindVertexArray(SharedGPUObjects::getFullScreenQuadVAO());

        for (unsigned face = 0; face < 6; face++)
        {

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                   m_specular_probe, level);
            GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            assert(status == GL_FRAMEBUFFER_COMPLETE);

            SpecularIBLGenerator::getInstance()->setTextureUnits(m_cube_map);
            SpecularIBLGenerator::getInstance()->setUniforms(M[face],
                                                             viewportSize);

            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        glActiveTexture(  GL_TEXTURE0
                        + SpecularIBLGenerator::getInstance()->m_tu_samples);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
        glBindTexture(GL_TEXTURE_BUFFER, 0);

        delete[] tmp;
        glDeleteTextures(1, &sampleTex);
        glDeleteBuffers(1, &sampleBuffer);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    glActiveTexture(GL_TEXTURE0);
#endif
}   // generateSpecularCubemap


// ----------------------------------------------------------------------------
/** Generate a skybox from 6 2d textures.
Out of legacy the sequence of textures maps to:
- 1st texture maps to GL_TEXTURE_CUBE_MAP_POSITIVE_Y
- 2nd texture maps to GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
- 3rd texture maps to GL_TEXTURE_CUBE_MAP_POSITIVE_X
- 4th texture maps to GL_TEXTURE_CUBE_MAP_NEGATIVE_X
- 5th texture maps to GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
- 6th texture maps to GL_TEXTURE_CUBE_MAP_POSITIVE_Z
*  \param skybox_textures sequence of 6 textures.
*/
Skybox::Skybox(const std::vector<video::ITexture *> &skybox_textures)
{
    m_skybox_textures = skybox_textures;

#if !defined(USE_GLES2)
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

    if (!skybox_textures.empty())
    {
        generateCubeMapFromTextures();
        generateSpecularCubemap();
    }
}

Skybox::~Skybox()
{
    glDeleteTextures(1, &m_cube_map);
    glDeleteTextures(1, &m_specular_probe);
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


