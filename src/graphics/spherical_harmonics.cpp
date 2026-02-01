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

#ifndef SERVER_ONLY

#include "graphics/spherical_harmonics.hpp"
#if defined(USE_GLES2)
#include "graphics/central_settings.hpp"
#endif
#include "graphics/irr_driver.hpp"
#include "utils/log.hpp"

#include <algorithm> 
#include <cassert>

#if __SSE2__ || _M_X64 || _M_IX86_FP >= 2
 #include <emmintrin.h>
 #define SIMD_SSE2_SUPPORT (1)
#endif
#if __SSE4_1__ || __AVX__
 #include <smmintrin.h>
 #define SIMD_SSE4_1_SUPPORT (1)
#endif

#if SIMD_SSE4_1_SUPPORT
 #include <smmintrin.h>
 #define SIMD_BLENDV_PS(x,y,mask) _mm_blendv_ps(x,y,mask)
#elif SIMD_SSE2_SUPPORT
 #define SIMD_BLENDV_PS(x,y,mask) _mm_or_ps(_mm_andnot_ps(mask,x),_mm_and_ps(y,mask))
#endif

#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
 #define ALWAYSINLINE __attribute__((always_inline))
 #define SIMD_ALIGN16 __attribute__((aligned(16)))
#elif defined(_MSC_VER)
 #define SIMD_ALIGN16 __declspec(align(16))
 #define ALWAYSINLINE __forceinline
#else
 #define ALWAYSINLINE
#endif


using namespace irr;

namespace 
{

    #if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(__i386__) || defined(__i386)  || defined(i386)

    /* Input is 0.0,255.0, output is 0.0,1.0 */
    static inline ALWAYSINLINE float srgb2linear( float v )
    {
        float v2, vpow, vpwsqrt;
        union
        {
            int32_t i;
            float f;
        } u;
        if( v <= (0.04045f*255.0f) )
            v = v * ( (1.0f/12.92f)*(1.0f/255.0f) );
        else
        {
            v = ( v + (0.055f*255.0f) ) * ( (1.0f/1.055f)*(1.0f/255.0f) );
            v2 = v * v;
            u.f = v * 5417434112.0f;
            u.i = (int32_t)( ( (float)u.i * 0.8f ) + 0.5f );
            vpow = u.f;
            vpwsqrt = sqrtf( vpow );
            v = ( ( v2 * vpwsqrt ) + ( ( ( v2 * v ) / vpwsqrt ) / sqrtf( vpwsqrt ) ) ) * 0.51011878327f;
        }
        return v;
    }

    #else

    /* Input is 0.0,255.0, output is 0.0,1.0 */
    /* Only for reference, this is waayyy too slow and should never be used */
    static inline ALWAYSINLINE float srgb2linear( float v )
    {
        v *= (1.0f/255.0f);
        if( v <= 0.04045f )
            v = v * (1.0f/12.92f);
        else
            v = pow( ( v + 0.055f ) * (1.0f/1.055f), 2.4f );
        return v;
    }

    #endif

    #if SIMD_SSE2_SUPPORT

    static const float SIMD_ALIGN16 srgbLinearConst00[4] = { 0.04045f*255.0f, 0.04045f*255.0f, 0.04045f*255.0f, 0.04045f*255.0f };
    static const float SIMD_ALIGN16 srgbLinearConst01[4] = { (1.0f/12.92f)*(1.0f/255.0f), (1.0f/12.92f)*(1.0f/255.0f), (1.0f/12.92f)*(1.0f/255.0f), (1.0f/12.92f)*(1.0f/255.0f) };
    static const float SIMD_ALIGN16 srgbLinearConst02[4] = { 0.055f*255.0f, 0.055f*255.0f, 0.055f*255.0f, 0.055f*255.0f };
    static const float SIMD_ALIGN16 srgbLinearConst03[4] = { (1.0f/1.055f)*(1.0f/255.0f), (1.0f/1.055f)*(1.0f/255.0f), (1.0f/1.055f)*(1.0f/255.0f), (1.0f/1.055f)*(1.0f/255.0f) };
    static const float SIMD_ALIGN16 srgbLinearConst04[4] = { 5417434112.0f, 5417434112.0f, 5417434112.0f, 5417434112.0f };
    static const float SIMD_ALIGN16 srgbLinearConst05[4] = { 0.8f, 0.8f, 0.8f, 0.8f };
    static const float SIMD_ALIGN16 srgbLinearConst06[4] = { 0.51011878327f, 0.51011878327f, 0.51011878327f, 0.51011878327f };

