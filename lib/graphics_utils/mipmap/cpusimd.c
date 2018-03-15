/* -----------------------------------------------------------------------------
 *
 * Copyright (c) 2008-2016 Alexis Naveros.
 *
 *
 * The SIMD trigonometry functions are Copyright (C) 2007  Julien Pommier
 * See copyright notice for simd4f_sin_ps(), simd4f_cos_ps(), simd4f_sincos_ps()
 *
 *
 * Some functions are Copyright (C) 2008  José Fonseca
 * See copyright notice for simd4f_exp2_ps(), simd4f_log2_ps(), simd4f_pow_ps()
 *
 *
 * Portions developed under contract to the SURVICE Engineering Company.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>


#include "cpusimd.h"


#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif


////


#if CPU_SSE_SUPPORT

const uint32_t CPU_ALIGN16 simd4fSignMask[4] = { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
const uint32_t CPU_ALIGN16 simd4fSignMaskInv[4] = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
const float CPU_ALIGN16 simd4fHalf[4] = { 0.5, 0.5, 0.5, 0.5 };
const float CPU_ALIGN16 simd4fOne[4] = { 1.0, 1.0, 1.0, 1.0 };
const float CPU_ALIGN16 simd4fTwo[4] = { 2.0, 2.0, 2.0, 2.0 };
const float CPU_ALIGN16 simd4fThree[4] = { 3.0, 3.0, 3.0, 3.0 };
const uint32_t CPU_ALIGN16 simd4uOne[4] = { 1, 1, 1, 1 };
const uint32_t CPU_ALIGN16 simd4uOneInv[4] = { ~1, ~1, ~1, ~1 };
const uint32_t CPU_ALIGN16 simd4uTwo[4] = { 2, 2, 2, 2 };
const uint32_t CPU_ALIGN16 simd4uFour[4] = { 4, 4, 4, 4 };
const float CPU_ALIGN16 simd4fQuarter[4] = { 0.25, 0.25, 0.25, 0.25 };
const float CPU_ALIGN16 simd4fPi[4] = { M_PI, M_PI, M_PI, M_PI };
const float CPU_ALIGN16 simd4fZeroOneTwoThree[4] = { 0.0, 1.0, 2.0, 3.0 };
const uint32_t CPU_ALIGN16 simd4fAlphaMask[4] = { 0x00000000, 0x00000000, 0x00000000, 0xffffffff };
const float CPU_ALIGN16 simd4f255[4] = { 255.0f, 255.0f, 255.0f, 255.0f };
const float CPU_ALIGN16 simd4f255Inv[4] = { 1.0f/255.0f, 1.0f/255.0f, 1.0f/255.0f, 1.0f/255.0f };

#endif


////


#if CPU_SSE2_SUPPORT


/* Copyright (C) 2007  Julien Pommier

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  (this is the zlib license)
*/

static const float CPU_ALIGN16 simd4f_cephes_FOPI[4] = { 1.27323954473516, 1.27323954473516, 1.27323954473516, 1.27323954473516 };
static const float CPU_ALIGN16 simd4f_minus_cephes_DP1[4] = { -0.78515625, -0.78515625, -0.78515625, -0.78515625 };
static const float CPU_ALIGN16 simd4f_minus_cephes_DP2[4] = { -2.4187564849853515625e-4, -2.4187564849853515625e-4, -2.4187564849853515625e-4, -2.4187564849853515625e-4 };
static const float CPU_ALIGN16 simd4f_minus_cephes_DP3[4] = { -3.77489497744594108e-8, -3.77489497744594108e-8, -3.77489497744594108e-8, -3.77489497744594108e-8 };
static const float CPU_ALIGN16 simd4f_sincof_p0[4] = { -1.9515295891E-4, -1.9515295891E-4, -1.9515295891E-4, -1.9515295891E-4 };
static const float CPU_ALIGN16 simd4f_sincof_p1[4] = { 8.3321608736E-3, 8.3321608736E-3, 8.3321608736E-3, 8.3321608736E-3 };
static const float CPU_ALIGN16 simd4f_sincof_p2[4] = { -1.6666654611E-1, -1.6666654611E-1, -1.6666654611E-1, -1.6666654611E-1 };
static const float CPU_ALIGN16 simd4f_coscof_p0[4] = { 2.443315711809948E-005, 2.443315711809948E-005, 2.443315711809948E-005, 2.443315711809948E-005 };
static const float CPU_ALIGN16 simd4f_coscof_p1[4] = { -1.388731625493765E-003, -1.388731625493765E-003, -1.388731625493765E-003, -1.388731625493765E-003 };
static const float CPU_ALIGN16 simd4f_coscof_p2[4] = { 4.166664568298827E-002, 4.166664568298827E-002, 4.166664568298827E-002, 4.166664568298827E-002 };

