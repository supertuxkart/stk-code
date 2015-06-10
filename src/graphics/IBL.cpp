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

#include "graphics/IBL.hpp"

#include "graphics/central_settings.hpp"
#include "graphics/gl_headers.hpp"
#include "graphics/shaders.hpp"
#include "graphics/shared_gpu_objects.hpp"

#include <cmath>
#include <set>


class SpecularIBLGenerator : public TextureShader<SpecularIBLGenerator, 1, 
                                                core::matrix4, float >
{
public:
    GLuint m_tu_samples;
    SpecularIBLGenerator()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                      GL_FRAGMENT_SHADER, "importance_sampling_specular.frag");
        assignUniforms("PermutationMatrix", "ViewportSize");
        m_tu_samples = 1;
        assignSamplerNames(0, "tex", ST_TRILINEAR_CUBEMAP);
        assignTextureUnit(m_tu_samples, "samples");
    }

};   // SpecularIBLGenerator

// ============================================================================
static void getXYZ(GLenum face, float i, float j, float &x, float &y, float &z)
{
    switch (face)
    {
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        x = 1.;
        y = -i;
        z = -j;
        break;
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        x = -1.;
        y = -i;
        z = j;
        break;
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        x = j;
        y = 1.;
        z = i;
        break;
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        x = j;
        y = -1;
        z = -i;
        break;
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        x = j;
        y = -i;
        z = 1;
        break;
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        x = -j;
        y = -i;
        z = -1;
        break;
    }

    float norm = sqrt(x * x + y * y + z * z);
    x /= norm, y /= norm, z /= norm;
    return;
}   // getXYZ


// ----------------------------------------------------------------------------
static void getYml(GLenum face, size_t edge_size,
    float *Y00,
    float *Y1minus1, float *Y10, float *Y11,
    float *Y2minus2, float *Y2minus1, float *Y20, float *Y21, float *Y22)
{
#pragma omp parallel for
    for (int i = 0; i < int(edge_size); i++)
    {
        for (unsigned j = 0; j < edge_size; j++)
        {
            float x, y, z;
            float fi = float(i), fj = float(j);
            fi /= edge_size, fj /= edge_size;
            fi = 2 * fi - 1, fj = 2 * fj - 1;
            getXYZ(face, fi, fj, x, y, z);

            // constant part of Ylm
            float c00 = 0.282095f;
            float c1minus1 = 0.488603f;
            float c10 = 0.488603f;
            float c11 = 0.488603f;
            float c2minus2 = 1.092548f;
            float c2minus1 = 1.092548f;
            float c21 = 1.092548f;
            float c20 = 0.315392f;
            float c22 = 0.546274f;

            size_t idx = i * edge_size + j;

            Y00[idx] = c00;
            Y1minus1[idx] = c1minus1 * y;
            Y10[idx] = c10 * z;
            Y11[idx] = c11 * x;
            Y2minus2[idx] = c2minus2 * x * y;
            Y2minus1[idx] = c2minus1 * y * z;
            Y21[idx] = c21 * x * z;
            Y20[idx] = c20 * (3 * z * z - 1);
            Y22[idx] = c22 * (x * x - y * y);
        }
    }
}