    /* Input is 0.0,255.0 ~ output is 0.0,1.0 */
    static inline ALWAYSINLINE __m128 srgb2linear4( __m128 vx )
    {
        __m128 vmask, vbase;
        __m128 vpow, vpwsqrtinv, vpwsqrt, vx2;
        vmask = _mm_cmple_ps( vx, *(__m128*)srgbLinearConst00 );
        vbase = _mm_mul_ps( vx, *(__m128*)srgbLinearConst01 );
        vx = _mm_mul_ps( _mm_add_ps( vx, *(__m128*)srgbLinearConst02 ), *(__m128*)srgbLinearConst03 );
        vx2 = _mm_mul_ps( vx, vx );
        vpow = _mm_castsi128_ps( _mm_cvtps_epi32( _mm_mul_ps( _mm_cvtepi32_ps( _mm_castps_si128( _mm_mul_ps( vx, *(__m128*)srgbLinearConst04 ) ) ), *(__m128*)srgbLinearConst05 ) ) );
        vpwsqrtinv = _mm_rsqrt_ps( vpow );
        vpwsqrt = _mm_mul_ps( vpow, vpwsqrtinv );
        vx = _mm_mul_ps( _mm_add_ps( _mm_mul_ps( vx2, vpwsqrt ), _mm_mul_ps( _mm_mul_ps( _mm_mul_ps( vx2, vx ), vpwsqrtinv ), _mm_rsqrt_ps( vpwsqrt ) ) ), *(__m128*)srgbLinearConst06 );
        return SIMD_BLENDV_PS( vx, vbase, vmask );
    }

    #endif

    // ------------------------------------------------------------------------
    /** Print the nine first spherical harmonics coefficients
     *  \param SH_coeff The nine spherical harmonics coefficients
     */  
    void displayCoeff(float *SH_coeff)
    {
        Log::debug("SphericalHarmonics", "L00:%f", SH_coeff[0]);
        Log::debug("SphericalHarmonics", "L1-1:%f, L10:%f, L11:%f", 
                   SH_coeff[1], SH_coeff[2], SH_coeff[3]);
        Log::debug("SphericalHarmonics", "L2-2:%f, L2-1:%f, L20:%f, L21:%f, L22:%f",
                   SH_coeff[4], SH_coeff[5], SH_coeff[6], SH_coeff[7], SH_coeff[8]);
    }   // displayCoeff

    // ------------------------------------------------------------------------
    /** Compute the value of the (i,j) texel of the environment map
     * from the spherical harmonics coefficients
     *  \param i The texel line
     *  \param j The texel column
     *  \param width The texture width
     *  \param height The texture height
     *  \param Coeff The 9 first SH coefficients for a color channel (blue, green or red)
     *  \param Yml The sphericals harmonics functions values on each texel of the cubemap
     */
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
    