__m128 simd4f_sin_ps( __m128 x )
{
  __m128 xmm1, xmm2, xmm3, sign_bit, y;
  __m128i emm0, emm2;

  xmm2 = _mm_setzero_ps();

  sign_bit = x;
  /* take the absolute value */
  x = _mm_and_ps( x, *(__m128 *)simd4fSignMaskInv );
  /* extract the sign bit (upper one) */
  sign_bit = _mm_and_ps(sign_bit, *(__m128 *)simd4fSignMask);

  /* scale by 4/Pi */
  y = _mm_mul_ps(x, *(__m128 *)simd4f_cephes_FOPI);

  /* store the integer part of y in mm0 */
  emm2 = _mm_cvttps_epi32(y);
  /* j=(j+1) & (~1) (see the cephes sources) */
  emm2 = _mm_add_epi32(emm2, *(__m128i*)simd4uOne);
  emm2 = _mm_and_si128(emm2, *(__m128i*)simd4uOneInv);
  y = _mm_cvtepi32_ps(emm2);

  /* get the swap sign flag */
  emm0 = _mm_and_si128(emm2, *(__m128i*)simd4uFour);
  emm0 = _mm_slli_epi32(emm0, 29);
  /* get the polynom selection mask
     there is one polynom for 0 <= x <= Pi/4
     and another one for Pi/4<x<=Pi/2
     Both branches will be computed.
  */
  emm2 = _mm_and_si128(emm2, *(__m128i*)simd4uTwo);
  emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());

  __m128 swap_sign_bit = _mm_castsi128_ps(emm0);
  __m128 poly_mask = _mm_castsi128_ps(emm2);
  sign_bit = _mm_xor_ps(sign_bit, swap_sign_bit);

  /* The magic pass: "Extended precision modular arithmetic"
     x = ((x - y * DP1) - y * DP2) - y * DP3; */
  xmm1 = *(__m128 *)simd4f_minus_cephes_DP1;
  xmm2 = *(__m128 *)simd4f_minus_cephes_DP2;
  xmm3 = *(__m128 *)simd4f_minus_cephes_DP3;
  xmm1 = _mm_mul_ps(y, xmm1);
  xmm2 = _mm_mul_ps(y, xmm2);
  xmm3 = _mm_mul_ps(y, xmm3);
  x = _mm_add_ps(x, xmm1);
  x = _mm_add_ps(x, xmm2);
  x = _mm_add_ps(x, xmm3);

  /* Evaluate the first polynom  (0 <= x <= Pi/4) */
  y = *(__m128 *)simd4f_coscof_p0;
  __m128 z = _mm_mul_ps(x,x);

  y = _mm_mul_ps(y, z);
  y = _mm_add_ps(y, *(__m128 *)simd4f_coscof_p1);
  y = _mm_mul_ps(y, z);
  y = _mm_add_ps(y, *(__m128 *)simd4f_coscof_p2);
  y = _mm_mul_ps(y, z);
  y = _mm_mul_ps(y, z);
  __m128 tmp = _mm_mul_ps(z, *(__m128 *)simd4fHalf);
  y = _mm_sub_ps(y, tmp);
  y = _mm_add_ps(y, *(__m128 *)simd4fOne);

  /* Evaluate the second polynom  (Pi/4 <= x <= 0) */

  __m128 y2 = *(__m128 *)simd4f_sincof_p0;
  y2 = _mm_mul_ps(y2, z);
  y2 = _mm_add_ps(y2, *(__m128 *)simd4f_sincof_p1);
  y2 = _mm_mul_ps(y2, z);
  y2 = _mm_add_ps(y2, *(__m128 *)simd4f_sincof_p2);
  y2 = _mm_mul_ps(y2, z);
  y2 = _mm_mul_ps(y2, x);
  y2 = _mm_add_ps(y2, x);

  /* select the correct result from the two polynoms */
  xmm3 = poly_mask;
  y2 = _mm_and_ps(xmm3, y2);
  y = _mm_andnot_ps(xmm3, y);
  y = _mm_add_ps(y,y2);
  /* update the sign */
  y = _mm_xor_ps(y, sign_bit);

  return y;
}



