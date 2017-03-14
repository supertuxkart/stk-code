/* -----------------------------------------------------------------------------
 *
 * Copyright (c) 2008-2016 Alexis Naveros.
 *
 * The SIMD trigonometry functions are Copyright (C) 2007  Julien Pommier
 * See copyright notice for simd4f_sin_ps(), simd4f_cos_ps(), simd4f_sincos_ps()
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


#ifndef CPUSIMD_H
#define CPUSIMD_H


////


#if __MMX__ || CPU_ENABLE_MMX
 #include <mmintrin.h>
 #define CPU_MMX_SUPPORT (1)
#endif
#if __SSE__ || _M_X64 || _M_IX86_FP >= 1  || CPU_ENABLE_SSE
 #include <xmmintrin.h>
 #define CPU_SSE_SUPPORT (1)
#endif
#if __SSE2__ || _M_X64 || _M_IX86_FP >= 2  || CPU_ENABLE_SSE2
 #include <emmintrin.h>
 #define CPU_SSE2_SUPPORT (1)
#endif
#if __SSE3__ || __AVX__ || CPU_ENABLE_SSE3
 #include <pmmintrin.h>
 #define CPU_SSE3_SUPPORT (1)
#endif
#if __SSSE3__ || __AVX__  || CPU_ENABLE_SSSE3
 #include <tmmintrin.h>
 #define CPU_SSSE3_SUPPORT (1)
#endif
#if __SSE4_1__ || __AVX__  || CPU_ENABLE_SSE4_1
 #include <smmintrin.h>
 #define CPU_SSE4_1_SUPPORT (1)
#endif
#if __SSE4_2__ || CPU_ENABLE_SSE4_2
 #include <nmmintrin.h>
 #define CPU_SSE4_2_SUPPORT (1)
#endif
#if __SSE4A__ || CPU_ENABLE_SSE4A
 #include <ammintrin.h>
 #define CPU_SSE4A_SUPPORT (1)
#endif
#if __AVX__ || CPU_ENABLE_AVX
 #include <immintrin.h>
 #define CPU_AVX_SUPPORT (1)
#endif
#if __AVX2__ || CPU_ENABLE_AVX2
 #include <immintrin.h>
 #define CPU_AVX2_SUPPORT (1)
#endif
#if __XOP__ || CPU_ENABLE_XOP
 #include <immintrin.h>
 #define CPU_XOP_SUPPORT (1)
#endif
#if __FMA3__ || CPU_ENABLE_FMA3
 #include <immintrin.h>
 #define CPU_FMA3_SUPPORT (1)
#endif
#if __FMA4__ || CPU_ENABLE_FMA4
 #include <immintrin.h>
 #define CPU_FMA4_SUPPORT (1)
#endif
#if __RDRND__ || CPU_ENABLE_RDRND
 #include <immintrin.h>
 #define CPU_RDRND_SUPPORT (1)
#endif
#if __POPCNT__ || CPU_ENABLE_POPCNT
 #include <popcntintrin.h>
 #define CPU_POPCNT_SUPPORT (1)
#endif
#if __LZCNT__ || CPU_ENABLE_LZCNT
 #include <lzcntintrin.h>
 #define CPU_LZCNT_SUPPORT (1)
#endif
#if __F16C__ || CPU_ENABLE_F16C
 #include <f16cintrin.h>
 #define CPU_F16C_SUPPORT (1)
#endif
#if __BMI__ || CPU_ENABLE_BMI
 #include <bmiintrin.h>
 #define CPU_BMI_SUPPORT (1)
#endif
#if __BMI2__ || CPU_ENABLE_BMI2
 #include <bmi2intrin.h>
 #define CPU_BMI2_SUPPORT (1)
#endif
#if __TBM__ || CPU_ENABLE_TBM
 #include <tbmintrin.h>
 #define CPU_TBM_SUPPORT (1)
#endif



#if defined(__GNUC__) || defined(__INTEL_COMPILER)
 #define CPU_ALIGN16 __attribute__((aligned(16)))
 #define CPU_ALIGN32 __attribute__((aligned(32)))
 #define CPU_ALIGN64 __attribute__((aligned(64)))
#elif defined(_MSC_VER)
 #define CPU_ALIGN16 __declspec(align(16))
 #define CPU_ALIGN64 __declspec(align(64))
#else
 #define CPU_ALIGN16
 #define CPU_ALIGN32
 #define CPU_ALIGN64
 #warning "SSE/AVX Disabled: Unsupported Compiler."
 #undef CPU_SSE_SUPPORT
 #undef CPU_SSE2_SUPPORT
 #undef CPU_SSE3_SUPPORT
 #undef CPU_SSSE3_SUPPORT
 #undef CPU_SSE4_1_SUPPORT
 #undef CPU_SSE4_2_SUPPORT
 #undef CPU_AVX_SUPPORT
 #undef CPU_AVX2_SUPPORT
 #undef CPU_XOP_SUPPORT
 #undef CPU_FMA3_SUPPORT
 #undef CPU_FMA4_SUPPORT
#endif


////


#if CPU_SSE_SUPPORT
 #define CPU_APPROX_DIV_FLOAT(z,w) _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(z),_mm_rcp_ss(_mm_set_ss(w))))
 #define CPU_APPROX_SQRT_FLOAT(z) _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(z),_mm_rsqrt_ss(_mm_set_ss(z))))
 #define CPU_APPROX_RSQRT_FLOAT(z) _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(z)))
 #define CPU_APPROX_DIVSQRT_FLOAT(z,w) _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(z),_mm_rsqrt_ss(_mm_set_ss(w))))
#else
 #define CPU_APPROX_DIV_FLOAT(z,w) ((z)/(w))
 #define CPU_APPROX_SQRT_FLOAT(z) (sqrtf(z))
 #define CPU_APPROX_RSQRT_FLOAT(z) (1.0/sqrtf(z))
 #define CPU_APPROX_DIVSQRT_FLOAT(z,w) ((z)/sqrtf(w))
#endif


#if CPU_SSE3_SUPPORT
 #define CPU_HADD_PS(vx,vy) _mm_hadd_ps(vx,vy)
 #define CPU_HADD_PD(vx,vy) _mm_hadd_pd(vx,vy)
#elif CPU_SSE_SUPPORT
 static inline __m128 CPU_HADD_PS( __m128 vx, __m128 vy )
 {
   __m128 vh, vl;
   vh = _mm_shuffle_ps( vx, vy, _MM_SHUFFLE(3,1,3,1) );
   vl = _mm_shuffle_ps( vx, vy, _MM_SHUFFLE(2,0,2,0) );
   return _mm_add_ps( vh, vl );
 }
 #define CPU_HADD_PD(vx,vy) _mm_add_sd(vx,_mm_unpackhi_pd(vy,vy))
#endif


#if CPU_SSE4_1_SUPPORT
 #define CPU_CVT_U8_TO_I32(x,vzero) _mm_cvtepu8_epi32(x)
 #define CPU_CVT_S8_TO_I32(x,vzero) _mm_cvtepi8_epi32(x)
#elif CPU_SSE2_SUPPORT
 #define CPU_CVT_U8_TO_I32(x,vzero) _mm_unpacklo_epi16(_mm_unpacklo_epi8((x),(vzero)),(vzero))
static inline __m128i CPU_CVT_S8_TO_I32( __m128i vx, __m128i vzero )
{
  __m128i vsign;
  vsign = _mm_cmpgt_epi8( vzero, vx );
  return _mm_unpacklo_epi16( _mm_unpacklo_epi8( vx, vsign ), _mm_unpacklo_epi8( vsign, vsign ) );
}
#endif


#if CPU_SSE4_1_SUPPORT
 #define CPU_BLENDV_PS(x,y,mask) _mm_blendv_ps(x,y,mask)
 #define CPU_BLENDV_PD(x,y,mask) _mm_blendv_pd(x,y,mask)
#elif CPU_SSE2_SUPPORT
 #define CPU_BLENDV_PS(x,y,mask) _mm_or_ps(_mm_andnot_ps(mask,x),_mm_and_ps(y,mask))
 #define CPU_BLENDV_PD(x,y,mask) _mm_or_pd(_mm_andnot_pd(mask,x),_mm_and_pd(y,mask))
#endif



/*
CPU_FMADD = ((f0*f1)+t0)
CPU_FMSUB = ((f0*f1)-t0)
*/
#if CPU_FMA3_SUPPORT
 #define CPU_FMADD_SS(f0,f1,t0) _mm_fmadd_ss(f0,f1,t0)
 #define CPU_FMADD_PS(f0,f1,t0) _mm_fmadd_ps(f0,f1,t0)
 #define CPU_FMADD_SD(f0,f1,t0) _mm_fmadd_sd(f0,f1,t0)
 #define CPU_FMADD_PD(f0,f1,t0) _mm_fmadd_pd(f0,f1,t0)
 #define CPU_FMSUB_SS(f0,f1,t0) _mm_fmsub_ss(f0,f1,t0)
 #define CPU_FMSUB_PS(f0,f1,t0) _mm_fmsub_ps(f0,f1,t0)
 #define CPU_FMSUB_SD(f0,f1,t0) _mm_fmsub_sd(f0,f1,t0)
 #define CPU_FMSUB_PD(f0,f1,t0) _mm_fmsub_pd(f0,f1,t0)
 #define CPU_FMADD256_SS(f0,f1,t0) _mm256_fmadd_ss(f0,f1,t0)
 #define CPU_FMADD256_PS(f0,f1,t0) _mm256_fmadd_ps(f0,f1,t0)
 #define CPU_FMADD256_SD(f0,f1,t0) _mm256_fmadd_sd(f0,f1,t0)
 #define CPU_FMADD256_PD(f0,f1,t0) _mm256_fmadd_pd(f0,f1,t0)
 #define CPU_FMSUB256_SS(f0,f1,t0) _mm256_fmsub_ss(f0,f1,t0)
 #define CPU_FMSUB256_PS(f0,f1,t0) _mm256_fmsub_ps(f0,f1,t0)
 #define CPU_FMSUB256_SD(f0,f1,t0) _mm256_fmsub_sd(f0,f1,t0)
 #define CPU_FMSUB256_PD(f0,f1,t0) _mm256_fmsub_pd(f0,f1,t0)
