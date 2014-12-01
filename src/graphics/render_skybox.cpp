#include "graphics/IBL.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/shaders.hpp"
#include "modes/world.hpp"
#include "utils/profiler.hpp"

#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN2(a, b) ((a) > (b) ? (b) : (a))

static float getTexelValue(unsigned i, unsigned j, size_t width, size_t height, float *Coeff, float *Y00, float *Y1minus1, float *Y10, float *Y11,
    float *Y2minus2, float * Y2minus1, float * Y20, float *Y21, float *Y22)
{
    float solidangle = 1.;
    size_t idx = i * height + j;
    float reconstructedVal = Y00[idx] * Coeff[0];
    reconstructedVal += Y1minus1[i * height + j] * Coeff[1] + Y10[i * height + j] * Coeff[2] + Y11[i * height + j] * Coeff[3];
    reconstructedVal += Y2minus2[idx] * Coeff[4] + Y2minus1[idx] * Coeff[5] + Y20[idx] * Coeff[6] + Y21[idx] * Coeff[7] + Y22[idx] * Coeff[8];
    reconstructedVal /= solidangle;
    return MAX2(255.0f * reconstructedVal, 0.f);
}

static void unprojectSH(float *output[], size_t width, size_t height,
    float *Y00[],
    float *Y1minus1[], float *Y10[], float *Y11[],
    float *Y2minus2[], float *Y2minus1[], float * Y20[], float *Y21[], float *Y22[],
    float *blueSHCoeff, float *greenSHCoeff, float *redSHCoeff)
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

                output[face][4 * height * i + 4 * j + 2] = getTexelValue(i, j, width, height,
                    redSHCoeff,
                    Y00[face], Y1minus1[face], Y10[face], Y11[face], Y2minus2[face], Y2minus1[face], Y20[face], Y21[face], Y22[face]);
                output[face][4 * height * i + 4 * j + 1] = getTexelValue(i, j, width, height,
                    greenSHCoeff,
                    Y00[face], Y1minus1[face], Y10[face], Y11[face], Y2minus2[face], Y2minus1[face], Y20[face], Y21[face], Y22[face]);
                output[face][4 * height * i + 4 * j] = getTexelValue(i, j, width, height,
                    blueSHCoeff,
                    Y00[face], Y1minus1[face], Y10[face], Y11[face], Y2minus2[face], Y2minus1[face], Y20[face], Y21[face], Y22[face]);
            }
        }
    }
}


static void displayCoeff(float *SHCoeff)
{
    printf("L00:%f\n", SHCoeff[0]);
    printf("L1-1:%f, L10:%f, L11:%f\n", SHCoeff[1], SHCoeff[2], SHCoeff[3]);
    printf("L2-2:%f, L2-1:%f, L20:%f, L21:%f, L22:%f\n", SHCoeff[4], SHCoeff[5], SHCoeff[6], SHCoeff[7], SHCoeff[8]);
}