    // ------------------------------------------------------------------------
    /** Return a normalized vector aiming at a texel on a cubemap
     *  \param face The face of the cubemap
     *  \param j The texel line in the face
     *  \param j The texel column in the face
     *  \param x The x vector component
     *  \param y The y vector component
     *  \param z The z vector component
     */    
    void getXYZ(GLenum face, float i, float j, float &x, float &y, float &z)
    {
        float norminv;
        switch (face)
        {
        case 0: // PosX
            x = 1.0f;
            y = -i;
            z = -j;
            break;
        case 1: // NegX
            x = -1.0f;
            y = -i;
            z = j;
            break;
        case 2: // PosY
            x = j;
            y = 1.0f;
            z = i;
            break;
        case 3: // NegY
            x = j;
            y = -1.0f;
            z = -i;
            break;
        case 4: // PosZ
            x = j;
            y = -i;
            z = 1.0f;
            break;
        case 5: // NegZ
            x = -j;
            y = -i;
            z = -1.0f;
            break;
        }

        norminv = 1.0f / sqrtf(x * x + y * y + z * z);
        x *= norminv, y *= norminv, z *= norminv;
        return;
    }   // getXYZ

    
} //namespace

// ----------------------------------------------------------------------------
/** Compute m_SH_coeff->red_SH_coeff, m_SH_coeff->green_SH_coeff 
 *  and m_SH_coeff->blue_SH_coeff from Yml values
 *  \param sh_rgba The 6 cubemap faces (sRGB byte textures)
 *  \param edge_size Size of the cubemap face
 */