#elif CPU_FMA4_SUPPORT
 #define CPU_FMADD_SS(f0,f1,t0) _mm_macc_ss(f0,f1,t0)
 #define CPU_FMADD_PS(f0,f1,t0) _mm_macc_ps(f0,f1,t0)
 #define CPU_FMADD_SD(f0,f1,t0) _mm_macc_sd(f0,f1,t0)
 #define CPU_FMADD_PD(f0,f1,t0) _mm_macc_pd(f0,f1,t0)
 #define CPU_FMSUB_SS(f0,f1,t0) _mm_msub_ss(f0,f1,t0)
 #define CPU_FMSUB_PS(f0,f1,t0) _mm_msub_ps(f0,f1,t0)
 #define CPU_FMSUB_SD(f0,f1,t0) _mm_msub_sd(f0,f1,t0)
 #define CPU_FMSUB_PD(f0,f1,t0) _mm_msub_pd(f0,f1,t0)
 #define CPU_FMADD256_SS(f0,f1,t0) _mm256_macc_ss(f0,f1,t0)
 #define CPU_FMADD256_PS(f0,f1,t0) _mm256_macc_ps(f0,f1,t0)
 #define CPU_FMADD256_SD(f0,f1,t0) _mm256_macc_sd(f0,f1,t0)
 #define CPU_FMADD256_PD(f0,f1,t0) _mm256_macc_pd(f0,f1,t0)
 #define CPU_FMSUB256_SS(f0,f1,t0) _mm256_msub_ss(f0,f1,t0)
 #define CPU_FMSUB256_PS(f0,f1,t0) _mm256_msub_ps(f0,f1,t0)
 #define CPU_FMSUB256_SD(f0,f1,t0) _mm256_msub_sd(f0,f1,t0)
 #define CPU_FMSUB256_PD(f0,f1,t0) _mm256_msub_pd(f0,f1,t0)
