#include "IBL.hpp"
#include "gl_headers.hpp"
#include "shaders.hpp"
#include <cmath>
#include <set>

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
}


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


static void projectSH(Color *CubemapFace[6], size_t edge_size,
    float *Y00[],
    float *Y1minus1[], float *Y10[], float *Y11[],
    float *Y2minus2[], float *Y2minus1[], float * Y20[], float *Y21[], float *Y22[],
    float *blueSHCoeff, float *greenSHCoeff, float *redSHCoeff
    )
{
    for (unsigned i = 0; i < 9; i++)
    {
        blueSHCoeff[i] = 0;
        greenSHCoeff[i] = 0;
        redSHCoeff[i] = 0;
    }

    float wh = float(edge_size * edge_size);
    for (unsigned face = 0; face < 6; face++)
    {
#pragma omp parallel for
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
                float b = CubemapFace[face][edge_size * i + j].Blue;
                float g = CubemapFace[face][edge_size * i + j].Green;
                float r = CubemapFace[face][edge_size * i + j].Red;

                blueSHCoeff[0] += b * Y00[face][idx] * solidangle;
                blueSHCoeff[1] += b * Y1minus1[face][idx] * solidangle;
                blueSHCoeff[2] += b * Y10[face][idx] * solidangle;
                blueSHCoeff[3] += b * Y11[face][idx] * solidangle;
                blueSHCoeff[4] += b * Y2minus2[face][idx] * solidangle;
                blueSHCoeff[5] += b * Y2minus1[face][idx] * solidangle;
                blueSHCoeff[6] += b * Y20[face][idx] * solidangle;
                blueSHCoeff[7] += b * Y21[face][idx] * solidangle;
                blueSHCoeff[8] += b * Y22[face][idx] * solidangle;

                greenSHCoeff[0] += g * Y00[face][idx] * solidangle;
                greenSHCoeff[1] += g * Y1minus1[face][idx] * solidangle;
                greenSHCoeff[2] += g * Y10[face][idx] * solidangle;
                greenSHCoeff[3] += g * Y11[face][idx] * solidangle;
                greenSHCoeff[4] += g * Y2minus2[face][idx] * solidangle;
                greenSHCoeff[5] += g * Y2minus1[face][idx] * solidangle;
                greenSHCoeff[6] += g * Y20[face][idx] * solidangle;
                greenSHCoeff[7] += g * Y21[face][idx] * solidangle;
                greenSHCoeff[8] += g * Y22[face][idx] * solidangle;


                redSHCoeff[0] += r * Y00[face][idx] * solidangle;
                redSHCoeff[1] += r * Y1minus1[face][idx] * solidangle;
                redSHCoeff[2] += r * Y10[face][idx] * solidangle;
                redSHCoeff[3] += r * Y11[face][idx] * solidangle;
                redSHCoeff[4] += r * Y2minus2[face][idx] * solidangle;
                redSHCoeff[5] += r * Y2minus1[face][idx] * solidangle;
                redSHCoeff[6] += r * Y20[face][idx] * solidangle;
                redSHCoeff[7] += r * Y21[face][idx] * solidangle;
                redSHCoeff[8] += r * Y22[face][idx] * solidangle;
            }
        }
    }
}

void SphericalHarmonics(Color *CubemapFace[6], size_t edge_size, float *blueSHCoeff, float *greenSHCoeff, float *redSHCoeff)
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

        getYml(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, edge_size, Y00[face], Y1minus1[face], Y10[face], Y11[face], Y2minus2[face], Y2minus1[face], Y20[face], Y21[face], Y22[face]);
    }

    projectSH(CubemapFace, edge_size,
        Y00,
        Y1minus1, Y10, Y11,
        Y2minus2, Y2minus1, Y20, Y21, Y22,
        blueSHCoeff, greenSHCoeff, redSHCoeff
        );

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
}

// From http://http.developer.nvidia.com/GPUGems3/gpugems3_ch20.html
/** Returns the index-th pair from Hammersley set of pseudo random set.
    Hammersley set is a uniform distribution between 0 and 1 for 2 components.
    We use the natural indexation on the set to avoid storing the whole set.
    \param index of the pair
    \param size of the set. */
