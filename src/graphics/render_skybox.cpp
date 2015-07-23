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
void IrrDriver::renderSkybox(const scene::ICameraSceneNode *camera)
{
    if(m_skybox)
    {
        m_skybox->render(camera);
    }
}   // renderSkybox
