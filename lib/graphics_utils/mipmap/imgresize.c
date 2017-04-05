/* -----------------------------------------------------------------------------
 *
 * Copyright (c) 2014-2017 Alexis Naveros.
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

#include "img.h"
#include "imgresize.h"


////


#define IM_RESIZE_DEBUG (0)
#define IM_RESIZE_DEBUG_PROGRESS (0)


////


#ifndef M_PI
 #define M_PI (3.14159265358979323846)
#endif

#ifndef ADDRESS
 #define ADDRESS(p,o) ((void *)(((char *)p)+(o)))
#endif

#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
 #define CC_ALWAYSINLINE __attribute__((always_inline))
 #if __STDC_VERSION__ >= 199901L
  #define CC_RESTRICT restrict
 #else
  #define CC_RESTRICT
 #endif
#elif defined(_MSC_VER)
 #define CC_ALWAYSINLINE __forceinline
 #define CC_RESTRICT __restrict
#else
 #define CC_ALWAYSINLINE
 #define CC_RESTRICT
#endif

static inline CC_ALWAYSINLINE uint32_t ccIsPow2Int32( uint32_t v )
{
  return ( ( v & ( v - 1 ) ) == 0 );
}

#define ROUND_POSITIVE_FLOAT(x) ((int)((x)+0.5f))


////


#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(__i386__) || defined(__i386) || defined(i386) || defined(_M_X64) || defined(_M_AMD64) || defined(_M_IX86) || defined(_X86_)

/* Input is 0.0,255.0, output is 0.0,1.0 */
static inline CC_ALWAYSINLINE float srgb2linear( float v )
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
    u.i = (int32_t)ROUND_POSITIVE_FLOAT( (float)u.i * 0.8f );
    vpow = u.f;
    vpwsqrt = sqrtf( vpow );
    v = ( ( v2 * vpwsqrt ) + ( ( ( v2 * v ) / vpwsqrt ) / sqrtf( vpwsqrt ) ) ) * 0.51011878327f;
  }
  return v;
}

/* Input is 0.0,1.0, output is 0.0,255.0 */
static inline CC_ALWAYSINLINE float linear2srgb( float v )
{
  float vpow;
  union
  {
    int32_t i;
    float f;
  } u;
  if( v <= 0.0031308f )
    v = v * (12.92f*255.0f);
  else
  {
    u.f = ( v * 6521909350804488192.0f );
    u.i = (int32_t)ROUND_POSITIVE_FLOAT( (float)u.i * 0.666666666666f );
    vpow = u.f;
    v = ( v * vpow ) + ( ( v * v ) / sqrtf( vpow ) );
    v = ( (1.055f*255.0f) * sqrtf( sqrtf( v * 0.5290553722f ) ) - (0.055f*255.0f) );
  }
  return v;
}

#else

/* Input is 0.0,255.0, output is 0.0,1.0 */
/* Only for reference, this is waayyy too slow and should never be used */
static inline CC_ALWAYSINLINE float srgb2linear( float v )
{
  v *= (1.0f/255.0f);
  if( v <= 0.04045f )
    v = v * (1.0f/12.92);
  else
    v = powf( ( v + 0.055f ) * (1.0f/1.055f), 2.4f );
  return v;
}

/* Input is 0.0,1.0, output is 0.0,255.0 */
/* Only for reference, this is waayyy too slow and should never be used */
static inline CC_ALWAYSINLINE float linear2srgb( float v )
{
  if( v <= 0.0031308f )
    v = v * 12.92f;
  else
    v = 1.055f * powf( v, 1.0f/2.4f ) - 0.055f;
  return 255.0f * v;
}

#endif


////


#if CPU_SSE2_SUPPORT

static const float CPU_ALIGN16 srgbLinearConst00[4] = { 0.04045f*255.0f, 0.04045f*255.0f, 0.04045f*255.0f, 1024.0f };
static const float CPU_ALIGN16 srgbLinearConst01[4] = { (1.0f/12.92f)*(1.0f/255.0f), (1.0f/12.92f)*(1.0f/255.0f), (1.0f/12.92f)*(1.0f/255.0f), 1.0f };
static const float CPU_ALIGN16 srgbLinearConst02[4] = { 0.055f*255.0f, 0.055f*255.0f, 0.055f*255.0f, 0.055f*255.0f };
static const float CPU_ALIGN16 srgbLinearConst03[4] = { (1.0f/1.055f)*(1.0f/255.0f), (1.0f/1.055f)*(1.0f/255.0f), (1.0f/1.055f)*(1.0f/255.0f), (1.0f/1.055f)*(1.0f/255.0f) };
static const float CPU_ALIGN16 srgbLinearConst04[4] = { 5417434112.0f, 5417434112.0f, 5417434112.0f, 5417434112.0f };
static const float CPU_ALIGN16 srgbLinearConst05[4] = { 0.8f, 0.8f, 0.8f, 0.8f };
static const float CPU_ALIGN16 srgbLinearConst06[4] = { 0.51011878327f, 0.51011878327f, 0.51011878327f, 0.51011878327f };
static const float CPU_ALIGN16 srgbLinearConst07[4] = { 0.0031308f, 0.0031308f, 0.0031308f, 1024.0f };
static const float CPU_ALIGN16 srgbLinearConst08[4] = { 12.92f*255.0f, 12.92f*255.0f, 12.92f*255.0f, 1.0f };
static const float CPU_ALIGN16 srgbLinearConst09[4] = { 6521909350804488192.0f, 6521909350804488192.0f, 6521909350804488192.0f, 6521909350804488192.0f };
static const float CPU_ALIGN16 srgbLinearConst10[4] = { 0.666666666666f, 0.666666666666f, 0.666666666666f, 0.666666666666f };
static const float CPU_ALIGN16 srgbLinearConst11[4] = { 0.5290553722f, 0.5290553722f, 0.5290553722f, 0.5290553722f };
static const float CPU_ALIGN16 srgbLinearConst12[4] = { 1.055f*255.0f, 1.055f*255.0f, 1.055f*255.0f, 1.055f*255.0f };
static const float CPU_ALIGN16 srgbLinearConst13[4] = { -0.055f*255.0f, -0.055f*255.0f, -0.055f*255.0f, -0.055f*255.0f };
static const float CPU_ALIGN16 srgbLinearConst14[4] = { 0.04045f*255.0f, 0.04045f*255.0f, 0.04045f*255.0f, 0.04045f*255.0f };
static const float CPU_ALIGN16 srgbLinearConst15[4] = { (1.0f/12.92f)*(1.0f/255.0f), (1.0f/12.92f)*(1.0f/255.0f), (1.0f/12.92f)*(1.0f/255.0f), (1.0f/12.92f)*(1.0f/255.0f) };

/* Input is 0.0,255.0 ~ output is 0.0,1.0 ~ alpha channel is passed as-is */
static inline CC_ALWAYSINLINE __m128 srgb2linear3( __m128 vx )
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
  return CPU_BLENDV_PS( vx, vbase, vmask );
}

/* Input is 0.0,1.0 ~ output is 0.0,255.0 ~ alpha channel is passed as-is */
static inline CC_ALWAYSINLINE __m128 linear2srgb3( __m128 vx )
{
  __m128 vmask, vbase, vpow;
  vmask = _mm_cmple_ps( vx, *(__m128*)srgbLinearConst07 );
  vbase = _mm_mul_ps( vx, *(__m128*)srgbLinearConst08 );
  vpow = _mm_castsi128_ps( _mm_cvtps_epi32( _mm_mul_ps( _mm_cvtepi32_ps( _mm_castps_si128( _mm_mul_ps( vx, *(__m128*)srgbLinearConst09 ) ) ), *(__m128*)srgbLinearConst10 ) ) );
  vx = _mm_add_ps( _mm_mul_ps( _mm_sqrt_ps( _mm_sqrt_ps( _mm_mul_ps( _mm_add_ps( _mm_mul_ps( vx, vpow ), _mm_mul_ps( _mm_mul_ps( vx, vx ), _mm_rsqrt_ps( vpow ) ) ), *(__m128*)srgbLinearConst11 ) ) ), *(__m128*)srgbLinearConst12 ), *(__m128*)srgbLinearConst13 );
  return CPU_BLENDV_PS( vx, vbase, vmask );
}

/* Input is 0.0,255.0 ~ output is 0.0,1.0 ~ alpha channel is passed as-is */
static inline CC_ALWAYSINLINE __m128 srgb2linear4( __m128 vx )
{
  __m128 vmask, vbase;
  __m128 vpow, vpwsqrtinv, vpwsqrt, vx2;
  vmask = _mm_cmple_ps( vx, *(__m128*)srgbLinearConst14 );
  vbase = _mm_mul_ps( vx, *(__m128*)srgbLinearConst15 );
  vx = _mm_mul_ps( _mm_add_ps( vx, *(__m128*)srgbLinearConst02 ), *(__m128*)srgbLinearConst03 );
  vx2 = _mm_mul_ps( vx, vx );
  vpow = _mm_castsi128_ps( _mm_cvtps_epi32( _mm_mul_ps( _mm_cvtepi32_ps( _mm_castps_si128( _mm_mul_ps( vx, *(__m128*)srgbLinearConst04 ) ) ), *(__m128*)srgbLinearConst05 ) ) );
  vpwsqrtinv = _mm_rsqrt_ps( vpow );
  vpwsqrt = _mm_mul_ps( vpow, vpwsqrtinv );
  vx = _mm_mul_ps( _mm_add_ps( _mm_mul_ps( vx2, vpwsqrt ), _mm_mul_ps( _mm_mul_ps( _mm_mul_ps( vx2, vx ), vpwsqrtinv ), _mm_rsqrt_ps( vpwsqrt ) ) ), *(__m128*)srgbLinearConst06 );
  return CPU_BLENDV_PS( vx, vbase, vmask );
}

#endif


////


static inline CC_ALWAYSINLINE double bessel( double x )
{
  double sum, t, y;

  /* Zeroth order Bessel function of the first kind. */
  sum = 1.0;
  y = x * x * 0.25;
  t = y;
  sum += t;
  t *= y * (1.0/(2.0*2.0));
  sum += t;
  t *= y * (1.0/(3.0*3.0));
  sum += t;
  t *= y * (1.0/(4.0*4.0));
  sum += t;
  t *= y * (1.0/(5.0*5.0));
  sum += t;
  t *= y * (1.0/(6.0*6.0));
  sum += t;
  t *= y * (1.0/(7.0*7.0));
  sum += t;
  t *= y * (1.0/(8.0*8.0));
  sum += t;
  t *= y * (1.0/(9.0*9.0));
  sum += t;
  t *= y * (1.0/(10.0*10.0));
  sum += t;
  t *= y * (1.0/(11.0*11.0));
  sum += t;
  t *= y * (1.0/(12.0*12.0));
  sum += t;
  t *= y * (1.0/(13.0*13.0));
  sum += t;
  t *= y * (1.0/(14.0*14.0));
  sum += t;

  return sum;
}

static inline CC_ALWAYSINLINE double kaiser( double x, double beta )
{
  return bessel( beta * sqrt( fmax( 0.0, 1.0 - ( x * x ) ) ) );
}

static inline CC_ALWAYSINLINE double sinc( double x )
{
  if( x == 0.0 )
    return 1.0;
  x = sin( x * M_PI ) / ( x * M_PI );
  return x;
}


////


#if CPU_SSE2_SUPPORT

static inline CC_ALWAYSINLINE __m128 simd4f_bessel( __m128 x )
{
  __m128 sum, t, y;

  sum = *(__m128 *)simd4fOne;
  y = _mm_mul_ps( *(__m128 *)simd4fQuarter, _mm_mul_ps( x, x ) );
  t = y;
  sum = _mm_add_ps( sum, t );
  t = _mm_mul_ps( t, _mm_mul_ps( y, _mm_set1_ps( 1.0f/(2.0f*2.0f) ) ) );
  sum = _mm_add_ps( sum, t );
  t = _mm_mul_ps( t, _mm_mul_ps( y, _mm_set1_ps( 1.0f/(3.0f*3.0f) ) ) );
  sum = _mm_add_ps( sum, t );
  t = _mm_mul_ps( t, _mm_mul_ps( y, _mm_set1_ps( 1.0f/(4.0f*4.0f) ) ) );
  sum = _mm_add_ps( sum, t );
  t = _mm_mul_ps( t, _mm_mul_ps( y, _mm_set1_ps( 1.0f/(5.0f*5.0f) ) ) );
  sum = _mm_add_ps( sum, t );
  t = _mm_mul_ps( t, _mm_mul_ps( y, _mm_set1_ps( 1.0f/(6.0f*6.0f) ) ) );
  sum = _mm_add_ps( sum, t );
  t = _mm_mul_ps( t, _mm_mul_ps( y, _mm_set1_ps( 1.0f/(7.0f*7.0f) ) ) );
  sum = _mm_add_ps( sum, t );
  t = _mm_mul_ps( t, _mm_mul_ps( y, _mm_set1_ps( 1.0f/(8.0f*8.0f) ) ) );
  sum = _mm_add_ps( sum, t );
  t = _mm_mul_ps( t, _mm_mul_ps( y, _mm_set1_ps( 1.0f/(9.0f*9.0f) ) ) );
  sum = _mm_add_ps( sum, t );
  t = _mm_mul_ps( t, _mm_mul_ps( y, _mm_set1_ps( 1.0f/(10.0f*10.0f) ) ) );
  sum = _mm_add_ps( sum, t );
  t = _mm_mul_ps( t, _mm_mul_ps( y, _mm_set1_ps( 1.0f/(11.0f*11.0f) ) ) );
  sum = _mm_add_ps( sum, t );
  t = _mm_mul_ps( t, _mm_mul_ps( y, _mm_set1_ps( 1.0f/(12.0f*12.0f) ) ) );
  sum = _mm_add_ps( sum, t );
  t = _mm_mul_ps( t, _mm_mul_ps( y, _mm_set1_ps( 1.0f/(13.0f*13.0f) ) ) );
  sum = _mm_add_ps( sum, t );
  t = _mm_mul_ps( t, _mm_mul_ps( y, _mm_set1_ps( 1.0f/(14.0f*14.0f) ) ) );
  sum = _mm_add_ps( sum, t );

  return sum;
}

static inline CC_ALWAYSINLINE __m128 simd4f_kaiser( __m128 x, __m128 beta )
{
  return simd4f_bessel( _mm_mul_ps( beta, _mm_sqrt_ps( _mm_max_ps( _mm_setzero_ps(), _mm_sub_ps( *(__m128 *)simd4fOne, _mm_mul_ps( x, x ) ) ) ) ) );
}

static inline CC_ALWAYSINLINE __m128 simd4f_sinc( __m128 x )
{
  __m128 zeromask;
  zeromask = _mm_cmpeq_ps( x, _mm_setzero_ps() );
  x = _mm_mul_ps( x, _mm_load_ps( simd4fPi ) );
  x = _mm_div_ps( simd4f_sin_ps( x ), x );
  x = CPU_BLENDV_PS( x, *(__m128 *)simd4fOne, zeromask );
  return x;
}

#endif


////


typedef struct
{
  int matrixsize;
  int matrixoffset;
  int matrixrowwidth;
  int matrixrowsize;
  int rowreturn;
  float *matrix;

  int minimumalpha;
  float dithersum;
  float minimumalphaf;
  float amplifynormal;
  float normalsustainfactor;
  void *alloc;

  unsigned char *srcdata;
  int width1;
  int width2;
  int width3;
  int width4;
  int height;
  int bytesperline;
} imStaticMatrixState;