// ----------------------------------------------------------------------------
static void projectSH(Color *cubemap_face[6], size_t edge_size, float *Y00[],
                      float *Y1minus1[], float *Y10[], float *Y11[],
                      float *Y2minus2[], float *Y2minus1[], float * Y20[],
                      float *Y21[], float *Y22[], float *blue_sh_coeff,
                      float *green_sh_coeff, float *red_sh_coeff)
{
    for (unsigned i = 0; i < 9; i++)
    {
        blue_sh_coeff[i] = 0;
        green_sh_coeff[i] = 0;
        red_sh_coeff[i] = 0;
    }

    float wh = float(edge_size * edge_size);
    float b0 = 0., b1 = 0., b2 = 0., b3 = 0., b4 = 0., b5 = 0., b6 = 0., b7 = 0., b8 = 0.;
    float r0 = 0., r1 = 0., r2 = 0., r3 = 0., r4 = 0., r5 = 0., r6 = 0., r7 = 0., r8 = 0.;
    float g0 = 0., g1 = 0., g2 = 0., g3 = 0., g4 = 0., g5 = 0., g6 = 0., g7 = 0., g8 = 0.;
    for (unsigned face = 0; face < 6; face++)
    {
#pragma omp parallel for reduction(+ : b0, b1, b2, b3, b4, b5, b6, b7, b8, \
                                       r0, r1, r2, r3, r4, r5, r6, r7, r8, \
                                       g0, g1, g2, g3, g4, g5, g6, g7, g8)
        for (int i = 0; i < int(edge_size); i++)
        {
            for (unsigned j = 0; j < edge_size; j++)
            {
                int idx = i * edge_size + j;
                float fi = float(i), fj = float(j);
                fi /= edge_size, fj /= edge_size;
                fi = 2 * fi - 1, fj = 2 * fj - 1;


                float d = sqrt(fi * fi + fj * fj + 1);

                // Constant obtained by projecting unprojected ref values
                float solidangle = 2.75f / (wh * pow(d, 1.5f));
                // pow(., 2.2) to convert from srgb
                float b = cubemap_face[face][edge_size * i + j].Blue;
                float g = cubemap_face[face][edge_size * i + j].Green;
                float r = cubemap_face[face][edge_size * i + j].Red;

                b0 += b * Y00[face][idx] * solidangle;
                b1 += b * Y1minus1[face][idx] * solidangle;
                b2 += b * Y10[face][idx] * solidangle;
                b3 += b * Y11[face][idx] * solidangle;
                b4 += b * Y2minus2[face][idx] * solidangle;
                b5 += b * Y2minus1[face][idx] * solidangle;
                b6 += b * Y20[face][idx] * solidangle;
                b7 += b * Y21[face][idx] * solidangle;
                b8 += b * Y22[face][idx] * solidangle;

                g0 += g * Y00[face][idx] * solidangle;
                g1 += g * Y1minus1[face][idx] * solidangle;
                g2 += g * Y10[face][idx] * solidangle;
                g3 += g * Y11[face][idx] * solidangle;
                g4 += g * Y2minus2[face][idx] * solidangle;
                g5 += g * Y2minus1[face][idx] * solidangle;
                g6 += g * Y20[face][idx] * solidangle;
                g7 += g * Y21[face][idx] * solidangle;
                g8 += g * Y22[face][idx] * solidangle;


                r0 += r * Y00[face][idx] * solidangle;
                r1 += r * Y1minus1[face][idx] * solidangle;
                r2 += r * Y10[face][idx] * solidangle;
                r3 += r * Y11[face][idx] * solidangle;
                r4 += r * Y2minus2[face][idx] * solidangle;
                r5 += r * Y2minus1[face][idx] * solidangle;
                r6 += r * Y20[face][idx] * solidangle;
                r7 += r * Y21[face][idx] * solidangle;
                r8 += r * Y22[face][idx] * solidangle;
            }
        }
    }

    blue_sh_coeff[0] = b0;
    blue_sh_coeff[1] = b1;
    blue_sh_coeff[2] = b2;
    blue_sh_coeff[3] = b3;
    blue_sh_coeff[4] = b4;
    blue_sh_coeff[5] = b5;
    blue_sh_coeff[6] = b6;
    blue_sh_coeff[7] = b7;
    blue_sh_coeff[8] = b8;

    red_sh_coeff[0] = r0;
    red_sh_coeff[1] = r1;
    red_sh_coeff[2] = r2;
    red_sh_coeff[3] = r3;
    red_sh_coeff[4] = r4;
    red_sh_coeff[5] = r5;
    red_sh_coeff[6] = r6;
    red_sh_coeff[7] = r7;
    red_sh_coeff[8] = r8;

    green_sh_coeff[0] = g0;
    green_sh_coeff[1] = g1;
    green_sh_coeff[2] = g2;
    green_sh_coeff[3] = g3;
    green_sh_coeff[4] = g4;
    green_sh_coeff[5] = g5;
    green_sh_coeff[6] = g6;
    green_sh_coeff[7] = g7;
    green_sh_coeff[8] = g8;
}   // projectSH