#else
 #define CPU_FMADD_SS(f0,f1,t0) _mm_add_ss(_mm_mul_ss(f0,f1),t0)
 #define CPU_FMADD_PS(f0,f1,t0) _mm_add_ps(_mm_mul_ps(f0,f1),t0)
 #define CPU_FMADD_SD(f0,f1,t0) _mm_add_sd(_mm_mul_sd(f0,f1),t0)
 #define CPU_FMADD_PD(f0,f1,t0) _mm_add_pd(_mm_mul_pd(f0,f1),t0)
 #define CPU_FMSUB_SS(f0,f1,t0) _mm_sub_ss(_mm_mul_ss(f0,f1),t0)
 #define CPU_FMSUB_PS(f0,f1,t0) _mm_sub_ps(_mm_mul_ps(f0,f1),t0)
 #define CPU_FMSUB_SD(f0,f1,t0) _mm_sub_sd(_mm_mul_sd(f0,f1),t0)
 #define CPU_FMSUB_PD(f0,f1,t0) _mm_sub_pd(_mm_mul_pd(f0,f1),t0)
 #define CPU_FMADD256_SS(f0,f1,t0) _mm256_add_ss(_mm256_mul_ss(f0,f1),t0)
 #define CPU_FMADD256_PS(f0,f1,t0) _mm256_add_ps(_mm256_mul_ps(f0,f1),t0)
 #define CPU_FMADD256_SD(f0,f1,t0) _mm256_add_sd(_mm256_mul_sd(f0,f1),t0)
 #define CPU_FMADD256_PD(f0,f1,t0) _mm256_add_pd(_mm256_mul_pd(f0,f1),t0)
 #define CPU_FMSUB256_SS(f0,f1,t0) _mm256_sub_ss(_mm256_mul_ss(f0,f1),t0)
 #define CPU_FMSUB256_PS(f0,f1,t0) _mm256_sub_ps(_mm256_mul_ps(f0,f1),t0)
 #define CPU_FMSUB256_SD(f0,f1,t0) _mm256_sub_sd(_mm256_mul_sd(f0,f1),t0)
 #define CPU_FMSUB256_PD(f0,f1,t0) _mm256_sub_pd(_mm256_mul_pd(f0,f1),t0)
