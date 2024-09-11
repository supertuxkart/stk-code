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
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 */

#if !defined(SIMDE_ARM_NEON_CVTM_H)
#define SIMDE_ARM_NEON_CVTM_H

#include "types.h"
#include "cvt.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
int64_t
simde_vcvtmh_s64_f16(simde_float16_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcvtmh_s64_f16(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(int64_t,
        simde_math_floorf(
        simde_float16_to_float32(a)));
  #else
    simde_float32 af = simde_float16_to_float32(a);
    if (HEDLEY_UNLIKELY(af <= HEDLEY_STATIC_CAST(simde_float32, INT64_MIN))) {
      return INT64_MIN;
    } else if (HEDLEY_UNLIKELY(af >= HEDLEY_STATIC_CAST(simde_float32, INT64_MAX))) {
      return INT64_MAX;
    } else if (HEDLEY_UNLIKELY(simde_isnanhf(a))) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(int64_t, simde_math_floorf(af));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtmh_s64_f16
  #define vcvtmh_s64_f16(a) simde_vcvtmh_s64_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
int32_t
simde_vcvtmh_s32_f16(simde_float16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcvtmh_s32_f16(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(int32_t,
        simde_math_floorf(
        simde_float16_to_float32(a)));
  #else
    simde_float32 af = simde_float16_to_float32(a);
    if (HEDLEY_UNLIKELY(af <= HEDLEY_STATIC_CAST(simde_float32, INT32_MIN))) {
      return INT32_MIN;
    } else if (HEDLEY_UNLIKELY(af >= HEDLEY_STATIC_CAST(simde_float32, INT32_MAX))) {
      return INT32_MAX;
    } else if (HEDLEY_UNLIKELY(simde_isnanhf(a))) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(int32_t, simde_math_floorf(af));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtmh_s32_f16
  #define vcvtmh_s32_f16(a) simde_vcvtmh_s32_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint64_t
simde_vcvtmh_u64_f16(simde_float16_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcvtmh_u64_f16(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(uint64_t,
        simde_math_floorf(
        simde_float16_to_float32(a)));
  #else
    simde_float32 af = simde_float16_to_float32(a);
    if (HEDLEY_UNLIKELY(af <= SIMDE_FLOAT32_C(0.0))) {
      return 0;
    } else if (HEDLEY_UNLIKELY(af >= HEDLEY_STATIC_CAST(simde_float32, UINT64_MAX))) {
      return UINT64_MAX;
    } else if (HEDLEY_UNLIKELY(simde_isnanhf(a))) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(uint64_t, simde_math_floorf(af));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtmh_u64_f16
  #define vcvtmh_u64_f16(a) simde_vcvtmh_u64_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde_vcvtmh_u32_f16(simde_float16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcvtmh_u32_f16(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(uint32_t,
        simde_math_floorf(
        simde_float16_to_float32(a)));
  #else
    simde_float32 af = simde_float16_to_float32(a);
    if (HEDLEY_UNLIKELY(af <= SIMDE_FLOAT32_C(0.0))) {
      return 0;
    } else if (HEDLEY_UNLIKELY(af >= HEDLEY_STATIC_CAST(simde_float32, UINT32_MAX))) {
      return UINT32_MAX;
    } else if (HEDLEY_UNLIKELY(simde_isnanhf(a))) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(uint32_t, simde_math_floorf(af));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtmh_u32_f16
  #define vcvtmh_u32_f16(a) simde_vcvtmh_u32_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint16_t
simde_vcvtmh_u16_f16(simde_float16_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcvtmh_u16_f16(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(uint16_t,
        simde_math_floorf(
        simde_float16_to_float32(a)));
  #else
    simde_float32 af = simde_float16_to_float32(a);
    if (HEDLEY_UNLIKELY(af <= SIMDE_FLOAT32_C(0.0))) {
      return 0;
    } else if (HEDLEY_UNLIKELY(af >= HEDLEY_STATIC_CAST(simde_float32, UINT16_MAX))) {
      return UINT16_MAX;
    } else if (HEDLEY_UNLIKELY(simde_isnanhf(a))) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(uint16_t, simde_math_floorf(af));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtmh_u16_f16
  #define vcvtmh_u16_f16(a) simde_vcvtmh_u16_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde_vcvtms_u32_f32(simde_float32 a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtms_u32_f32(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(uint32_t, simde_math_floorf(a));
  #else
    if (HEDLEY_UNLIKELY(a <= SIMDE_FLOAT32_C(0.0))) {
      return 0;
    } else if (HEDLEY_UNLIKELY(a >= HEDLEY_STATIC_CAST(simde_float32, UINT32_MAX))) {
      return UINT32_MAX;
    } else if (HEDLEY_UNLIKELY(simde_math_isnanf(a))) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(uint32_t, simde_math_floorf(a));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtms_u32_f32
  #define vcvtms_u32_f32(a) simde_vcvtms_u32_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint64_t
simde_vcvtmd_u64_f64(simde_float64 a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtmd_u64_f64(a);
  #elif defined(SIMDE_FAST_CONVERSION_RANGE)
    return HEDLEY_STATIC_CAST(uint64_t, simde_math_floor(a));
  #else
    if (HEDLEY_UNLIKELY(a <= SIMDE_FLOAT64_C(0.0))) {
      return 0;
    } else if (HEDLEY_UNLIKELY(a >= HEDLEY_STATIC_CAST(simde_float64, UINT64_MAX))) {
      return UINT64_MAX;
    } else if (simde_math_isnan(a)) {
      return 0;
    } else {
      return HEDLEY_STATIC_CAST(uint64_t, simde_math_floor(a));
    }
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtmd_u64_f64
  #define vcvtmd_u64_f64(a) simde_vcvtmd_u64_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vcvtmq_u16_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcvtmq_u16_f16(a);
  #else
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_uint16x8_private r_;

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vcvtmh_u16_f16(a_.values[i]);
    }

    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtmq_u16_f16
  #define vcvtmq_u16_f16(a) simde_vcvtmq_u16_f16(a)
#endif


SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vcvtmq_u32_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_46844)
    return vcvtmq_u32_f32(a);
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
        r_.values[i] = simde_vcvtms_u32_f32(a_.values[i]);
      }
    #endif

    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtmq_u32_f32
  #define vcvtmq_u32_f32(a) simde_vcvtmq_u32_f32(a)
#endif


SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vcvtmq_u64_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtmq_u64_f64(a);
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
        r_.values[i] = simde_vcvtmd_u64_f64(a_.values[i]);
      }
    #endif

    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtmq_u64_f64
  #define vcvtmq_u64_f64(a) simde_vcvtmq_u64_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vcvtm_u16_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcvtm_u16_f16(a);
  #else
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_uint16x4_private r_;

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vcvtmh_u16_f16(a_.values[i]);
    }

    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtm_u16_f16
  #define vcvtm_u16_f16(a) simde_vcvtm_u16_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vcvtm_u32_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtm_u32_f32(a);
  #else
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_uint32x2_private r_;

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vcvtms_u32_f32(a_.values[i]);
    }

    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtm_u32_f32
  #define vcvtm_u32_f32(a) simde_vcvtm_u32_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vcvtm_u64_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcvtm_u64_f64(a);
  #else
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_uint64x1_private r_;

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vcvtmd_u64_f64(a_.values[i]);
    }

    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtm_u64_f64
  #define vcvtm_u64_f64(a) simde_vcvtm_u64_f64(a)
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* SIMDE_ARM_NEON_CVTM_H */