static int imBuildStaticMatrix( imStaticMatrixState * CC_RESTRICT state, int sizedivisor, float hopcount, float alpha )
{
  int i, j, minx, maxx;
  double x, xshift, hopsize, offset, scalefactor, hopcountinv, beta, linsq, sum;
  double *linear;
  float suminv;
  float *matrix;

  if( alpha > 16.0f )
    alpha = 16.0f;
  beta = (double)alpha * (double)M_PI;
  hopcountinv = 1.0 / (double)hopcount;

  scalefactor = 1.0 / (double)sizedivisor;
  hopsize = 0.5 * (double)sizedivisor;
  offset = hopsize - 0.5;
  minx = (int)ceil( ( (double)-hopcount * hopsize ) + offset );
  maxx = (int)floor( ( (double)hopcount * hopsize ) + offset );
  state->matrixoffset = minx;
  state->matrixsize = ( maxx - minx ) + 1;
  state->matrixrowwidth = ( state->matrixsize + 3 ) & ~3;
  state->rowreturn = state->matrixrowwidth - state->matrixsize;
  state->matrixrowsize = state->matrixrowwidth * sizeof(float);

#if IM_RESIZE_DEBUG
  printf( "ResizeMatrix ; scalefactor %.3f, offset %.3f, hopsize %.3f\n", scalefactor, offset, hopsize );
#endif

  linear = malloc( state->matrixrowwidth * sizeof(double) );
  for( i = 0 ; i < state->matrixsize ; i++ )
  {
    x = (double)( i + state->matrixoffset );
    xshift = 2.0 * scalefactor * ( x - offset );
    linear[i] = sinc( xshift ) * kaiser( hopcountinv * xshift, beta );
#if IM_RESIZE_DEBUG
    printf( " x[%+.3f] = %+.3f ( %+.3f * %+.3f )\n", x, linear[i], sinc( xshift ), kaiser( hopcountinv * xshift, beta ) );
#endif
  }
  for( ; i < state->matrixrowwidth ; i++ )
    linear[i] = 0.0;

  /* Build normalized state */
  state->alloc = malloc( ( state->matrixsize * state->matrixrowsize ) + 16 );
  state->matrix = (void *)( ( (uintptr_t)state->alloc + 0xf ) & ~0xf );
  matrix = state->matrix;
  sum = 0.0;
  for( i = 0 ; i < state->matrixsize ; i++ )
  {
    for( j = 0 ; j < state->matrixsize ; j++ )
    {
      linsq = linear[i] * linear[j];
      matrix[j] = (float)linsq;
      sum += linsq;
    }
    for( ; j < state->matrixrowwidth ; j++ )
      matrix[j] = 0.0f;
    matrix += state->matrixrowwidth;
  }
  free( linear );

#if IM_RESIZE_DEBUG
  printf( "Matrix sum : %f\n", sum );
#endif

  suminv = (float)( 1.0 / sum );
  j = state->matrixsize * state->matrixrowwidth;
  for( i = 0 ; i < j ; i++ )
    state->matrix[i] *= suminv;

#if IM_RESIZE_DEBUG
  printf( "Matrix %dx%d :\n", state->matrixsize, state->matrixsize );
  for( i = 0 ; i < state->matrixsize ; i++ )
  {
    for( j = 0 ; j < state->matrixsize ; j++ )
      printf( " %+.6f", state->matrix[ ( i * state->matrixrowwidth ) + j ] );
    printf( "\n" );
  }
  printf( "Matrix Offset : %d\n", state->matrixoffset );
  printf( "Matrix Size : %d\n", state->matrixsize );
  printf( "Matrix Rowwidth : %d\n", state->matrixrowwidth );
#endif

  return 1;
}


static void imFreeStaticState( imStaticMatrixState * CC_RESTRICT state )
{
  free( state->alloc );
  state->alloc = 0;
  return;
}


////


typedef struct
{
  int matrixsizex, matrixsizey;
  int matrixoffsetx, matrixoffsety;
  float *linearx;
  float *lineary;
  float beta;
  float hopcountinv;

  float dithersum;
  int minimumalpha;
  float minimumalphaf;
  float amplifynormal;
  float normalsustainfactor;
  void *alloc;

  unsigned char *srcdata;
  int width1;
  int width2;
  int width3;
  int width4;
  int height;
  int bytesperline;
} imGenericMatrixState;


static inline int imAllocGenericState( imGenericMatrixState *state, float scalex, float scaley, float hopcount, float alpha )
{
  int allocx, allocy, size;
  void *align;
  if( alpha > 16.0f )
    alpha = 16.0f;
  allocx = ( (int)ceilf( hopcount / scalex ) + 2 + 3 ) & ~0x3;
  allocy = ( (int)ceilf( hopcount / scaley ) + 2 + 3 ) & ~0x3;
  size = ( ( allocx + allocy ) * sizeof(float) ) + 16;
  state->alloc = malloc( size );
  memset( state->alloc, 0, size );
  align = (void *)( ( (uintptr_t)state->alloc + 0xf ) & ~0xf );
  state->linearx = align;
  state->lineary = ADDRESS( align, allocx * sizeof(float) );
  state->beta = alpha * (float)M_PI;
  state->hopcountinv = 1.0f / (float)hopcount;
  return 1;
}

static inline void imBuildGenericLinearX( imGenericMatrixState *state, float scalex, float scaleinvx, float sourcex, float hopcount, float alpha, int width )
{
  int i, minx, maxx;
  float hopsizex, offsetx;
  float *linearx;

  hopsizex = 0.5f * scaleinvx;
  offsetx = (float)sourcex;
  minx = (int)ceil( ( -hopcount * hopsizex ) + offsetx );
  maxx = (int)floor( ( hopcount * hopsizex ) + offsetx );
  state->matrixsizex = ( maxx - minx ) + 1;
  state->matrixoffsetx = ( minx + ( width << 8 ) ) % width;

  linearx = state->linearx;
  scalex *= 2.0f;
#if CPU_SSE2_SUPPORT
  for( i = 0 ; i < state->matrixsizex ; i += 4 )
  {
    __m128 vx, vxshift;
    vx = _mm_add_ps( _mm_set1_ps( (float)( i + minx ) ), _mm_load_ps( simd4fZeroOneTwoThree ) );
    vxshift = _mm_mul_ps( _mm_set1_ps( scalex ), _mm_sub_ps( vx, _mm_set1_ps( offsetx ) ) );
    _mm_store_ps( &linearx[i], _mm_mul_ps( simd4f_sinc( vxshift ), simd4f_kaiser( _mm_mul_ps( _mm_set1_ps( state->hopcountinv ), vxshift ), _mm_set1_ps( state->beta ) ) ) );
 #if IM_RESIZE_DEBUG
    printf( " linearx[%d] = %.3f\n", i+minx+0, linearx[i+0] );
    printf( " linearx[%d] = %.3f\n", i+minx+1, linearx[i+1] );
    printf( " linearx[%d] = %.3f\n", i+minx+2, linearx[i+2] );
    printf( " linearx[%d] = %.3f\n", i+minx+3, linearx[i+3] );
 #endif
  }
#else
  for( i = 0 ; i < state->matrixsizex ; i++ )
  {
    float x, xshift;
    x = (float)( i + minx );
    xshift = scalex * ( x - offsetx );
    linearx[i] = (float)( sinc( xshift ) * kaiser( state->hopcountinv * xshift, state->beta ) );
 #if IM_RESIZE_DEBUG
    printf( " linearx[%+.3f] = %.3f ( %+.3f * %+.3f )\n", x, linearx[i], sinc( xshift ), kaiser( state->hopcountinv * xshift, state->beta ) );
 #endif
  }
#endif

  return;
}

static inline void imBuildGenericLinearY( imGenericMatrixState *state, float scaley, float scaleinvy, float sourcey, float hopcount, float alpha, int height )
{
  int i, miny, maxy;
  float hopsizey, offsety;
  float *lineary;

  hopsizey = 0.5f * scaleinvy;
  offsety = (float)sourcey;
  miny = (int)ceil( ( -hopcount * hopsizey ) + offsety );
  maxy = (int)floor( ( hopcount * hopsizey ) + offsety );
  state->matrixsizey = ( maxy - miny ) + 1;
  state->matrixoffsety = ( miny + ( height << 8 ) ) % height;

  lineary = state->lineary;
  scaley *= 2.0f;
#if CPU_SSE2_SUPPORT
  for( i = 0 ; i < state->matrixsizey ; i += 4 )
  {
    __m128 vy, vyshift;
    vy = _mm_add_ps( _mm_set1_ps( (float)( i + miny ) ), _mm_load_ps( simd4fZeroOneTwoThree ) );
    vyshift = _mm_mul_ps( _mm_set1_ps( scaley ), _mm_sub_ps( vy, _mm_set1_ps( offsety ) ) );
    _mm_store_ps( &lineary[i], _mm_mul_ps( simd4f_sinc( vyshift ), simd4f_kaiser( _mm_mul_ps( _mm_set1_ps( state->hopcountinv ), vyshift ), _mm_set1_ps( state->beta ) ) ) );
 #if IM_RESIZE_DEBUG
    printf( " lineary[%d] = %.3f\n", i+miny+0, lineary[i+0] );
    printf( " lineary[%d] = %.3f\n", i+miny+1, lineary[i+1] );
    printf( " lineary[%d] = %.3f\n", i+miny+2, lineary[i+2] );
    printf( " lineary[%d] = %.3f\n", i+miny+3, lineary[i+3] );
 #endif
  }
#else
  for( i = 0 ; i < state->matrixsizey ; i++ )
  {
    float y, yshift;
    y = (float)( i + miny );
    yshift = scaley * ( y - offsety );
    lineary[i] = (float)( sinc( yshift ) * kaiser( state->hopcountinv * yshift, state->beta ) );
 #if IM_RESIZE_DEBUG
    printf( " lineary[%+.3f] = %.3f ( %+.3f * %+.3f )\n", y, lineary[i], sinc( yshift ), kaiser( state->hopcountinv * yshift, state->beta ) );
 #endif
  }
#endif

  return;
}


static inline void imFreeGenericState( imGenericMatrixState *state )
{
  free( state->alloc );
  state->alloc = 0;
  return;
}



////////////////////////////////////////////////////////////////////////////////



static void imStaticKernel1Linear( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
  float f, sum0;
  float *matrix;
  unsigned char *src;

  sum0 = 0.0f;
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx;
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
      f = matrix[x];
      sum0 += f * (float)src[ mapx + 0 ];
      mapx++;
      if( mapx >= state->width1 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );

  return;
}

static void imStaticKernel2Linear( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
  float f, sum0, sum1;
  float *matrix;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx << 1;
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
      f = matrix[x];
      sum0 += f * (float)src[ mapx + 0 ];
      sum1 += f * (float)src[ mapx + 1 ];
      mapx += 2;
      if( mapx >= state->width2 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );

  return;
}

static void imStaticKernel3Linear( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
  float f, sum0, sum1, sum2;
  float *matrix;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx + ( pointx << 1 );
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
      f = matrix[x];
      sum0 += f * (float)src[ mapx + 0 ];
      sum1 += f * (float)src[ mapx + 1 ];
      sum2 += f * (float)src[ mapx + 2 ];
      mapx += 3;
      if( mapx >= state->width3 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );

  return;
}

static void imStaticKernel4Linear( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
#if CPU_SSE2_SUPPORT
  __m128 vsum, vsrc;
  __m128i vzero;
#else
  float f, sum0, sum1, sum2, sum3;
#endif
  float *matrix;
  unsigned char *src;

#if CPU_SSE2_SUPPORT
  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
#else
  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
#endif
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx << 2;
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
#if CPU_SSE2_SUPPORT
      vsrc = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx ] ) ), vzero ) );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_set1_ps( matrix[x] ), vsrc ) );
#else
      f = matrix[x];
      sum0 += f * (float)src[ mapx + 0 ];
      sum1 += f * (float)src[ mapx + 1 ];
      sum2 += f * (float)src[ mapx + 2 ];
      sum3 += f * (float)src[ mapx + 3 ];
#endif
      mapx += 4;
      if( mapx >= state->width4 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

#if CPU_SSE2_SUPPORT
  _mm_store_ss( (void *)dst, _mm_castsi128_ps( _mm_packus_epi16( _mm_packs_epi32( _mm_cvtps_epi32( vsum ), vzero ), vzero ) ) );
#else
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );
  dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );
#endif

  return;
}


#if CPU_SSE2_SUPPORT

static void imStaticKernel4Linear_Core( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
  __m128 vsum, vf, v0, v1, v2, v3;
  __m128i vzero;
  float *matrix;
  unsigned char *src;

  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx << 2;
    for( x = 0 ; x < state->matrixsize ; x += 4 )
    {
      vf = _mm_load_ps( &matrix[x] );
      v0 = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx + 0 ] ) ), vzero ) );
      v1 = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx + 4 ] ) ), vzero ) );
      v2 = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx + 8 ] ) ), vzero ) );
      v3 = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx + 12 ] ) ), vzero ) );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_shuffle_ps( vf, vf, 0x00 ), v0 ) );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_shuffle_ps( vf, vf, 0x55 ), v1 ) );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_shuffle_ps( vf, vf, 0xaa ), v2 ) );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_shuffle_ps( vf, vf, 0xff ), v3 ) );
      mapx += 16;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  _mm_store_ss( (void *)dst, _mm_castsi128_ps( _mm_packus_epi16( _mm_packs_epi32( _mm_cvtps_epi32( vsum ), vzero ), vzero ) ) );

  return;
}

#endif


////


static void imStaticKernel4LinearAlphaNorm( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
  float f, sum0, sum1, sum2, sum3;
  float *matrix;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx << 2;
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
      f = matrix[x] * (float)src[ mapx + 3 ];
      sum0 += f * (float)src[ mapx + 0 ];
      sum1 += f * (float)src[ mapx + 1 ];
      sum2 += f * (float)src[ mapx + 2 ];
      sum3 += f;
      mapx += 4;
      if( mapx >= state->width4 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  if( sum3 >= state->minimumalphaf )
  {
    f = 1.0f / sum3;
    dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, ( sum0 * f ) + 0.5f ) ) );
    dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, ( sum1 * f ) + 0.5f ) ) );
    dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, ( sum2 * f ) + 0.5f ) ) );
    dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );
  }
  else
  {
    dst[0] = 0;
    dst[1] = 0;
    dst[2] = 0;
    dst[3] = 0;
  }

  return;
}


#if CPU_SSE2_SUPPORT

static void imStaticKernel4LinearAlphaNorm_Core( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
  uint32_t pixel;
  float *matrix;
  unsigned char *src;
  __m128 vsum0, vsum1, vsum2, vsum3;
  __m128 vf, valpha, vr, vg, vb, va, vsrcf;
  __m128i vsrc, vshufmask;
  __m128i vzero;

 #if CPU_SSSE3_SUPPORT
  vshufmask = _mm_setr_epi8( 0x00,0x04,0x08,0x0c, 0x01,0x05,0x09,0x0d, 0x02,0x06,0x0a,0x0e, 0x03,0x07,0x0b,0x0f );
 #endif
  vsum0 = _mm_setzero_ps();
  vsum1 = _mm_setzero_ps();
  vsum2 = _mm_setzero_ps();
  vsum3 = _mm_setzero_ps();
  vzero = _mm_castps_si128( _mm_setzero_ps() );

  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx << 2;
    for( x = 0 ; x < state->matrixsize ; x += 4 )
    {
      vf = _mm_load_ps( &matrix[x] );
      /* Load 16 bytes and unpack as RRRR,GGGG,BBBB,AAAA in one SSE register */
      vsrc = _mm_loadu_si128( (void *)&src[ mapx ] );
 #if CPU_SSSE3_SUPPORT
      vsrc = _mm_shuffle_epi8( vsrc, vshufmask );
 #else
      vshufmask = _mm_shuffle_epi32( vsrc, 0x39 );
      vsrc = _mm_unpacklo_epi16( _mm_unpacklo_epi8( vsrc, vshufmask ), _mm_unpackhi_epi8( vsrc, vshufmask ) );
 #endif
      /* Break that into 4 SSE registers as floats: vR,vG,vB,vA */
      vsrcf = _mm_castsi128_ps( vsrc );
      vr = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( vsrcf ), vzero ) );
 #if CPU_SSE3_SUPPORT
      vg = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_movehdup_ps( vsrcf ) ), vzero ) );
 #else
      vg = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_shuffle_ps( vsrcf, vsrcf, 0x55 ) ), vzero ) );
 #endif
      vb = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_movehl_ps( vsrcf, vsrcf ) ), vzero ) );
      va = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_shuffle_ps( vsrcf, vsrcf, 0xff ) ), vzero ) );
      valpha = _mm_mul_ps( va, vf );
      vsum0 = _mm_add_ps( vsum0, _mm_mul_ps( vr, valpha ) );
      vsum1 = _mm_add_ps( vsum1, _mm_mul_ps( vg, valpha ) );
      vsum2 = _mm_add_ps( vsum2, _mm_mul_ps( vb, valpha ) );
      vsum3 = _mm_add_ps( vsum3, valpha );
      mapx += 16;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

 #if CPU_SSE3_SUPPORT
  vsum0 = _mm_hadd_ps( vsum0, vsum1 );
  vsum2 = _mm_hadd_ps( vsum2, vsum3 );
  vsum0 = _mm_hadd_ps( vsum0, vsum2 );
 #else
  vsum0 = _mm_add_ps( _mm_unpacklo_ps( vsum0, vsum2 ), _mm_unpackhi_ps( vsum0, vsum2 ) );
  vsum1 = _mm_add_ps( _mm_unpacklo_ps( vsum1, vsum3 ), _mm_unpackhi_ps( vsum1, vsum3 ) );
  vsum0 = _mm_add_ps( _mm_unpacklo_ps( vsum0, vsum1 ), _mm_unpackhi_ps( vsum0, vsum1 ) );
 #endif

  valpha = _mm_shuffle_ps( vsum0, vsum0, 0xff );
  pixel = 0;
  if( _mm_comige_ss( valpha, _mm_load_ss( &state->minimumalphaf ) ) )
  {
    __m128i vpixel;
    vsum0 = _mm_mul_ps( vsum0, _mm_rcp_ps( valpha ) );
    vsum0 = CPU_BLENDV_PS( vsum0, valpha, *(__m128 *)simd4fAlphaMask );
    vpixel = _mm_cvtps_epi32( vsum0 );
    vpixel = _mm_packs_epi32( vpixel, vpixel );
    vpixel = _mm_packus_epi16( vpixel, vpixel );
    pixel = (uint32_t)_mm_cvtsi128_si32( vpixel );
  }
  *(uint32_t *)dst = pixel;

  return;
}