/* almost the same as sin_ps */
__m128 simd4f_cos_ps( __m128 x )
{
  __m128 xmm1, xmm2, xmm3, y;
  __m128i emm0, emm2;

  xmm2 = _mm_setzero_ps();

  /* take the absolute value */
  x = _mm_and_ps(x, *(__m128*)simd4fSignMaskInv);
  
  /* scale by 4/Pi */
  y = _mm_mul_ps(x, *(__m128*)simd4f_cephes_FOPI);
  
  /* store the integer part of y in mm0 */
  emm2 = _mm_cvttps_epi32(y);
  /* j=(j+1) & (~1) (see the cephes sources) */
  emm2 = _mm_add_epi32(emm2, *(__m128i*)simd4uOne);
  emm2 = _mm_and_si128(emm2, *(__m128i*)simd4uOneInv);
  y = _mm_cvtepi32_ps(emm2);

  emm2 = _mm_sub_epi32(emm2, *(__m128i*)simd4uTwo);
  
  /* get the swap sign flag */
  emm0 = _mm_andnot_si128(emm2, *(__m128i*)simd4uFour);
  emm0 = _mm_slli_epi32(emm0, 29);
  /* get the polynom selection mask */
  emm2 = _mm_and_si128(emm2, *(__m128i*)simd4uTwo);
  emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());
  
  __m128 sign_bit = _mm_castsi128_ps(emm0);
  __m128 poly_mask = _mm_castsi128_ps(emm2);
  /* The magic pass: "Extended precision modular arithmetic" 
     x = ((x - y * DP1) - y * DP2) - y * DP3; */
  xmm1 = *(__m128*)simd4f_minus_cephes_DP1;
  xmm2 = *(__m128*)simd4f_minus_cephes_DP2;
  xmm3 = *(__m128*)simd4f_minus_cephes_DP3;
  xmm1 = _mm_mul_ps(y, xmm1);
  xmm2 = _mm_mul_ps(y, xmm2);
  xmm3 = _mm_mul_ps(y, xmm3);
  x = _mm_add_ps(x, xmm1);
  x = _mm_add_ps(x, xmm2);
  x = _mm_add_ps(x, xmm3);
  
  /* Evaluate the first polynom  (0 <= x <= Pi/4) */
  y = *(__m128*)simd4f_coscof_p0;
  __m128 z = _mm_mul_ps(x,x);

  y = _mm_mul_ps(y, z);
  y = _mm_add_ps(y, *(__m128*)simd4f_coscof_p1);
  y = _mm_mul_ps(y, z);
  y = _mm_add_ps(y, *(__m128*)simd4f_coscof_p2);
  y = _mm_mul_ps(y, z);
  y = _mm_mul_ps(y, z);
  __m128 tmp = _mm_mul_ps(z, *(__m128*)simd4fHalf);
  y = _mm_sub_ps(y, tmp);
  y = _mm_add_ps(y, *(__m128*)simd4fOne);
  
  /* Evaluate the second polynom  (Pi/4 <= x <= 0) */

  __m128 y2 = *(__m128*)simd4f_sincof_p0;
  y2 = _mm_mul_ps(y2, z);
  y2 = _mm_add_ps(y2, *(__m128*)simd4f_sincof_p1);
  y2 = _mm_mul_ps(y2, z);
  y2 = _mm_add_ps(y2, *(__m128*)simd4f_sincof_p2);
  y2 = _mm_mul_ps(y2, z);
  y2 = _mm_mul_ps(y2, x);
  y2 = _mm_add_ps(y2, x);

  /* select the correct result from the two polynoms */  
  xmm3 = poly_mask;
  y2 = _mm_and_ps(xmm3, y2); //, xmm3);
  y = _mm_andnot_ps(xmm3, y);
  y = _mm_add_ps(y,y2);
  /* update the sign */
  y = _mm_xor_ps(y, sign_bit);

  return y;
}