void SphericalHarmonics::generateSphericalHarmonics(unsigned char *sh_rgba[6],
                                                    unsigned int edge_size)
{

#if SIMD_SSE2_SUPPORT

    float wh = float(edge_size * edge_size);
    float edge_size_inv;
    float fi, fj, fi2p1;
    unsigned char *shface;
    __m128 sh0, sh1, sh2, sh3, sh4, sh5, sh6, sh7, sh8;

    // constant part of Ylm
    const float c00 = 0.282095f;
    const float c1minus1 = 0.488603f;
    const float c10 = 0.488603f;
    const float c11 = 0.488603f;
    const float c2minus2 = 1.092548f;
    const float c2minus1 = 1.092548f;
    const float c21 = 1.092548f;
    const float c20 = 0.315392f;
    const float c22 = 0.546274f;

    sh0 = _mm_setzero_ps();
    sh1 = _mm_setzero_ps();
    sh2 = _mm_setzero_ps();
    sh3 = _mm_setzero_ps();
    sh4 = _mm_setzero_ps();
    sh5 = _mm_setzero_ps();
    sh6 = _mm_setzero_ps();
    sh7 = _mm_setzero_ps();
    sh8 = _mm_setzero_ps();

    edge_size_inv = 2.0f / edge_size;
    for (unsigned face = 0; face < 6; face++)
    {
        shface = sh_rgba[face];
        for (int i = 0; i < int(edge_size); i++)
        {
            int shidx = ( i * edge_size ) * 4;
            fi = (float(i) * edge_size_inv) - 1.0f;
            fi2p1 = (fi * fi) + 1.0f;
            for (unsigned j = 0; j < edge_size; j++, shidx += 4)
            {
                float d, solidangle, dinv;
                float shx, shy, shz;
                __m128 vrgb;

                fj = (float(j) * edge_size_inv) - 1.0f;
                d = sqrtf(fi2p1 + (fj * fj));
                dinv = 1.0f / d;
                // Constant obtained by projecting unprojected ref values
                solidangle = 2.75f / (wh * sqrtf(d*d*d));
 #if SIMD_SSE4_1_SUPPORT
                vrgb = _mm_cvtepi32_ps( _mm_cvtepu8_epi32( _mm_castps_si128( _mm_load_ss( (float *)&shface[shidx+0] ) ) ) );
 #else
                vrgb = _mm_cvtepi32_ps( _mm_unpacklo_epi16( _mm_unpacklo_epi8( _mm_castps_si128( _mm_load_ss( (float *)&shface[shidx+0] ) ), _mm_setzero_si128() ), _mm_setzero_si128() ) );
 #endif
                vrgb = _mm_mul_ps( srgb2linear4( vrgb ), _mm_set1_ps( solidangle ) );

                // TODO: This is very suboptimal, and a pity to break such a streamlined loop
                // I think people will disapprove if I unroll the 'face' loop 6 times...
                // What about some branchless vblendps masking?
                switch (face)
                {
                case 0: // PosX
                    shx = 1.0f;
                    shy = -fi;
                    shz = -fj;
                    break;
                case 1: // NegX
                    shx = -1.0f;
                    shy = -fi;
                    shz = fj;
                    break;
                case 2: // PosY
                    shx = fj;
                    shy = 1.0f;
                    shz = fi;
                    break;
                case 3: // NegY
                    shx = fj;
                    shy = -1.0f;
                    shz = -fi;
                    break;
                case 4: // PosZ
                    shx = fj;
                    shy = -fi;
                    shz = 1.0f;
                    break;
                case 5: // NegZ
                    shx = -fj;
                    shy = -fi;
                    shz = -1.0f;
                    break;
                }
                shx *= dinv;
                shy *= dinv;
                shz *= dinv;

                sh0 = _mm_add_ps( sh0, vrgb );
                sh1 = _mm_add_ps( sh1, _mm_mul_ps( vrgb, _mm_set1_ps( shy ) ) );
                sh2 = _mm_add_ps( sh2, _mm_mul_ps( vrgb, _mm_set1_ps( shz ) ) );
                sh3 = _mm_add_ps( sh3, _mm_mul_ps( vrgb, _mm_set1_ps( shx ) ) );
                sh4 = _mm_add_ps( sh4, _mm_mul_ps( vrgb, _mm_set1_ps( shx * shy ) ) );
                sh5 = _mm_add_ps( sh5, _mm_mul_ps( vrgb, _mm_set1_ps( shy * shz ) ) );
                sh6 = _mm_add_ps( sh6, _mm_mul_ps( vrgb, _mm_set1_ps( shx * shz ) ) );
                sh7 = _mm_add_ps( sh7, _mm_mul_ps( vrgb, _mm_set1_ps( ((3.0f * shz * shz) - 1.0f) ) ) );
                sh8 = _mm_add_ps( sh8, _mm_mul_ps( vrgb, _mm_set1_ps( ((shx * shx) - (shy * shy)) ) ) );
            }
        }
    }

    sh0 = _mm_mul_ps( sh0, _mm_set1_ps( c00 ) );
    sh1 = _mm_mul_ps( sh1, _mm_set1_ps( c1minus1 ) );
    sh2 = _mm_mul_ps( sh2, _mm_set1_ps( c10 ) );
    sh3 = _mm_mul_ps( sh3, _mm_set1_ps( c11 ) );
    sh4 = _mm_mul_ps( sh4, _mm_set1_ps( c2minus2 ) );
    sh5 = _mm_mul_ps( sh5, _mm_set1_ps( c2minus1 ) );
    sh6 = _mm_mul_ps( sh6, _mm_set1_ps( c21 ) );
    sh7 = _mm_mul_ps( sh7, _mm_set1_ps( c20 ) );
    sh8 = _mm_mul_ps( sh8, _mm_set1_ps( c22 ) );

    m_SH_coeff->blue_SH_coeff[0] = _mm_cvtss_f32( sh0 );
    m_SH_coeff->blue_SH_coeff[1] = _mm_cvtss_f32( sh1 );
    m_SH_coeff->blue_SH_coeff[2] = _mm_cvtss_f32( sh2 );
    m_SH_coeff->blue_SH_coeff[3] = _mm_cvtss_f32( sh3 );
    m_SH_coeff->blue_SH_coeff[4] = _mm_cvtss_f32( sh4 );
    m_SH_coeff->blue_SH_coeff[5] = _mm_cvtss_f32( sh5 );
    m_SH_coeff->blue_SH_coeff[6] = _mm_cvtss_f32( sh6 );
    m_SH_coeff->blue_SH_coeff[7] = _mm_cvtss_f32( sh7 );
    m_SH_coeff->blue_SH_coeff[8] = _mm_cvtss_f32( sh8 );

    m_SH_coeff->green_SH_coeff[0] = _mm_cvtss_f32( _mm_shuffle_ps( sh0, sh0, 0x55 ) );
    m_SH_coeff->green_SH_coeff[1] = _mm_cvtss_f32( _mm_shuffle_ps( sh1, sh1, 0x55 ) );
    m_SH_coeff->green_SH_coeff[2] = _mm_cvtss_f32( _mm_shuffle_ps( sh2, sh2, 0x55 ) );
    m_SH_coeff->green_SH_coeff[3] = _mm_cvtss_f32( _mm_shuffle_ps( sh3, sh3, 0x55 ) );
    m_SH_coeff->green_SH_coeff[4] = _mm_cvtss_f32( _mm_shuffle_ps( sh4, sh4, 0x55 ) );
    m_SH_coeff->green_SH_coeff[5] = _mm_cvtss_f32( _mm_shuffle_ps( sh5, sh5, 0x55 ) );
    m_SH_coeff->green_SH_coeff[6] = _mm_cvtss_f32( _mm_shuffle_ps( sh6, sh6, 0x55 ) );
    m_SH_coeff->green_SH_coeff[7] = _mm_cvtss_f32( _mm_shuffle_ps( sh7, sh7, 0x55 ) );
    m_SH_coeff->green_SH_coeff[8] = _mm_cvtss_f32( _mm_shuffle_ps( sh8, sh8, 0x55 ) );

    m_SH_coeff->red_SH_coeff[0] = _mm_cvtss_f32( _mm_movehl_ps( sh0, sh0 ) );
    m_SH_coeff->red_SH_coeff[1] = _mm_cvtss_f32( _mm_movehl_ps( sh1, sh1 ) );
    m_SH_coeff->red_SH_coeff[2] = _mm_cvtss_f32( _mm_movehl_ps( sh2, sh2 ) );
    m_SH_coeff->red_SH_coeff[3] = _mm_cvtss_f32( _mm_movehl_ps( sh3, sh3 ) );
    m_SH_coeff->red_SH_coeff[4] = _mm_cvtss_f32( _mm_movehl_ps( sh4, sh4 ) );
    m_SH_coeff->red_SH_coeff[5] = _mm_cvtss_f32( _mm_movehl_ps( sh5, sh5 ) );
    m_SH_coeff->red_SH_coeff[6] = _mm_cvtss_f32( _mm_movehl_ps( sh6, sh6 ) );
    m_SH_coeff->red_SH_coeff[7] = _mm_cvtss_f32( _mm_movehl_ps( sh7, sh7 ) );
    m_SH_coeff->red_SH_coeff[8] = _mm_cvtss_f32( _mm_movehl_ps( sh8, sh8 ) );

#else

    float wh = float(edge_size * edge_size);
    float b0 = 0., b1 = 0., b2 = 0., b3 = 0., b4 = 0., b5 = 0., b6 = 0., b7 = 0., b8 = 0.;
    float g0 = 0., g1 = 0., g2 = 0., g3 = 0., g4 = 0., g5 = 0., g6 = 0., g7 = 0., g8 = 0.;
    float r0 = 0., r1 = 0., r2 = 0., r3 = 0., r4 = 0., r5 = 0., r6 = 0., r7 = 0., r8 = 0.;
    float edge_size_inv;
    float fi, fj, fi2p1;
    unsigned char *shface;

    // constant part of Ylm
    const float c00 = 0.282095f;
    const float c1minus1 = 0.488603f;
    const float c10 = 0.488603f;
    const float c11 = 0.488603f;
    const float c2minus2 = 1.092548f;
    const float c2minus1 = 1.092548f;
    const float c21 = 1.092548f;
    const float c20 = 0.315392f;
    const float c22 = 0.546274f;

    edge_size_inv = 2.0f / edge_size;
    for (unsigned face = 0; face < 6; face++)
    {
        shface = sh_rgba[face];
        for (int i = 0; i < int(edge_size); i++)
        {
            int shidx = ( i * edge_size ) * 4;
            fi = (float(i) * edge_size_inv) - 1.0f;
            fi2p1 = (fi * fi) + 1.0f;
            for (unsigned j = 0; j < edge_size; j++, shidx += 4)
            {
                fj = (float(j) * edge_size_inv) - 1.0f;

                float d = sqrtf(fi2p1 + (fj * fj));
                // Constant obtained by projecting unprojected ref values
                float solidangle = 2.75f / (wh * sqrtf(d*d*d));
                float r, g, b;
                float y00, y1m1, y10, y11, y2m2, y2m1, y21, y20, y22;
                float shx, shy, shz;

                b = srgb2linear( float(shface[shidx+0]) ) * solidangle;
                g = srgb2linear( float(shface[shidx+1]) ) * solidangle;
                r = srgb2linear( float(shface[shidx+2]) ) * solidangle;

                getXYZ(face, fi, fj, shx, shy, shz);
                y00 = c00;
                y1m1 = c1minus1 * shy;
                y10 = c10 * shz;
                y11 = c11 * shx;
                y2m2 = c2minus2 * shx * shy;
                y2m1 = c2minus1 * shy * shz;
                y21 = c21 * shx * shz;
                y20 = c20 * ((3.0f * shz * shz) - 1.0f);
                y22 = c22 * ((shx * shx) - (shy * shy));

                b0 += b * y00;
                b1 += b * y1m1;
                b2 += b * y10;
                b3 += b * y11;
                b4 += b * y2m2;
                b5 += b * y2m1;
                b6 += b * y20;
                b7 += b * y21;
                b8 += b * y22;

                g0 += g * y00;
                g1 += g * y1m1;
                g2 += g * y10;
                g3 += g * y11;
                g4 += g * y2m2;
                g5 += g * y2m1;
                g6 += g * y20;
                g7 += g * y21;
                g8 += g * y22;

                r0 += r * y00;
                r1 += r * y1m1;
                r2 += r * y10;
                r3 += r * y11;
                r4 += r * y2m2;
                r5 += r * y2m1;
                r6 += r * y20;
                r7 += r * y21;
                r8 += r * y22;
            }
        }
    }

    m_SH_coeff->blue_SH_coeff[0] = b0;
    m_SH_coeff->blue_SH_coeff[1] = b1;
    m_SH_coeff->blue_SH_coeff[2] = b2;
    m_SH_coeff->blue_SH_coeff[3] = b3;
    m_SH_coeff->blue_SH_coeff[4] = b4;
    m_SH_coeff->blue_SH_coeff[5] = b5;
    m_SH_coeff->blue_SH_coeff[6] = b6;
    m_SH_coeff->blue_SH_coeff[7] = b7;
    m_SH_coeff->blue_SH_coeff[8] = b8;

    m_SH_coeff->red_SH_coeff[0] = r0;
    m_SH_coeff->red_SH_coeff[1] = r1;
    m_SH_coeff->red_SH_coeff[2] = r2;
    m_SH_coeff->red_SH_coeff[3] = r3;
    m_SH_coeff->red_SH_coeff[4] = r4;
    m_SH_coeff->red_SH_coeff[5] = r5;
    m_SH_coeff->red_SH_coeff[6] = r6;
    m_SH_coeff->red_SH_coeff[7] = r7;
    m_SH_coeff->red_SH_coeff[8] = r8;

    m_SH_coeff->green_SH_coeff[0] = g0;
    m_SH_coeff->green_SH_coeff[1] = g1;
    m_SH_coeff->green_SH_coeff[2] = g2;
    m_SH_coeff->green_SH_coeff[3] = g3;
    m_SH_coeff->green_SH_coeff[4] = g4;
    m_SH_coeff->green_SH_coeff[5] = g5;
    m_SH_coeff->green_SH_coeff[6] = g6;
    m_SH_coeff->green_SH_coeff[7] = g7;
    m_SH_coeff->green_SH_coeff[8] = g8;

#endif
/*
printf( "#### SH ; Coeffs R ; %f %f %f %f %f %f %f %f\n", m_SH_coeff->red_SH_coeff[0], m_SH_coeff->red_SH_coeff[1], m_SH_coeff->red_SH_coeff[2], m_SH_coeff->red_SH_coeff[3], m_SH_coeff->red_SH_coeff[4], m_SH_coeff->red_SH_coeff[5], m_SH_coeff->red_SH_coeff[6], m_SH_coeff->red_SH_coeff[7] );
printf( "#### SH ; Coeffs G ; %f %f %f %f %f %f %f %f\n", m_SH_coeff->green_SH_coeff[0], m_SH_coeff->green_SH_coeff[1], m_SH_coeff->green_SH_coeff[2], m_SH_coeff->green_SH_coeff[3], m_SH_coeff->green_SH_coeff[4], m_SH_coeff->green_SH_coeff[5], m_SH_coeff->green_SH_coeff[6], m_SH_coeff->green_SH_coeff[7] );
printf( "#### SH ; Coeffs B ; %f %f %f %f %f %f %f %f\n", m_SH_coeff->blue_SH_coeff[0], m_SH_coeff->blue_SH_coeff[1], m_SH_coeff->blue_SH_coeff[2], m_SH_coeff->blue_SH_coeff[3], m_SH_coeff->blue_SH_coeff[4], m_SH_coeff->blue_SH_coeff[5], m_SH_coeff->blue_SH_coeff[6], m_SH_coeff->blue_SH_coeff[7] );
*/
}   // projectSH