#endif


////


static void imStaticKernel1sRGB( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
#if CPU_SSE2_SUPPORT
  __m128 vsum, vsrc;
  __m128i vzero;
#else
  float f, sum0;
#endif
  float *matrix;
  unsigned char *src;

#if CPU_SSE2_SUPPORT
  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
#else
  sum0 = 0.0f;
#endif
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx;
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
#if CPU_SSE2_SUPPORT
      vsrc = _mm_set_ss( (float)src[ mapx + 0 ] );
      vsrc = srgb2linear3( vsrc );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_set_ss( matrix[x] ), vsrc ) );
#else
      f = matrix[x];
      sum0 += f * srgb2linear( (float)src[ mapx + 0 ] );
#endif
      mapx++;
      if( mapx >= state->width1 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

#if CPU_SSE2_SUPPORT
  dst[0] = _mm_cvtsi128_si32( _mm_packus_epi16( _mm_packs_epi32( _mm_cvtps_epi32( linear2srgb3( vsum ) ), vzero ), vzero ) );
#else
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum0 ) + 0.5f ) ) );
#endif

  return;
}

static void imStaticKernel2sRGB( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
#if CPU_SSE2_SUPPORT
  __m128 vsum, vsrc;
  __m128i vzero;
#else
  float f, sum0, sum1;
#endif
  float *matrix;
  unsigned char *src;

#if CPU_SSE2_SUPPORT
  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
#else
  sum0 = 0.0f;
  sum1 = 0.0f;
#endif
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx << 1;
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
#if CPU_SSE2_SUPPORT
      vsrc = _mm_set_ps( 0.0f, 0.0f, (float)src[ mapx + 1 ], (float)src[ mapx + 0 ] );
      vsrc = srgb2linear3( vsrc );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_set1_ps( matrix[x] ), vsrc ) );
#else
      f = matrix[x];
      sum0 += f * srgb2linear( (float)src[ mapx + 0 ] );
      sum1 += f * srgb2linear( (float)src[ mapx + 1 ] );
#endif
      mapx += 2;
      if( mapx >= state->width2 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

#if CPU_SSE2_SUPPORT
  union
  {
    char c[4];
    uint32_t i;
  } u;
  vsum = linear2srgb3( vsum );
  _mm_store_ss( (void *)&u.i, _mm_castsi128_ps( _mm_packus_epi16( _mm_packs_epi32( _mm_cvtps_epi32( vsum ), vzero ), vzero ) ) );
  dst[0] = u.c[0];
  dst[1] = u.c[1];
#else
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum0 ) + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum1 ) + 0.5f ) ) );
#endif

  return;
}

static void imStaticKernel3sRGB( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
#if CPU_SSE2_SUPPORT
  __m128 vsum, vsrc;
  __m128i vzero;
#else
  float f, sum0, sum1, sum2;
#endif
  float *matrix;
  unsigned char *src;

#if CPU_SSE2_SUPPORT
  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
#else
  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
#endif
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx + ( pointx << 1 );
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
#if CPU_SSE2_SUPPORT
      vsrc = _mm_set_ps( 0.0f, (float)src[ mapx + 2 ], (float)src[ mapx + 1 ], (float)src[ mapx + 0 ] );
      vsrc = srgb2linear3( vsrc );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_set1_ps( matrix[x] ), vsrc ) );
#else
      f = matrix[x];
      sum0 += f * srgb2linear( (float)src[ mapx + 0 ] );
      sum1 += f * srgb2linear( (float)src[ mapx + 1 ] );
      sum2 += f * srgb2linear( (float)src[ mapx + 2 ] );
#endif
      mapx += 3;
      if( mapx >= state->width3 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

#if CPU_SSE2_SUPPORT
  union
  {
    char c[4];
    uint32_t i;
  } u;
  vsum = linear2srgb3( vsum );
  _mm_store_ss( (void *)&u.i, _mm_castsi128_ps( _mm_packus_epi16( _mm_packs_epi32( _mm_cvtps_epi32( vsum ), vzero ), vzero ) ) );
  dst[0] = u.c[0];
  dst[1] = u.c[1];
  dst[2] = u.c[2];
#else
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum0 ) + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum1 ) + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum2 ) + 0.5f ) ) );
#endif

  return;
}

static void imStaticKernel4sRGB( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
#if CPU_SSE2_SUPPORT
  __m128 vsum, vsrc;
  __m128i vzero;
#else
  float f, sum0, sum1, sum2, sum3;
#endif
  float *matrix;
  unsigned char *src;

#if CPU_SSE2_SUPPORT
  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
#else
  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
#endif
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx << 2;
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
#if CPU_SSE2_SUPPORT
      vsrc = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx ] ) ), vzero ) );
      vsrc = srgb2linear3( vsrc );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_set1_ps( matrix[x] ), vsrc ) );
#else
      f = matrix[x];
      sum0 += f * srgb2linear( (float)src[ mapx + 0 ] );
      sum1 += f * srgb2linear( (float)src[ mapx + 1 ] );
      sum2 += f * srgb2linear( (float)src[ mapx + 2 ] );
      sum3 += f * (float)src[ mapx + 3 ];
#endif
      mapx += 4;
      if( mapx >= state->width4 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

#if CPU_SSE2_SUPPORT
  vsum = linear2srgb3( vsum );
  _mm_store_ss( (void *)dst, _mm_castsi128_ps( _mm_packus_epi16( _mm_packs_epi32( _mm_cvtps_epi32( vsum ), vzero ), vzero ) ) );
#else
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum0 ) + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum1 ) + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum2 ) + 0.5f ) ) );
  dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );
#endif

  return;
}


#if CPU_SSE2_SUPPORT

static void imStaticKernel3sRGB_Core( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
  __m128 vsum0, vsum1, vsum2, vsrc0, vsrc1, vsrc2, vf;
  __m128i vzero;
  float *matrix;
  unsigned char *src;
  union
  {
    char c[4];
    uint32_t i;
  } u;

  vsum0 = _mm_setzero_ps();
  vsum1 = _mm_setzero_ps();
  vsum2 = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx + ( pointx << 1 );
    for( x = 0 ; x < state->matrixsize ; x += 4 )
    {
      vsrc0 = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx+0 ] ) ), vzero ) );
      vsrc1 = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx+4 ] ) ), vzero ) );
      vsrc2 = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx+8 ] ) ), vzero ) );
      vsrc0 = srgb2linear4( vsrc0 );
      vsrc1 = srgb2linear4( vsrc1 );
      vsrc2 = srgb2linear4( vsrc2 );
      vf = _mm_load_ps( &matrix[x] );
      vsum0 = _mm_add_ps( vsum0, _mm_mul_ps( _mm_shuffle_ps( vf, vf, 0x40 ), vsrc0 ) );
      vsum1 = _mm_add_ps( vsum1, _mm_mul_ps( _mm_shuffle_ps( vf, vf, 0xA5 ), vsrc1 ) );
      vsum2 = _mm_add_ps( vsum2, _mm_mul_ps( _mm_shuffle_ps( vf, vf, 0xFE ), vsrc2 ) );
      mapx += 12;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

 #if CPU_SSSE3_SUPPORT
  vsum0 = _mm_add_ps( vsum0, _mm_castsi128_ps( _mm_alignr_epi8( _mm_castps_si128( vsum1 ), _mm_castps_si128( vsum0 ), 12 ) ) );
  vsum0 = _mm_add_ps( vsum0, _mm_castsi128_ps( _mm_alignr_epi8( _mm_castps_si128( vsum2 ), _mm_castps_si128( vsum1 ), 8 ) ) );
  vsum0 = _mm_add_ps( vsum0, _mm_castsi128_ps( _mm_alignr_epi8( _mm_castps_si128( vsum2 ), _mm_castps_si128( vsum2 ), 4 ) ) );
 #else
  vf = _mm_shuffle_ps( vsum0, vsum1, 0x4f );
  vsum0 = _mm_add_ps( vsum0, _mm_shuffle_ps( vf, vf, 0x38 ) );
  vsum0 = _mm_add_ps( vsum0, _mm_shuffle_ps( vsum1, vsum2, 0x0E ) );
  vsum0 = _mm_add_ps( vsum0, _mm_shuffle_ps( vsum2, vsum2, 0x39 ) );
 #endif

  vsum0 = linear2srgb3( vsum0 );
  _mm_store_ss( (void *)&u.i, _mm_castsi128_ps( _mm_packus_epi16( _mm_packs_epi32( _mm_cvtps_epi32( vsum0 ), vzero ), vzero ) ) );
  dst[0] = u.c[0];
  dst[1] = u.c[1];
  dst[2] = u.c[2];

  return;
}

static void imStaticKernel4sRGB_Core( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
  __m128 vsum, vsrc0, vsrc1;
  __m128i vzero;
  float *matrix;
  unsigned char *src;

  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx << 2;
    for( x = 0 ; x < state->matrixsize ; x += 2 )
    {
      vsrc0 = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx+0 ] ) ), vzero ) );
      vsrc1 = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx+4 ] ) ), vzero ) );
      vsrc0 = srgb2linear3( vsrc0 );
      vsrc1 = srgb2linear3( vsrc1 );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_set1_ps( matrix[x+0] ), vsrc0 ) );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_set1_ps( matrix[x+1] ), vsrc1 ) );
      mapx += 8;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  vsum = linear2srgb3( vsum );
  _mm_store_ss( (void *)dst, _mm_castsi128_ps( _mm_packus_epi16( _mm_packs_epi32( _mm_cvtps_epi32( vsum ), vzero ), vzero ) ) );

  return;
}

#endif


////


static void imStaticKernel4sRGBAlphaNorm( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
#if CPU_SSE2_SUPPORT
  __m128 vsum, vsrc, valpha;
  __m128i vzero;
  uint32_t pixel;
#else
  float f, sum0, sum1, sum2, sum3;
#endif
  float *matrix;
  unsigned char *src;

#if CPU_SSE2_SUPPORT
  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
#else
  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
#endif
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx << 2;
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
#if CPU_SSE2_SUPPORT
      vsrc = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx ] ) ), vzero ) );
      valpha = _mm_shuffle_ps( vsrc, _mm_set_ss( 1.0f ), 0x0f );
      vsrc = srgb2linear3( vsrc );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_mul_ps( _mm_shuffle_ps( valpha, valpha, 0xC0 ), _mm_set1_ps( matrix[x] ) ), vsrc ) );
#else
      f = matrix[x] * (float)src[ mapx + 3 ];
      sum0 += f * srgb2linear( (float)src[ mapx + 0 ] );
      sum1 += f * srgb2linear( (float)src[ mapx + 1 ] );
      sum2 += f * srgb2linear( (float)src[ mapx + 2 ] );
      sum3 += f;
#endif
      mapx += 4;
      if( mapx >= state->width4 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

#if CPU_SSE2_SUPPORT
  valpha = _mm_shuffle_ps( vsum, vsum, 0xff );
  pixel = 0;
  if( _mm_comige_ss( valpha, _mm_load_ss( &state->minimumalphaf ) ) )
  {
    __m128i vpixel;
    vsum = _mm_mul_ps( vsum, _mm_rcp_ps( valpha ) );
    vsum = CPU_BLENDV_PS( vsum, valpha, *(__m128 *)simd4fAlphaMask );
    vsum = linear2srgb3( vsum );
    vpixel = _mm_cvtps_epi32( vsum );
    vpixel = _mm_packs_epi32( vpixel, vpixel );
    vpixel = _mm_packus_epi16( vpixel, vpixel );
    pixel = (uint32_t)_mm_cvtsi128_si32( vpixel );
  }
  *(uint32_t *)dst = pixel;
#else
  if( sum3 >= state->minimumalphaf )
  {
    f = 1.0f / sum3;
    dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum0 * f ) + 0.5f ) ) );
    dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum1 * f ) + 0.5f ) ) );
    dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum2 * f ) + 0.5f ) ) );
    dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );
  }
  else
  {
    dst[0] = 0;
    dst[1] = 0;
    dst[2] = 0;
    dst[3] = 0;
  }
#endif

  return;
}


#if CPU_SSE2_SUPPORT

static void imStaticKernel4sRGBAlphaNorm_Core( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
  __m128 vsum, vsrc0, vsrc1, valpha0, valpha1;
  __m128i vzero;
  uint32_t pixel;
  float *matrix;
  unsigned char *src;

  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx << 2;
    for( x = 0 ; x < state->matrixsize ; x += 2 )
    {
      vsrc0 = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx+0 ] ) ), vzero ) );
      vsrc1 = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx+4 ] ) ), vzero ) );
      valpha0 = _mm_shuffle_ps( vsrc0, _mm_set_ss( 1.0f ), 0x0f );
      valpha1 = _mm_shuffle_ps( vsrc1, _mm_set_ss( 1.0f ), 0x0f );
      vsrc0 = srgb2linear3( vsrc0 );
      vsrc1 = srgb2linear3( vsrc1 );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_mul_ps( _mm_shuffle_ps( valpha0, valpha0, 0xC0 ), _mm_set1_ps( matrix[x+0] ) ), vsrc0 ) );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_mul_ps( _mm_shuffle_ps( valpha1, valpha1, 0xC0 ), _mm_set1_ps( matrix[x+1] ) ), vsrc1 ) );
      mapx += 8;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  valpha0 = _mm_shuffle_ps( vsum, vsum, 0xff );
  pixel = 0;
  if( _mm_comige_ss( valpha0, _mm_load_ss( &state->minimumalphaf ) ) )
  {
    __m128i vpixel;
    vsum = _mm_mul_ps( vsum, _mm_rcp_ps( valpha0 ) );
    vsum = CPU_BLENDV_PS( vsum, valpha0, *(__m128 *)simd4fAlphaMask );
    vsum = linear2srgb3( vsum );
    vpixel = _mm_cvtps_epi32( vsum );
    vpixel = _mm_packs_epi32( vpixel, vpixel );
    vpixel = _mm_packus_epi16( vpixel, vpixel );
    pixel = (uint32_t)_mm_cvtsi128_si32( vpixel );
  }
  *(uint32_t *)dst = pixel;

  return;
}

#endif

////


static void imStaticKernel3Normal( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
  float f, sum0, sum1, sum2, suminv;
  float *matrix;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx + ( pointx << 1 );
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
      f = matrix[x];
      sum0 += f * (float)src[ mapx + 0 ];
      sum1 += f * (float)src[ mapx + 1 ];
      sum2 += f * (float)src[ mapx + 2 ];
      mapx += 3;
      if( mapx >= state->width3 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  sum0 -= 0.5f*255.0f;
  sum1 -= 0.5f*255.0f;
  sum2 -= 0.5f*255.0f;
  sum0 *= state->amplifynormal;
  sum1 *= state->amplifynormal;
  suminv = (0.5f*255.0f) / sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) );
  sum0 = (0.5f*255.0f) + ( sum0 * suminv );
  sum1 = (0.5f*255.0f) + ( sum1 * suminv );
  sum2 = (0.5f*255.0f) + ( sum2 * suminv );
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );

  return;
}

static void imStaticKernel4Normal( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
#if CPU_SSE2_SUPPORT
  __m128 vsum, vsrc;
  __m128i vzero;
#else
  float f;
#endif
  float sum0, sum1, sum2, sum3, suminv;
  float *matrix;
  unsigned char *src;

#if CPU_SSE2_SUPPORT
  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
#else
  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
#endif
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx << 2;
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
#if CPU_SSE2_SUPPORT
      vsrc = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx ] ) ), vzero ) );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_set1_ps( matrix[x] ), vsrc ) );
#else
      f = matrix[x];
      sum0 += f * (float)src[ mapx + 0 ];
      sum1 += f * (float)src[ mapx + 1 ];
      sum2 += f * (float)src[ mapx + 2 ];
      sum3 += f * (float)src[ mapx + 3 ];