#endif


////


#if CPU_SSE_SUPPORT

extern const uint32_t simd4fSignMask[4];
extern const uint32_t simd4fSignMaskInv[4];
extern const float simd4fHalf[4];
extern const float simd4fOne[4];
extern const float simd4fTwo[4];
extern const float simd4fThree[4];
extern const uint32_t simd4uOne[4];
extern const uint32_t simd4uOneInv[4];
extern const uint32_t simd4uTwo[4];
extern const uint32_t simd4uFour[4];
extern const float simd4fQuarter[4];
extern const float simd4fPi[4];
extern const float simd4fZeroOneTwoThree[4];
extern const uint32_t simd4fAlphaMask[4];
extern const float simd4f255[4];
extern const float simd4f255Inv[4];

#endif


#if CPU_SSE2_SUPPORT

/* Input range between -8192 and 8192 */
__m128 simd4f_sin_ps( __m128 x );
__m128 simd4f_cos_ps( __m128 x );
void simd4f_sincos_ps( __m128 x, __m128 *s, __m128 *c );

#endif

#if CPU_SSE2_SUPPORT

__m128 simd4f_exp2_ps( __m128 x );
__m128 simd4f_log2_ps( __m128 x );
__m128 simd4f_pow_ps( __m128 x, __m128 y );

#endif

#if CPU_SSE2_SUPPORT

__m128 simd4f_pow12d5_ps( __m128 arg );
__m128 simd4f_pow5d12_ps( __m128 arg );

#endif


////


#if CPU_SSE2_SUPPORT

#ifndef CC_ALWAYSINLINE
 #if defined(__GNUC__) || defined(__INTEL_COMPILER)
  #define CC_ALWAYSINLINE __attribute__((always_inline))
 #else
  #define CC_ALWAYSINLINE
 #endif
#endif

