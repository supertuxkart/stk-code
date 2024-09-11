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
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 */

#if !defined(SIMDE_ARM_NEON_CVTN_H)
#define SIMDE_ARM_NEON_CVTN_H

#include "types.h"
#include "cvt.h"
#include "calt.h"
#include "cagt.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vcvtnq_s32_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtnq_s32_f32(a);
  #else
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_int32x4_private r_;

    #if defined(SIMDE_X86_SSE2_NATIVE)
      if (HEDLEY_UNLIKELY(_MM_GET_ROUNDING_MODE() != _MM_ROUND_NEAREST)) {
        unsigned int rounding_mode = _MM_GET_ROUNDING_MODE();
        _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
        r_.m128i = _mm_cvtps_epi32(a_.m128);
        _MM_SET_ROUNDING_MODE(rounding_mode);
      } else {
        r_.m128i = _mm_cvtps_epi32(a_.m128);
      }
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = HEDLEY_STATIC_CAST(int32_t, simde_math_roundevenf(a_.values[i]));
      }
    #endif

    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtnq_s32_f32
  #define vcvtnq_s32_f32(a) simde_vcvtnq_s32_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vcvtnq_s64_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtnq_s64_f64(a);
  #else
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_int64x2_private r_;

    #if defined(SIMDE_X86_AVX512DQ_NATIVE) && defined(SIMDE_X86_AVX512VL_NATIVE)
      if (HEDLEY_UNLIKELY(_MM_GET_ROUNDING_MODE() != _MM_ROUND_NEAREST)) {
        unsigned int rounding_mode = _MM_GET_ROUNDING_MODE();
        _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
        r_.m128i = _mm_cvtpd_epi64(a_.m128d);
        _MM_SET_ROUNDING_MODE(rounding_mode);
      } else {
        r_.m128i = _mm_cvtpd_epi64(a_.m128d);
      }
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = HEDLEY_STATIC_CAST(int64_t, simde_math_roundeven(a_.values[i]));
      }
    #endif

    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtnq_s64_f64
  #define vcvtnq_s64_f64(a) simde_vcvtnq_s64_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
