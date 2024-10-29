/* SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Copyright:
 *   2023      Michael R. Crusoe <crusoe@debian.org>
 */

#if !defined(SIMDE_X86_AVX512_GATHER_H)
#define SIMDE_X86_AVX512_GATHER_H

#include "types.h"
#include "../avx2.h"
#include "extract.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde__m512
simde_mm512_i32gather_ps(simde__m512i vindex, const void* base_addr, const int32_t scale)
    SIMDE_REQUIRE_CONSTANT(scale)
    HEDLEY_REQUIRE_MSG((scale && scale <= 8 && !(scale & (scale - 1))), "`scale' must be a power of two less than or equal to 8") {
  simde__m512i_private vindex_ = simde__m512i_to_private(vindex);
  simde__m512_private r_ = simde__m512_to_private(simde_mm512_setzero_ps());
  const uint8_t* addr = HEDLEY_REINTERPRET_CAST(const uint8_t*, base_addr);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(vindex_.i32) / sizeof(vindex_.i32[0])) ; i++) {
    const uint8_t* src = addr + (HEDLEY_STATIC_CAST(size_t , vindex_.i32[i]) * HEDLEY_STATIC_CAST(size_t , scale));
    simde_float32 dst;
    simde_memcpy(&dst, src, sizeof(dst));
    r_.f32[i] = dst;
  }

  return simde__m512_from_private(r_);
}
#if defined(SIMDE_X86_AVX512F_NATIVE) && (!defined(__clang__) || SIMDE_DETECT_CLANG_VERSION_CHECK(10,0,0))
  #define simde_mm512_i32gather_ps(vindex, base_addr, scale) _mm512_i32gather_ps((vindex), (base_addr), (scale))
#elif defined(SIMDE_X86_AVX2_NATIVE) && defined(SIMDE_STATEMENT_EXPR_)
  #define simde_mm512_i32gather_ps(vindex, base_addr, scale) SIMDE_STATEMENT_EXPR_(({\
    simde__m512_private simde_mm512_i32gather_ps_r_; \
    simde__m512i_private simde_mm512_i32gather_ps_vindex_ = simde__m512i_to_private((vindex)); \
    simde_mm512_i32gather_ps_r_.m256[0] = _mm256_i32gather_ps( \
      HEDLEY_STATIC_CAST(float const*, (base_addr)), simde_mm512_i32gather_ps_vindex_.m256i[0], (scale)); \
    simde_mm512_i32gather_ps_r_.m256[1] = _mm256_i32gather_ps( \
      HEDLEY_STATIC_CAST(float const*, (base_addr)), simde_mm512_i32gather_ps_vindex_.m256i[1], (scale)); \
    simde__m512_from_private(simde_mm512_i32gather_ps_r_); \
  }))
#elif defined(SIMDE_X86_AVX2_NATIVE) && !defined(SIMDE_STATEMENT_EXPR_)
  #define simde_mm512_i32gather_ps(vindex, base_addr, scale) \
    simde_x_mm512_set_m256( \
      _mm256_i32gather_ps(HEDLEY_STATIC_CAST(float const*, (base_addr)), \
        simde_mm512_extracti32x8_epi32((vindex), 1), (scale)), \
      _mm256_i32gather_ps(HEDLEY_STATIC_CAST(float const*, (base_addr)), \
        simde_mm512_extracti32x8_epi32((vindex), 0), (scale)) )
#endif
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
  #undef _mm512_i32gather_ps
  #define _mm512_i32gather_ps(vindex, base_addr, scale) simde_mm512_i32gather_ps((vindex), (base_addr), (scale))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde__m256i