// ----------------------------------------------------------------------------
/** Generate the 9 first SH coefficients for each color channel
 *  using the cubemap provided by CubemapFace.
 *  \param textures sequence of 6 square textures.
 *  \param row/columns count of textures.
 */
void generateSphericalHarmonics(Color *cubemap_face[6], size_t edge_size,
                                float *blue_sh_coeff, float *green_sh_coeff,
                                float *red_sh_coeff)
{
    float *Y00[6];
    float *Y1minus1[6];
    float *Y10[6];
    float *Y11[6];
    float *Y2minus2[6];
    float *Y2minus1[6];
    float *Y20[6];
    float *Y21[6];
    float *Y22[6];

    for (unsigned face = 0; face < 6; face++)
    {
        Y00[face] = new float[edge_size * edge_size];
        Y1minus1[face] = new float[edge_size * edge_size];
        Y10[face] = new float[edge_size * edge_size];
        Y11[face] = new float[edge_size * edge_size];
        Y2minus2[face] = new float[edge_size * edge_size];
        Y2minus1[face] = new float[edge_size * edge_size];
        Y20[face] = new float[edge_size * edge_size];
        Y21[face] = new float[edge_size * edge_size];
        Y22[face] = new float[edge_size * edge_size];

        getYml(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, edge_size, Y00[face],
               Y1minus1[face], Y10[face], Y11[face], Y2minus2[face],
               Y2minus1[face], Y20[face], Y21[face], Y22[face]);
    }

    projectSH(cubemap_face, edge_size, Y00, Y1minus1, Y10, Y11, Y2minus2,
              Y2minus1, Y20, Y21, Y22, blue_sh_coeff, green_sh_coeff,
              red_sh_coeff);

    for (unsigned face = 0; face < 6; face++)
    {
        delete[] Y00[face];
        delete[] Y1minus1[face];
        delete[] Y10[face];
        delete[] Y11[face];
        delete[] Y2minus2[face];
        delete[] Y2minus1[face];
        delete[] Y20[face];
        delete[] Y21[face];
        delete[] Y22[face];
    }
}   // generateSphericalHarmonics

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

/** Returns a pseudo random (theta, phi) generated from a probability density function modeled after cos distribtion function.
\param a pseudo random float pair from a uniform density function between 0 and 1. */
std::pair<float, float> ImportanceSamplingCos(std::pair<float, float> Seeds)
{    return std::make_pair(acosf(Seeds.first), 2.f * 3.14f * Seeds.second);
}

/** Returns a pseudo random (theta, phi) generated from a probability density function modeled after GGX distribtion function.
    From "Real Shading in Unreal Engine 4" paper
\param a pseudo random float pair from a uniform density function between 0 and 1.
\param exponent from the Phong formula. */
std::pair<float, float> ImportanceSamplingGGX(std::pair<float, float> Seeds, float roughness)
{
    float a = roughness * roughness;
    float CosTheta = sqrtf((1.f - Seeds.second) / (1.f + (a * a - 1.f) * Seeds.second));
    return std::make_pair(acosf(CosTheta), 2.f * 3.14f * Seeds.first);
}

static float G1_Schlick(const core::vector3df &V, const core::vector3df &normal, float k)
{
    float NdotV = V.dotProduct(normal);
    NdotV = NdotV > 0.f ? NdotV : 0.f;
    NdotV = NdotV < 1.f ? NdotV : 1.f;
    return 1.f / (NdotV * (1.f - k) + k);
}

float G_Smith(const core::vector3df &lightdir, const core::vector3df &viewdir, const core::vector3df &normal, float roughness)
{
    float k = (roughness + 1.f) * (roughness + 1.f) / 8.f;
    return G1_Schlick(lightdir, normal, k) * G1_Schlick(viewdir, normal, k);
}


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

// ----------------------------------------------------------------------------
GLuint generateSpecularCubemap(GLuint probe)
{
    GLuint cubemap_texture;

    glGenTextures(1, &cubemap_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
    size_t cubemap_size = 256;
    for (int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F,
                     cubemap_size, cubemap_size, 0, GL_BGRA, GL_FLOAT, 0);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    if (!CVS->isDefferedEnabled())
        return cubemap_texture;

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
                                   cubemap_texture, level);
            GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            assert(status == GL_FRAMEBUFFER_COMPLETE);

            SpecularIBLGenerator::getInstance()->setTextureUnits(probe);
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
    return cubemap_texture;
}   // generateSpecularCubemap