#endif
      mapx += 4;
      if( mapx >= state->width4 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

#if CPU_SSE2_SUPPORT
  vsum = _mm_sub_ps( vsum, _mm_set_ps( 0.0f, 0.5f*255.0f, 0.5f*255.0f, 0.5f*255.0f ) );
  sum0 = _mm_cvtss_f32( vsum );
 #if CPU_SSE3_SUPPORT
  sum1 = _mm_cvtss_f32( _mm_movehdup_ps( vsum ) );
 #else
  sum1 = _mm_cvtss_f32( _mm_shuffle_ps( vsum, vsum, 0x55 ) );
 #endif
  sum2 = _mm_cvtss_f32( _mm_movehl_ps( vsum, vsum ) );
  sum3 = _mm_cvtss_f32( _mm_shuffle_ps( vsum, vsum, 0xff ) );
#else
  sum0 -= 0.5f*255.0f;
  sum1 -= 0.5f*255.0f;
  sum2 -= 0.5f*255.0f;
#endif
  sum0 *= state->amplifynormal;
  sum1 *= state->amplifynormal;
  suminv = (0.5f*255.0f) / sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) );
  sum0 = (0.5f*255.0f) + ( sum0 * suminv );
  sum1 = (0.5f*255.0f) + ( sum1 * suminv );
  sum2 = (0.5f*255.0f) + ( sum2 * suminv );
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );
  dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );

  return;
}


////


static void imStaticKernel3NormalSustain( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
  float f, v0, v1, v2, energy, sum0, sum1, sum2, sumenergy, suminv;
  float *matrix;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sumenergy = 0.0f;
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx + ( pointx << 1 );
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
      f = matrix[x];
      v0 = f * ( (float)src[ mapx + 0 ] - 127.5f );
      v1 = f * ( (float)src[ mapx + 1 ] - 127.5f );
      v2 = f * ( (float)src[ mapx + 2 ] - 127.5f );
      sum0 += v0;
      sum1 += v1;
      sum2 += v2;
      energy = ( v0 * v0 ) + ( v1 * v1 );
      if( energy )
        sumenergy += sqrtf( energy ) / sqrtf( energy + ( v2 * v2 ) );
      mapx += 3;
      if( mapx >= state->width3 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  sum0 *= state->amplifynormal;
  sum1 *= state->amplifynormal;
  suminv = (0.5f*255.0f) / fmaxf( 0.0625f, sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) ) );
  sum0 *= suminv;
  sum1 *= suminv;
  sum2 *= suminv;
  energy = sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) );
  sumenergy *= state->normalsustainfactor;
  if( energy < sumenergy )
  {
    f = fminf( sumenergy / energy, 8.0f );
    sum0 *= f;
    sum1 *= f;
    suminv = (0.5f*255.0f) / fmaxf( 0.0625f, sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) ) );
    sum0 *= suminv;
    sum1 *= suminv;
    sum2 *= suminv;
  }
  sum0 += (0.5f*255.0f);
  sum1 += (0.5f*255.0f);
  sum2 += (0.5f*255.0f);
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );

  return;
}

static void imStaticKernel4NormalSustain( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
  float f, v0, v1, v2, v3, energy, sum0, sum1, sum2, sum3, sumenergy, suminv;
  float *matrix;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
  sumenergy = 0.0f;
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx << 2;
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
      f = matrix[x];
      v0 = f * ( (float)src[ mapx + 0 ] - 127.5f );
      v1 = f * ( (float)src[ mapx + 1 ] - 127.5f );
      v2 = f * ( (float)src[ mapx + 2 ] - 127.5f );
      v3 = f * (float)src[ mapx + 3 ];
      sum0 += v0;
      sum1 += v1;
      sum2 += v2;
      sum3 += v3;
      energy = ( v0 * v0 ) + ( v1 * v1 );
      if( energy )
        sumenergy += sqrtf( energy ) / sqrtf( energy + ( v2 * v2 ) );
      mapx += 4;
      if( mapx >= state->width4 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  sum0 *= state->amplifynormal;
  sum1 *= state->amplifynormal;
  suminv = (0.5f*255.0f) / fmaxf( 0.0625f, sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) ) );
  sum0 *= suminv;
  sum1 *= suminv;
  sum2 *= suminv;
  energy = sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) );
  sumenergy *= state->normalsustainfactor;
  if( energy < sumenergy )
  {
    f = fminf( sumenergy / energy, 8.0f );
    sum0 *= f;
    sum1 *= f;
    suminv = (0.5f*255.0f) / fmaxf( 0.0625f, sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) ) );
    sum0 *= suminv;
    sum1 *= suminv;
    sum2 *= suminv;
  }
  sum0 += (0.5f*255.0f);
  sum1 += (0.5f*255.0f);
  sum2 += (0.5f*255.0f);
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );
  dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );

  return;
}


////


static void imStaticKernel4NormalSustainAlphaNorm( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy;
  float f, v0, v1, v2, v3, energy, sum0, sum1, sum2, sum3, sumenergy, suminv;
  float *matrix;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
  sumenergy = 0.0f;
  matrix = state->matrix;
  mapy = pointy;
  for( y = 0 ; y < state->matrixsize ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = pointx << 2;
    for( x = 0 ; x < state->matrixsize ; x++ )
    {
      f = matrix[x] * (float)src[ mapx + 3 ];
      v0 = f * ( (float)src[ mapx + 0 ] - 127.5f );
      v1 = f * ( (float)src[ mapx + 1 ] - 127.5f );
      v2 = f * ( (float)src[ mapx + 2 ] - 127.5f );
      v3 = f;
      sum0 += v0;
      sum1 += v1;
      sum2 += v2;
      sum3 += v3;
      energy = ( v0 * v0 ) + ( v1 * v1 );
      if( energy )
        sumenergy += sqrtf( energy ) / sqrtf( energy + ( v2 * v2 ) );
      mapx += 4;
      if( mapx >= state->width4 )
        mapx = 0;
    }
    matrix = ADDRESS( matrix, state->matrixrowsize );
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  if( sum3 >= state->minimumalphaf )
  {
    f = 1.0f / sum3;
    sum0 *= f;
    sum1 *= f;
    sum2 *= f;
    sum0 *= state->amplifynormal;
    sum1 *= state->amplifynormal;
    suminv = (0.5f*255.0f) / fmaxf( 0.0625f, sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) ) );
    sum0 *= suminv;
    sum1 *= suminv;
    sum2 *= suminv;
    energy = sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) );
    sumenergy *= state->normalsustainfactor;
    if( energy < sumenergy )
    {
      f = fminf( sumenergy / energy, 8.0f );
      sum0 *= f;
      sum1 *= f;
      suminv = (0.5f*255.0f) / fmaxf( 0.0625f, sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) ) );
      sum0 *= suminv;
      sum1 *= suminv;
      sum2 *= suminv;
    }
    sum0 += (0.5f*255.0f);
    sum1 += (0.5f*255.0f);
    sum2 += (0.5f*255.0f);
    dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
    dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
    dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );
    dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );
  }
  else
  {
    dst[0] = 0;
    dst[1] = 0;
    dst[2] = 0;
    dst[3] = 0;
  }

  return;
}


////


static void imStaticKernelPoT3Water( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy, heightmask, widthmask;
  int minx, maxx, miny, maxy;
  float f, sum0, sum1, sum2, suminv;
  float *matrix;
  unsigned char *src;

  minx = pointx;
  maxx = minx + state->matrixsize;
  miny = pointy;
  maxy = miny + state->matrixsize;
  heightmask = state->height - 1;
  widthmask = state->width1 - 1;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  matrix = state->matrix;
  for( y = miny ; y < maxy ; y++ )
  {
    mapy = y & heightmask;
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    for( x = minx ; x < maxx ; x++, matrix++ )
    {
      mapx = x & widthmask;
      mapx += mapx << 1;
      f = *matrix;
      sum0 += (float)src[ mapx + 0 ] * f;
      sum1 += (float)src[ mapx + 1 ] * f;
      sum2 += (float)src[ mapx + 2 ] * f;
    }
    matrix += state->rowreturn;
  }

  sum0 *= 1.0f/255.0f;
  sum1 *= 1.0f/255.0f;
  sum2 *= 1.0f/255.0f;
  sum0 = 2.0f * ( sum0 - 0.5f );
  sum1 = 2.0f * ( sum1 - 0.5f );
  suminv = sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) );
  if( suminv < 0.75f )
  {
    suminv = 0.5f / suminv;
    sum0 = 0.5f + ( sum0 * suminv );
    sum1 = 0.5f + ( sum1 * suminv );
  }
  if( sum2 > 0.1f )
  {
    state->dithersum += sum2;
    if( sum2 > 0.45f )
      sum2 = 1.0f;
    else if( ( sum2 < 0.3f ) && ( state->dithersum < 1.0f ) )
      sum2 = 0.0f;
    else
      sum2 = ( ( sum2 + state->dithersum ) < 0.45f ? 0.0f : 1.0f );
    state->dithersum -= sum2;
  }
  sum0 *= 255.0f;
  sum1 *= 255.0f;
  sum2 *= 255.0f;
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );

  return;
}

static void imStaticKernelPoT4Water( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy, heightmask, widthmask;
  int minx, maxx, miny, maxy;
  float f, sum0, sum1, sum2, sum3, suminv;
  float *matrix;
  unsigned char *src;

  minx = pointx;
  maxx = minx + state->matrixsize;
  miny = pointy;
  maxy = miny + state->matrixsize;
  heightmask = state->height - 1;
  widthmask = state->width1 - 1;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
  matrix = state->matrix;
  for( y = miny ; y < maxy ; y++ )
  {
    mapy = y & heightmask;
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    for( x = minx ; x < maxx ; x++, matrix++ )
    {
      mapx = x & widthmask;
      mapx <<= 2;
      f = *matrix;
      sum0 += (float)src[ mapx + 0 ] * f;
      sum1 += (float)src[ mapx + 1 ] * f;
      sum2 += (float)src[ mapx + 2 ] * f;
      sum3 += (float)src[ mapx + 3 ] * f;
    }
    matrix += state->rowreturn;
  }

  sum0 *= 1.0f/255.0f;
  sum1 *= 1.0f/255.0f;
  sum2 *= 1.0f/255.0f;
  sum0 = 2.0f * ( sum0 - 0.5f );
  sum1 = 2.0f * ( sum1 - 0.5f );
  suminv = sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) );
  if( suminv < 0.75f )
  {
    suminv = 0.5f / suminv;
    sum0 = 0.5f + ( sum0 * suminv );
    sum1 = 0.5f + ( sum1 * suminv );
  }
  if( sum2 > 0.1f )
  {
    state->dithersum += sum2;
    if( sum2 > 0.45f )
      sum2 = 1.0f;
    else if( ( sum2 < 0.3f ) && ( state->dithersum < 1.0f ) )
      sum2 = 0.0f;
    else
      sum2 = ( ( sum2 + state->dithersum ) < 0.45f ? 0.0f : 1.0f );
    state->dithersum -= sum2;
  }
  sum0 *= 255.0f;
  sum1 *= 255.0f;
  sum2 *= 255.0f;
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );
  dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );

  return;
}


////


static void imStaticKernelPoT4Plant( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state )
{
  int x, y, mapx, mapy, heightmask, widthmask;
  int minx, maxx, miny, maxy;
  float f, sum0, sum1, sum2, sum3;
  float *matrix;
  unsigned char *src;

  minx = pointx;
  maxx = minx + state->matrixsize;
  miny = pointy;
  maxy = miny + state->matrixsize;
  heightmask = state->height - 1;
  widthmask = state->width1 - 1;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
  matrix = state->matrix;
  for( y = miny ; y < maxy ; y++ )
  {
    mapy = y & heightmask;
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    for( x = minx ; x < maxx ; x++, matrix++ )
    {
      mapx = x & widthmask;
      mapx <<= 2;
      f = *matrix;
      sum0 += (float)src[ mapx + 0 ] * f;
      sum1 += (float)src[ mapx + 1 ] * f;
      sum2 += (float)src[ mapx + 2 ] * f;
      sum3 += (float)src[ mapx + 3 ] * f;
    }
    matrix += state->rowreturn;
  }

  sum3 *= 1.25f;
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );
  dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );

  return;
}


////


int imReduceImageKaiserDataDivisor( unsigned char *dstdata, unsigned char *srcdata, int width, int height, int bytesperpixel, int bytesperline, int sizedivisor, imReduceOptions *options )
{
  int filter, x, y, pointx, pointy, basex, basey, pow2flag;
  int newwidth, newheight;
  unsigned char *dst;
  imStaticMatrixState state;
  void (*applykernel)( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state );
#if CPU_SSE2_SUPPORT
  int corebase, corerange;
  void (*applykernelcore)( unsigned char *dst, int pointx, int pointy, imStaticMatrixState * CC_RESTRICT state );
#endif

  filter = options->filter;
  imBuildStaticMatrix( &state, sizedivisor, options->hopcount, options->alpha );

  newwidth = ( width < sizedivisor ) ? 1 : ( ( width + sizedivisor - 1 ) / sizedivisor );
  newheight = ( height < sizedivisor ) ? 1 : ( ( height + sizedivisor - 1 ) / sizedivisor );

  pow2flag = ccIsPow2Int32( width ) && ccIsPow2Int32( height );
  applykernel = 0;
#if CPU_SSE2_SUPPORT
  applykernelcore = 0;
#endif

  if( filter == IM_REDUCE_FILTER_LINEAR )
  {
    if( bytesperpixel == 4 )
    {
      applykernel = imStaticKernel4Linear;
#if CPU_SSE2_SUPPORT
      applykernelcore = imStaticKernel4Linear_Core;
#endif
    }
    else if( bytesperpixel == 3 )
      applykernel = imStaticKernel3Linear;
    else if( bytesperpixel == 2 )
      applykernel = imStaticKernel2Linear;
    else if( bytesperpixel == 1 )
      applykernel = imStaticKernel1Linear;
  }
  else if( filter == IM_REDUCE_FILTER_LINEAR_ALPHANORM )
  {
    if( bytesperpixel == 4 )
    {
      applykernel = imStaticKernel4LinearAlphaNorm;
#if CPU_SSE2_SUPPORT
      applykernelcore = imStaticKernel4LinearAlphaNorm_Core;
#endif
    }
    else if( bytesperpixel == 3 )
      applykernel = imStaticKernel3Linear;
    else if( bytesperpixel == 2 )
      applykernel = imStaticKernel2Linear;
    else if( bytesperpixel == 1 )
      applykernel = imStaticKernel1Linear;
  }
  else if( filter == IM_REDUCE_FILTER_SRGB )
  {
    if( bytesperpixel == 4 )
    {
      applykernel = imStaticKernel4sRGB;
#if CPU_SSE2_SUPPORT
      applykernelcore = imStaticKernel4sRGB_Core;
#endif
    }
    else if( bytesperpixel == 3 )
    {
      applykernel = imStaticKernel3sRGB;
#if CPU_SSE2_SUPPORT
      applykernelcore = imStaticKernel3sRGB_Core;
#endif
    }
    else if( bytesperpixel == 2 )
      applykernel = imStaticKernel2sRGB;
    else if( bytesperpixel == 1 )
      applykernel = imStaticKernel1sRGB;
  }
  else if( filter == IM_REDUCE_FILTER_SRGB_ALPHANORM )
  {
    if( bytesperpixel == 4 )
    {
      applykernel = imStaticKernel4sRGBAlphaNorm;
#if CPU_SSE2_SUPPORT
      applykernelcore = imStaticKernel4sRGBAlphaNorm_Core;
#endif
    }
    else if( bytesperpixel == 3 )
      applykernel = imStaticKernel3sRGB;
    else if( bytesperpixel == 2 )
      applykernel = imStaticKernel2sRGB;
    else if( bytesperpixel == 1 )
      applykernel = imStaticKernel1sRGB;
  }
  else if( filter == IM_REDUCE_FILTER_NORMALMAP )
  {
    if( bytesperpixel == 4 )
      applykernel = imStaticKernel4Normal;
    else if( bytesperpixel == 3 )
      applykernel = imStaticKernel3Normal;
  }
  else if( filter == IM_REDUCE_FILTER_NORMALMAP_SUSTAIN )
  {
    if( bytesperpixel == 4 )
      applykernel = imStaticKernel4NormalSustain;
    else if( bytesperpixel == 3 )
      applykernel = imStaticKernel3NormalSustain;
  }
  else if( filter == IM_REDUCE_FILTER_NORMALMAP_SUSTAIN_ALPHANORM )
  {
    if( bytesperpixel == 4 )
      applykernel = imStaticKernel4NormalSustainAlphaNorm;
    else if( bytesperpixel == 3 )
      applykernel = imStaticKernel3NormalSustain;
  }
  else if( filter == IM_REDUCE_FILTER_WATERMAP )
  {
    if( ( bytesperpixel == 4 ) && ( pow2flag ) )
      applykernel = imStaticKernelPoT4Water;
    else if( ( bytesperpixel == 3 ) && ( pow2flag ) )
      applykernel = imStaticKernelPoT3Water;
  }
  else if( filter == IM_REDUCE_FILTER_PLANTMAP )
  {
    if( ( bytesperpixel == 4 ) && ( pow2flag ) )
      applykernel = imStaticKernelPoT4Plant;
  }

  if( !applykernel )
    return 0;

#if CPU_SSE2_SUPPORT
  corebase = -state.matrixoffset;
  corerange = ( newwidth + state.matrixoffset ) - corebase;
#endif

  state.dithersum = 0.0f;
  if( ( newwidth | newheight ) > 2 )
    state.dithersum = 0.5f;

  state.srcdata = srcdata;
  state.width1 = width * 1;
  state.width2 = width * 2;
  state.width3 = width * 3;
  state.width4 = width * 4;
  state.height = height;
  state.bytesperline = bytesperline;

  state.minimumalpha = 4;
  state.minimumalphaf = (float)state.minimumalpha;
  state.amplifynormal = fmaxf( 1.0f, options->amplifynormal );
  state.normalsustainfactor = options->normalsustainfactor;

  basex = ( state.matrixoffset + ( width << 8 ) ) % width;
  basey = ( state.matrixoffset + ( height << 8 ) ) % height;
  while( basex < 0 )
    basex += width;
  while( basey < 0 )
    basey += height;

#if CPU_SSE2_SUPPORT
  if( applykernelcore )
  {
    dst = dstdata;
    pointy = basey;
    for( y = 0 ; y < newheight ; y++ )
    {
      pointx = basex;
      for( x = 0 ; x < newwidth ; x++, dst += bytesperpixel )
      {
        ( (unsigned int)( x - corebase ) < corerange ? applykernelcore : applykernel )( dst, pointx, pointy, &state );
        pointx += sizedivisor;
        while( pointx >= width )
          pointx -= width;
      }
      pointy += sizedivisor;
      while( pointy >= height )
        pointy -= height;
    }
  }
  else
#endif
  {
    dst = dstdata;
    pointy = basey;
    for( y = 0 ; y < newheight ; y++ )
    {
      pointx = basex;
      for( x = 0 ; x < newwidth ; x++, dst += bytesperpixel )
      {
        applykernel( dst, pointx, pointy, &state );
        pointx += sizedivisor;
        while( pointx >= width )
          pointx -= width;
      }
      pointy += sizedivisor;
      while( pointy >= height )
        pointy -= height;
    }
  }

  imFreeStaticState( &state );

  return 1;
}