simde_mm512_i64gather_epi32(simde__m512i vindex, const void* base_addr, const int32_t scale)
    SIMDE_REQUIRE_CONSTANT(scale)
    HEDLEY_REQUIRE_MSG((scale && scale <= 8 && !(scale & (scale - 1))), "`scale' must be a power of two less than or equal to 8") {
  simde__m512i_private vindex_;
  simde__m256i_private r_;
  vindex_ = simde__m512i_to_private(vindex);
  r_ = simde__m256i_to_private(simde_mm256_setzero_si256());
  const uint8_t* addr = HEDLEY_REINTERPRET_CAST(const uint8_t*, base_addr);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(vindex_.i64) / sizeof(vindex_.i64[0])) ; i++) {
    const uint8_t* src = addr + (HEDLEY_STATIC_CAST(size_t , vindex_.i64[i]) * HEDLEY_STATIC_CAST(size_t , scale));
    int32_t dst;
    simde_memcpy(&dst, src, sizeof(dst));
    r_.i32[i] = dst;
  }

  return simde__m256i_from_private(r_);
}
#if defined(SIMDE_X86_AVX512F_NATIVE)
  #define simde_mm512_i64gather_epi32(vindex, base_addr, scale) _mm512_i64gather_epi32((vindex), (base_addr), (scale))
#elif defined(SIMDE_X86_AVX2_NATIVE) && defined(SIMDE_STATEMENT_EXPR_)
  #define simde_mm512_i64gather_epi32(vindex, base_addr, scale) SIMDE_STATEMENT_EXPR_(({\
    simde__m256i_private simde_mm512_i64gather_epi32_r_; \
    simde__m512i_private simde_mm512_i64gather_epi32_vindex_ = simde__m512i_to_private((vindex)); \
    simde_mm512_i64gather_epi32_r_.m128i[0] = _mm256_i64gather_epi32( \
      HEDLEY_STATIC_CAST(int const*, (base_addr)), simde_mm512_i64gather_epi32_vindex_.m256i[0], (scale)); \
    simde_mm512_i64gather_epi32_r_.m128i[1] = _mm256_i64gather_epi32( \
      HEDLEY_STATIC_CAST(int const*, (base_addr)), simde_mm512_i64gather_epi32_vindex_.m256i[1], (scale)); \
    simde__m256i_from_private(simde_mm512_i64gather_epi32_r_); \
  }))
#elif defined(SIMDE_X86_AVX2_NATIVE) && !defined(SIMDE_STATEMENT_EXPR_)
  #define simde_mm512_i64gather_epi32(vindex, base_addr, scale) \
    _mm256_insertf128_si256( \
      _mm256_castsi128_si256( \
        _mm256_i64gather_epi32(HEDLEY_STATIC_CAST(int const*, (base_addr)), \
          simde_mm512_extracti64x4_epi64((vindex), 0), (scale))), \
      _mm256_i64gather_epi32(HEDLEY_STATIC_CAST(int const*, (base_addr)), \
        simde_mm512_extracti64x4_epi64((vindex), 1), (scale)), \
      1)
#endif
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
  #undef _mm512_i64gather_epi32
  #define _mm512_i64gather_epi32(vindex, base_addr, scale) simde_mm512_i64gather_epi32((vindex), (base_addr), (scale))
#endif

#if defined(SIMDE_X86_AVX512F_NATIVE)
  #define simde_mm512_mask_i64gather_epi32(src, k, vindex, base_addr, scale) _mm512_mask_i64gather_epi32((src), (k), (vindex), (base_addr), (scale))
#else
  #define simde_mm512_mask_i64gather_epi32(src, k, vindex, base_addr, scale) simde_mm256_mask_mov_epi32(src, k, simde_mm512_i64gather_epi32((vindex), (base_addr), (scale)))
#endif
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
  #undef _mm512_mask_i64gather_epi32
  #define _mm512_mask_i64gather_epi32(src, k, vindex, base_addr, scale) simde_mm512_mask_i64gather_epi32((src), (k), (vindex), (base_addr), (scale))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde__m512i