/* since sin_ps and cos_ps are almost identical, sincos_ps could replace both of them..
   it is almost as fast, and gives you a free cosine with your sine */
void simd4f_sincos_ps( __m128 x, __m128 *s, __m128 *c )
{
  __m128 xmm1, xmm2, xmm3, sign_bit_sin, y;
  __m128i emm0, emm2, emm4;

  xmm3 = _mm_setzero_ps();

  sign_bit_sin = x;
  /* take the absolute value */
  x = _mm_and_ps(x, *(__m128*)simd4fSignMaskInv);
  /* extract the sign bit (upper one) */
  sign_bit_sin = _mm_and_ps(sign_bit_sin, *(__m128*)simd4fSignMask);
  
  /* scale by 4/Pi */
  y = _mm_mul_ps(x, *(__m128*)simd4f_cephes_FOPI);
    
  /* store the integer part of y in emm2 */
  emm2 = _mm_cvttps_epi32(y);

  /* j=(j+1) & (~1) (see the cephes sources) */
  emm2 = _mm_add_epi32(emm2, *(__m128i*)simd4uOne);
  emm2 = _mm_and_si128(emm2, *(__m128i*)simd4uOneInv);
  y = _mm_cvtepi32_ps(emm2);

  emm4 = emm2;

  /* get the swap sign flag for the sine */
  emm0 = _mm_and_si128(emm2, *(__m128i*)simd4uFour);
  emm0 = _mm_slli_epi32(emm0, 29);
  __m128 swap_sign_bit_sin = _mm_castsi128_ps(emm0);

  /* get the polynom selection mask for the sine*/
  emm2 = _mm_and_si128(emm2, *(__m128i*)simd4uTwo);
  emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());
  __m128 poly_mask = _mm_castsi128_ps(emm2);

  /* The magic pass: "Extended precision modular arithmetic" 
     x = ((x - y * DP1) - y * DP2) - y * DP3; */
  xmm1 = *(__m128*)simd4f_minus_cephes_DP1;
  xmm2 = *(__m128*)simd4f_minus_cephes_DP2;
  xmm3 = *(__m128*)simd4f_minus_cephes_DP3;
  xmm1 = _mm_mul_ps(y, xmm1);
  xmm2 = _mm_mul_ps(y, xmm2);
  xmm3 = _mm_mul_ps(y, xmm3);
  x = _mm_add_ps(x, xmm1);
  x = _mm_add_ps(x, xmm2);
  x = _mm_add_ps(x, xmm3);

  emm4 = _mm_sub_epi32(emm4, *(__m128i*)simd4uTwo);
  emm4 = _mm_andnot_si128(emm4, *(__m128i*)simd4uFour);
  emm4 = _mm_slli_epi32(emm4, 29);
  __m128 sign_bit_cos = _mm_castsi128_ps(emm4);

  sign_bit_sin = _mm_xor_ps(sign_bit_sin, swap_sign_bit_sin);
  
  /* Evaluate the first polynom  (0 <= x <= Pi/4) */
  __m128 z = _mm_mul_ps(x,x);
  y = *(__m128*)simd4f_coscof_p0;

  y = _mm_mul_ps(y, z);
  y = _mm_add_ps(y, *(__m128*)simd4f_coscof_p1);
  y = _mm_mul_ps(y, z);
  y = _mm_add_ps(y, *(__m128*)simd4f_coscof_p2);
  y = _mm_mul_ps(y, z);
  y = _mm_mul_ps(y, z);
  __m128 tmp = _mm_mul_ps(z, *(__m128*)simd4fHalf);
  y = _mm_sub_ps(y, tmp);
  y = _mm_add_ps(y, *(__m128*)simd4fOne);
  
  /* Evaluate the second polynom  (Pi/4 <= x <= 0) */

  __m128 y2 = *(__m128*)simd4f_sincof_p0;
  y2 = _mm_mul_ps(y2, z);
  y2 = _mm_add_ps(y2, *(__m128*)simd4f_sincof_p1);
  y2 = _mm_mul_ps(y2, z);
  y2 = _mm_add_ps(y2, *(__m128*)simd4f_sincof_p2);
  y2 = _mm_mul_ps(y2, z);
  y2 = _mm_mul_ps(y2, x);
  y2 = _mm_add_ps(y2, x);

  /* select the correct result from the two polynoms */  
  xmm3 = poly_mask;
  __m128 ysin2 = _mm_and_ps(xmm3, y2);
  __m128 ysin1 = _mm_andnot_ps(xmm3, y);
  y2 = _mm_sub_ps(y2,ysin2);
  y = _mm_sub_ps(y, ysin1);

  xmm1 = _mm_add_ps(ysin1,ysin2);
  xmm2 = _mm_add_ps(y,y2);
 
  /* update the sign */
  *s = _mm_xor_ps(xmm1, sign_bit_sin);
  *c = _mm_xor_ps(xmm2, sign_bit_cos);
}


