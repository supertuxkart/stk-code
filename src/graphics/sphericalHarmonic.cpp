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


#include "graphics/IBL.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/sphericalHarmonic.hpp"
#include "utils/log.hpp"

#include <algorithm> 
#include <cassert>


#include <irrlicht.h>

using namespace irr;

namespace 
{ 
    void convertToFloatTexture(unsigned char *sh_rgba[6], unsigned sh_w, unsigned sh_h, Color *float_tex_cube[6])
    {
        for (unsigned i = 0; i < 6; i++)
        {
            float_tex_cube[i] = new Color[sh_w * sh_h];
            for (unsigned j = 0; j < sh_w * sh_h; j++)
            {
                float_tex_cube[i][j].Blue = powf(float(0xFF & sh_rgba[i][4 * j]) / 255.f, 2.2f);
                float_tex_cube[i][j].Green = powf(float(0xFF & sh_rgba[i][4 * j + 1]) / 255.f, 2.2f);
                float_tex_cube[i][j].Red = powf(float(0xFF & sh_rgba[i][4 * j + 2]) / 255.f, 2.2f);
            }
        }    
    } //convertToFloatTexture

    // ----------------------------------------------------------------------------
    void displayCoeff(float *SH_coeff)
    {
        Log::debug("SphericalHarmonic", "L00:%f", SH_coeff[0]);
        Log::debug("SphericalHarmonic", "L1-1:%f, L10:%f, L11:%f", SH_coeff[1], SH_coeff[2], SH_coeff[3]);
        Log::debug("SphericalHarmonic", "L2-2:%f, L2-1:%f, L20:%f, L21:%f, L22:%f",
                SH_coeff[4], SH_coeff[5], SH_coeff[6], SH_coeff[7], SH_coeff[8]);
    }   // displayCoeff

    // ----------------------------------------------------------------------------
    float getTexelValue(unsigned i, unsigned j, size_t width, size_t height,
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
                         +  Y11[i * height + j] * Coeff[3];
        reconstructedVal += Y2minus2[idx] * Coeff[4] 
                         + Y2minus1[idx] * Coeff[5] + Y20[idx] * Coeff[6]
                         + Y21[idx] * Coeff[7] + Y22[idx] * Coeff[8];
        reconstructedVal /= solidangle;
        return std::max(255.0f * reconstructedVal, 0.f);
    }   // getTexelValue
    
} //namespace


// ----------------------------------------------------------------------------
SphericalHarmonic::SphericalHarmonic(const std::vector<video::ITexture *> &spherical_harmonics_textures)
{
    assert(spherical_harmonics_textures.size() == 6);
    
    m_spherical_harmonics_textures = spherical_harmonics_textures;

    const unsigned texture_permutation[] = { 2, 3, 0, 1, 5, 4 };
    unsigned char *sh_rgba[6];
    unsigned sh_w = 0, sh_h = 0;

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

        video::IImage* image = irr_driver->getVideoDriver()->createImageFromData(
            m_spherical_harmonics_textures[idx]->getColorFormat(),
            m_spherical_harmonics_textures[idx]->getSize(),
            m_spherical_harmonics_textures[idx]->lock(),
            false
            );
        m_spherical_harmonics_textures[idx]->unlock();

        image->copyToScaling(sh_rgba[i], sh_w, sh_h);
        delete image;
    } //for (unsigned i = 0; i < 6; i++)

    Color *float_tex_cube[6];
    convertToFloatTexture(sh_rgba, sh_w, sh_h, float_tex_cube);
    generateSphericalHarmonics(float_tex_cube, sh_w, m_blue_SH_coeff, m_green_SH_coeff, m_red_SH_coeff);

    for (unsigned i = 0; i < 6; i++)
    {
        delete[] sh_rgba[i];
        delete[] float_tex_cube[i];
    }
    
}// SphericalHarmonic(const std::vector<video::ITexture *> &spherical_harmonics_textures)

// ----------------------------------------------------------------------------
SphericalHarmonic::SphericalHarmonic(const video::SColor &ambient)
{
    unsigned char *sh_rgba[6];
    unsigned sh_w = 16;
    unsigned sh_h = 16;

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

    Color *float_tex_cube[6];
    convertToFloatTexture(sh_rgba, sh_w, sh_h, float_tex_cube);
    generateSphericalHarmonics(float_tex_cube, sh_w, m_blue_SH_coeff, m_green_SH_coeff, m_red_SH_coeff);

    for (unsigned i = 0; i < 6; i++)
    {
        delete[] sh_rgba[i];
        delete[] float_tex_cube[i];
    }

    // Diffuse env map is x 0.25, compensate
    for (unsigned i = 0; i < 9; i++)
    {
        m_blue_SH_coeff[i] *= 4;
        m_green_SH_coeff[i] *= 4;
        m_red_SH_coeff[i] *= 4;
    }
 
}// SphericalHarmonic(const video::SColor &ambient)

// ----------------------------------------------------------------------------
void SphericalHarmonic::printCoeff() {
    Log::debug("SphericalHarmonic", "Blue_SH:");
    displayCoeff(m_blue_SH_coeff);
    Log::debug("SphericalHarmonic", "Green_SH:");
    displayCoeff(m_green_SH_coeff);
    Log::debug("SphericalHarmonic", "Red_SH:");
    displayCoeff(m_red_SH_coeff);  
} //printCoeff


// ----------------------------------------------------------------------------
void SphericalHarmonic::unprojectSH(float *output[], size_t width, size_t height,
                                    float *Y00[], float *Y1minus1[], float *Y10[],
                                    float *Y11[], float *Y2minus2[], float *Y2minus1[],
                                    float * Y20[], float *Y21[], float *Y22[])
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
                    getTexelValue(i, j, width, height, m_red_SH_coeff, Y00[face],
                                  Y1minus1[face], Y10[face], Y11[face],
                                  Y2minus2[face], Y2minus1[face], Y20[face],
                                  Y21[face], Y22[face]);
                output[face][4 * height * i + 4 * j + 1] = 
                    getTexelValue(i, j, width, height, m_green_SH_coeff, Y00[face],
                                  Y1minus1[face], Y10[face], Y11[face],
                                  Y2minus2[face], Y2minus1[face], Y20[face],
                                  Y21[face], Y22[face]);
                output[face][4 * height * i + 4 * j] = 
                    getTexelValue(i, j, width, height, m_blue_SH_coeff, Y00[face],
                                  Y1minus1[face], Y10[face], Y11[face],
                                  Y2minus2[face], Y2minus1[face], Y20[face],
                                  Y21[face], Y22[face]);
            }
        }
    }
}   // unprojectSH