std::pair<float, float> HammersleySequence(int index, int samples)
{
    float InvertedBinaryRepresentation = 0.;
    for (size_t i = 0; i < 32; i++)
    {
        InvertedBinaryRepresentation += ((index >> i) & 0x1) * powf(.5, (float) (i + 1.));
    }
    return std::make_pair(float(index) / float(samples), InvertedBinaryRepresentation);
}


/** Returns a pseudo random (theta, phi) generated from a probability density function modeled after Phong function.
    \param a pseudo random float pair from a uniform density function between 0 and 1.
    \param exponent from the Phong formula. */
std::pair<float, float> ImportanceSamplingPhong(std::pair<float, float> Seeds, float exponent)
{
    return std::make_pair(acosf(powf(Seeds.first, 1.f / (exponent + 1.f))), 2.f * 3.14f * Seeds.second);
}

static
core::matrix4 getPermutationMatrix(size_t indexX, float valX, size_t indexY, float valY, size_t indexZ, float valZ)
{
    core::matrix4 resultMat;
    float *M = resultMat.pointer();
    memset(M, 0, 16 * sizeof(float));
    assert(indexX < 4);
    assert(indexY < 4);
    assert(indexZ < 4);
    M[indexX] = valX;
    M[4 + indexY] = valY;
    M[8 + indexZ] = valZ;
    return resultMat;
}

GLuint generateSpecularCubemap(GLuint probe)
{
    GLuint cubemap_texture;

    glGenTextures(1, &cubemap_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
    size_t cubemap_size = 256;
    for (int i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, cubemap_size, cubemap_size, 0, GL_BGRA, GL_FLOAT, 0);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    if (!UserConfigParams::m_dynamic_lights)
        return cubemap_texture;

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, cubemap_size, cubemap_size);
    GLenum bufs[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, bufs);
    glUseProgram(UtilShader::SpecularIBLGenerator::getInstance()->Program);

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
        // Blinn Phong can be approximated by Phong with 4x the specular coefficient
        // See http://seblagarde.wordpress.com/2012/03/29/relationship-between-phong-and-blinn-lighting-model/
        // NOTE : Removed because it makes too sharp reflexion
        float roughness = (8 - level) * pow(2.f, 10.f) / 8.f;
        float viewportSize = float(1 << (8 - level));

        float *tmp = new float[2048];
        for (unsigned i = 0; i < 1024; i++)
        {
            std::pair<float, float> sample = ImportanceSamplingPhong(HammersleySequence(i, 1024), roughness);
            tmp[2 * i] = sample.first;
            tmp[2 * i + 1] = sample.second;
        }

        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0 + UtilShader::SpecularIBLGenerator::getInstance()->TU_Samples);
        GLuint sampleTex, sampleBuffer;
        glGenBuffers(1, &sampleBuffer);
        glBindBuffer(GL_TEXTURE_BUFFER, sampleBuffer);
        glBufferData(GL_TEXTURE_BUFFER, 2048 * sizeof(float), tmp, GL_STATIC_DRAW);
        glGenTextures(1, &sampleTex);
        glBindTexture(GL_TEXTURE_BUFFER, sampleTex);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, sampleBuffer);
        glBindVertexArray(SharedObject::FullScreenQuadVAO);

        for (unsigned face = 0; face < 6; face++)
        {

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, cubemap_texture, level);
            GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            assert(status == GL_FRAMEBUFFER_COMPLETE);

            UtilShader::SpecularIBLGenerator::getInstance()->SetTextureUnits(probe);
            UtilShader::SpecularIBLGenerator::getInstance()->setUniforms(M[face], viewportSize);

            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        glActiveTexture(GL_TEXTURE0 + UtilShader::SpecularIBLGenerator::getInstance()->TU_Samples);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
        glBindTexture(GL_TEXTURE_BUFFER, 0);

        delete[] tmp;
        glDeleteTextures(1, &sampleTex);
        glDeleteBuffers(1, &sampleBuffer);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    return cubemap_texture;
}