// ----------------------------------------------------------------------------
SphericalHarmonics::SphericalHarmonics(const std::vector<video::IImage *> &spherical_harmonics_textures)
{
    m_SH_coeff = new SHCoefficients;
    setTextures(spherical_harmonics_textures);
}

// ----------------------------------------------------------------------------
/** When spherical harmonics textures are not defined, SH coefficents are computed
 *  from ambient light
 */
SphericalHarmonics::SphericalHarmonics(const video::SColor &ambient)
{
    //make sure m_ambient and ambient are not equal
    m_ambient = (ambient==0) ? 1 : 0;
    m_SH_coeff = new SHCoefficients;
    setAmbientLight(ambient);
}

SphericalHarmonics::~SphericalHarmonics()
{
    delete m_SH_coeff;
}


/** Compute spherical harmonics coefficients from 6 textures */
void SphericalHarmonics::setTextures(const std::vector<video::IImage *> &spherical_harmonics_textures)
{
    assert(spherical_harmonics_textures.size() == 6);
    
    m_spherical_harmonics_textures = spherical_harmonics_textures;

    const unsigned texture_permutation[] = { 2, 3, 0, 1, 5, 4 };
    unsigned char *sh_rgba[6];
    unsigned sh_w = 0, sh_h = 0;

    for (unsigned i = 0; i < 6; i++)
    {
        sh_w = std::max(sh_w, m_spherical_harmonics_textures[i]->getDimension().Width);
        sh_h = std::max(sh_h, m_spherical_harmonics_textures[i]->getDimension().Height);
    }

    for (unsigned i = 0; i < 6; i++)
        sh_rgba[i] = new unsigned char[sh_w * sh_h * 4];
    
    for (unsigned i = 0; i < 6; i++)
    {
        unsigned idx = texture_permutation[i];
        m_spherical_harmonics_textures[idx]->copyToScaling(sh_rgba[i], sh_w, sh_h);
        m_spherical_harmonics_textures[idx]->drop();
    } //for (unsigned i = 0; i < 6; i++)

    generateSphericalHarmonics(sh_rgba, sh_w);

    for (unsigned i = 0; i < 6; i++)
        delete[] sh_rgba[i];
} //setSphericalHarmonicsTextures