static inline CC_ALWAYSINLINE __m128 simd4f_pow12d5_inline_ps( __m128 vx )
{
  __m128 vpow, vpwsqrtinv, vpwsqrt, vx2;
  vx2 = _mm_mul_ps( vx, vx );
  vpow = _mm_castsi128_ps( _mm_cvtps_epi32( _mm_mul_ps( _mm_cvtepi32_ps( _mm_castps_si128( _mm_mul_ps( vx, _mm_set1_ps( 5417434112.0f ) ) ) ), _mm_set1_ps( 0.8f ) ) ) );
  vpwsqrtinv = _mm_rsqrt_ps( vpow );
  vpwsqrt = _mm_mul_ps( vpow, vpwsqrtinv );
  return _mm_mul_ps( _mm_add_ps( _mm_mul_ps( vx2, vpwsqrt ), _mm_mul_ps( _mm_mul_ps( _mm_mul_ps( vx2, vx ), vpwsqrtinv ), _mm_rsqrt_ps( vpwsqrt ) ) ), _mm_set1_ps( 0.51011878327f ) );
}

static inline CC_ALWAYSINLINE __m128 simd4f_pow5d12_inline_ps( __m128 vx )
{
  __m128 vpow;
  vpow = _mm_castsi128_ps( _mm_cvtps_epi32( _mm_mul_ps( _mm_cvtepi32_ps( _mm_castps_si128( _mm_mul_ps( vx, _mm_set1_ps( 6521909350804488192.0f ) ) ) ), _mm_set1_ps( 0.666666666666f ) ) ) );
  vx = _mm_mul_ps( _mm_add_ps( _mm_mul_ps( vx, vpow ), _mm_mul_ps( _mm_mul_ps( vx, vx ), _mm_rsqrt_ps( vpow ) ) ), _mm_set1_ps( 0.5290553722f ) );
#if 0
  vx = _mm_mul_ps( vx, _mm_rsqrt_ps( vx ) );
  vx = _mm_mul_ps( vx, _mm_rsqrt_ps( vx ) );
#else
  vx = _mm_sqrt_ps( vx );
  vx = _mm_sqrt_ps( vx );
#endif
  return vx;
}

#endif


////


#if CPU_SSE_SUPPORT

static inline void simdPrintDebugSSE4f( char *str, __m128 v )
{
  float CPU_ALIGN16 store[4];
  _mm_store_ps( (void *)store, v );
  printf( "%s %f %f %f %f\n", str, (double)store[0], (double)store[1], (double)store[2], (double)store[3] );
  return;
}

static inline void simdPrintDebugSSE2d( char *str, __m128d v )
{
  double CPU_ALIGN16 store[2];
  _mm_store_pd( (void *)store, v );
  printf( "%s %f %f\n", str, store[0], store[1] );
  return;
}

static inline void simdPrintDebugSSE16u8( char *str, __m128i v )
{
  uint8_t CPU_ALIGN16 store[16];
  _mm_store_si128( (void *)store, v );
  printf( "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", str, store[0], store[1], store[2], store[3], store[4], store[5], store[6], store[7], store[8], store[9], store[10], store[11], store[12], store[13], store[14], store[15] );
  return;
}

static inline void simdPrintDebugSSE8u16( char *str, __m128i v )
{
  uint16_t CPU_ALIGN16 store[8];
  _mm_store_si128( (void *)store, v );
  printf( "%s %d %d %d %d %d %d %d %d\n", str, store[0], store[1], store[2], store[3], store[4], store[5], store[6], store[7] );
  return;
}

static inline void simdPrintDebugSSE4u32( char *str, __m128i v )
{
  uint32_t CPU_ALIGN16 store[4];
  _mm_store_si128( (void *)store, v );
  printf( "%s %d %d %d %d\n", str, store[0], store[1], store[2], store[3] );
  return;
}

static inline void simdPrintDebugSSE2u64( char *str, __m128i v )
{
  uint64_t CPU_ALIGN16 store[2];
  _mm_store_si128( (void *)store, v );
  printf( "%s %lld %lld\n", str, (long long)store[0], (long long)store[1] );
  return;
}

#endif


////


#endif

