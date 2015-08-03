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