int imReduceImageKaiserDivisor( imgImage *imgdst, imgImage *imgsrc, int sizedivisor, imReduceOptions *options )
{
  int width, height;
  int newwidth, newheight, retvalue;

  width = imgsrc->format.width;
  height = imgsrc->format.height;
  newwidth = ( width < sizedivisor ) ? 1 : ( ( width + sizedivisor - 1 ) / sizedivisor );
  newheight = ( height < sizedivisor ) ? 1 : ( ( height + sizedivisor - 1 ) / sizedivisor );

  imgdst->format.width = newwidth;
  imgdst->format.height = newheight;
  imgdst->format.type = imgsrc->format.type;
  imgdst->format.bytesperpixel = imgsrc->format.bytesperpixel;
  imgdst->format.bytesperline = imgdst->format.width * imgdst->format.bytesperpixel;
  imgdst->data = malloc( imgdst->format.height * imgdst->format.bytesperline );
  if( !( imgdst->data ) )
    return 0;

  retvalue = imReduceImageKaiserDataDivisor( imgdst->data, imgsrc->data, width, height, imgsrc->format.bytesperpixel, imgsrc->format.bytesperline, sizedivisor, options );

  return retvalue;
}



////////////////////////////////////////////////////////////////////////////////



static void imDynamicKernel1Linear( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
  float f, sum0;
  float matrixsum;
  unsigned char *src;

  sum0 = 0.0f;
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx;
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
      f = state->linearx[x] * state->lineary[y];
      sum0 += f * (float)src[ mapx + 0 ];
      matrixsum += f;
      mapx++;
      if( mapx >= state->width1 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  sum0 /= matrixsum;
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );

  return;
}


static void imDynamicKernel2Linear( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
  float f, sum0, sum1;
  float matrixsum;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx << 1;
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
      f = state->linearx[x] * state->lineary[y];
      sum0 += f * (float)src[ mapx + 0 ];
      sum1 += f * (float)src[ mapx + 1 ];
      matrixsum += f;
      mapx += 2;
      if( mapx >= state->width2 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  matrixsum = 1.0f / matrixsum;
  sum0 *= matrixsum;
  sum1 *= matrixsum;
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );

  return;
}


static void imDynamicKernel3Linear( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
  float f, sum0, sum1, sum2;
  float matrixsum;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx + ( state->matrixoffsetx << 1 );
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
      f = state->linearx[x] * state->lineary[y];
      sum0 += f * (float)src[ mapx + 0 ];
      sum1 += f * (float)src[ mapx + 1 ];
      sum2 += f * (float)src[ mapx + 2 ];
      matrixsum += f;
      mapx += 3;
      if( mapx >= state->width3 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  matrixsum = 1.0f / matrixsum;
  sum0 *= matrixsum;
  sum1 *= matrixsum;
  sum2 *= matrixsum;
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );

  return;
}


static void imDynamicKernel4Linear( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
  float f, sum0, sum1, sum2, sum3;
  float matrixsum;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx << 2;
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
      f = state->linearx[x] * state->lineary[y];
      sum0 += f * (float)src[ mapx + 0 ];
      sum1 += f * (float)src[ mapx + 1 ];
      sum2 += f * (float)src[ mapx + 2 ];
      sum3 += f * (float)src[ mapx + 3 ];
      matrixsum += f;
      mapx += 4;
      if( mapx >= state->width4 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  matrixsum = 1.0f / matrixsum;
  sum0 *= matrixsum;
  sum1 *= matrixsum;
  sum2 *= matrixsum;
  sum3 *= matrixsum;
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );
  dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );

  return;
}


////


static void imDynamicKernel4LinearAlphaNorm( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
  float f, sum0, sum1, sum2, sum3, alpha;
  float matrixsum;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx << 2;
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
      f = state->linearx[x] * state->lineary[y];
      alpha = (float)src[ mapx + 3 ] * f;
      sum0 += alpha * (float)src[ mapx + 0 ];
      sum1 += alpha * (float)src[ mapx + 1 ];
      sum2 += alpha * (float)src[ mapx + 2 ];
      sum3 += alpha;
      matrixsum += f;
      mapx += 4;
      if( mapx >= state->width4 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  matrixsum = 1.0f / matrixsum;
  sum0 *= matrixsum;
  sum1 *= matrixsum;
  sum2 *= matrixsum;
  sum3 *= matrixsum;
  if( sum3 >= state->minimumalphaf )
  {
    f = 1.0f / sum3;
    dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, ( sum0 * f ) + 0.5f ) ) );
    dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, ( sum1 * f ) + 0.5f ) ) );
    dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, ( sum2 * f ) + 0.5f ) ) );
    dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );
  }
  else
  {
    dst[0] = 0;
    dst[1] = 0;
    dst[2] = 0;
    dst[3] = 0;
  }

  return;
}


#if CPU_SSE2_SUPPORT

static void imDynamicKernel4LinearAlphaNorm_Core( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
  uint32_t pixel;
  unsigned char *src;
  __m128 vmatrixsum, vsum0, vsum1, vsum2, vsum3;
  __m128 vlx, vly, vf, valpha, vr, vg, vb, va, vsrcf;
  __m128i vsrc, vshufmask;
  __m128i vzero;

 #if CPU_SSSE3_SUPPORT
  vshufmask = _mm_setr_epi8( 0x00,0x04,0x08,0x0c, 0x01,0x05,0x09,0x0d, 0x02,0x06,0x0a,0x0e, 0x03,0x07,0x0b,0x0f );
 #endif
  vsum0 = _mm_setzero_ps();
  vsum1 = _mm_setzero_ps();
  vsum2 = _mm_setzero_ps();
  vsum3 = _mm_setzero_ps();
  vmatrixsum = _mm_setzero_ps();
  vzero = _mm_castps_si128( _mm_setzero_ps() );

  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx << 2;
    vly = _mm_set1_ps( state->lineary[y] );
    for( x = 0 ; x < state->matrixsizex ; x += 4 )
    {
      vlx = _mm_load_ps( &state->linearx[x] );
      /* Load 16 bytes and unpack as RRRR,GGGG,BBBB,AAAA in one SSE register */
      vsrc = _mm_loadu_si128( (void *)&src[ mapx ] );
 #if CPU_SSSE3_SUPPORT
      vsrc = _mm_shuffle_epi8( vsrc, vshufmask );
 #else
      vshufmask = _mm_shuffle_epi32( vsrc, 0x39 );
      vsrc = _mm_unpacklo_epi16( _mm_unpacklo_epi8( vsrc, vshufmask ), _mm_unpackhi_epi8( vsrc, vshufmask ) );
 #endif
      /* Break that into 4 SSE registers as floats: vR,vG,vB,vA */
      vsrcf = _mm_castsi128_ps( vsrc );
      vr = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( vsrcf ), vzero ) );
 #if CPU_SSE3_SUPPORT
      vg = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_movehdup_ps( vsrcf ) ), vzero ) );
 #else
      vg = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_shuffle_ps( vsrcf, vsrcf, 0x55 ) ), vzero ) );
 #endif
      vb = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_movehl_ps( vsrcf, vsrcf ) ), vzero ) );
      va = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_shuffle_ps( vsrcf, vsrcf, 0xff ) ), vzero ) );
      vf = _mm_mul_ps( vlx, vly );
      valpha = _mm_mul_ps( va, vf );
      vsum0 = _mm_add_ps( vsum0, _mm_mul_ps( vr, valpha ) );
      vsum1 = _mm_add_ps( vsum1, _mm_mul_ps( vg, valpha ) );
      vsum2 = _mm_add_ps( vsum2, _mm_mul_ps( vb, valpha ) );
      vsum3 = _mm_add_ps( vsum3, valpha );
      vmatrixsum = _mm_add_ps( vmatrixsum, vf );      
      mapx += 16;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

 #if CPU_SSE3_SUPPORT
  vmatrixsum = _mm_hadd_ps( vmatrixsum, vmatrixsum );
  vmatrixsum = _mm_hadd_ps( vmatrixsum, vmatrixsum );
 #else
  vmatrixsum = _mm_add_ps( vmatrixsum, _mm_shuffle_ps( vmatrixsum, vmatrixsum, 0x4e ) );
  vmatrixsum = _mm_add_ps( vmatrixsum, _mm_shuffle_ps( vmatrixsum, vmatrixsum, 0x39 ) );
 #endif

 #if CPU_SSE3_SUPPORT
  vsum0 = _mm_hadd_ps( vsum0, vsum1 );
  vsum2 = _mm_hadd_ps( vsum2, vsum3 );
  vsum0 = _mm_hadd_ps( vsum0, vsum2 );
 #else
  vsum0 = _mm_add_ps( _mm_unpacklo_ps( vsum0, vsum2 ), _mm_unpackhi_ps( vsum0, vsum2 ) );
  vsum1 = _mm_add_ps( _mm_unpacklo_ps( vsum1, vsum3 ), _mm_unpackhi_ps( vsum1, vsum3 ) );
  vsum0 = _mm_add_ps( _mm_unpacklo_ps( vsum0, vsum1 ), _mm_unpackhi_ps( vsum0, vsum1 ) );
 #endif
  vsum0 = _mm_div_ps( vsum0, vmatrixsum );

  valpha = _mm_shuffle_ps( vsum0, vsum0, 0xff );
  pixel = 0;
  if( _mm_comige_ss( valpha, _mm_load_ss( &state->minimumalphaf ) ) )
  {
    __m128i vpixel;
    vsum0 = _mm_mul_ps( vsum0, _mm_rcp_ps( valpha ) );
    vsum0 = CPU_BLENDV_PS( vsum0, valpha, *(__m128 *)simd4fAlphaMask );
    vpixel = _mm_cvtps_epi32( vsum0 );
    vpixel = _mm_packs_epi32( vpixel, vpixel );
    vpixel = _mm_packus_epi16( vpixel, vpixel );
    pixel = (uint32_t)_mm_cvtsi128_si32( vpixel );
  }
  *(uint32_t *)dst = pixel;

  return;
}

#endif


////


static void imDynamicKernel1sRGB( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
#if CPU_SSE2_SUPPORT
  __m128 vsum, vsrc;
  __m128i vzero;
#else
  float sum0;
#endif
  float f, matrixsum;
  unsigned char *src;

#if CPU_SSE2_SUPPORT
  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
#else
  sum0 = 0.0f;
#endif
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx;
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
#if CPU_SSE2_SUPPORT
      f = state->linearx[x] * state->lineary[y];
      vsrc = _mm_set_ss( (float)src[ mapx + 0 ] );
      vsrc = srgb2linear3( vsrc );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_set1_ps( f ), vsrc ) );
#else
      f = state->linearx[x] * state->lineary[y];
      sum0 += f * srgb2linear( (float)src[ mapx + 0 ] );
#endif
      matrixsum += f;
      mapx++;
      if( mapx >= state->width1 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

#if CPU_SSE2_SUPPORT
  vsum = linear2srgb3( _mm_div_ps( vsum, _mm_set1_ps( matrixsum ) ) );
  dst[0] = _mm_cvtsi128_si32( _mm_packus_epi16( _mm_packs_epi32( _mm_cvtps_epi32( vsum ), vzero ), vzero ) );
#else
  sum0 /= matrixsum;
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum0 ) + 0.5f ) ) );
#endif

  return;
}


static void imDynamicKernel2sRGB( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
#if CPU_SSE2_SUPPORT
  __m128 vsum, vsrc;
  __m128i vzero;
#else
  float sum0, sum1;
#endif
  float f, matrixsum;
  unsigned char *src;

#if CPU_SSE2_SUPPORT
  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
#else
  sum0 = 0.0f;
  sum1 = 0.0f;
#endif
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx << 1;
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
#if CPU_SSE2_SUPPORT
      f = state->linearx[x] * state->lineary[y];
      vsrc = _mm_set_ps( 0.0f, 0.0f, (float)src[ mapx + 1 ], (float)src[ mapx + 0 ] );
      vsrc = srgb2linear3( vsrc );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_set1_ps( f ), vsrc ) );
#else
      f = state->linearx[x] * state->lineary[y];
      sum0 += f * srgb2linear( (float)src[ mapx + 0 ] );
      sum1 += f * srgb2linear( (float)src[ mapx + 1 ] );
#endif
      matrixsum += f;
      mapx += 2;
      if( mapx >= state->width2 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

#if CPU_SSE2_SUPPORT
  union
  {
    char c[4];
    uint32_t i;
  } u;
  vsum = linear2srgb3( _mm_div_ps( vsum, _mm_set1_ps( matrixsum ) ) );
  _mm_store_ss( (void *)&u.i, _mm_castsi128_ps( _mm_packus_epi16( _mm_packs_epi32( _mm_cvtps_epi32( vsum ), vzero ), vzero ) ) );
  dst[0] = u.c[0];
  dst[1] = u.c[1];
#else
  matrixsum = 1.0f / matrixsum;
  sum0 *= matrixsum;
  sum1 *= matrixsum;
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum0 ) + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum1 ) + 0.5f ) ) );
#endif

  return;
}


static void imDynamicKernel3sRGB( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
#if CPU_SSE2_SUPPORT
  __m128 vsum, vsrc;
  __m128i vzero;
#else
  float sum0, sum1, sum2;
#endif
  float f, matrixsum;
  unsigned char *src;

#if CPU_SSE2_SUPPORT
  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
#else
  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
#endif
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx + ( state->matrixoffsetx << 1 );
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
#if CPU_SSE2_SUPPORT
      f = state->linearx[x] * state->lineary[y];
      vsrc = _mm_set_ps( 0.0f, (float)src[ mapx + 2 ], (float)src[ mapx + 1 ], (float)src[ mapx + 0 ] );
      vsrc = srgb2linear3( vsrc );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_set1_ps( f ), vsrc ) );
#else
      f = state->linearx[x] * state->lineary[y];
      sum0 += f * srgb2linear( (float)src[ mapx + 0 ] );
      sum1 += f * srgb2linear( (float)src[ mapx + 1 ] );
      sum2 += f * srgb2linear( (float)src[ mapx + 2 ] );