simde_mm512_i64gather_epi64(simde__m512i vindex, const void* base_addr, const int32_t scale)
    SIMDE_REQUIRE_CONSTANT(scale)
    HEDLEY_REQUIRE_MSG((scale && scale <= 8 && !(scale & (scale - 1))), "`scale' must be a power of two less than or equal to 8") {
  simde__m512i_private
    vindex_ = simde__m512i_to_private(vindex),
    r_ = simde__m512i_to_private(simde_mm512_setzero_si512());
  const uint8_t* addr = HEDLEY_REINTERPRET_CAST(const uint8_t*, base_addr);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(vindex_.i64) / sizeof(vindex_.i64[0])) ; i++) {
    const uint8_t* src = addr + (HEDLEY_STATIC_CAST(size_t , vindex_.i64[i]) * HEDLEY_STATIC_CAST(size_t , scale));
    int64_t dst;
    simde_memcpy(&dst, src, sizeof(dst));
    r_.i64[i] = dst;
  }

  return simde__m512i_from_private(r_);
}
#if defined(SIMDE_X86_AVX512F_NATIVE)
  #define simde_mm512_i64gather_epi64(vindex, base_addr, scale) _mm512_i64gather_epi64((vindex), (base_addr), (scale))
#elif defined(SIMDE_X86_AVX2_NATIVE) && defined(SIMDE_STATEMENT_EXPR_)
  #define simde_mm512_i64gather_epi64(vindex, base_addr, scale) SIMDE_STATEMENT_EXPR_(({\
    simde__m512i_private simde_mm512_i64gather_epi64_r_, \
      simde_mm512_i64gather_epi64_vindex_ = simde__m512i_to_private((vindex)); \
    simde_mm512_i64gather_epi64_r_.m256i[0] = _mm256_i64gather_epi64( \
      HEDLEY_STATIC_CAST(long long const*, (base_addr)), simde_mm512_i64gather_epi64_vindex_.m256i[0], (scale)); \
    simde_mm512_i64gather_epi64_r_.m256i[1] = _mm256_i64gather_epi64( \
      HEDLEY_STATIC_CAST(long long const*, (base_addr)), simde_mm512_i64gather_epi64_vindex_.m256i[1], (scale)); \
    simde__m512i_from_private(simde_mm512_i64gather_epi64_r_); \
  }))
#elif defined(SIMDE_X86_AVX2_NATIVE) && !defined(SIMDE_STATEMENT_EXPR_)
  #define simde_mm512_i64gather_epi64(vindex, base_addr, scale) \
    simde_x_mm512_set_m256i( \
      _mm256_i64gather_epi64(HEDLEY_STATIC_CAST(long long const*, (base_addr)), \
        simde_mm512_extracti32x8_epi32((vindex), 1), (scale)), \
      _mm256_i64gather_epi64(HEDLEY_STATIC_CAST(long long const*, (base_addr)), \
        simde_mm512_extracti32x8_epi32((vindex), 0), (scale)) )
#endif
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
  #undef _mm512_i64gather_epi64
  #define _mm512_i64gather_epi64(vindex, base_addr, scale) simde_mm512_i64gather_epi64(vindex, (base_addr), (scale))
#endif

#if defined(SIMDE_X86_AVX512F_NATIVE)
  #define simde_mm512_mask_i64gather_epi64(src, k, vindex, base_addr, scale) _mm512_mask_i64gather_epi64((src), (k), (vindex), (base_addr), (scale))
#else
  #define simde_mm512_mask_i64gather_epi64(src, k, vindex, base_addr, scale) simde_mm512_mask_mov_epi64((src), (k), simde_mm512_i64gather_epi64((vindex), (base_addr), (scale)))
#endif
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
  #undef _mm512_mask_i64gather_epi64
  #define _mm512_mask_i64gather_epi64(src, k, vindex, base_addr, scale) simde_mm512_mask_i64gather_epi64((src), (k), (vindex), (base_addr), (scale))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde__m512d
