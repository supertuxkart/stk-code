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

#if !defined(SIMDE_X86_AVX512_REDUCE_H)
#define SIMDE_X86_AVX512_REDUCE_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#if defined(__clang__) && SIMDE_FLOAT16_API == SIMDE_FLOAT16_API_FP16
SIMDE_DIAGNOSTIC_DISABLE_DOUBLE_PROMOTION_
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16
simde_mm512_reduce_max_ph(simde__m512h a) {
  #if defined(SIMDE_X86_AVX512FP16_NATIVE)
    return _mm512_reduce_max_ph(a);
  #else
    simde__m512h_private a_;
    simde_float16 r;
    a_ = simde__m512h_to_private(a);

    r = SIMDE_NINFINITYHF;
    #if defined(SIMDE_FLOAT16_VECTOR)
    SIMDE_VECTORIZE_REDUCTION(max:r)
    #endif
    for (size_t i = 0 ; i < (sizeof(a_.f16) / sizeof(a_.f16[0])) ; i++) {
      r = simde_float16_to_float32(a_.f16[i]) > simde_float16_to_float32(r) ? a_.f16[i] : r;
    }
    return r;
  #endif
}
#if defined(SIMDE_X86_AVX512FP16_ENABLE_NATIVE_ALIASES)
#  define _mm512_reduce_max_ph(a) simde_mm512_reduce_max_ph((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16
simde_mm512_reduce_min_ph(simde__m512h a) {
  #if defined(SIMDE_X86_AVX512FP16_NATIVE)
    return _mm512_reduce_min_ph(a);
  #else
    simde__m512h_private a_;
    simde_float16 r;
    a_ = simde__m512h_to_private(a);

    r = SIMDE_INFINITYHF;
    #if defined(SIMDE_FLOAT16_VECTOR)
    SIMDE_VECTORIZE_REDUCTION(min:r)
    #endif
    for (size_t i = 0 ; i < (sizeof(a_.f16) / sizeof(a_.f16[0])) ; i++) {
      r = simde_float16_to_float32(a_.f16[i]) < simde_float16_to_float32(r) ? a_.f16[i] : r;
    }
    return r;
  #endif
}
#if defined(SIMDE_X86_AVX512FP16_ENABLE_NATIVE_ALIASES)
#  define _mm512_reduce_min_ph(a) simde_mm512_reduce_min_ph((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
int32_t
simde_mm512_reduce_max_epi32(simde__m512i a) {
  #if defined(SIMDE_X86_AVX512F_NATIVE)
    return _mm512_reduce_max_epi32(a);
  #else
    simde__m512i_private a_;
    int32_t r;
    a_ = simde__m512i_to_private(a);

    r = -INT32_MAX;
    SIMDE_VECTORIZE_REDUCTION(max:r)
    for (size_t i = 0 ; i < (sizeof(a_.i32) / sizeof(a_.i32[0])) ; i++) {
      r = a_.i32[i] > r ? a_.i32[i] : r;
    }
    return r;
  #endif
}
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
#  define _mm512_reduce_max_epi32(a) simde_mm512_reduce_max_epi32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
int64_t
simde_mm512_reduce_max_epi64(simde__m512i a) {
  #if defined(SIMDE_X86_AVX512F_NATIVE)
    return _mm512_reduce_max_epi64(a);
  #else
    simde__m512i_private a_;
    int64_t r;
    a_ = simde__m512i_to_private(a);

    r = -INT64_MAX;
    SIMDE_VECTORIZE_REDUCTION(max:r)
    for (size_t i = 0 ; i < (sizeof(a_.i64) / sizeof(a_.i64[0])) ; i++) {
      r = a_.i64[i] > r ? a_.i64[i] : r;
    }
    return r;
  #endif
}
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
#  define _mm512_reduce_max_epi64(a) simde_mm512_reduce_max_epi64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde_mm512_reduce_max_epu32(simde__m512i a) {
  #if defined(SIMDE_X86_AVX512F_NATIVE)
    return _mm512_reduce_max_epu32(a);
  #else
    simde__m512i_private a_;
    uint32_t r;
    a_ = simde__m512i_to_private(a);

    r = 0;
    SIMDE_VECTORIZE_REDUCTION(max:r)
    for (size_t i = 0 ; i < (sizeof(a_.u32) / sizeof(a_.u32[0])) ; i++) {
      r = a_.u32[i] > r ? a_.u32[i] : r;
    }
    return r;
  #endif
}
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
#  define _mm512_reduce_max_epu32(a) simde_mm512_reduce_max_epu32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint64_t
simde_mm512_reduce_max_epu64(simde__m512i a) {
  #if defined(SIMDE_X86_AVX512F_NATIVE)
    return _mm512_reduce_max_epu64(a);
  #else
    simde__m512i_private a_;
    uint64_t r;
    a_ = simde__m512i_to_private(a);

    r = 0;
    SIMDE_VECTORIZE_REDUCTION(max:r)
    for (size_t i = 0 ; i < (sizeof(a_.u64) / sizeof(a_.u64[0])) ; i++) {
      r = a_.u64[i] > r ? a_.u64[i] : r;
    }
    return r;
  #endif
}
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
#  define _mm512_reduce_max_epu64(a) simde_mm512_reduce_max_epu64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64
simde_mm512_reduce_max_pd(simde__m512d a) {
  #if defined(SIMDE_X86_AVX512F_NATIVE)
    return _mm512_reduce_max_pd(a);
  #else
    simde__m512d_private a_;
    simde_float64 r;
    a_ = simde__m512d_to_private(a);

    r = -SIMDE_MATH_INFINITY;
    SIMDE_VECTORIZE_REDUCTION(max:r)
    for (size_t i = 0 ; i < (sizeof(a_.f64) / sizeof(a_.f64[0])) ; i++) {
      r = a_.f64[i] > r ? a_.f64[i] : r;
    }
    return r;
  #endif
}
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
#  define _mm512_reduce_max_pd(a) simde_mm512_reduce_max_pd((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32
simde_mm512_reduce_max_ps(simde__m512 a) {
  #if defined(SIMDE_X86_AVX512F_NATIVE)
    return _mm512_reduce_max_ps(a);
  #else
    simde__m512_private a_;
    simde_float32 r;
    a_ = simde__m512_to_private(a);

    r = -SIMDE_MATH_INFINITYF;
    SIMDE_VECTORIZE_REDUCTION(max:r)
    for (size_t i = 0 ; i < (sizeof(a_.f32) / sizeof(a_.f32[0])) ; i++) {
      r = a_.f32[i] > r ? a_.f32[i] : r;
    }
    return r;
  #endif
}
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
#  define _mm512_reduce_max_ps(a) simde_mm512_reduce_max_ps((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
int32_t
simde_mm512_reduce_min_epi32(simde__m512i a) {
  #if defined(SIMDE_X86_AVX512F_NATIVE)
    return _mm512_reduce_min_epi32(a);
  #else
    simde__m512i_private a_;
    int32_t r;
    a_ = simde__m512i_to_private(a);

    r = INT32_MAX;
    SIMDE_VECTORIZE_REDUCTION(min:r)
    for (size_t i = 0 ; i < (sizeof(a_.i32) / sizeof(a_.i32[0])) ; i++) {
      r = a_.i32[i] < r ? a_.i32[i] : r;
    }
    return r;
  #endif
}
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
#  define _mm512_reduce_min_epi32(a) simde_mm512_reduce_min_epi32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
int64_t
simde_mm512_reduce_min_epi64(simde__m512i a) {
  #if defined(SIMDE_X86_AVX512F_NATIVE)
    return _mm512_reduce_min_epi64(a);
  #else
    simde__m512i_private a_;
    int64_t r;
    a_ = simde__m512i_to_private(a);

    r = INT64_MAX;
    SIMDE_VECTORIZE_REDUCTION(min:r)
    for (size_t i = 0 ; i < (sizeof(a_.i64) / sizeof(a_.i64[0])) ; i++) {
      r = a_.i64[i] < r ? a_.i64[i] : r;
    }
    return r;
  #endif
}
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
#  define _mm512_reduce_min_epi64(a) simde_mm512_reduce_min_epi64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde_mm512_reduce_min_epu32(simde__m512i a) {
  #if defined(SIMDE_X86_AVX512F_NATIVE)
    return _mm512_reduce_min_epu32(a);
  #else
    simde__m512i_private a_;
    uint32_t r;
    a_ = simde__m512i_to_private(a);

    r = UINT32_MAX;
    SIMDE_VECTORIZE_REDUCTION(min:r)
    for (size_t i = 0 ; i < (sizeof(a_.u32) / sizeof(a_.u32[0])) ; i++) {
      r = a_.u32[i] < r ? a_.u32[i] : r;
    }
    return r;
  #endif
}
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
#  define _mm512_reduce_min_epu32(a) simde_mm512_reduce_min_epu32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint64_t
simde_mm512_reduce_min_epu64(simde__m512i a) {
  #if defined(SIMDE_X86_AVX512F_NATIVE)
    return _mm512_reduce_min_epu64(a);
  #else
    simde__m512i_private a_;
    uint64_t r;
    a_ = simde__m512i_to_private(a);

    r = UINT64_MAX;
    SIMDE_VECTORIZE_REDUCTION(min:r)
    for (size_t i = 0 ; i < (sizeof(a_.u64) / sizeof(a_.u64[0])) ; i++) {
      r = a_.u64[i] < r ? a_.u64[i] : r;
    }
    return r;
  #endif
}
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
#  define _mm512_reduce_min_epu64(a) simde_mm512_reduce_min_epu64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64
simde_mm512_reduce_min_pd(simde__m512d a) {
  #if defined(SIMDE_X86_AVX512F_NATIVE)
    return _mm512_reduce_min_pd(a);
  #else
    simde__m512d_private a_;
    simde_float64 r;
    a_ = simde__m512d_to_private(a);

    r = SIMDE_MATH_INFINITY;
    SIMDE_VECTORIZE_REDUCTION(min:r)
    for (size_t i = 0 ; i < (sizeof(a_.f64) / sizeof(a_.f64[0])) ; i++) {
      r = a_.f64[i] < r ? a_.f64[i] : r;
    }
    return r;
  #endif
}
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
#  define _mm512_reduce_min_pd(a) simde_mm512_reduce_min_pd((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32
simde_mm512_reduce_min_ps(simde__m512 a) {
  #if defined(SIMDE_X86_AVX512F_NATIVE)
    return _mm512_reduce_min_ps(a);
  #else
    simde__m512_private a_;
    simde_float32 r;
    a_ = simde__m512_to_private(a);

    r = SIMDE_MATH_INFINITYF;
    SIMDE_VECTORIZE_REDUCTION(min:r)
    for (size_t i = 0 ; i < (sizeof(a_.f32) / sizeof(a_.f32[0])) ; i++) {
      r = a_.f32[i] < r ? a_.f32[i] : r;
    }
    return r;
  #endif
}
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
#  define _mm512_reduce_min_ps(a) simde_mm512_reduce_min_ps((a))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_X86_AVX512_REDUCE_H) */