// Only for 9 coefficients
/*static void testSH(unsigned char *color[6], size_t width, size_t height,
    float *blueSHCoeff, float *greenSHCoeff, float *redSHCoeff)
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

    float *testoutput[6];
    for (unsigned i = 0; i < 6; i++)
    {
        testoutput[i] = new float[width * height * 4];
        for (unsigned j = 0; j < width * height; j++)
        {
            testoutput[i][4 * j] = float(0xFF & color[i][4 * j]);
            testoutput[i][4 * j + 1] = float(0xFF & color[i][4 * j + 1]);
            testoutput[i][4 * j + 2] = float(0xFF & color[i][4 * j + 2]);
        }
    }

    for (unsigned face = 0; face < 6; face++)
    {
        Y00[face] = new float[width * height];
        Y1minus1[face] = new float[width * height];
        Y10[face] = new float[width * height];
        Y11[face] = new float[width * height];
        Y2minus2[face] = new float[width * height];
        Y2minus1[face] = new float[width * height];
        Y20[face] = new float[width * height];
        Y21[face] = new float[width * height];
        Y22[face] = new float[width * height];

        getYml(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, width, height, Y00[face], Y1minus1[face], Y10[face], Y11[face], Y2minus2[face], Y2minus1[face], Y20[face], Y21[face], Y22[face]);
    }

        blueSHCoeff[0] = 0.54,
    blueSHCoeff[1] = .6, blueSHCoeff[2] = -.27, blueSHCoeff[3] = .01,
    blueSHCoeff[4] = -.12, blueSHCoeff[5] = -.47, blueSHCoeff[6] = -.15, blueSHCoeff[7] = .14, blueSHCoeff[8] = -.3;
    greenSHCoeff[0] = .44,
    greenSHCoeff[1] = .35, greenSHCoeff[2] = -.18, greenSHCoeff[3] = -.06,
    greenSHCoeff[4] = -.05, greenSHCoeff[5] = -.22, greenSHCoeff[6] = -.09, greenSHCoeff[7] = .21, greenSHCoeff[8] = -.05;
    redSHCoeff[0] = .79,
    redSHCoeff[1] = .39, redSHCoeff[2] = -.34, redSHCoeff[3] = -.29,
    redSHCoeff[4] = -.11, redSHCoeff[5] = -.26, redSHCoeff[6] = -.16, redSHCoeff[7] = .56, redSHCoeff[8] = .21;

    printf("Blue:\n");
    displayCoeff(blueSHCoeff);
    printf("Green:\n");
    displayCoeff(greenSHCoeff);
    printf("Red:\n");
    displayCoeff(redSHCoeff);*/

/*    projectSH(testoutput, width, height,
        Y00,
        Y1minus1, Y10, Y11,
        Y2minus2, Y2minus1, Y20, Y21, Y22,
        blueSHCoeff, greenSHCoeff, redSHCoeff
        );*/

    //printf("Blue:\n");
    //displayCoeff(blueSHCoeff);
    //printf("Green:\n");
    //displayCoeff(greenSHCoeff);
    //printf("Red:\n");
    //displayCoeff(redSHCoeff);



    // Convolute in frequency space
    /*    float A0 = 3.141593;
    float A1 = 2.094395;
    float A2 = 0.785398;
    blueSHCoeff[0] *= A0;
    greenSHCoeff[0] *= A0;
    redSHCoeff[0] *= A0;
    for (unsigned i = 0; i < 3; i++)
    {
    blueSHCoeff[1 + i] *= A1;
    greenSHCoeff[1 + i] *= A1;
    redSHCoeff[1 + i] *= A1;
    }
    for (unsigned i = 0; i < 5; i++)
    {
    blueSHCoeff[4 + i] *= A2;
    greenSHCoeff[4 + i] *= A2;
    redSHCoeff[4 + i] *= A2;
    }


    unprojectSH(testoutput, width, height,
    Y00,
    Y1minus1, Y10, Y11,
    Y2minus2, Y2minus1, Y20, Y21, Y22,
    blueSHCoeff, greenSHCoeff, redSHCoeff
    );*/


    /*    printf("Blue:\n");
    displayCoeff(blueSHCoeff);
    printf("Green:\n");
    displayCoeff(greenSHCoeff);
    printf("Red:\n");
    displayCoeff(redSHCoeff);

    printf("\nAfter projection\n\n");



    for (unsigned i = 0; i < 6; i++)
    {
        for (unsigned j = 0; j < width * height; j++)
        {
            color[i][4 * j] = char(MIN2(testoutput[i][4 * j], 255));
            color[i][4 * j + 1] = char(MIN2(testoutput[i][4 * j + 1], 255));
            color[i][4 * j + 2] = char(MIN2(testoutput[i][4 * j + 2], 255));
        }
    }

    for (unsigned face = 0; face < 6; face++)
    {
        delete[] testoutput[face];
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
}*/