simde_mm512_i64gather_pd(simde__m512i vindex, const void* base_addr, const int32_t scale)
    SIMDE_REQUIRE_CONSTANT(scale)
    HEDLEY_REQUIRE_MSG((scale && scale <= 8 && !(scale & (scale - 1))), "`scale' must be a power of two less than or equal to 8") {
  simde__m512i_private vindex_;
  simde__m512d_private r_;
  vindex_ = simde__m512i_to_private(vindex);
  r_ = simde__m512d_to_private(simde_mm512_setzero_pd());
  const uint8_t* addr = HEDLEY_REINTERPRET_CAST(const uint8_t*, base_addr);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(vindex_.i64) / sizeof(vindex_.i64[0])) ; i++) {
    const uint8_t* src = addr + (HEDLEY_STATIC_CAST(size_t , vindex_.i64[i]) * HEDLEY_STATIC_CAST(size_t , scale));
    simde_float64 dst;
    simde_memcpy(&dst, src, sizeof(dst));
    r_.f64[i] = dst;
  }

  return simde__m512d_from_private(r_);
}
#if defined(SIMDE_X86_AVX512F_NATIVE)
  #define simde_mm512_i64gather_pd(vindex, base_addr, scale) _mm512_i64gather_pd((vindex), (base_addr), (scale))
#elif defined(SIMDE_X86_AVX2_NATIVE) && defined(SIMDE_STATEMENT_EXPR_)
  #define simde_mm512_i64gather_pd(vindex, base_addr, scale) SIMDE_STATEMENT_EXPR_(({\
    simde__m512d_private simde_mm512_i64gather_pd_r_; \
    simde__m512i_private simde_mm512_i64gather_pd_vindex_ = simde__m512i_to_private((vindex)); \
    simde_mm512_i64gather_pd_r_.m256d[0] = _mm256_i64gather_pd( \
      HEDLEY_STATIC_CAST(double const*, (base_addr)), simde_mm512_i64gather_pd_vindex_.m256i[0], (scale)); \
    simde_mm512_i64gather_pd_r_.m256d[1] = _mm256_i64gather_pd( \
      HEDLEY_STATIC_CAST(double const*, (base_addr)), simde_mm512_i64gather_pd_vindex_.m256i[1], (scale)); \
    simde__m512d_from_private(simde_mm512_i64gather_pd_r_); \
  }))
#elif defined(SIMDE_X86_AVX2_NATIVE) && !defined(SIMDE_STATEMENT_EXPR_)
  #define simde_mm512_i64gather_pd(vindex, base_addr, scale) \
    simde_x_mm512_set_m256d( \
      _mm256_i64gather_pd(HEDLEY_STATIC_CAST(double const*, (base_addr)), \
        simde_mm512_extracti64x4_epi64((vindex), 1), (scale)), \
      _mm256_i64gather_pd(HEDLEY_STATIC_CAST(double const*, (base_addr)), \
        simde_mm512_extracti64x4_epi64((vindex), 0), (scale)) )
#endif
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
  #undef _mm512_i64gather_pd
  #define _mm512_i64gather_pd(vindex, base_addr, scale) simde_mm512_i64gather_pd((vindex), (base_addr), (scale))
#endif

#if defined(SIMDE_X86_AVX512F_NATIVE)
  #define simde_mm512_mask_i64gather_pd(src, k, vindex, base_addr, scale) _mm512_mask_i64gather_pd((src), (k), (vindex), (base_addr), (scale))
#else
  #define simde_mm512_mask_i64gather_pd(src, k, vindex, base_addr, scale) simde_mm512_mask_mov_pd((src), (k), simde_mm512_i64gather_pd((vindex), (base_addr), (scale)))
#endif
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
  #undef _mm512_mask_i64gather_pd
  #define _mm512_mask_i64gather_pd(src, k, vindex, base_addr, scale) simde_mm512_mask_i64gather_pd((src), (k), (vindex), (base_addr), (scale))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde__m256