#endif


////


#if CPU_SSE2_SUPPORT


/* Copyright (C) 2008  José Fonseca
   http://jrfonseca.blogspot.ca/2008/09/fast-sse2-pow-tables-or-polynomials.html
   MIT license

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#define POLY0(x,c0) _mm_set1_ps(c0)
#define POLY1(x,c0,c1) _mm_add_ps(_mm_mul_ps(POLY0(x, c1), x), _mm_set1_ps(c0))
#define POLY2(x,c0,c1,c2) _mm_add_ps(_mm_mul_ps(POLY1(x, c1, c2), x), _mm_set1_ps(c0))
#define POLY3(x,c0,c1,c2,c3) _mm_add_ps(_mm_mul_ps(POLY2(x, c1, c2, c3), x), _mm_set1_ps(c0))
#define POLY4(x,c0,c1,c2,c3,c4) _mm_add_ps(_mm_mul_ps(POLY3(x, c1, c2, c3, c4), x), _mm_set1_ps(c0))
#define POLY5(x,c0,c1,c2,c3,c4,c5) _mm_add_ps(_mm_mul_ps(POLY4(x, c1, c2, c3, c4, c5), x), _mm_set1_ps(c0))

#define EXP_POLY_DEGREE 3
#define LOG_POLY_DEGREE 5

__m128 simd4f_exp2_ps( __m128 x )
{
  __m128i ipart;
  __m128 fpart, expipart, expfpart;

  x = _mm_min_ps( x, _mm_set1_ps(  129.00000f ) );
  x = _mm_max_ps( x, _mm_set1_ps( -126.99999f ) );
  /* ipart = int(x - 0.5) */
  ipart = _mm_cvtps_epi32( _mm_sub_ps( x, _mm_set1_ps( 0.5f ) ) );
  /* fpart = x - ipart */
  fpart = _mm_sub_ps( x, _mm_cvtepi32_ps( ipart ) );
  /* expipart = (float) (1 << ipart) */
  expipart = _mm_castsi128_ps( _mm_slli_epi32( _mm_add_epi32( ipart, _mm_set1_epi32( 127 ) ), 23 ) );
  /* minimax polynomial fit of 2**x, in range [-0.5, 0.5[ */
