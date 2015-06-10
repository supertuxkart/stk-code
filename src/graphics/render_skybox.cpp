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

#include "central_settings.hpp"
#include "graphics/IBL.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/shaders.hpp"
#include "modes/world.hpp"
#include "utils/profiler.hpp"

#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN2(a, b) ((a) > (b) ? (b) : (a))

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

// ============================================================================
static float getTexelValue(unsigned i, unsigned j, size_t width, size_t height,
                           float *Coeff, float *Y00, float *Y1minus1,
                           float *Y10, float *Y11, float *Y2minus2, 
                           float * Y2minus1, float * Y20, float *Y21,
                           float *Y22)
{
    float solidangle = 1.;
    size_t idx = i * height + j;
    float reconstructedVal = Y00[idx] * Coeff[0];
    reconstructedVal += Y1minus1[i * height + j] * Coeff[1] 
                     +  Y10[i * height + j] * Coeff[2] 
                     + Y11[i * height + j] * Coeff[3];
    reconstructedVal += Y2minus2[idx] * Coeff[4] 
                      + Y2minus1[idx] * Coeff[5] + Y20[idx] * Coeff[6]
                      + Y21[idx] * Coeff[7] + Y22[idx] * Coeff[8];
    reconstructedVal /= solidangle;
    return MAX2(255.0f * reconstructedVal, 0.f);
}   // getTexelValue

// ----------------------------------------------------------------------------
static void unprojectSH(float *output[], size_t width, size_t height,
                        float *Y00[], float *Y1minus1[], float *Y10[],
                        float *Y11[], float *Y2minus2[], float *Y2minus1[],
                        float * Y20[], float *Y21[], float *Y22[],
                        float *blueSHCoeff, float *greenSHCoeff,
                        float *redSHCoeff)
{
    for (unsigned face = 0; face < 6; face++)
    {
        for (unsigned i = 0; i < width; i++)
        {
            for (unsigned j = 0; j < height; j++)
            {
                float fi = float(i), fj = float(j);
                fi /= width, fj /= height;
                fi = 2 * fi - 1, fj = 2 * fj - 1;

                output[face][4 * height * i + 4 * j + 2] =
                    getTexelValue(i, j, width, height, redSHCoeff, Y00[face],
                                  Y1minus1[face], Y10[face], Y11[face],
                                  Y2minus2[face], Y2minus1[face], Y20[face],
                                  Y21[face], Y22[face]);
                output[face][4 * height * i + 4 * j + 1] = 
                    getTexelValue(i, j, width, height, greenSHCoeff, Y00[face],
                                  Y1minus1[face], Y10[face], Y11[face],
                                  Y2minus2[face], Y2minus1[face], Y20[face],
                                  Y21[face], Y22[face]);
                output[face][4 * height * i + 4 * j] = 
                    getTexelValue(i, j, width, height, blueSHCoeff, Y00[face],
                                  Y1minus1[face], Y10[face], Y11[face],
                                  Y2minus2[face], Y2minus1[face], Y20[face],
                                  Y21[face], Y22[face]);
            }
        }
    }
}   // unprojectSH

// ----------------------------------------------------------------------------
static void displayCoeff(float *SHCoeff)
{
    printf("L00:%f\n", SHCoeff[0]);
    printf("L1-1:%f, L10:%f, L11:%f\n", SHCoeff[1], SHCoeff[2], SHCoeff[3]);
    printf("L2-2:%f, L2-1:%f, L20:%f, L21:%f, L22:%f\n",
            SHCoeff[4], SHCoeff[5], SHCoeff[6], SHCoeff[7], SHCoeff[8]);
}   // displayCoeff

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
GLuint generateCubeMapFromTextures(const std::vector<video::ITexture *> &textures)
{
    assert(textures.size() == 6);

    GLuint result;
    glGenTextures(1, &result);

    unsigned size = 0;
    for (unsigned i = 0; i < 6; i++)
    {
        size = MAX2(size, textures[i]->getSize().Width);
        size = MAX2(size, textures[i]->getSize().Height);
    }

    const unsigned texture_permutation[] = { 2, 3, 0, 1, 5, 4 };
    char *rgba[6];
    for (unsigned i = 0; i < 6; i++)
        rgba[i] = new char[size * size * 4];
    for (unsigned i = 0; i < 6; i++)
    {
        unsigned idx = texture_permutation[i];

        video::IImage* image = irr_driver->getVideoDriver()
            ->createImageFromData(textures[idx]->getColorFormat(),
                                  textures[idx]->getSize(),
                                  textures[idx]->lock(), false   );
        textures[idx]->unlock();

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

// ----------------------------------------------------------------------------
void IrrDriver::prepareSkybox()
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    generateDiffuseCoefficients();
	DFG_LUT = generateSpecularDFGLUT();
    if (!SkyboxTextures.empty())
    {
        SkyboxCubeMap = generateCubeMapFromTextures(SkyboxTextures);
        SkyboxSpecularProbe = generateSpecularCubemap(SkyboxCubeMap);
    }
}   // prepareSkybox

// ----------------------------------------------------------------------------
void IrrDriver::generateDiffuseCoefficients()
{
    const unsigned texture_permutation[] = { 2, 3, 0, 1, 5, 4 };

    unsigned sh_w = 0, sh_h = 0;
    unsigned char *sh_rgba[6];

    if (SphericalHarmonicsTextures.size() == 6)
    {

        for (unsigned i = 0; i < 6; i++)
        {
            sh_w = MAX2(sh_w, SphericalHarmonicsTextures[i]->getSize().Width);
            sh_h = MAX2(sh_h, SphericalHarmonicsTextures[i]->getSize().Height);
        }

        for (unsigned i = 0; i < 6; i++)
            sh_rgba[i] = new unsigned char[sh_w * sh_h * 4];
        for (unsigned i = 0; i < 6; i++)
        {
            unsigned idx = texture_permutation[i];

            video::IImage* image = getVideoDriver()->createImageFromData(
                SphericalHarmonicsTextures[idx]->getColorFormat(),
                SphericalHarmonicsTextures[idx]->getSize(),
                SphericalHarmonicsTextures[idx]->lock(),
                false
                );
            SphericalHarmonicsTextures[idx]->unlock();

            image->copyToScaling(sh_rgba[i], sh_w, sh_h);
            delete image;
        }

    }
    else
    {
        sh_w = 16;
        sh_h = 16;

        video::SColor ambient = m_scene_manager->getAmbientLight().toSColor();

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

    generateSphericalHarmonics(FloatTexCube, sh_w, blueSHCoeff, greenSHCoeff, redSHCoeff);

    for (unsigned i = 0; i < 6; i++)
    {
        delete[] sh_rgba[i];
        delete[] FloatTexCube[i];
    }

    if (SphericalHarmonicsTextures.size() != 6)
    {
        // Diffuse env map is x 0.25, compensate
        for (unsigned i = 0; i < 9; i++)
        {
            blueSHCoeff[i] *= 4;
            greenSHCoeff[i] *= 4;
            redSHCoeff[i] *= 4;
        }
    }
}   // generateDiffuseCoefficients

// ----------------------------------------------------------------------------
void IrrDriver::renderSkybox(const scene::ICameraSceneNode *camera)
{
    if (SkyboxTextures.empty())
        return;
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    assert(SkyboxTextures.size() == 6);

    glDisable(GL_BLEND);

    SkyboxShader::getInstance()->use();
    SkyboxShader::getInstance()->bindVertexArray();
    SkyboxShader::getInstance()->setUniforms();

    SkyboxShader::getInstance()->setTextureUnits(SkyboxCubeMap);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}   // renderSkybox