#endif
      matrixsum += f;
      mapx += 3;
      if( mapx >= state->width3 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

#if CPU_SSE2_SUPPORT
  union
  {
    char c[4];
    uint32_t i;
  } u;
  vsum = linear2srgb3( _mm_div_ps( vsum, _mm_set1_ps( matrixsum ) ) );
  _mm_store_ss( (void *)&u.i, _mm_castsi128_ps( _mm_packus_epi16( _mm_packs_epi32( _mm_cvtps_epi32( vsum ), vzero ), vzero ) ) );
  dst[0] = u.c[0];
  dst[1] = u.c[1];
  dst[2] = u.c[2];
#else
  matrixsum = 1.0f / matrixsum;
  sum0 *= matrixsum;
  sum1 *= matrixsum;
  sum2 *= matrixsum;
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum0 ) + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum1 ) + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum2 ) + 0.5f ) ) );
#endif

  return;
}


static void imDynamicKernel4sRGB( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
#if CPU_SSE2_SUPPORT
  __m128 vsum, vsrc;
  __m128i vzero;
#else
  float sum0, sum1, sum2, sum3;
#endif
  float f, matrixsum;
  unsigned char *src;

#if CPU_SSE2_SUPPORT
  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
#else
  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
#endif
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx << 2;
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
#if CPU_SSE2_SUPPORT
      f = state->linearx[x] * state->lineary[y];
      vsrc = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx ] ) ), vzero ) );
      vsrc = srgb2linear3( vsrc );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_set1_ps( f ), vsrc ) );
#else
      f = state->linearx[x] * state->lineary[y];
      sum0 += f * srgb2linear( (float)src[ mapx + 0 ] );
      sum1 += f * srgb2linear( (float)src[ mapx + 1 ] );
      sum2 += f * srgb2linear( (float)src[ mapx + 2 ] );
      sum3 += f * (float)src[ mapx + 3 ];
#endif
      matrixsum += f;
      mapx += 4;
      if( mapx >= state->width4 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

#if CPU_SSE2_SUPPORT
  vsum = linear2srgb3( _mm_div_ps( vsum, _mm_set1_ps( matrixsum ) ) );
  _mm_store_ss( (void *)dst, _mm_castsi128_ps( _mm_packus_epi16( _mm_packs_epi32( _mm_cvtps_epi32( vsum ), vzero ), vzero ) ) );
#else
  matrixsum = 1.0f / matrixsum;
  sum0 *= matrixsum;
  sum1 *= matrixsum;
  sum2 *= matrixsum;
  sum3 *= matrixsum;
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum0 ) + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum1 ) + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum2 ) + 0.5f ) ) );
  dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );
#endif

  return;
}


////


static void imDynamicKernel4sRGBAlphaNorm( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
#if CPU_SSE2_SUPPORT
  __m128 vsum, vsrc, valpha;
  __m128i vzero;
  uint32_t pixel;
#else
  float sum0, sum1, sum2, sum3, alpha;
#endif
  float f, matrixsum;
  unsigned char *src;

#if CPU_SSE2_SUPPORT
  vsum = _mm_setzero_ps();
  vzero = _mm_setzero_si128();
#else
  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
#endif
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx << 2;
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
#if CPU_SSE2_SUPPORT
      f = state->linearx[x] * state->lineary[y];
      vsrc = _mm_cvtepi32_ps( CPU_CVT_U8_TO_I32( _mm_castps_si128( _mm_load_ss( (void *)&src[ mapx ] ) ), vzero ) );
      valpha = _mm_shuffle_ps( vsrc, _mm_set_ss( 1.0f ), 0x0f );
      vsrc = srgb2linear3( vsrc );
      vsum = _mm_add_ps( vsum, _mm_mul_ps( _mm_mul_ps( _mm_shuffle_ps( valpha, valpha, 0xC0 ), _mm_set1_ps( f ) ), vsrc ) );
#else
      f = state->linearx[x] * state->lineary[y];
      alpha = (float)src[ mapx + 3 ] * f;
      sum0 += alpha * srgb2linear( (float)src[ mapx + 0 ] );
      sum1 += alpha * srgb2linear( (float)src[ mapx + 1 ] );
      sum2 += alpha * srgb2linear( (float)src[ mapx + 2 ] );
      sum3 += alpha;
#endif
      matrixsum += f;
      mapx += 4;
      if( mapx >= state->width4 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

#if CPU_SSE2_SUPPORT
  vsum = _mm_div_ps( vsum, _mm_set1_ps( matrixsum ) );
  valpha = _mm_shuffle_ps( vsum, vsum, 0xff );
  pixel = 0;
  if( _mm_comige_ss( valpha, _mm_load_ss( &state->minimumalphaf ) ) )
  {
    __m128i vpixel;
    vsum = _mm_mul_ps( vsum, _mm_rcp_ps( valpha ) );
    vsum = CPU_BLENDV_PS( vsum, valpha, *(__m128 *)simd4fAlphaMask );
    vsum = linear2srgb3( vsum );
    vpixel = _mm_cvtps_epi32( vsum );
    vpixel = _mm_packs_epi32( vpixel, vpixel );
    vpixel = _mm_packus_epi16( vpixel, vpixel );
    pixel = (uint32_t)_mm_cvtsi128_si32( vpixel );
  }
  *(uint32_t *)dst = pixel;
#else
  matrixsum = 1.0f / matrixsum;
  sum0 *= matrixsum;
  sum1 *= matrixsum;
  sum2 *= matrixsum;
  sum3 *= matrixsum;
  if( sum3 >= state->minimumalphaf )
  {
    f = 1.0f / sum3;
    dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum0 * f ) + 0.5f ) ) );
    dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum1 * f ) + 0.5f ) ) );
    dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum2 * f ) + 0.5f ) ) );
    dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );
  }
  else
  {
    dst[0] = 0;
    dst[1] = 0;
    dst[2] = 0;
    dst[3] = 0;
  }
#endif

  return;
}


////


static void imDynamicKernel3Normal( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
  float f, sum0, sum1, sum2;
  float matrixsum, suminv;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx + ( state->matrixoffsetx << 1 );
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
      f = state->linearx[x] * state->lineary[y];
      sum0 += f * (float)src[ mapx + 0 ];
      sum1 += f * (float)src[ mapx + 1 ];
      sum2 += f * (float)src[ mapx + 2 ];
      matrixsum += f;
      mapx += 3;
      if( mapx >= state->width3 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  matrixsum = (1.0f/255.0f) / matrixsum;
  sum0 *= matrixsum;
  sum1 *= matrixsum;
  sum2 *= matrixsum;
  sum0 -= 0.5f;
  sum1 -= 0.5f;
  sum2 -= 0.5f;
  sum0 *= state->amplifynormal;
  sum1 *= state->amplifynormal;
  suminv = (0.5f*255.0f) / sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) );
  sum0 = (0.5f*255.0f) + ( sum0 * suminv );
  sum1 = (0.5f*255.0f) + ( sum1 * suminv );
  sum2 = (0.5f*255.0f) + ( sum2 * suminv );
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );

  return;
}


static void imDynamicKernel4Normal( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
  float f, sum0, sum1, sum2, sum3;
  float matrixsum, suminv;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx << 2;
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
      f = state->linearx[x] * state->lineary[y];
      sum0 += f * (float)src[ mapx + 0 ];
      sum1 += f * (float)src[ mapx + 1 ];
      sum2 += f * (float)src[ mapx + 2 ];
      sum3 += f * (float)src[ mapx + 3 ];
      matrixsum += f;
      mapx += 4;
      if( mapx >= state->width4 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  matrixsum = (1.0f/255.0f) / matrixsum;
  sum0 *= matrixsum;
  sum1 *= matrixsum;
  sum2 *= matrixsum;
  sum3 *= matrixsum;
  sum0 -= 0.5f;
  sum1 -= 0.5f;
  sum2 -= 0.5f;
  sum0 *= state->amplifynormal;
  sum1 *= state->amplifynormal;
  suminv = (0.5f*255.0f) / sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) );
  sum0 = (0.5f*255.0f) + ( sum0 * suminv );
  sum1 = (0.5f*255.0f) + ( sum1 * suminv );
  sum2 = (0.5f*255.0f) + ( sum2 * suminv );
  sum3 *= 255.0f;
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );
  dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );

  return;
}


////


static void imDynamicKernel3NormalSustain( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
  float f, v0, v1, v2, energy, sum0, sum1, sum2, sumenergy;
  float matrixsum, suminv;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sumenergy = 0.0f;
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx + ( state->matrixoffsetx << 1 );
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
      f = state->linearx[x] * state->lineary[y];
      v0 = f * ( (float)src[ mapx + 0 ] - 127.5f );
      v1 = f * ( (float)src[ mapx + 1 ] - 127.5f );
      v2 = f * ( (float)src[ mapx + 2 ] - 127.5f );
      sum0 += v0;
      sum1 += v1;
      sum2 += v2;
      energy = ( v0 * v0 ) + ( v1 * v1 );
      if( energy )
        sumenergy += sqrtf( energy ) / sqrtf( energy + ( v2 * v2 ) );
      matrixsum += f;
      mapx += 3;
      if( mapx >= state->width3 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  matrixsum = (1.0f/255.0f) / matrixsum;
  sum0 *= matrixsum;
  sum1 *= matrixsum;
  sum2 *= matrixsum;
  sum0 *= state->amplifynormal;
  sum1 *= state->amplifynormal;
  suminv = (0.5f*255.0f) / fmaxf( 0.0625f, sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) ) );
  sum0 *= suminv;
  sum1 *= suminv;
  sum2 *= suminv;
  energy = sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) );
  sumenergy *= state->normalsustainfactor;
  if( energy < sumenergy )
  {
    f = fminf( sumenergy / energy, 8.0f );
    sum0 *= f;
    sum1 *= f;
    suminv = (0.5f*255.0f) / fmaxf( 0.0625f, sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) ) );
    sum0 *= suminv;
    sum1 *= suminv;
    sum2 *= suminv;
  }
  sum0 += (0.5f*255.0f);
  sum1 += (0.5f*255.0f);
  sum2 += (0.5f*255.0f);
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );

  return;
}

static void imDynamicKernel4NormalSustain( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
  float f, v0, v1, v2, v3, energy, sum0, sum1, sum2, sum3, sumenergy;
  float matrixsum, suminv;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
  sumenergy = 0.0f;
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx << 2;
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
      f = state->linearx[x] * state->lineary[y];
      v0 = f * ( (float)src[ mapx + 0 ] - 127.5f );
      v1 = f * ( (float)src[ mapx + 1 ] - 127.5f );
      v2 = f * ( (float)src[ mapx + 2 ] - 127.5f );
      v3 = f * (float)src[ mapx + 3 ];
      sum0 += v0;
      sum1 += v1;
      sum2 += v2;
      sum3 += v3;
      energy = ( v0 * v0 ) + ( v1 * v1 );
      if( energy )
        sumenergy += sqrtf( energy ) / sqrtf( energy + ( v2 * v2 ) );
      matrixsum += f;
      mapx += 4;
      if( mapx >= state->width4 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  matrixsum = (1.0f/255.0f) / matrixsum;
  sum0 *= matrixsum;
  sum1 *= matrixsum;
  sum2 *= matrixsum;
  sum3 *= matrixsum;
  sum0 *= state->amplifynormal;
  sum1 *= state->amplifynormal;
  suminv = (0.5f*255.0f) / fmaxf( 0.0625f, sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) ) );
  sum0 *= suminv;
  sum1 *= suminv;
  sum2 *= suminv;
  energy = sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) );
  sumenergy *= state->normalsustainfactor;
  if( energy < sumenergy )
  {
    f = fminf( sumenergy / energy, 8.0f );
    sum0 *= f;
    sum1 *= f;
    suminv = (0.5f*255.0f) / fmaxf( 0.0625f, sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) ) );
    sum0 *= suminv;
    sum1 *= suminv;
    sum2 *= suminv;
  }
  sum0 += (0.5f*255.0f);
  sum1 += (0.5f*255.0f);
  sum2 += (0.5f*255.0f);
  sum3 *= 255.0f;
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );
  dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );


  return;
}


////


static void imDynamicKernel4NormalSustainAlphaNorm( unsigned char *dst, imGenericMatrixState *state )
{
  int x, y, mapx, mapy;
  float f, alpha, v0, v1, v2, v3, energy, sum0, sum1, sum2, sum3, sumenergy;
  float matrixsum, suminv;
  unsigned char *src;

  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 0.0f;
  sumenergy = 0.0f;
  matrixsum = 0.0f;
  mapy = state->matrixoffsety;
  for( y = 0 ; y < state->matrixsizey ; y++ )
  {
    src = ADDRESS( state->srcdata, ( mapy * state->bytesperline ) );
    mapx = state->matrixoffsetx << 2;
    for( x = 0 ; x < state->matrixsizex ; x++ )
    {
      f = state->linearx[x] * state->lineary[y];
      alpha = (float)src[ mapx + 3 ] * f;
      v0 = alpha * ( (float)src[ mapx + 0 ] - 127.5f );
      v1 = alpha * ( (float)src[ mapx + 1 ] - 127.5f );
      v2 = alpha * ( (float)src[ mapx + 2 ] - 127.5f );
      v3 = alpha;
      sum0 += v0;
      sum1 += v1;
      sum2 += v2;
      sum3 += v3;
      energy = ( v0 * v0 ) + ( v1 * v1 );
      if( energy )
        sumenergy += sqrtf( energy ) / sqrtf( energy + ( v2 * v2 ) );
      matrixsum += f;
      mapx += 4;
      if( mapx >= state->width4 )
        mapx = 0;
    }
    mapy++;
    if( mapy >= state->height )
      mapy = 0;
  }

  matrixsum = 1.0f / matrixsum;
  sum0 *= matrixsum;
  sum1 *= matrixsum;
  sum2 *= matrixsum;
  sum3 *= matrixsum;
  if( sum3 >= state->minimumalphaf )
  {
    f = 1.0f / sum3;
    sum0 *= f;
    sum1 *= f;
    sum2 *= f;
    sum0 *= state->amplifynormal;
    sum1 *= state->amplifynormal;
    suminv = (0.5f*255.0f) / fmaxf( 0.0625f, sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) ) );
    sum0 *= suminv;
    sum1 *= suminv;
    sum2 *= suminv;
    energy = sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) );
    sumenergy *= state->normalsustainfactor;
    if( energy < sumenergy )
    {
      f = fminf( sumenergy / energy, 8.0f );
      sum0 *= f;
      sum1 *= f;
      suminv = (0.5f*255.0f) / fmaxf( 0.0625f, sqrtf( ( sum0 * sum0 ) + ( sum1 * sum1 ) + ( sum2 * sum2 ) ) );
      sum0 *= suminv;
      sum1 *= suminv;
      sum2 *= suminv;
    }
    sum0 += (0.5f*255.0f);
    sum1 += (0.5f*255.0f);
    sum2 += (0.5f*255.0f);
    dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum0 + 0.5f ) ) );
    dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum1 + 0.5f ) ) );
    dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum2 + 0.5f ) ) );
    dst[3] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, sum3 + 0.5f ) ) );
  }
  else
  {
    dst[0] = 0;
    dst[1] = 0;
    dst[2] = 0;
    dst[3] = 0;
  }

  return;
}


////