#if EXP_POLY_DEGREE == 5
  expfpart = POLY5( fpart, 9.9999994e-1f, 6.9315308e-1f, 2.4015361e-1f, 5.5826318e-2f, 8.9893397e-3f, 1.8775767e-3f );
#elif EXP_POLY_DEGREE == 4
  expfpart = POLY4( fpart, 1.0000026f, 6.9300383e-1f, 2.4144275e-1f, 5.2011464e-2f, 1.3534167e-2f );
#elif EXP_POLY_DEGREE == 3
  expfpart = POLY3( fpart, 9.9992520e-1f, 6.9583356e-1f, 2.2606716e-1f, 7.8024521e-2f );
#elif EXP_POLY_DEGREE == 2
  expfpart = POLY2( fpart, 1.0017247f, 6.5763628e-1f, 3.3718944e-1f );
#else
#error
#endif
  return _mm_mul_ps(expipart, expfpart);
}

__m128 simd4f_log2_ps( __m128 x )
{
  __m128i expmask, mantmask, i;
  __m128 one, vexp, mant, logmant;

  expmask = _mm_set1_epi32( 0x7f800000 );
  mantmask = _mm_set1_epi32( 0x007fffff );
  one = _mm_set1_ps( 1.0f );
  i = _mm_castps_si128( x );
  /* exp = (float) exponent(x) */
  vexp = _mm_cvtepi32_ps( _mm_sub_epi32( _mm_srli_epi32( _mm_and_si128( i, expmask ), 23 ), _mm_set1_epi32( 127 ) ) );
  /* mant = (float) mantissa(x) */
  mant = _mm_or_ps( _mm_castsi128_ps( _mm_and_si128( i, mantmask ) ), one );
  /* Minimax polynomial fit of log2(x)/(x - 1), for x in range [1, 2[ 
   * These coefficients can be generate with 
   * http://www.boost.org/doc/libs/1_36_0/libs/math/doc/sf_and_dist/html/math_toolkit/toolkit/internals2/minimax.html
   */
#if LOG_POLY_DEGREE == 6
  logmant = POLY5( mant, 3.11578814719469302614f, -3.32419399085241980044f, 2.59883907202499966007f, -1.23152682416275988241f, 0.318212422185251071475f, -0.0344359067839062357313f );
#elif LOG_POLY_DEGREE == 5
  logmant = POLY4( mant, 2.8882704548164776201f, -2.52074962577807006663f, 1.48116647521213171641f, -0.465725644288844778798f, 0.0596515482674574969533f );
#elif LOG_POLY_DEGREE == 4
  logmant = POLY3( mant, 2.61761038894603480148f, -1.75647175389045657003f, 0.688243882994381274313f, -0.107254423828329604454f );
#elif LOG_POLY_DEGREE == 3
  logmant = POLY2( mant, 2.28330284476918490682f, -1.04913055217340124191f, 0.204446009836232697516f );
#else
#error
#endif
  /* This effectively increases the polynomial degree by one, but ensures that log2(1) == 0*/
  logmant = _mm_mul_ps( logmant, _mm_sub_ps(mant, one ) );
  return _mm_add_ps( logmant, vexp );
}


__m128 simd4f_pow_ps( __m128 x, __m128 y )
{
  return simd4f_exp2_ps( _mm_mul_ps( simd4f_log2_ps( x ), y ) );
}


#endif


////


#if CPU_SSE2_SUPPORT


/*
By Potatoswatter
http://stackoverflow.com/questions/6475373/optimizations-for-pow-with-const-non-integer-exponent
*/