simde_mm512_i64gather_ps(simde__m512i vindex, const void* base_addr, const int32_t scale)
    SIMDE_REQUIRE_CONSTANT(scale)
    HEDLEY_REQUIRE_MSG((scale && scale <= 8 && !(scale & (scale - 1))), "`scale' must be a power of two less than or equal to 8") {
  simde__m512i_private vindex_;
  simde__m256_private r_;
  vindex_ = simde__m512i_to_private(vindex);
  r_ = simde__m256_to_private(simde_mm256_setzero_ps());
  const uint8_t* addr = HEDLEY_REINTERPRET_CAST(const uint8_t*, base_addr);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(vindex_.i64) / sizeof(vindex_.i64[0])) ; i++) {
    const uint8_t* src = addr + (HEDLEY_STATIC_CAST(size_t , vindex_.i64[i]) * HEDLEY_STATIC_CAST(size_t , scale));
    simde_float32 dst;
    simde_memcpy(&dst, src, sizeof(dst));
    r_.f32[i] = dst;
  }

  return simde__m256_from_private(r_);
}
#if defined(SIMDE_X86_AVX512F_NATIVE)
  #define simde_mm512_i64gather_ps(vindex, base_addr, scale) _mm512_i64gather_ps((vindex), (base_addr), (scale))
#elif defined(SIMDE_X86_AVX2_NATIVE) && defined(SIMDE_STATEMENT_EXPR_)
  #define simde_mm512_i64gather_ps(vindex, base_addr, scale) SIMDE_STATEMENT_EXPR_(({\
    simde__m256_private simde_mm512_i64gather_ps_r_; \
    simde__m512i_private simde_mm512_i64gather_ps_vindex_ = simde__m512i_to_private((vindex)); \
    simde_mm512_i64gather_ps_r_.m128[0] = _mm256_i64gather_ps( \
      HEDLEY_STATIC_CAST(float const*, (base_addr)), simde_mm512_i64gather_ps_vindex_.m256i[0], (scale)); \
    simde_mm512_i64gather_ps_r_.m128[1] = _mm256_i64gather_ps( \
      HEDLEY_STATIC_CAST(float const*, (base_addr)), simde_mm512_i64gather_ps_vindex_.m256i[1], (scale)); \
    simde__m256_from_private(simde_mm512_i64gather_ps_r_); \
  }))
#elif defined(SIMDE_X86_AVX2_NATIVE) && !defined(SIMDE_STATEMENT_EXPR_)
  #define simde_mm512_i64gather_ps(vindex, base_addr, scale) \
    _mm256_insertf128_ps( \
      _mm256_castps128_ps256( \
        _mm256_i64gather_ps(HEDLEY_STATIC_CAST(float const*, (base_addr)), \
          simde_mm512_extracti64x4_epi64((vindex), 0), (scale))), \
      _mm256_i64gather_ps(HEDLEY_STATIC_CAST(float const*, (base_addr)), \
        simde_mm512_extracti64x4_epi64((vindex), 1), (scale)), \
      1)
#endif
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
  #undef _mm512_i64gather_ps
  #define _mm512_i64gather_ps(vindex, base_addr, scale) simde_mm512_i64gather_ps((vindex), (base_addr), (scale))
#endif

#if defined(SIMDE_X86_AVX512F_NATIVE)
  #define simde_mm512_mask_i64gather_ps(src, k, vindex, base_addr, scale) _mm512_mask_i64gather_ps((src), (k), (vindex), (base_addr), (scale))
#else
  #define simde_mm512_mask_i64gather_ps(src, k, vindex, base_addr, scale) simde_mm256_mask_mov_ps((src), (k), simde_mm512_i64gather_ps((vindex), (base_addr), (scale)))
#endif
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
  #undef _mm512_mask_i64gather_ps
  #define _mm512_mask_i64gather_ps(src, k, vindex, base_addr, scale) simde_mm512_mask_i64gather_ps((src), (k), (vindex), (base_addr), (scale))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_X86_AVX512_GATHER_H) */