static
std::pair<float, float> getSpecularDFG(float roughness, float NdotV)
{
    // We assume a local referential where N points in Y direction
    core::vector3df V(sqrtf(1.f - NdotV * NdotV), NdotV, 0.f);

    float DFG1 = 0., DFG2 = 0.;
    for (unsigned sample = 0; sample < 1024; sample++)
    {
        std::pair<float, float> ThetaPhi = ImportanceSamplingGGX(getHammersleySequence(sample, 1024), roughness);
        float Theta = ThetaPhi.first, Phi = ThetaPhi.second;
        core::vector3df H(sinf(Theta) * cosf(Phi), cosf(Theta), sinf(Theta) * sinf(Phi));
        core::vector3df L = 2 * H.dotProduct(V) * H - V;
        float NdotL = L.Y;
        if (NdotL > 0.)
        {
            float VdotH = V.dotProduct(H);
            VdotH = VdotH > 0.f ? VdotH : 0.f;
            VdotH = VdotH < 1.f ? VdotH : 1.f;
            float Fc = powf(1.f - VdotH, 5.f);
            float G = G_Smith(L, V, core::vector3df(0.f, 1.f, 0.f), roughness);
            DFG1 += (1.f - Fc) * G * VdotH;
            DFG2 += Fc * G * VdotH;
        }
    }
    return std::make_pair(DFG1 / 1024, DFG2 / 1024);
}


static float getDiffuseDFG(float roughness, float NdotV)
{
    // We assume a local referential where N points in Y direction
    core::vector3df V(sqrtf(1.f - NdotV * NdotV), NdotV, 0.f);
    float DFG = 0.f;
    for (unsigned sample = 0; sample < 1024; sample++)
    {
        std::pair<float, float> ThetaPhi = ImportanceSamplingCos(getHammersleySequence(sample, 1024));
        float Theta = ThetaPhi.first, Phi = ThetaPhi.second;
        core::vector3df L(sinf(Theta) * cosf(Phi), cosf(Theta), sinf(Theta) * sinf(Phi));
        float NdotL = L.Y;
        if (NdotL > 0.f)
        {
            core::vector3df H = (L + V).normalize();
            float LdotH = L.dotProduct(H);
            float f90 = .5f + 2.f * LdotH * LdotH * roughness * roughness;
            DFG += (1.f + (f90 - 1.f) * (1.f - powf(NdotL, 5.f))) * (1.f + (f90 - 1.f) * (1.f - powf(NdotV, 5.f)));
        }
    }
    return DFG / 1024;
}


GLuint generateSpecularDFGLUT()
{
    size_t DFG_LUT_size = 128;
    float *texture_content = new float[4 * DFG_LUT_size * DFG_LUT_size];

#pragma omp parallel for
    for (int i = 0; i < int(DFG_LUT_size); i++)
    {
        float roughness = .05f + .95f * float(i) / float(DFG_LUT_size - 1);
        for (unsigned j = 0; j < DFG_LUT_size; j++)
        {
            float NdotV = float(j) / float(DFG_LUT_size - 1);
            std::pair<float, float> DFG = getSpecularDFG(roughness, NdotV);
            texture_content[4 * (i * DFG_LUT_size + j)] = DFG.first;
            texture_content[4 * (i * DFG_LUT_size + j) + 1] = DFG.second;
            texture_content[4 * (i * DFG_LUT_size + j) + 2] = getDiffuseDFG(roughness, NdotV);
            texture_content[4 * (i * DFG_LUT_size + j) + 3] = 0.;
        }
    }

    GLuint DFG_LUT_texture;
    glGenTextures(1, &DFG_LUT_texture);
    glBindTexture(GL_TEXTURE_2D, DFG_LUT_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, DFG_LUT_size, DFG_LUT_size, 0, GL_RGBA, GL_FLOAT, texture_content);

    delete[] texture_content;
    return DFG_LUT_texture;

}