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

#if !defined(SIMDE_ARM_NEON_CVT_N_H)
#define SIMDE_ARM_NEON_CVT_N_H

#include "types.h"
#include "cvt.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
uint16_t
simde_vcvth_n_u16_f16(simde_float16_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 16) {
  return simde_vcvth_u16_f16(
      simde_float16_from_float32(
      simde_float16_to_float32(a) * HEDLEY_STATIC_CAST(simde_float32_t, pow(2, n))));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vcvth_n_u16_f16(a, n) vcvth_n_u16_f16(a, n)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvth_n_u16_f16
  #define vcvth_n_u16_f16(a, n) simde_vcvth_n_u16_f16(a, n)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16_t
simde_vcvth_n_f16_s16(int16_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 16) {
  return simde_float16_from_float32(
      HEDLEY_STATIC_CAST(simde_float32_t,
      HEDLEY_STATIC_CAST(simde_float64_t, a) / pow(2, n)));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vcvth_n_f16_s16(a, n) vcvth_n_f16_s16(a, n)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvth_n_f16_s16
  #define vcvth_n_f16_s16(a, n) simde_vcvth_n_f16_s16(a, n)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16_t
simde_vcvth_n_f16_u16(uint16_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 16) {
  return simde_float16_from_float32(
      HEDLEY_STATIC_CAST(simde_float32_t,
      HEDLEY_STATIC_CAST(simde_float64_t, a) / pow(2, n)));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vcvth_n_f16_u16(a, n) vcvth_n_f16_u16(a, n)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvth_n_f16_u16
  #define vcvth_n_f16_u16(a, n) simde_vcvth_n_f16_u16(a, n)
#endif

SIMDE_FUNCTION_ATTRIBUTES
int32_t
simde_vcvts_n_s32_f32(simde_float32_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  return simde_vcvts_s32_f32(a * HEDLEY_STATIC_CAST(simde_float32_t, pow(2, n)));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcvts_n_s32_f32(a, n) vcvts_n_s32_f32(a, n)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvts_n_s32_f32
  #define vcvts_n_s32_f32(a, n) simde_vcvts_n_s32_f32(a, n)
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde_vcvts_n_u32_f32(simde_float32_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  return simde_vcvts_u32_f32(a * HEDLEY_STATIC_CAST(simde_float32_t, pow(2, n)));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcvts_n_u32_f32(a, n) vcvts_n_u32_f32(a, n)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvts_n_u32_f32
  #define vcvts_n_u32_f32(a, n) simde_vcvts_n_u32_f32(a, n)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32_t
simde_vcvts_n_f32_s32(int32_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  return HEDLEY_STATIC_CAST(simde_float32_t,
      HEDLEY_STATIC_CAST(simde_float64_t, a) / pow(2, n));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcvts_n_f32_s32(a, n) vcvts_n_f32_s32(a, n)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvts_n_f32_s32
  #define vcvts_n_f32_s32(a, n) simde_vcvts_n_f32_s32(a, n)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32_t
simde_vcvts_n_f32_u32(uint32_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  return HEDLEY_STATIC_CAST(simde_float32_t,
      HEDLEY_STATIC_CAST(simde_float64_t, a) / pow(2, n));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcvts_n_f32_u32(a, n) vcvts_n_f32_u32(a, n)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvts_n_f32_u32
  #define vcvts_n_f32_u32(a, n) simde_vcvts_n_f32_u32(a, n)
#endif

SIMDE_FUNCTION_ATTRIBUTES
int64_t
simde_vcvtd_n_s64_f64(simde_float64_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 64) {
  return simde_vcvtd_s64_f64(a * pow(2, n));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcvtd_n_s64_f64(a, n) vcvtd_n_s64_f64(a, n)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtd_n_s64_f64
  #define vcvtd_n_s64_f64(a, n) simde_vcvtd_n_s64_f64(a, n)
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint64_t
simde_vcvtd_n_u64_f64(simde_float64_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 64) {
  return simde_vcvtd_u64_f64(a * pow(2, n));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcvtd_n_u64_f64(a, n) vcvtd_n_u64_f64(a, n)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtd_n_u64_f64
  #define vcvtd_n_u64_f64(a, n) simde_vcvtd_n_u64_f64(a, n)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64_t
simde_vcvtd_n_f64_s64(int64_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 64) {
  return HEDLEY_STATIC_CAST(simde_float64_t, a) / pow(2, n);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcvtd_n_f64_s64(a, n) vcvtd_n_f64_s64(a, n)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtd_n_f64_s64
  #define vcvtd_n_f64_s64(a, n) simde_vcvtd_n_f64_s64(a, n)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64_t
simde_vcvtd_n_f64_u64(uint64_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 64) {
  return HEDLEY_STATIC_CAST(simde_float64_t, a) / pow(2, n);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcvtd_n_f64_u64(a, n) vcvtd_n_f64_u64(a, n)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtd_n_f64_u64
  #define vcvtd_n_f64_u64(a, n) simde_vcvtd_n_f64_u64(a, n)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vcvt_n_s32_f32(simde_float32x2_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  simde_float32x2_private a_ = simde_float32x2_to_private(a);
  simde_int32x2_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vcvts_s32_f32(a_.values[i] * HEDLEY_STATIC_CAST(simde_float32_t, pow(2, n)));
  }

  return simde_int32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vcvt_n_s32_f32(a, n) vcvt_n_s32_f32((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcvt_n_s32_f32
  #define vcvt_n_s32_f32(a, n) simde_vcvt_n_s32_f32((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vcvt_n_s64_f64(simde_float64x1_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 64) {
  simde_float64x1_private a_ = simde_float64x1_to_private(a);
  simde_int64x1_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vcvtd_s64_f64(a_.values[i] * pow(2, n));
  }

  return simde_int64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcvt_n_s64_f64(a, n) vcvt_n_s64_f64((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvt_n_s64_f64
  #define vcvt_n_s64_f64(a, n) simde_vcvt_n_s64_f64((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vcvt_n_u16_f16(simde_float16x4_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 16) {
  simde_float16x4_private a_ = simde_float16x4_to_private(a);
  simde_uint16x4_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vcvth_u16_f16(simde_float16_from_float32(
          simde_float16_to_float32(a_.values[i]) *
          HEDLEY_STATIC_CAST(simde_float32_t, pow(2, n))));
  }

  return simde_uint16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vcvt_n_u16_f16(a, n) vcvt_n_u16_f16((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcvt_n_u16_f16
  #define vcvt_n_u16_f16(a, n) simde_vcvt_n_u16_f16((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vcvt_n_u32_f32(simde_float32x2_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  simde_float32x2_private a_ = simde_float32x2_to_private(a);
  simde_uint32x2_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vcvts_u32_f32(a_.values[i] * HEDLEY_STATIC_CAST(simde_float32_t, pow(2, n)));
  }

  return simde_uint32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vcvt_n_u32_f32(a, n) vcvt_n_u32_f32((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcvt_n_u32_f32
  #define vcvt_n_u32_f32(a, n) simde_vcvt_n_u32_f32((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vcvt_n_u64_f64(simde_float64x1_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 64) {
  simde_float64x1_private a_ = simde_float64x1_to_private(a);
  simde_uint64x1_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vcvtd_u64_f64(a_.values[i] * pow(2, n));
  }

  return simde_uint64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_46844)
  #define simde_vcvt_n_u64_f64(a, n) vcvt_n_u64_f64((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvt_n_u64_f64
  #define vcvt_n_u64_f64(a, n) simde_vcvt_n_u64_f64((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vcvtq_n_s32_f32(simde_float32x4_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  simde_float32x4_private a_ = simde_float32x4_to_private(a);
  simde_int32x4_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vcvts_s32_f32(a_.values[i] * HEDLEY_STATIC_CAST(simde_float32_t, pow(2, n)));
  }

  return simde_int32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vcvtq_n_s32_f32(a, n) vcvtq_n_s32_f32((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcvtq_n_s32_f32
  #define vcvtq_n_s32_f32(a, n) simde_vcvtq_n_s32_f32((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vcvtq_n_s64_f64(simde_float64x2_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 64) {
  simde_float64x2_private a_ = simde_float64x2_to_private(a);
  simde_int64x2_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vcvtd_s64_f64(a_.values[i] * pow(2, n));
  }

  return simde_int64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcvtq_n_s64_f64(a, n) vcvtq_n_s64_f64((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtq_n_s64_f64
  #define vcvtq_n_s64_f64(a, n) simde_vcvtq_n_s64_f64((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vcvtq_n_u16_f16(simde_float16x8_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 16) {
  simde_float16x8_private a_ = simde_float16x8_to_private(a);
  simde_uint16x8_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vcvth_u16_f16(simde_float16_from_float32(
          simde_float16_to_float32(a_.values[i]) *
          HEDLEY_STATIC_CAST(simde_float32_t, pow(2, n))));
  }

  return simde_uint16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
#define simde_vcvtq_n_u16_f16(a, n) vcvtq_n_u16_f16((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtq_n_u16_f16
  #define vcvtq_n_u16_f16(a, n) simde_vcvtq_n_u16_f16((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vcvtq_n_u32_f32(simde_float32x4_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  simde_float32x4_private a_ = simde_float32x4_to_private(a);
  simde_uint32x4_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vcvts_u32_f32(a_.values[i] * HEDLEY_STATIC_CAST(simde_float32_t, pow(2, n)));
  }

  return simde_uint32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_CLANG_46844)
  #define simde_vcvtq_n_u32_f32(a, n) vcvtq_n_u32_f32((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcvtq_n_u32_f32
  #define vcvtq_n_u32_f32(a, n) simde_vcvtq_n_u32_f32((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vcvtq_n_u64_f64(simde_float64x2_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 64) {
  simde_float64x2_private a_ = simde_float64x2_to_private(a);
  simde_uint64x2_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vcvtd_u64_f64(a_.values[i] * pow(2, n));
  }

  return simde_uint64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_46844)
  #define simde_vcvtq_n_u64_f64(a, n) vcvtq_n_u64_f64((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtq_n_u64_f64
  #define vcvtq_n_u64_f64(a, n) simde_vcvtq_n_u64_f64((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vcvt_n_f16_u16(simde_uint16x4_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 16) {
  simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
  simde_float16x4_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_float16_from_float32(HEDLEY_STATIC_CAST(simde_float32, HEDLEY_STATIC_CAST(simde_float64_t, a_.values[i]) / pow(2, n)));
  }

  return simde_float16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vcvt_n_f16_u16(a, n) vcvt_n_f16_u16((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcvt_n_f16_u16
  #define vcvt_n_f16_u16(a, n) simde_vcvt_n_f16_u16((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vcvt_n_f16_s16(simde_int16x4_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 16) {
  simde_int16x4_private a_ = simde_int16x4_to_private(a);
  simde_float16x4_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_float16_from_float32(HEDLEY_STATIC_CAST(simde_float32_t, HEDLEY_STATIC_CAST(simde_float64_t, a_.values[i]) / pow(2, n)));
  }

  return simde_float16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vcvt_n_f16_s16(a, n) vcvt_n_f16_s16((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcvt_n_f16_s16
  #define vcvt_n_f16_s16(a, n) simde_vcvt_n_f16_s16((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vcvtq_n_f16_u16(simde_uint16x8_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 16) {
  simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
  simde_float16x8_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_float16_from_float32(HEDLEY_STATIC_CAST(simde_float32_t, HEDLEY_STATIC_CAST(simde_float64_t, a_.values[i]) / pow(2, n)));
  }

  return simde_float16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vcvtq_n_f16_u16(a, n) vcvtq_n_f16_u16((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtq_n_f16_u16
  #define vcvtq_n_f16_u16(a, n) simde_vcvtq_n_f16_u16((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vcvtq_n_f16_s16(simde_int16x8_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 16) {
  simde_int16x8_private a_ = simde_int16x8_to_private(a);
  simde_float16x8_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_float16_from_float32(HEDLEY_STATIC_CAST(simde_float32_t, (a_.values[i] / pow(2, n))));
  }

  return simde_float16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vcvtq_n_f16_s16(a, n) vcvtq_n_f16_s16((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtq_n_f16_s16
  #define vcvtq_n_f16_s16(a, n) simde_vcvtq_n_f16_s16((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vcvt_n_f32_u32(simde_uint32x2_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
  simde_float32x2_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = HEDLEY_STATIC_CAST(simde_float32_t, HEDLEY_STATIC_CAST(simde_float64_t, a_.values[i]) / pow(2, n));
  }

  return simde_float32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vcvt_n_f32_u32(a, n) vcvt_n_f32_u32((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcvt_n_f32_u32
  #define vcvt_n_f32_u32(a, n) simde_vcvt_n_f32_u32((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vcvt_n_f32_s32(simde_int32x2_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  simde_int32x2_private a_ = simde_int32x2_to_private(a);
  simde_float32x2_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = HEDLEY_STATIC_CAST(simde_float32_t, HEDLEY_STATIC_CAST(simde_float64_t, a_.values[i]) / pow(2, n));
  }

  return simde_float32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vcvt_n_f32_s32(a, n) vcvt_n_f32_s32((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcvt_n_f32_s32
  #define vcvt_n_f32_s32(a, n) simde_vcvt_n_f32_s32((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vcvt_n_f64_u64(simde_uint64x1_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 64) {
  simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
  simde_float64x1_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = HEDLEY_STATIC_CAST(simde_float64_t, HEDLEY_STATIC_CAST(simde_float64_t, a_.values[i]) / pow(2, n));
  }

  return simde_float64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcvt_n_f64_u64(a, n) vcvt_n_f64_u64((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvt_n_f64_u64
  #define vcvt_n_f64_u64(a, n) simde_vcvt_n_f64_u64((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vcvtq_n_f64_u64(simde_uint64x2_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 64) {
  simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
  simde_float64x2_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = HEDLEY_STATIC_CAST(simde_float64_t, HEDLEY_STATIC_CAST(simde_float64_t, a_.values[i]) / pow(2, n));
  }

  return simde_float64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcvtq_n_f64_u64(a, n) vcvtq_n_f64_u64((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtq_n_f64_u64
  #define vcvtq_n_f64_u64(a, n) simde_vcvtq_n_f64_u64((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vcvt_n_f64_s64(simde_int64x1_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 64) {
  simde_int64x1_private a_ = simde_int64x1_to_private(a);
  simde_float64x1_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = HEDLEY_STATIC_CAST(simde_float64_t, HEDLEY_STATIC_CAST(simde_float64_t, a_.values[i]) / pow(2, n));
  }

  return simde_float64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcvt_n_f64_s64(a, n) vcvt_n_f64_s64((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvt_n_f64_s64
  #define vcvt_n_f64_s64(a, n) simde_vcvt_n_f64_s64((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vcvtq_n_f64_s64(simde_int64x2_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 64) {
  simde_int64x2_private a_ = simde_int64x2_to_private(a);
  simde_float64x2_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = HEDLEY_STATIC_CAST(simde_float64_t, HEDLEY_STATIC_CAST(simde_float64_t, a_.values[i]) / pow(2, n));
  }

  return simde_float64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcvtq_n_f64_s64(a, n) vcvtq_n_f64_s64((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcvtq_n_f64_s64
  #define vcvtq_n_f64_s64(a, n) simde_vcvtq_n_f64_s64((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vcvtq_n_f32_s32(simde_int32x4_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  simde_int32x4_private a_ = simde_int32x4_to_private(a);
  simde_float32x4_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = HEDLEY_STATIC_CAST(simde_float32_t, HEDLEY_STATIC_CAST(simde_float64_t, a_.values[i]) / pow(2, n));
  }

  return simde_float32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vcvtq_n_f32_s32(a, n) vcvtq_n_f32_s32((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcvtq_n_f32_s32
  #define vcvtq_n_f32_s32(a, n) simde_vcvtq_n_f32_s32((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vcvtq_n_f32_u32(simde_uint32x4_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
  simde_float32x4_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = HEDLEY_STATIC_CAST(simde_float32_t, HEDLEY_STATIC_CAST(simde_float64_t, a_.values[i]) / pow(2, n));
  }

  return simde_float32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vcvtq_n_f32_u32(a, n) vcvtq_n_f32_u32((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcvtq_n_f32_u32
  #define vcvtq_n_f32_u32(a, n) simde_vcvtq_n_f32_u32((a), (n))
#endif


SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* SIMDE_ARM_NEON_CVT_N_H */