int imReduceImageKaiserData( unsigned char *dstdata, unsigned char *srcdata, int width, int height, int bytesperpixel, int bytesperline, int newwidth, int newheight, imReduceOptions *options )
{
  int filter, x, y;
  float scalex, scaley, scaleinvx, scaleinvy;
  float sourcex, sourcey;
  unsigned char *dst;
  imGenericMatrixState state;
  void (*applykernel)( unsigned char *dst, imGenericMatrixState *state );
#if CPU_SSE2_SUPPORT
  void (*applykernelcore)( unsigned char *dst, imGenericMatrixState *state );
#endif

  filter = options->filter;
  if( ( newwidth > width ) || ( newheight > height ) )
    return 0;

  applykernel = 0;
#if CPU_SSE2_SUPPORT
  applykernelcore = 0;
#endif

  if( filter == IM_REDUCE_FILTER_LINEAR )
  {
    if( bytesperpixel == 4 )
      applykernel = imDynamicKernel4Linear;
    else if( bytesperpixel == 3 )
      applykernel = imDynamicKernel3Linear;
    else if( bytesperpixel == 2 )
      applykernel = imDynamicKernel2Linear;
    else if( bytesperpixel == 1 )
      applykernel = imDynamicKernel1Linear;
  }
  else if( filter == IM_REDUCE_FILTER_LINEAR_ALPHANORM )
  {
    if( bytesperpixel == 4 )
    {
      applykernel = imDynamicKernel4LinearAlphaNorm;
#if CPU_SSE2_SUPPORT
      applykernelcore = imDynamicKernel4LinearAlphaNorm_Core;
#endif
    }
    else if( bytesperpixel == 3 )
      applykernel = imDynamicKernel3Linear;
    else if( bytesperpixel == 2 )
      applykernel = imDynamicKernel2Linear;
    else if( bytesperpixel == 1 )
      applykernel = imDynamicKernel1Linear;
  }
  else if( filter == IM_REDUCE_FILTER_SRGB )
  {
    if( bytesperpixel == 4 )
      applykernel = imDynamicKernel4sRGB;
    else if( bytesperpixel == 3 )
      applykernel = imDynamicKernel3sRGB;
    else if( bytesperpixel == 2 )
      applykernel = imDynamicKernel2sRGB;
    else if( bytesperpixel == 1 )
      applykernel = imDynamicKernel1sRGB;
  }
  else if( filter == IM_REDUCE_FILTER_SRGB_ALPHANORM )
  {
    if( bytesperpixel == 4 )
      applykernel = imDynamicKernel4sRGBAlphaNorm;
    else if( bytesperpixel == 3 )
      applykernel = imDynamicKernel3sRGB;
    else if( bytesperpixel == 2 )
      applykernel = imDynamicKernel2sRGB;
    else if( bytesperpixel == 1 )
      applykernel = imDynamicKernel1sRGB;
  }
  else if( filter == IM_REDUCE_FILTER_NORMALMAP )
  {
    if( bytesperpixel == 4 )
      applykernel = imDynamicKernel4Normal;
    else if( bytesperpixel == 3 )
      applykernel = imDynamicKernel3Normal;
  }
  else if( filter == IM_REDUCE_FILTER_NORMALMAP_SUSTAIN )
  {
    if( bytesperpixel == 4 )
      applykernel = imDynamicKernel4NormalSustain;
    else if( bytesperpixel == 3 )
      applykernel = imDynamicKernel3NormalSustain;
  }
  else if( filter == IM_REDUCE_FILTER_NORMALMAP_SUSTAIN_ALPHANORM )
  {
    if( bytesperpixel == 4 )
      applykernel = imDynamicKernel4NormalSustainAlphaNorm;
    else if( bytesperpixel == 3 )
      applykernel = imDynamicKernel3NormalSustain;
  }

  if( !applykernel )
    return 0;

  state.minimumalpha = 4;
  state.minimumalphaf = (float)state.minimumalpha;
  state.amplifynormal = fmaxf( 1.0f, options->amplifynormal );
  state.normalsustainfactor = options->normalsustainfactor;

  state.dithersum = 0.0f;
  if( ( newwidth | newheight ) > 2 )
    state.dithersum = 0.5f;

  state.srcdata = srcdata;
  state.width1 = width * 1;
  state.width2 = width * 2;
  state.width3 = width * 3;
  state.width4 = width * 4;
  state.height = height;
  state.bytesperline = bytesperline;

  scalex = (float)newwidth / (float)width;
  scaley = (float)newheight / (float)height;
  scaleinvx = (float)width / (float)newwidth;
  scaleinvy = (float)height / (float)newheight;

  imAllocGenericState( &state, scalex, scaley, options->hopcount, options->alpha );

#if CPU_SSE2_SUPPORT
  if( applykernelcore )
  {
    dst = dstdata;
    for( y = 0 ; y < newheight ; y++ )
    {
      sourcey = ( ( (float)y + 0.5f ) * scaleinvy ) - 0.5f;
      imBuildGenericLinearY( &state, scaley, scaleinvy, sourcey, options->hopcount, options->alpha, height );
      for( x = 0 ; x < newwidth ; x++, dst += bytesperpixel )
      {
        sourcex = ( ( (float)x + 0.5f ) * scaleinvx ) - 0.5f;
        imBuildGenericLinearX( &state, scalex, scaleinvx, sourcex, options->hopcount, options->alpha, width );
        if( ( state.matrixoffsetx + ( ( state.matrixsizex + 3 ) & ~3 ) ) < width )
          applykernelcore( dst, &state );
        else
          applykernel( dst, &state );
      }
    }
  }
  else
#endif
  {
    dst = dstdata;
    for( y = 0 ; y < newheight ; y++ )
    {
      sourcey = ( ( (float)y + 0.5f ) * scaleinvy ) - 0.5f;
      imBuildGenericLinearY( &state, scaley, scaleinvy, sourcey, options->hopcount, options->alpha, height );
      for( x = 0 ; x < newwidth ; x++, dst += bytesperpixel )
      {
        sourcex = ( ( (float)x + 0.5f ) * scaleinvx ) - 0.5f;
        imBuildGenericLinearX( &state, scalex, scaleinvx, sourcex, options->hopcount, options->alpha, width );
        applykernel( dst, &state );
      }
    }
  }

  imFreeGenericState( &state );

  return 1;
}


int imReduceImageKaiser( imgImage *imgdst, imgImage *imgsrc, int newwidth, int newheight, imReduceOptions *options )
{
  int retvalue;

  imgdst->format.width = newwidth;
  imgdst->format.height = newheight;
  imgdst->format.type = imgsrc->format.type;
  imgdst->format.bytesperpixel = imgsrc->format.bytesperpixel;
  imgdst->format.bytesperline = imgdst->format.width * imgdst->format.bytesperpixel;
  imgdst->data = malloc( imgdst->format.height * imgdst->format.bytesperline );
  if( !( imgdst->data ) )
    return 0;

  retvalue = imReduceImageKaiserData( imgdst->data, imgsrc->data, imgsrc->format.width, imgsrc->format.height, imgsrc->format.bytesperpixel, imgsrc->format.bytesperline, newwidth, newheight, options );

  return retvalue;
}



////////////////////////////////////////////////////////////////////////////////



static inline CC_ALWAYSINLINE void imReduceHalfBox1Linear( unsigned char *dst, unsigned char *src, int bytesperpixel, int bytesperline, float *dithersum )
{
  dst[0] = (unsigned char)( ( (int)src[0] + (int)src[bytesperpixel+0] + (int)src[bytesperline+0] + (int)src[bytesperpixel+bytesperline+0] + 2 ) >> 2 );
  return;
}

static inline CC_ALWAYSINLINE void imReduceHalfBox2Linear( unsigned char *dst, unsigned char *src, int bytesperpixel, int bytesperline, float *dithersum )
{
  dst[0] = (unsigned char)( ( (int)src[0] + (int)src[bytesperpixel+0] + (int)src[bytesperline+0] + (int)src[bytesperpixel+bytesperline+0] + 2 ) >> 2 );
  dst[1] = (unsigned char)( ( (int)src[1] + (int)src[bytesperpixel+1] + (int)src[bytesperline+1] + (int)src[bytesperpixel+bytesperline+1] + 2 ) >> 2 );
  return;
}

static inline CC_ALWAYSINLINE void imReduceHalfBox3Linear( unsigned char *dst, unsigned char *src, int bytesperpixel, int bytesperline, float *dithersum )
{
  dst[0] = (unsigned char)( ( (int)src[0] + (int)src[bytesperpixel+0] + (int)src[bytesperline+0] + (int)src[bytesperpixel+bytesperline+0] + 2 ) >> 2 );
  dst[1] = (unsigned char)( ( (int)src[1] + (int)src[bytesperpixel+1] + (int)src[bytesperline+1] + (int)src[bytesperpixel+bytesperline+1] + 2 ) >> 2 );
  dst[2] = (unsigned char)( ( (int)src[2] + (int)src[bytesperpixel+2] + (int)src[bytesperline+2] + (int)src[bytesperpixel+bytesperline+2] + 2 ) >> 2 );
  return;
}

static inline CC_ALWAYSINLINE void imReduceHalfBox4Linear( unsigned char *dst, unsigned char *src, int bytesperpixel, int bytesperline, float *dithersum )
{
  dst[0] = (unsigned char)( ( (int)src[0] + (int)src[bytesperpixel+0] + (int)src[bytesperline+0] + (int)src[bytesperpixel+bytesperline+0] + 2 ) >> 2 );
  dst[1] = (unsigned char)( ( (int)src[1] + (int)src[bytesperpixel+1] + (int)src[bytesperline+1] + (int)src[bytesperpixel+bytesperline+1] + 2 ) >> 2 );
  dst[2] = (unsigned char)( ( (int)src[2] + (int)src[bytesperpixel+2] + (int)src[bytesperline+2] + (int)src[bytesperpixel+bytesperline+2] + 2 ) >> 2 );
  dst[3] = (unsigned char)( ( (int)src[3] + (int)src[bytesperpixel+3] + (int)src[bytesperline+3] + (int)src[bytesperpixel+bytesperline+3] + 2 ) >> 2 );
  return;
}

static inline CC_ALWAYSINLINE void imReduceHalfBox1sRGB( unsigned char *dst, unsigned char *src, int bytesperpixel, int bytesperline, float *dithersum )
{
  int i, offset[4];
  float sum0;
  offset[0] = 0;
  offset[1] = bytesperpixel;
  offset[2] = bytesperline;
  offset[3] = bytesperline + bytesperpixel;
  sum0 = 0.0f;
  for( i = 0 ; i < 4 ; i++ )
    sum0 += srgb2linear( (float)src[offset[i]+0] );
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum0 * 0.25f ) + 0.5f ) ) );
  return;
}

static inline CC_ALWAYSINLINE void imReduceHalfBox2sRGB( unsigned char *dst, unsigned char *src, int bytesperpixel, int bytesperline, float *dithersum )
{
  int i, offset[4];
  float sum0, sum1;
  offset[0] = 0;
  offset[1] = bytesperpixel;
  offset[2] = bytesperline;
  offset[3] = bytesperline + bytesperpixel;
  sum0 = 0.0f;
  sum1 = 0.0f;
  for( i = 0 ; i < 4 ; i++ )
  {
    sum0 += srgb2linear( (float)src[offset[i]+0] );
    sum1 += srgb2linear( (float)src[offset[i]+1] );
  }
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum0 * 0.25f ) + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum1 * 0.25f ) + 0.5f ) ) );
  return;
}

static inline CC_ALWAYSINLINE void imReduceHalfBox3sRGB( unsigned char *dst, unsigned char *src, int bytesperpixel, int bytesperline, float *dithersum )
{
  int i, offset[4];
  float sum0, sum1, sum2;
  offset[0] = 0;
  offset[1] = bytesperpixel;
  offset[2] = bytesperline;
  offset[3] = bytesperline + bytesperpixel;
  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  for( i = 0 ; i < 4 ; i++ )
  {
    sum0 += srgb2linear( (float)src[offset[i]+0] );
    sum1 += srgb2linear( (float)src[offset[i]+1] );
    sum2 += srgb2linear( (float)src[offset[i]+2] );
  }
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum0 * 0.25f ) + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum1 * 0.25f ) + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum2 * 0.25f ) + 0.5f ) ) );
  return;
}

static inline CC_ALWAYSINLINE void imReduceHalfBox4sRGB( unsigned char *dst, unsigned char *src, int bytesperpixel, int bytesperline, float *dithersum )
{
  int i, offset[4], sum3;
  float sum0, sum1, sum2;
  offset[0] = 0;
  offset[1] = bytesperpixel;
  offset[2] = bytesperline;
  offset[3] = bytesperline + bytesperpixel;
  sum0 = 0.0f;
  sum1 = 0.0f;
  sum2 = 0.0f;
  sum3 = 2;
  for( i = 0 ; i < 4 ; i++ )
  {
    sum0 += srgb2linear( (float)src[offset[i]+0] );
    sum1 += srgb2linear( (float)src[offset[i]+1] );
    sum2 += srgb2linear( (float)src[offset[i]+2] );
    sum3 += (int)src[offset[i]+2];
  }
  dst[0] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum0 * 0.25f ) + 0.5f ) ) );
  dst[1] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum1 * 0.25f ) + 0.5f ) ) );
  dst[2] = (unsigned char)( fmaxf( 0.0f, fminf( 255.0f, linear2srgb( sum2 * 0.25f ) + 0.5f ) ) );
  dst[3] = (unsigned char)( sum3 >> 2 );
  return;
}

static inline CC_ALWAYSINLINE void imReduceHalfBox3Normal( unsigned char *dst, unsigned char *src, int bytesperpixel, int bytesperline, float *dithersum )
{
  float v0, v1, v2, suminv;

  v0 = (1.0f/1020.0f) * (float)( (int)src[0] + (int)src[bytesperpixel+0] + (int)src[bytesperline+0] + (int)src[bytesperpixel+bytesperline+0] );
  v1 = (1.0f/1020.0f) * (float)( (int)src[1] + (int)src[bytesperpixel+1] + (int)src[bytesperline+1] + (int)src[bytesperpixel+bytesperline+1] );
  v2 = (1.0f/1020.0f) * (float)( (int)src[2] + (int)src[bytesperpixel+2] + (int)src[bytesperline+2] + (int)src[bytesperpixel+bytesperline+2] );
  v0 = 2.0f * ( v0 - 0.5f );
  v1 = 2.0f * ( v1 - 0.5f );
  v2 = 2.0f * ( v2 - 0.5f );
  suminv = 0.5f / sqrtf( ( v0 * v0 ) + ( v1 * v1 ) + ( v2 * v2 ) );
  v0 = 0.5f + ( v0 * suminv );
  v1 = 0.5f + ( v1 * suminv );
  v2 = 0.5f + ( v2 * suminv );
  dst[0] = (unsigned char)ROUND_POSITIVE_FLOAT( 255.0f * v0 );
  dst[1] = (unsigned char)ROUND_POSITIVE_FLOAT( 255.0f * v1 );
  dst[2] = (unsigned char)ROUND_POSITIVE_FLOAT( 255.0f * v2 );

  return;
}

static inline CC_ALWAYSINLINE void imReduceHalfBox4Normal( unsigned char *dst, unsigned char *src, int bytesperpixel, int bytesperline, float *dithersum )
{
  float v0, v1, v2, suminv;

  v0 = (1.0f/1020.0f) * (float)( (int)src[0] + (int)src[bytesperpixel+0] + (int)src[bytesperline+0] + (int)src[bytesperpixel+bytesperline+0] );
  v1 = (1.0f/1020.0f) * (float)( (int)src[1] + (int)src[bytesperpixel+1] + (int)src[bytesperline+1] + (int)src[bytesperpixel+bytesperline+1] );
  v2 = (1.0f/1020.0f) * (float)( (int)src[2] + (int)src[bytesperpixel+2] + (int)src[bytesperline+2] + (int)src[bytesperpixel+bytesperline+2] );
  v0 = 2.0f * ( v0 - 0.5f );
  v1 = 2.0f * ( v1 - 0.5f );
  v2 = 2.0f * ( v2 - 0.5f );
  suminv = 0.5f / sqrtf( ( v0 * v0 ) + ( v1 * v1 ) + ( v2 * v2 ) );
  v0 = 0.5f + ( v0 * suminv );
  v1 = 0.5f + ( v1 * suminv );
  v2 = 0.5f + ( v2 * suminv );
  dst[0] = (unsigned char)ROUND_POSITIVE_FLOAT( 255.0f * v0 );
  dst[1] = (unsigned char)ROUND_POSITIVE_FLOAT( 255.0f * v1 );
  dst[2] = (unsigned char)ROUND_POSITIVE_FLOAT( 255.0f * v2 );
  dst[3] = (unsigned char)( ( (int)src[3] + (int)src[bytesperpixel+3] + (int)src[bytesperline+3] + (int)src[bytesperpixel+bytesperline+3] + 2 ) >> 2 );

  return;
}

static inline CC_ALWAYSINLINE void imReduceHalfBox3Water( unsigned char *dst, unsigned char *src, int bytesperpixel, int bytesperline, float *dithersum )
{
  float v0, v1, v2, suminv;

  v0 = (1.0f/1020.0f) * (float)( (int)src[0] + (int)src[bytesperpixel+0] + (int)src[bytesperline+0] + (int)src[bytesperpixel+bytesperline+0] );
  v1 = (1.0f/1020.0f) * (float)( (int)src[1] + (int)src[bytesperpixel+1] + (int)src[bytesperline+1] + (int)src[bytesperpixel+bytesperline+1] );
  v2 = (1.0f/1020.0f) * (float)( (int)src[2] + (int)src[bytesperpixel+2] + (int)src[bytesperline+2] + (int)src[bytesperpixel+bytesperline+2] );

  v0 = 2.0f * ( v0 - 0.5f );
  v1 = 2.0f * ( v1 - 0.5f );
  suminv = sqrtf( ( v0 * v0 ) + ( v1 * v1 ) );
  if( suminv < 0.75f )
  {
    suminv = 0.5f / suminv;
    v0 = 0.5f + ( v0 * suminv );
    v1 = 0.5f + ( v1 * suminv );
  }
  if( v2 > 0.1f )
  {
    *dithersum += v2;
    if( v2 > 0.45f )
      v2 = 1.0f;
    else if( ( v2 < 0.3f ) && ( *dithersum < 1.0f ) )
      v2 = 0.0f;
    else
      v2 = ( ( v2 + *dithersum ) < 0.45f ? 0.0f : 1.0f );
    *dithersum -= v2;
  }
  v0 *= 255.0f;
  v1 *= 255.0f;
  v2 *= 255.0f;

  dst[0] = (int)( fmaxf( 0.0f, fminf( 255.0f, v0 + 0.5f ) ) );
  dst[1] = (int)( fmaxf( 0.0f, fminf( 255.0f, v1 + 0.5f ) ) );
  dst[2] = (int)( fmaxf( 0.0f, fminf( 255.0f, v2 + 0.5f ) ) );

  return;
}