/** Compute spherical harmonics coefficients from ambient light */
void SphericalHarmonics::setAmbientLight(const video::SColor &ambient)
{    
    //do not recompute SH coefficients if we already use the same ambient light
    if((m_spherical_harmonics_textures.size() != 6) && (ambient == m_ambient))
        return;
        
    m_spherical_harmonics_textures.clear();
    m_ambient = ambient;
    
    unsigned char *sh_rgba[6];
    unsigned sh_w = 16;
    unsigned sh_h = 16;
    unsigned ambr, ambg, ambb;

    ambr = ambient.getRed();
    ambg = ambient.getGreen();
    ambb = ambient.getBlue();

    for (unsigned i = 0; i < 6; i++)
    {
        sh_rgba[i] = new unsigned char[sh_w * sh_h * 4];
        for (unsigned j = 0; j < sh_w * sh_h * 4; j += 4)
        {
            sh_rgba[i][j] = ambb;
            sh_rgba[i][j + 1] = ambg;
            sh_rgba[i][j + 2] = ambr;
            sh_rgba[i][j + 3] = 255;
        }
    }    

    generateSphericalHarmonics(sh_rgba, sh_w);

    for (unsigned i = 0; i < 6; i++)
        delete[] sh_rgba[i];

    // Diffuse env map is x 0.25, compensate
    for (unsigned i = 0; i < 9; i++)
    {
        m_SH_coeff->blue_SH_coeff[i] *= 4;
        m_SH_coeff->green_SH_coeff[i] *= 4;
        m_SH_coeff->red_SH_coeff[i] *= 4;
    }    
} //setAmbientLight

