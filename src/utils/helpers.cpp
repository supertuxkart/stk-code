//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Lauri Kasanen
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

#include "graphics/irr_driver.hpp"
#include "utils/helpers.hpp"

#include <math.h>
#include <algorithm>

/*

Copyright (C) 2011 by-2015 Ashima Arts (Simplex noise)
Copyright (C) 2011 by-2015 Lauri Kasanen (cpu port)


Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

static inline float mod289(float x) {
//    return x - floorf(x * (1.0 / 289.0)) * 289.0;
    return fmodf(x, 289);
}

static inline float permute(float x) {
    return mod289(((x*34.0f)+1.0f)*x);
}

// Vectorized 2d simplex noise.
float noise2d(float v1, float v2) {

    const float C[] = {
        0.211324865405187f,
        0.366025403784439f,
        -0.577350269189626f,
        0.024390243902439f };

    // First corner
    float i[2];
    i[0] = floorf(v1 + v1*C[1] + v2*C[1]);
    i[1] = floorf(v2 + v1*C[1] + v2*C[1]);

    float x0[2];
    x0[0] = v1 - i[0] + i[0]*C[0] + i[1]*C[0];
    x0[1] = v2 - i[1] + i[0]*C[0] + i[1]*C[0];

    // Other corners
    float i1[2];
    if (x0[0] > x0[1]) {
        i1[0] = 1;
        i1[1] = 0;
    } else {
        i1[0] = 0;
        i1[1] = 1;
    }

    float x12[4];
    x12[0] = x0[0] + C[0] - i1[0];
    x12[1] = x0[1] + C[0] - i1[1];
    x12[2] = x0[0] + C[2];
    x12[3] = x0[1] + C[2];

    // Permutations
    i[0] = mod289(i[0]);
    i[1] = mod289(i[1]);

    float p[3];
    p[0] = permute(permute(i[1]) + i[0]);
    p[1] = permute(permute(i[1] + i1[1]) + i[0] + i1[0]);
    p[2] = permute(permute(i[1] + 1) + i[0] + 1);

    float m[3];
    m[0] = std::max<float>(0.5f - x0[0]*x0[0] - x0[1]*x0[1], 0);
    m[1] = std::max<float>(0.5f - x12[0]*x12[0] - x12[1]*x12[1], 0);
    m[2] = std::max<float>(0.5f - x12[2]*x12[2] - x12[3]*x12[3], 0);

    m[0] = m[0] * m[0] * m[0] * m[0];
    m[1] = m[1] * m[1] * m[1] * m[1];
    m[2] = m[2] * m[2] * m[2] * m[2];

    // Gradients
    float tmp;

    float x[3];
    x[0] = 2 * modff(p[0] * C[3], &tmp) - 1;
    x[1] = 2 * modff(p[1] * C[3], &tmp) - 1;
    x[2] = 2 * modff(p[2] * C[3], &tmp) - 1;

    float h[3];
    h[0] = fabsf(x[0]) - 0.5f;
    h[1] = fabsf(x[1]) - 0.5f;
    h[2] = fabsf(x[2]) - 0.5f;

    float ox[3];
    ox[0] = floorf(x[0] + 0.5f);
    ox[1] = floorf(x[1] + 0.5f);
    ox[2] = floorf(x[2] + 0.5f);

    float a0[3];
    a0[0] = x[0] - ox[0];
    a0[1] = x[1] - ox[1];
    a0[2] = x[2] - ox[2];

    // Normalize
    m[0] *= 1.79284291400159f - 0.85373472095314f * (a0[0]*a0[0] + h[0]*h[0]);
    m[1] *= 1.79284291400159f - 0.85373472095314f * (a0[1]*a0[1] + h[1]*h[1]);
    m[2] *= 1.79284291400159f - 0.85373472095314f * (a0[2]*a0[2] + h[2]*h[2]);

    // Compute final value
    float g[3];
    g[0] = a0[0] * x0[0] + h[0] * x0[1];
    g[1] = a0[1] * x12[0] + h[1] * x12[1];
    g[2] = a0[2] * x12[2] + h[2] * x12[3];

    return 130 * (m[0] * g[0] + m[1] * g[1] + m[2] * g[2]);
}