void swapPixels(char *old_img, char *new_img, unsigned stride, unsigned old_i, unsigned old_j, unsigned new_i, unsigned new_j)
{
    new_img[4 * (stride * new_i + new_j)] = old_img[4 * (stride * old_i + old_j)];
    new_img[4 * (stride * new_i + new_j) + 1] = old_img[4 * (stride * old_i + old_j) + 1];
    new_img[4 * (stride * new_i + new_j) + 2] = old_img[4 * (stride * old_i + old_j) + 2];
    new_img[4 * (stride * new_i + new_j) + 3] = old_img[4 * (stride * old_i + old_j) + 3];
}


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
        size = MAX2(size, textures[i]->getOriginalSize().Width);
        size = MAX2(size, textures[i]->getOriginalSize().Height);
    }

    const unsigned texture_permutation[] = { 2, 3, 0, 1, 5, 4 };
    char *rgba[6];
    for (unsigned i = 0; i < 6; i++)
        rgba[i] = new char[size * size * 4];
    for (unsigned i = 0; i < 6; i++)
    {
        unsigned idx = texture_permutation[i];

        video::IImage* image = irr_driver->getVideoDriver()->createImageFromData(
            textures[idx]->getColorFormat(),
            textures[idx]->getSize(),
            textures[idx]->lock(),
            false
            );
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
        if (irr_driver->usesTextureCompression())
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_COMPRESSED_SRGB_ALPHA, size, size, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)rgba[i]);
        else
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB_ALPHA, size, size, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)rgba[i]);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    for (unsigned i = 0; i < 6; i++)
        delete[] rgba[i];
    return result;
}

void IrrDriver::generateSkyboxCubemap()
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    assert(SkyboxTextures.size() == 6);
    SkyboxCubeMap = generateCubeMapFromTextures(SkyboxTextures);
}

void IrrDriver::generateDiffuseCoefficients()
{
    if (!m_SH_dirty)
        return;
    m_SH_dirty = false;
    const unsigned texture_permutation[] = { 2, 3, 0, 1, 5, 4 };

    unsigned sh_w = 0, sh_h = 0;
    unsigned char *sh_rgba[6];

    if (SphericalHarmonicsTextures.size() == 6)
    {

        for (unsigned i = 0; i < 6; i++)
        {
            sh_w = MAX2(sh_w, SphericalHarmonicsTextures[i]->getOriginalSize().Width);
            sh_h = MAX2(sh_h, SphericalHarmonicsTextures[i]->getOriginalSize().Height);
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
        int sh_w = 16;
        int sh_h = 16;

        video::SColor ambient = m_scene_manager->getAmbientLight().toSColor();

        for (unsigned i = 0; i < 6; i++)
        {
            sh_rgba[i] = new unsigned char[sh_w * sh_h * 4];

            for (int j = 0; j < sh_w * sh_h * 4; j += 4)
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

    SphericalHarmonics(FloatTexCube, sh_w, blueSHCoeff, greenSHCoeff, redSHCoeff);

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
}

void IrrDriver::renderSkybox(const scene::ICameraSceneNode *camera)
{
    if (SkyboxTextures.empty())
        return;
    if (!SkyboxCubeMap)
        generateSkyboxCubemap();
    glBindVertexArray(MeshShader::SkyboxShader::getInstance()->cubevao);
    glDisable(GL_CULL_FACE);
    assert(SkyboxTextures.size() == 6);

    core::matrix4 translate;
    translate.setTranslation(camera->getAbsolutePosition());

    // Draw the sky box between the near and far clip plane
    const f32 viewDistance = (camera->getNearValue() + camera->getFarValue()) * 0.5f;
    core::matrix4 scale;
    scale.setScale(core::vector3df(viewDistance, viewDistance, viewDistance));
    core::matrix4 transform = translate * scale;
    core::matrix4 invtransform;
    transform.getInverse(invtransform);
    glDisable(GL_BLEND);

    glUseProgram(MeshShader::SkyboxShader::getInstance()->Program);
    MeshShader::SkyboxShader::getInstance()->setUniforms(transform);
    MeshShader::SkyboxShader::getInstance()->SetTextureUnits(SkyboxCubeMap);

    glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