int64_t
simde_vcvtnh_s64_f16(simde_float16_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcvtnh_s64_f16(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(int64_t, simde_math_roundevenf(simde_float16_to_float32(a)));
  #else
    simde_float32 a_ = simde_float16_to_float32(a);
    if (HEDLEY_UNLIKELY(a_ < HEDLEY_STATIC_CAST(simde_float32, INT64_MIN))) {
      return INT64_MIN;
    } else if (HEDLEY_UNLIKELY(a_ > HEDLEY_STATIC_CAST(simde_float32, INT64_MAX))) {
      return INT64_MAX;
    } else if (simde_math_isnanf(a_)) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(int64_t, simde_math_roundevenf(a_));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtnh_s64_f16
  #define vcvtnh_s64_f16(a) simde_vcvtnh_s64_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
int32_t
simde_vcvtnh_s32_f16(simde_float16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcvtnh_s32_f16(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(int32_t, simde_math_roundevenf(simde_float16_to_float32(a)));
  #else
    simde_float32 a_ = simde_float16_to_float32(a);
    if (HEDLEY_UNLIKELY(a_ < HEDLEY_STATIC_CAST(simde_float32, INT32_MIN))) {
      return INT32_MIN;
    } else if (HEDLEY_UNLIKELY(a_ > HEDLEY_STATIC_CAST(simde_float32, INT32_MAX))) {
      return INT32_MAX;
    } else if (simde_math_isnanf(a_)) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(int32_t, simde_math_roundevenf(a_));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtnh_s32_f16
  #define vcvtnh_s32_f16(a) simde_vcvtnh_s32_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint64_t
simde_vcvtnh_u64_f16(simde_float16_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcvtnh_u64_f16(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(uint64_t, simde_math_roundevenf(simde_float16_to_float32(a)));
  #else
    simde_float32 a_ = simde_float16_to_float32(a);
    if (HEDLEY_UNLIKELY(a_ < HEDLEY_STATIC_CAST(simde_float32, 0))) {
      return 0;
    } else if (HEDLEY_UNLIKELY(a_ > HEDLEY_STATIC_CAST(simde_float32, UINT64_MAX))) {
      return UINT64_MAX;
    } else if (simde_math_isnanf(a_)) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(uint64_t, simde_math_roundevenf(a_));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtnh_u64_f16
  #define vcvtnh_u64_f16(a) simde_vcvtnh_u64_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde_vcvtnh_u32_f16(simde_float16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcvtnh_u32_f16(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(uint32_t, simde_math_roundevenf(simde_float16_to_float32(a)));
  #else
    simde_float32 a_ = simde_float16_to_float32(a);
    if (HEDLEY_UNLIKELY(a_ < HEDLEY_STATIC_CAST(simde_float32, 0))) {
      return 0;
    } else if (HEDLEY_UNLIKELY(a_ > HEDLEY_STATIC_CAST(simde_float32, UINT32_MAX))) {
      return UINT32_MAX;
    } else if (simde_math_isnanf(a_)) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(uint32_t, simde_math_roundevenf(a_));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtnh_u32_f16
  #define vcvtnh_u32_f16(a) simde_vcvtnh_u32_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint16_t
simde_vcvtnh_u16_f16(simde_float16_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcvtnh_u16_f16(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(uint16_t, simde_math_roundevenf(simde_float16_to_float32(a)));
  #else
    simde_float32 a_ = simde_float16_to_float32(a);
    if (HEDLEY_UNLIKELY(a_ < HEDLEY_STATIC_CAST(simde_float32, 0))) {
      return 0;
    } else if (HEDLEY_UNLIKELY(a_ > HEDLEY_STATIC_CAST(simde_float32, UINT16_MAX))) {
      return UINT16_MAX;
    } else if (simde_math_isnanf(a_)) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(uint16_t, simde_math_roundevenf(a_));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtnh_u16_f16
  #define vcvtnh_u16_f16(a) simde_vcvtnh_u16_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
int32_t
simde_vcvtns_s32_f32(simde_float32 a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtns_s32_f32(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(int32_t, simde_math_roundevenf(a));
  #else
    if (HEDLEY_UNLIKELY(a < HEDLEY_STATIC_CAST(simde_float32, INT32_MIN))) {
      return INT32_MIN;
    } else if (HEDLEY_UNLIKELY(a > HEDLEY_STATIC_CAST(simde_float32, INT32_MAX))) {
      return INT32_MAX;
    } else if (HEDLEY_UNLIKELY(simde_math_isnanf(a))) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(int32_t, simde_math_roundevenf(a));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtns_s32_f32
  #define vcvtns_s32_f32(a) simde_vcvtns_s32_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde_vcvtns_u32_f32(simde_float32 a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtns_u32_f32(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(uint32_t, simde_math_roundevenf(a));
  #else
    if (HEDLEY_UNLIKELY(a < SIMDE_FLOAT32_C(0.0))) {
      return 0;
    } else if (HEDLEY_UNLIKELY(a > HEDLEY_STATIC_CAST(simde_float32, UINT32_MAX))) {
      return UINT32_MAX;
    } else if (HEDLEY_UNLIKELY(simde_math_isnanf(a))) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(uint32_t, simde_math_roundevenf(a));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtns_u32_f32
  #define vcvtns_u32_f32(a) simde_vcvtns_u32_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vcvtnq_u32_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_46844)
    return vcvtnq_u32_f32(a);
  #else
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_uint32x4_private r_;

    #if 0 && defined(SIMDE_X86_AVX512F_NATIVE) && defined(SIMDE_X86_AVX512VL_NATIVE)
      // Hmm.. this doesn't work, unlike the signed versions
      if (HEDLEY_UNLIKELY(_MM_GET_ROUNDING_MODE() != _MM_ROUND_NEAREST)) {
        unsigned int rounding_mode = _MM_GET_ROUNDING_MODE();
        _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
        r_.m128i = _mm_cvtps_epu32(a_.m128);
        _MM_SET_ROUNDING_MODE(rounding_mode);
      } else {
        r_.m128i = _mm_cvtps_epu32(a_.m128);
      }
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = simde_vcvtns_u32_f32(a_.values[i]);
      }
    #endif

    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtnq_u32_f32
  #define vcvtnq_u32_f32(a) simde_vcvtnq_u32_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
int64_t
simde_vcvtnd_s64_f64(simde_float64 a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtnd_s64_f64(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(int64_t, simde_math_roundeven(a));
  #else
    if (HEDLEY_UNLIKELY(a < HEDLEY_STATIC_CAST(simde_float64, INT64_MIN))) {
      return INT64_MIN;
    } else if (HEDLEY_UNLIKELY(a > HEDLEY_STATIC_CAST(simde_float64, INT64_MAX))) {
      return INT64_MAX;
    } else if (simde_math_isnan(a)) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(int64_t, simde_math_roundeven(a));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtnd_s64_f64
  #define vcvtnd_s64_f64(a) simde_vcvtnd_s64_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint64_t
simde_vcvtnd_u64_f64(simde_float64 a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtnd_u64_f64(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(uint64_t, simde_math_roundeven(a));
  #else
    if (HEDLEY_UNLIKELY(a < SIMDE_FLOAT64_C(0.0))) {
      return 0;
    } else if (HEDLEY_UNLIKELY(a > HEDLEY_STATIC_CAST(simde_float64, UINT64_MAX))) {
      return UINT64_MAX;
    } else if (simde_math_isnan(a)) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(uint64_t, simde_math_roundeven(a));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtnd_u64_f64
  #define vcvtnd_u64_f64(a) simde_vcvtnd_u64_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vcvtnq_u64_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtnq_u64_f64(a);
  #else
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_uint64x2_private r_;

    #if 0 && defined(SIMDE_X86_AVX512DQ_NATIVE) && defined(SIMDE_X86_AVX512VL_NATIVE)
      // Hmm.. this doesn't work, unlike the signed versions
      if (HEDLEY_UNLIKELY(_MM_GET_ROUNDING_MODE() != _MM_ROUND_NEAREST)) {
        unsigned int rounding_mode = _MM_GET_ROUNDING_MODE();
        _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
        r_.m128i = _mm_cvtpd_epu64(a_.m128d);
        _MM_SET_ROUNDING_MODE(rounding_mode);
      } else {
        r_.m128i = _mm_cvtpd_epu64(a_.m128d);
      }
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = simde_vcvtnd_u64_f64(a_.values[i]);
      }
    #endif

    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtnq_u64_f64
  #define vcvtnq_u64_f64(a) simde_vcvtnq_u64_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vcvtnq_u16_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcvtnq_u16_f16(a);
  #else
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_uint16x8_private r_;

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vcvtnh_u16_f16(a_.values[i]);
    }

    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtnq_u16_f16
  #define vcvtnq_u16_f16(a) simde_vcvtnq_u16_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vcvtn_u16_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcvtn_u16_f16(a);
  #else
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_uint16x4_private r_;

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vcvtnh_u16_f16(a_.values[i]);
    }

    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtn_u16_f16
  #define vcvtn_u16_f16(a) simde_vcvtn_u16_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vcvtn_u32_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtn_u32_f32(a);
  #else
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_uint32x2_private r_;

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vcvtns_u32_f32(a_.values[i]);
    }

    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtn_u32_f32
  #define vcvtn_u32_f32(a) simde_vcvtn_u32_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vcvtn_s32_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtn_s32_f32(a);
  #else
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_int32x2_private r_;

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vcvtns_s32_f32(a_.values[i]);
    }

    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtn_s32_f32
  #define vcvtn_s32_f32(a) simde_vcvtn_s32_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vcvtn_s64_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtn_s64_f64(a);
  #else
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_int64x1_private r_;

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vcvtnd_s64_f64(a_.values[i]);
    }

    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtn_s64_f64
  #define vcvtn_s64_f64(a) simde_vcvtn_s64_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vcvtn_u64_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtn_u64_f64(a);
  #else
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_uint64x1_private r_;

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vcvtnd_u64_f64(a_.values[i]);
    }

    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtn_u64_f64
  #define vcvtn_u64_f64(a) simde_vcvtn_u64_f64(a)
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* SIMDE_ARM_NEON_CVTN_H */