// ----------------------------------------------------------------------------
/** Print spherical harmonics coefficients (debug) */
void SphericalHarmonics::printCoeff() {
    Log::debug("SphericalHarmonics", "Blue_SH:");
    displayCoeff(m_SH_coeff->blue_SH_coeff);
    Log::debug("SphericalHarmonics", "Green_SH:");
    displayCoeff(m_SH_coeff->green_SH_coeff);
    Log::debug("SphericalHarmonics", "Red_SH:");
    displayCoeff(m_SH_coeff->red_SH_coeff);  
} //printCoeff

// ----------------------------------------------------------------------------
/** Compute the the environment map from the spherical harmonics coefficients
*  \param width The texture width
*  \param height The texture height
*  \param Yml The sphericals harmonics functions values
*  \param[out] output The environment map texels values
*/    
void SphericalHarmonics::unprojectSH(unsigned int width, unsigned int height,
                                     float *Y00[], float *Y1minus1[], float *Y10[],
                                     float *Y11[], float *Y2minus2[], float *Y2minus1[],
                                     float *Y20[], float *Y21[], float *Y22[],
                                     float *output[])
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
                    getTexelValue(i, j, width, height, m_SH_coeff->red_SH_coeff, Y00[face],
                                Y1minus1[face], Y10[face], Y11[face],
                                Y2minus2[face], Y2minus1[face], Y20[face],
                                Y21[face], Y22[face]);
                output[face][4 * height * i + 4 * j + 1] = 
                    getTexelValue(i, j, width, height, m_SH_coeff->green_SH_coeff, Y00[face],
                                Y1minus1[face], Y10[face], Y11[face],
                                Y2minus2[face], Y2minus1[face], Y20[face],
                                Y21[face], Y22[face]);
                output[face][4 * height * i + 4 * j] = 
                    getTexelValue(i, j, width, height, m_SH_coeff->blue_SH_coeff, Y00[face],
                                Y1minus1[face], Y10[face], Y11[face],
                                Y2minus2[face], Y2minus1[face], Y20[face],
                                Y21[face], Y22[face]);
            }
        }
    }
}   // unprojectSH

#endif   // !SERVER_ONLY

