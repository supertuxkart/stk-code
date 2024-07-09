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
 *   2020      Evan Nemerson <evan@nemerson.com>
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 *   2023      Yung-Cheng Su <eric20607@gapp.nthu.edu.tw>
 */

#if !defined(SIMDE_ARM_NEON_MUL_LANE_H)
#define SIMDE_ARM_NEON_MUL_LANE_H

#include "types.h"
#include "mul.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_float16_t
simde_vmulh_lane_f16(simde_float16_t a, simde_float16x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  return simde_vmulh_f16(a, simde_float16x4_to_private(b).values[lane]);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #if defined(__clang__) && !SIMDE_DETECT_CLANG_VERSION_CHECK(11,0,0)
    #define simde_vmulh_lane_f16(a, b, lane) \
    SIMDE_DISABLE_DIAGNOSTIC_EXPR_(SIMDE_DIAGNOSTIC_DISABLE_VECTOR_CONVERSION_, vmulh_lane_f16(a, b, lane))
  #else
    #define simde_vmulh_lane_f16(a, b, lane) vmulh_lane_f16((a), (b), (lane))
  #endif
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulh_lane_f16
  #define vmulh_lane_f16(a, b, lane) simde_vmulh_lane_f16(a, b, lane)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64_t
simde_vmuld_lane_f64(simde_float64_t a, simde_float64x1_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 0) {
  return a * simde_float64x1_to_private(b).values[lane];
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #if defined(__clang__) && !SIMDE_DETECT_CLANG_VERSION_CHECK(11,0,0)
    #define simde_vmuld_lane_f64(a, b, lane) \
    SIMDE_DISABLE_DIAGNOSTIC_EXPR_(SIMDE_DIAGNOSTIC_DISABLE_VECTOR_CONVERSION_, vmuld_lane_f64(a, b, lane))
  #else
    #define simde_vmuld_lane_f64(a, b, lane) vmuld_lane_f64((a), (b), (lane))
  #endif
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmuld_lane_f64
  #define vmuld_lane_f64(a, b, lane) simde_vmuld_lane_f64(a, b, lane)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64_t
simde_vmuld_laneq_f64(simde_float64_t a, simde_float64x2_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
  return a * simde_float64x2_to_private(b).values[lane];
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #if defined(__clang__) && !SIMDE_DETECT_CLANG_VERSION_CHECK(11,0,0)
    #define simde_vmuld_laneq_f64(a, b, lane) \
    SIMDE_DISABLE_DIAGNOSTIC_EXPR_(SIMDE_DIAGNOSTIC_DISABLE_VECTOR_CONVERSION_, vmuld_laneq_f64(a, b, lane))
  #else
    #define simde_vmuld_laneq_f64(a, b, lane) vmuld_laneq_f64((a), (b), (lane))
  #endif
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmuld_laneq_f64
  #define vmuld_laneq_f64(a, b, lane) simde_vmuld_laneq_f64(a, b, lane)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32_t
simde_vmuls_lane_f32(simde_float32_t a, simde_float32x2_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
  return a * simde_float32x2_to_private(b).values[lane];
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #if defined(__clang__) && !SIMDE_DETECT_CLANG_VERSION_CHECK(11,0,0)
    #define simde_vmuls_lane_f32(a, b, lane) \
    SIMDE_DISABLE_DIAGNOSTIC_EXPR_(SIMDE_DIAGNOSTIC_DISABLE_VECTOR_CONVERSION_, vmuls_lane_f32(a, b, lane))
  #else
    #define simde_vmuls_lane_f32(a, b, lane) vmuls_lane_f32((a), (b), (lane))
  #endif
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmuls_lane_f32
  #define vmuls_lane_f32(a, b, lane) simde_vmuls_lane_f32(a, b, lane)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16_t
simde_vmulh_laneq_f16(simde_float16_t a, simde_float16x8_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  return simde_vmulh_f16(a, simde_float16x8_to_private(b).values[lane]);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #if defined(__clang__) && !SIMDE_DETECT_CLANG_VERSION_CHECK(11,0,0)
    #define simde_vmulh_laneq_f16(a, b, lane) \
    SIMDE_DISABLE_DIAGNOSTIC_EXPR_(SIMDE_DIAGNOSTIC_DISABLE_VECTOR_CONVERSION_, vmulh_laneq_f16(a, b, lane))
  #else
    #define simde_vmulh_laneq_f16(a, b, lane) vmulh_laneq_f16((a), (b), (lane))
  #endif
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulh_laneq_f16
  #define vmulh_laneq_f16(a, b, lane) simde_vmulh_laneq_f16(a, b, lane)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32_t
simde_vmuls_laneq_f32(simde_float32_t a, simde_float32x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  return a * simde_float32x4_to_private(b).values[lane];
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #if defined(__clang__) && !SIMDE_DETECT_CLANG_VERSION_CHECK(11,0,0)
    #define simde_vmuls_laneq_f32(a, b, lane) \
    SIMDE_DISABLE_DIAGNOSTIC_EXPR_(SIMDE_DIAGNOSTIC_DISABLE_VECTOR_CONVERSION_, vmuls_laneq_f32(a, b, lane))
  #else
    #define simde_vmuls_laneq_f32(a, b, lane) vmuls_laneq_f32((a), (b), (lane))
  #endif
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmuls_laneq_f32
  #define vmuls_laneq_f32(a, b, lane) simde_vmuls_laneq_f32(a, b, lane)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vmul_lane_f16(simde_float16x4_t a, simde_float16x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_float16x4_private
    r_,
    a_ = simde_float16x4_to_private(a),
    b_ = simde_float16x4_to_private(b);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vmulh_f16(a_.values[i], b_.values[lane]);
  }

  return simde_float16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vmul_lane_f16(a, b, lane) vmul_lane_f16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vmul_lane_f16
  #define vmul_lane_f16(a, b, lane) simde_vmul_lane_f16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vmul_lane_f32(simde_float32x2_t a, simde_float32x2_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
  simde_float32x2_private
    r_,
    a_ = simde_float32x2_to_private(a),
    b_ = simde_float32x2_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv64 = __riscv_vfmul_vf_f32m1(a_.sv64, b_.values[lane], 2);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_float32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmul_lane_f32(a, b, lane) vmul_lane_f32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmul_lane_f32
  #define vmul_lane_f32(a, b, lane) simde_vmul_lane_f32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vmul_lane_f64(simde_float64x1_t a, simde_float64x1_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 0) {
  simde_float64x1_private
    r_,
    a_ = simde_float64x1_to_private(a),
    b_ = simde_float64x1_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv64 = __riscv_vfmul_vf_f64m1(a_.sv64, b_.values[lane], 1);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_float64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmul_lane_f64(a, b, lane) vmul_lane_f64((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmul_lane_f64
  #define vmul_lane_f64(a, b, lane) simde_vmul_lane_f64((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vmul_lane_s16(simde_int16x4_t a, simde_int16x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_int16x4_private
    r_,
    a_ = simde_int16x4_to_private(a),
    b_ = simde_int16x4_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv64 = __riscv_vmul_vx_i16m1(a_.sv64, b_.values[lane], 4);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_int16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmul_lane_s16(a, b, lane) vmul_lane_s16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmul_lane_s16
  #define vmul_lane_s16(a, b, lane) simde_vmul_lane_s16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vmul_lane_s32(simde_int32x2_t a, simde_int32x2_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
  simde_int32x2_private
    r_,
    a_ = simde_int32x2_to_private(a),
    b_ = simde_int32x2_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv64 = __riscv_vmul_vx_i32m1(a_.sv64, b_.values[lane], 2);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_int32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmul_lane_s32(a, b, lane) vmul_lane_s32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmul_lane_s32
  #define vmul_lane_s32(a, b, lane) simde_vmul_lane_s32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vmul_lane_u16(simde_uint16x4_t a, simde_uint16x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_uint16x4_private
    r_,
    a_ = simde_uint16x4_to_private(a),
    b_ = simde_uint16x4_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv64 = __riscv_vmul_vx_u16m1(a_.sv64, b_.values[lane], 4);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_uint16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmul_lane_u16(a, b, lane) vmul_lane_u16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmul_lane_u16
  #define vmul_lane_u16(a, b, lane) simde_vmul_lane_u16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vmul_lane_u32(simde_uint32x2_t a, simde_uint32x2_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
  simde_uint32x2_private
    r_,
    a_ = simde_uint32x2_to_private(a),
    b_ = simde_uint32x2_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv64 = __riscv_vmul_vx_u32m1(a_.sv64, b_.values[lane], 2);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_uint32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmul_lane_u32(a, b, lane) vmul_lane_u32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmul_lane_u32
  #define vmul_lane_u32(a, b, lane) simde_vmul_lane_u32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vmul_laneq_s16(simde_int16x4_t a, simde_int16x8_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_int16x4_private
    r_,
    a_ = simde_int16x4_to_private(a);
  simde_int16x8_private
    b_ = simde_int16x8_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv64 = __riscv_vmul_vx_i16m1(a_.sv64, b_.values[lane], 4);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_int16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmul_laneq_s16(a, b, lane) vmul_laneq_s16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmul_laneq_s16
  #define vmul_laneq_s16(a, b, lane) simde_vmul_laneq_s16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vmul_laneq_s32(simde_int32x2_t a, simde_int32x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_int32x2_private
    r_,
    a_ = simde_int32x2_to_private(a);
  simde_int32x4_private
    b_ = simde_int32x4_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv64 = __riscv_vmul_vx_i32m1(a_.sv64, b_.values[lane], 2);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_int32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmul_laneq_s32(a, b, lane) vmul_laneq_s32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmul_laneq_s32
  #define vmul_laneq_s32(a, b, lane) simde_vmul_laneq_s32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vmul_laneq_u16(simde_uint16x4_t a, simde_uint16x8_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_uint16x4_private
    r_,
    a_ = simde_uint16x4_to_private(a);
  simde_uint16x8_private
    b_ = simde_uint16x8_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv64 = __riscv_vmul_vx_u16m1(a_.sv64, b_.values[lane], 4);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_uint16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmul_laneq_u16(a, b, lane) vmul_laneq_u16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmul_laneq_u16
  #define vmul_laneq_u16(a, b, lane) simde_vmul_laneq_u16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vmul_laneq_u32(simde_uint32x2_t a, simde_uint32x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_uint32x2_private
    r_,
    a_ = simde_uint32x2_to_private(a);
  simde_uint32x4_private
    b_ = simde_uint32x4_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv64 = __riscv_vmul_vx_u32m1(a_.sv64, b_.values[lane], 2);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_uint32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmul_laneq_u32(a, b, lane) vmul_laneq_u32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmul_laneq_u32
  #define vmul_laneq_u32(a, b, lane) simde_vmul_laneq_u32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vmulq_lane_f16(simde_float16x8_t a, simde_float16x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_float16x8_private
    r_,
    a_ = simde_float16x8_to_private(a);
  simde_float16x4_private b_ = simde_float16x4_to_private(b);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vmulh_f16(a_.values[i], b_.values[lane]);
  }

  return simde_float16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vmulq_lane_f16(a, b, lane) vmulq_lane_f16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vmulq_lane_f16
  #define vmulq_lane_f16(a, b, lane) simde_vmulq_lane_f16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vmulq_lane_f32(simde_float32x4_t a, simde_float32x2_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
  simde_float32x4_private
    r_,
    a_ = simde_float32x4_to_private(a);
  simde_float32x2_private b_ = simde_float32x2_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv128 = __riscv_vfmul_vf_f32m1(a_.sv128, b_.values[lane], 4);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_float32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmulq_lane_f32(a, b, lane) vmulq_lane_f32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmulq_lane_f32
  #define vmulq_lane_f32(a, b, lane) simde_vmulq_lane_f32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vmulq_lane_f64(simde_float64x2_t a, simde_float64x1_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 0) {
  simde_float64x2_private
    r_,
    a_ = simde_float64x2_to_private(a);
  simde_float64x1_private b_ = simde_float64x1_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv128 = __riscv_vfmul_vf_f64m1(a_.sv128, b_.values[lane], 2);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_float64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmulq_lane_f64(a, b, lane) vmulq_lane_f64((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulq_lane_f64
  #define vmulq_lane_f64(a, b, lane) simde_vmulq_lane_f64((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vmulq_lane_s16(simde_int16x8_t a, simde_int16x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_int16x8_private
    r_,
    a_ = simde_int16x8_to_private(a);
  simde_int16x4_private b_ = simde_int16x4_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv128 = __riscv_vmul_vx_i16m1(a_.sv128, b_.values[lane], 8);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_int16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmulq_lane_s16(a, b, lane) vmulq_lane_s16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmulq_lane_s16
  #define vmulq_lane_s16(a, b, lane) simde_vmulq_lane_s16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vmulq_lane_s32(simde_int32x4_t a, simde_int32x2_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
  simde_int32x4_private
    r_,
    a_ = simde_int32x4_to_private(a);
  simde_int32x2_private b_ = simde_int32x2_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv128 = __riscv_vmul_vx_i32m1(a_.sv128, b_.values[lane], 4);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_int32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmulq_lane_s32(a, b, lane) vmulq_lane_s32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmulq_lane_s32
  #define vmulq_lane_s32(a, b, lane) simde_vmulq_lane_s32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vmulq_lane_u16(simde_uint16x8_t a, simde_uint16x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_uint16x8_private
    r_,
    a_ = simde_uint16x8_to_private(a);
  simde_uint16x4_private b_ = simde_uint16x4_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv128 = __riscv_vmul_vx_u16m1(a_.sv128, b_.values[lane], 8);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_uint16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmulq_lane_u16(a, b, lane) vmulq_lane_u16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmulq_lane_u16
  #define vmulq_lane_u16(a, b, lane) simde_vmulq_lane_u16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vmulq_lane_u32(simde_uint32x4_t a, simde_uint32x2_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
  simde_uint32x4_private
    r_,
    a_ = simde_uint32x4_to_private(a);
  simde_uint32x2_private b_ = simde_uint32x2_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv128 = __riscv_vmul_vx_u32m1(a_.sv128, b_.values[lane], 4);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_uint32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmulq_lane_u32(a, b, lane) vmulq_lane_u32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmulq_lane_u32
  #define vmulq_lane_u32(a, b, lane) simde_vmulq_lane_u32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vmulq_laneq_f16(simde_float16x8_t a, simde_float16x8_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_float16x8_private
    r_,
    a_ = simde_float16x8_to_private(a),
    b_ = simde_float16x8_to_private(b);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vmulh_f16(a_.values[i], b_.values[lane]);
  }

  return simde_float16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vmulq_laneq_f16(a, b, lane) vmulq_laneq_f16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulq_laneq_f16
  #define vmulq_laneq_f16(a, b, lane) simde_vmulq_laneq_f16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vmulq_laneq_f32(simde_float32x4_t a, simde_float32x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_float32x4_private
    r_,
    a_ = simde_float32x4_to_private(a),
    b_ = simde_float32x4_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv128 = __riscv_vfmul_vf_f32m1(a_.sv128, b_.values[lane], 4);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_float32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmulq_laneq_f32(a, b, lane) vmulq_laneq_f32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulq_laneq_f32
  #define vmulq_laneq_f32(a, b, lane) simde_vmulq_laneq_f32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vmulq_laneq_f64(simde_float64x2_t a, simde_float64x2_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
  simde_float64x2_private
    r_,
    a_ = simde_float64x2_to_private(a),
    b_ = simde_float64x2_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv128 = __riscv_vfmul_vf_f64m1(a_.sv128, b_.values[lane], 2);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_float64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmulq_laneq_f64(a, b, lane) vmulq_laneq_f64((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulq_laneq_f64
  #define vmulq_laneq_f64(a, b, lane) simde_vmulq_laneq_f64((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vmulq_laneq_s16(simde_int16x8_t a, simde_int16x8_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_int16x8_private
    r_,
    a_ = simde_int16x8_to_private(a),
    b_ = simde_int16x8_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv128 = __riscv_vmul_vx_i16m1(a_.sv128, b_.values[lane], 8);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_int16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmulq_laneq_s16(a, b, lane) vmulq_laneq_s16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulq_laneq_s16
  #define vmulq_laneq_s16(a, b, lane) simde_vmulq_laneq_s16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vmulq_laneq_s32(simde_int32x4_t a, simde_int32x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_int32x4_private
    r_,
    a_ = simde_int32x4_to_private(a),
    b_ = simde_int32x4_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv128 = __riscv_vmul_vx_i32m1(a_.sv128, b_.values[lane], 4);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_int32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmulq_laneq_s32(a, b, lane) vmulq_laneq_s32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulq_laneq_s32
  #define vmulq_laneq_s32(a, b, lane) simde_vmulq_laneq_s32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vmulq_laneq_u16(simde_uint16x8_t a, simde_uint16x8_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_uint16x8_private
    r_,
    a_ = simde_uint16x8_to_private(a),
    b_ = simde_uint16x8_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv128 = __riscv_vmul_vx_u16m1(a_.sv128, b_.values[lane], 8);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_uint16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmulq_laneq_u16(a, b, lane) vmulq_laneq_u16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulq_laneq_u16
  #define vmulq_laneq_u16(a, b, lane) simde_vmulq_laneq_u16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vmulq_laneq_u32(simde_uint32x4_t a, simde_uint32x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_uint32x4_private
    r_,
    a_ = simde_uint32x4_to_private(a),
    b_ = simde_uint32x4_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv128 = __riscv_vmul_vx_u32m1(a_.sv128, b_.values[lane], 4);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_uint32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmulq_laneq_u32(a, b, lane) vmulq_laneq_u32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulq_laneq_u32
  #define vmulq_laneq_u32(a, b, lane) simde_vmulq_laneq_u32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vmul_laneq_f16(simde_float16x4_t a, simde_float16x8_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_float16x4_private
    r_,
    a_ = simde_float16x4_to_private(a);
  simde_float16x8_private b_ = simde_float16x8_to_private(b);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vmulh_f16(a_.values[i], b_.values[lane]);
  }

  return simde_float16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vmul_laneq_f16(a, b, lane) vmul_laneq_f16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmul_laneq_f16
  #define vmul_laneq_f16(a, b, lane) simde_vmul_laneq_f16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vmul_laneq_f32(simde_float32x2_t a, simde_float32x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_float32x2_private
    r_,
    a_ = simde_float32x2_to_private(a);
  simde_float32x4_private b_ = simde_float32x4_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv64 = __riscv_vfmul_vf_f32m1(a_.sv64, b_.values[lane], 2);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_float32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmul_laneq_f32(a, b, lane) vmul_laneq_f32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmul_laneq_f32
  #define vmul_laneq_f32(a, b, lane) simde_vmul_laneq_f32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vmul_laneq_f64(simde_float64x1_t a, simde_float64x2_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
  simde_float64x1_private
    r_,
    a_ = simde_float64x1_to_private(a);
  simde_float64x2_private b_ = simde_float64x2_to_private(b);

  #if defined(SIMDE_RISCV_V_NATIVE)
    r_.sv64 = __riscv_vfmul_vf_f64m1(a_.sv64, b_.values[lane], 1);
  #else
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] * b_.values[lane];
    }
  #endif

  return simde_float64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmul_laneq_f64(a, b, lane) vmul_laneq_f64((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmul_laneq_f64
  #define vmul_laneq_f64(a, b, lane) simde_vmul_laneq_f64((a), (b), (lane))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_MUL_LANE_H) */