static inline CC_ALWAYSINLINE void imReduceHalfBox4Water( unsigned char *dst, unsigned char *src, int bytesperpixel, int bytesperline, float *dithersum )
{
  float v0, v1, v2, suminv;

  v0 = (1.0f/1020.0f) * (float)( (int)src[0] + (int)src[bytesperpixel+0] + (int)src[bytesperline+0] + (int)src[bytesperpixel+bytesperline+0] );
  v1 = (1.0f/1020.0f) * (float)( (int)src[1] + (int)src[bytesperpixel+1] + (int)src[bytesperline+1] + (int)src[bytesperpixel+bytesperline+1] );
  v2 = (1.0f/1020.0f) * (float)( (int)src[2] + (int)src[bytesperpixel+2] + (int)src[bytesperline+2] + (int)src[bytesperpixel+bytesperline+2] );

  v0 = 2.0f * ( v0 - 0.5f );
  v1 = 2.0f * ( v1 - 0.5f );
  suminv = sqrtf( ( v0 * v0 ) + ( v1 * v1 ) );
  if( suminv < 0.75f )
  {
    suminv = 0.5f / suminv;
    v0 = 0.5f + ( v0 * suminv );
    v1 = 0.5f + ( v1 * suminv );
  }
  if( v2 > 0.1f )
  {
    *dithersum += v2;
    if( v2 > 0.45f )
      v2 = 1.0f;
    else if( ( v2 < 0.3f ) && ( *dithersum < 1.0f ) )
      v2 = 0.0f;
    else
      v2 = ( ( v2 + *dithersum ) < 0.45f ? 0.0f : 1.0f );
    *dithersum -= v2;
  }
  v0 *= 255.0f;
  v1 *= 255.0f;
  v2 *= 255.0f;

  dst[0] = (int)( fmaxf( 0.0f, fminf( 255.0f, v0 + 0.5f ) ) );
  dst[1] = (int)( fmaxf( 0.0f, fminf( 255.0f, v1 + 0.5f ) ) );
  dst[2] = (int)( fmaxf( 0.0f, fminf( 255.0f, v2 + 0.5f ) ) );
  dst[3] = (unsigned char)( ( (int)src[3] + (int)src[bytesperpixel+3] + (int)src[bytesperline+3] + (int)src[bytesperpixel+bytesperline+3] + 2 ) >> 2 );

  return;
}

static inline CC_ALWAYSINLINE void imReduceHalfBox4Plant( unsigned char *dst, unsigned char *src, int bytesperpixel, int bytesperline, float *dithersum )
{
  int alpha;

  dst[0] = (unsigned char)( ( (int)src[0] + (int)src[bytesperpixel+0] + (int)src[bytesperline+0] + (int)src[bytesperpixel+bytesperline+0] + 2 ) >> 2 );
  dst[1] = (unsigned char)( ( (int)src[1] + (int)src[bytesperpixel+1] + (int)src[bytesperline+1] + (int)src[bytesperpixel+bytesperline+1] + 2 ) >> 2 );
  dst[2] = (unsigned char)( ( (int)src[2] + (int)src[bytesperpixel+2] + (int)src[bytesperline+2] + (int)src[bytesperpixel+bytesperline+2] + 2 ) >> 2 );

  alpha = ( (int)src[3] + (int)src[bytesperpixel+3] + (int)src[bytesperline+3] + (int)src[bytesperpixel+bytesperline+3] );
  alpha += alpha >> 2;
  alpha = ( alpha + 2 ) >> 2;
  if( alpha > 255 )
    alpha = 255;
  dst[3] = (unsigned char)alpha;

  return;
}


static inline CC_ALWAYSINLINE void imReduceImageHalfBoxWork( unsigned char *dst, unsigned char *src, int width, int height, int bytesperpixel, int bytesperline, void (*work)( unsigned char *dst, unsigned char *src, int bytesperpixel, int bytesperline, float *dithersum ) )
{
  int x, y, newwidth, newheight, rowoffset;
  float dithersum;
  newwidth = ( width < 2 ) ? 1 : ( ( width + 1 ) / 2 );
  newheight = ( height < 2 ) ? 1 : ( ( height + 1 ) / 2 );
  rowoffset = bytesperline + ( bytesperpixel * ( width - ( newwidth << 1 ) ) );
  dithersum = 0.0f;
  if( ( newwidth | newheight ) > 2 )
    dithersum = 0.5f;
  for( y = 0 ; y < newheight ; y++ )
  {
    for( x = 0 ; x < newwidth ; x++, src += bytesperpixel, dst += bytesperpixel )
    {
      work( dst, src, bytesperpixel, bytesperline, &dithersum );
      src += bytesperpixel;
    }
    src += rowoffset;
  }
  return;
}


int imReduceImageHalfBoxData( unsigned char *dstdata, unsigned char *srcdata, int width, int height, int bytesperpixel, int bytesperline, imReduceOptions *options )
{
  int filter, retval;

  filter = options->filter;
  retval = 1;
  if( ( filter == IM_REDUCE_FILTER_LINEAR ) || ( filter == IM_REDUCE_FILTER_LINEAR_ALPHANORM ) )
  {
    if( bytesperpixel == 4 )
      imReduceImageHalfBoxWork( dstdata, srcdata, width, height, bytesperpixel, bytesperline, imReduceHalfBox4Linear );
    else if( bytesperpixel == 3 )
      imReduceImageHalfBoxWork( dstdata, srcdata, width, height, bytesperpixel, bytesperline, imReduceHalfBox3Linear );
    else if( bytesperpixel == 2 )
      imReduceImageHalfBoxWork( dstdata, srcdata, width, height, bytesperpixel, bytesperline, imReduceHalfBox2Linear );
    else if( bytesperpixel == 1 )
      imReduceImageHalfBoxWork( dstdata, srcdata, width, height, bytesperpixel, bytesperline, imReduceHalfBox1Linear );
    else
      retval = 0;
  }
  else if( ( filter == IM_REDUCE_FILTER_SRGB ) || ( filter == IM_REDUCE_FILTER_SRGB_ALPHANORM ) )
  {
    if( bytesperpixel == 4 )
      imReduceImageHalfBoxWork( dstdata, srcdata, width, height, bytesperpixel, bytesperline, imReduceHalfBox4sRGB );
    else if( bytesperpixel == 3 )
      imReduceImageHalfBoxWork( dstdata, srcdata, width, height, bytesperpixel, bytesperline, imReduceHalfBox3sRGB );
    else if( bytesperpixel == 2 )
      imReduceImageHalfBoxWork( dstdata, srcdata, width, height, bytesperpixel, bytesperline, imReduceHalfBox2sRGB );
    else if( bytesperpixel == 1 )
      imReduceImageHalfBoxWork( dstdata, srcdata, width, height, bytesperpixel, bytesperline, imReduceHalfBox1sRGB );
    else
      retval = 0;
  }
  else if( ( filter == IM_REDUCE_FILTER_NORMALMAP ) || ( filter == IM_REDUCE_FILTER_NORMALMAP_ALPHANORM ) || ( filter == IM_REDUCE_FILTER_NORMALMAP_SUSTAIN ) || ( filter == IM_REDUCE_FILTER_NORMALMAP_SUSTAIN_ALPHANORM ) )
  {
    if( bytesperpixel == 4 )
      imReduceImageHalfBoxWork( dstdata, srcdata, width, height, bytesperpixel, bytesperline, imReduceHalfBox4Normal );
    else if( bytesperpixel == 3 )
      imReduceImageHalfBoxWork( dstdata, srcdata, width, height, bytesperpixel, bytesperline, imReduceHalfBox3Normal );
    else
      retval = 0;
  }
  else if( filter == IM_REDUCE_FILTER_WATERMAP )
  {
    if( bytesperpixel == 4 )
      imReduceImageHalfBoxWork( dstdata, srcdata, width, height, bytesperpixel, bytesperline, imReduceHalfBox4Water );
    else if( bytesperpixel == 3 )
      imReduceImageHalfBoxWork( dstdata, srcdata, width, height, bytesperpixel, bytesperline, imReduceHalfBox3Water );
    else
      retval = 0;
  }
  else if( filter == IM_REDUCE_FILTER_PLANTMAP )
  {
    if( bytesperpixel == 4 )
      imReduceImageHalfBoxWork( dstdata, srcdata, width, height, bytesperpixel, bytesperline, imReduceHalfBox4Plant );
    else
      retval = 0;
  }
  else
    retval = 0;

  return retval;
}


int imReduceImageHalfBox( imgImage *imgdst, imgImage *imgsrc, imReduceOptions *options )
{
  int newwidth, newheight, retvalue;

  newwidth = ( ( imgsrc->format.width < 2 ) ? 1 : ( ( imgsrc->format.width + 1 ) / 2 ) );
  newheight = ( ( imgsrc->format.height < 2 ) ? 1 : ( ( imgsrc->format.height + 1 ) / 2 ) );

  imgdst->format.width = newwidth;
  imgdst->format.height = newheight;
  imgdst->format.type = imgsrc->format.type;
  imgdst->format.bytesperpixel = imgsrc->format.bytesperpixel;
  imgdst->format.bytesperline = imgdst->format.width * imgdst->format.bytesperpixel;
  imgdst->data = malloc( imgdst->format.height * imgdst->format.bytesperline );

  retvalue = imReduceImageHalfBoxData( imgdst->data, imgsrc->data, imgsrc->format.width, imgsrc->format.height, imgsrc->format.bytesperpixel, imgsrc->format.bytesperline, options );

  return retvalue;
}



////////////////////////////////////////////////////////////////////////////////



int imBuildMipmapCascade( imMipmapCascade *cascade, void *imagedata, int width, int height, int layercount, int bytesperpixel, int bytesperline, imReduceOptions *options, int cascadeflags )
{
  int layerindex, level, srclevel, srcwidth, srcheight, method, divisor;
  int levelwidth, levelheight;
  void *src, *dst;

  cascade->width = width;
  cascade->height = height;
  cascade->layercount = layercount;
  cascade->bytesperpixel = bytesperpixel;
  cascade->bytesperline = bytesperline;
  cascade->options = options;

  /* No need for mipmaps */
  if( ( cascade->width == 1 ) && ( cascade->height == 1 ) )
    return 1;
  if( bytesperpixel != 4 )
    cascadeflags &= ~( IM_CASCADE_FLAGS_COLOR_BORDER_BASE | IM_CASCADE_FLAGS_COLOR_BORDER_MIPMAPS );

  /* Allocate all the mipmap levels */
  if( !( layercount ) )
    layercount = 1;
  cascade->mipmap[0] = imagedata;
  levelwidth = cascade->width;
  levelheight = cascade->height;
  for( level = 1 ; ; level++ )
  {
    levelwidth = ( levelwidth < 2 ) ? 1 : ( levelwidth >> 1 );
    levelheight = ( levelheight < 2 ) ? 1 : ( levelheight >> 1 );
    if( !( cascade->mipmap[level] = malloc( levelwidth * levelheight * layercount * bytesperpixel ) ) )
      return 0;
    if( ( levelwidth == 1 ) && ( levelheight == 1 ) )
      break;
  }
  cascade->mipmap[level+1] = 0;

  if( cascadeflags & IM_CASCADE_FLAGS_COLOR_BORDER_BASE )
    imPropagateAlphaBorder( imagedata, width, height * layercount, bytesperpixel, bytesperline );

  /* For every layer, compute all its mipmap */
  for( layerindex = 0 ; layerindex < layercount ; layerindex++ )
  {
    levelwidth = cascade->width;
    levelheight = cascade->height;
    for( level = 1 ; cascade->mipmap[level] ; level++ )
    {
      levelwidth = ( levelwidth < 2 ) ? 1 : ( levelwidth >> 1 );
      levelheight = ( levelheight < 2 ) ? 1 : ( levelheight >> 1 );
      dst = ADDRESS( cascade->mipmap[level], layerindex * levelwidth * levelheight * bytesperpixel );

      /* Decide what method and source level to pick */
      if( ( levelwidth | levelheight ) >= 16 )
      {
        srclevel = level - 2;
        if( srclevel < 0 )
          srclevel = 0;
        method = 1;
      }
      else
      {
        srclevel = level - 1;
        method = 0;
      }
#if DEBUG_VERBOSE
      printf( "Tex level %d, srclevel %d, layer %d, filter %d, method %d : %d x %d\n", level, srclevel, layerindex, options->filter, method, levelwidth, levelheight );
#endif
      srcwidth = width >> srclevel;
      if( !( srcwidth ) )
        srcwidth = 1;
      srcheight = height >> srclevel;
      if( !( srcheight ) )
        srcheight = 1;
      if( srclevel )
        src = ADDRESS( cascade->mipmap[srclevel], layerindex * srcheight * srcwidth * bytesperpixel );
      else
        src = ADDRESS( cascade->mipmap[srclevel], layerindex * srcheight * cascade->bytesperline );

      divisor = 1 << ( level - srclevel );
      if( ( ( levelwidth * divisor ) != srcwidth ) || ( ( levelheight * divisor ) != srcheight ) )
        method = 2;

      if( method == 2 )
      {
        if( !( imReduceImageKaiserData( dst, src, srcwidth, srcheight, bytesperpixel, srcwidth * bytesperpixel, levelwidth, levelheight, options ) ) )
        {
          printf( "ERROR AT %s:%d\n", __FILE__, __LINE__ );
          return 0;
        }
      }
      else if( method == 1 )
      {
        if( !( imReduceImageKaiserDataDivisor( dst, src, srcwidth, srcheight, bytesperpixel, srcwidth * bytesperpixel, divisor, options ) ) )
        {
          printf( "ERROR AT %s:%d\n", __FILE__, __LINE__ );
          return 0;
        }
      }
      else
      {
        if( !( imReduceImageHalfBoxData( dst, src, srcwidth, srcheight, bytesperpixel, srcwidth * bytesperpixel, options ) ) )
        {
          printf( "ERROR AT %s:%d\n", __FILE__, __LINE__ );
          return 0;
        }
      }

      if( cascadeflags & IM_CASCADE_FLAGS_COLOR_BORDER_MIPMAPS )
        imPropagateAlphaBorder( dst, levelwidth, levelheight, bytesperpixel, levelwidth * bytesperpixel );
    }
  }

  return 1;
}


void imFreeMipmapCascade( imMipmapCascade *cascade )
{
  int level;
  for( level = 1 ; ; level++ )
  {
    if( !( cascade->mipmap[level] ) )
      break;
    free( cascade->mipmap[level] );
    cascade->mipmap[level] = 0;
  }
  return;
}


////


#define IM_PIXEL_ALPHA_MASK (0xff000000)
#define IM_PIXEL_RGB_MASK (0x00ffffff)

void imPropagateAlphaBorder( unsigned char *imagedata, int width, int height, int bytesperpixel, int bytesperline )
{
  int x, y, backtrackflag;
  uint32_t pixel, refcolor, prevrowpixel;
  uint32_t *row, *prevrow;

  if( bytesperpixel != 4 )
    return;
  row = (uint32_t *)imagedata;
  prevrow = row;
  for( y = 0 ; y < height ; y++ )
  {
    refcolor = 0;
    backtrackflag = 0;
    for( x = 0 ; x < width ; x++ )
    {
      pixel = row[x];
      prevrowpixel = prevrow[x];
      if( pixel & IM_PIXEL_ALPHA_MASK )
      {
        /* Pixel has some color, spread to neighbor if applicable */
        refcolor = pixel & IM_PIXEL_RGB_MASK;
        if( backtrackflag )
        {
          row[x-1] = refcolor;
          backtrackflag = 0;
        }
        if( !( prevrowpixel & IM_PIXEL_ALPHA_MASK ) )
          prevrow[x] = refcolor;
      }
      else
      {
        /* Pixel is fully transparent, spread from neighbor if applicable */
        if( refcolor )
        {
          row[x] = refcolor;
          backtrackflag = 0;
          refcolor = 0;
        }
        else if( prevrowpixel & IM_PIXEL_ALPHA_MASK )
        {
          row[x] = prevrowpixel & IM_PIXEL_RGB_MASK;
          backtrackflag = 0;
        }
        else
          backtrackflag = 1;
      }
    }
    prevrow = row;
    row = ADDRESS( row, bytesperline );
  }

  return;
}