#ifndef CC_ALWAYSINLINE
 #if defined(__GNUC__) || defined(__INTEL_COMPILER)
  #define CC_ALWAYSINLINE __attribute__((always_inline))
 #else
  #define CC_ALWAYSINLINE
 #endif
#endif

static inline CC_ALWAYSINLINE __m128 simd4f_fastpow_ps( __m128 arg, uint32_t expnum, uint32_t expden, uint32_t coeffnum, uint32_t coeffden )
{
  __m128 ret = arg;
  float corrfactor, powfactor;
  /* Apply a constant pre-correction factor. */
  corrfactor = exp2( 127.0 * expden / expnum - 127.0 ) * pow( 1.0 * coeffnum / coeffden, 1.0 * expden / expnum );
  powfactor = 1.0 * expnum / expden;
  ret = _mm_mul_ps( ret, _mm_set1_ps( corrfactor ) );
  /* Reinterpret arg as integer to obtain logarithm. */
  ret = _mm_cvtepi32_ps( _mm_castps_si128( ret ) );
  /* Multiply logarithm by power. */
  ret = _mm_mul_ps( ret, _mm_set1_ps( powfactor ) );
  /* Convert back to "integer" to exponentiate. */
  ret = _mm_castsi128_ps( _mm_cvtps_epi32( ret ) );
  return ret;
}

__m128 simd4f_pow12d5_ps( __m128 arg )
{
  /* Lower exponents provide lower initial error, but too low causes overflow. */
  __m128 xf = simd4f_fastpow_ps( arg, 4, 5, (int)( 1.38316186f * (float)1e9 ), (int)1e9 );
  /* Imprecise 4-cycle sqrt is still far better than fastpow, good enough. */
  __m128 xfm4 = _mm_rsqrt_ps( xf );
  __m128 xf4 = _mm_mul_ps( xf, xfm4 );
  /* Precisely calculate x^2 and x^3 */
  __m128 x2 = _mm_mul_ps( arg, arg );
  __m128 x3 = _mm_mul_ps( x2, arg );
  /* Overestimate of x^2 * x^0.4 */
  x2 = _mm_mul_ps( x2, xf4 );
  /* Get x^-0.2 from x^0.4, and square it for x^-0.4. Combine into x^-0.6. */
  __m128 xfm2 = _mm_rsqrt_ps( xf4 );
  x3 = _mm_mul_ps( x3, xfm4 );
  x3 = _mm_mul_ps( x3, xfm2 );
  return _mm_mul_ps( _mm_add_ps( x2, x3 ), _mm_set1_ps( 1.0f/1.960131704207789f * 0.9999f ) );
}

__m128 simd4f_pow5d12_ps( __m128 arg )
{
  /* 5/12 is too small, so compute the 4th root of 20/12 instead. */
  /* 20/12 = 5/3 = 1 + 2/3 = 2 - 1/3. 2/3 is a suitable argument for fastpow. */
  /* weighting coefficient: a^-1/2 = 2 a; a = 2^-2/3 */
  __m128 xf = simd4f_fastpow_ps( arg, 2, 3, (int)( 0.629960524947437f * (float)1e9 ), (int)1e9 );
  __m128 xover = _mm_mul_ps( arg, xf );
  __m128 xfm1 = _mm_rsqrt_ps( xf );
  __m128 x2 = _mm_mul_ps( arg, arg );
  __m128 xunder = _mm_mul_ps( x2, xfm1 );
  /* sqrt2 * over + 2 * sqrt2 * under */
  __m128 xavg = _mm_mul_ps( _mm_set1_ps( 1.0f/( 3.0f * 0.629960524947437f ) * 0.999852f ), _mm_add_ps( xover, xunder ) );
  xavg = _mm_mul_ps( xavg, _mm_rsqrt_ps( xavg ) );
  xavg = _mm_mul_ps( xavg, _mm_rsqrt_ps( xavg ) );
  return xavg;
}

#endif


////

