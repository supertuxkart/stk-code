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

#if !defined(SIMDE_ARM_NEON_MULX_LANE_H)
#define SIMDE_ARM_NEON_MULX_LANE_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_float16_t
simde_vmulxh_lane_f16(simde_float16_t a, simde_float16x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  return simde_float16_from_float32(
      simde_float16_to_float32(a) *
      simde_float16_to_float32(simde_float16x4_to_private(b).values[lane]));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vmulxh_lane_f16(a, b, lane) vmulxh_lane_f16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulxh_lane_f16
  #define vmulxh_lane_f16(a, b, lane) simde_vmulxh_lane_f16(a, b, lane)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32_t
simde_vmulxs_lane_f32(simde_float32_t a, simde_float32x2_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
  return a * simde_float32x2_to_private(b).values[lane];
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmulxs_lane_f32(a, b, lane) vmulxs_lane_f32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulxs_lane_f32
  #define vmulxs_lane_f32(a, b, lane) simde_vmulxs_lane_f32(a, b, lane)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64_t
simde_vmulxd_lane_f64(simde_float64_t a, simde_float64x1_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 0) {
  return a * simde_float64x1_to_private(b).values[lane];
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmulxd_lane_f64(a, b, lane) vmulxd_lane_f64((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulxd_lane_f64
  #define vmulxd_lane_f64(a, b, lane) simde_vmulxd_lane_f64(a, b, lane)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16_t
simde_vmulxh_laneq_f16(simde_float16_t a, simde_float16x8_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  return simde_float16_from_float32(
      simde_float16_to_float32(a) *
      simde_float16_to_float32(simde_float16x8_to_private(b).values[lane]));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vmulxh_laneq_f16(a, b, lane) vmulxh_laneq_f16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulxh_laneq_f16
  #define vmulxh_laneq_f16(a, b, lane) simde_vmulxh_laneq_f16(a, b, lane)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32_t
simde_vmulxs_laneq_f32(simde_float32_t a, simde_float32x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  return a * simde_float32x4_to_private(b).values[lane];
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmulxs_laneq_f32(a, b, lane) vmulxs_laneq_f32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulxs_laneq_f32
  #define vmulxs_laneq_f32(a, b, lane) simde_vmulxs_laneq_f32(a, b, lane)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64_t
simde_vmulxd_laneq_f64(simde_float64_t a, simde_float64x2_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
  return a * simde_float64x2_to_private(b).values[lane];
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmulxd_laneq_f64(a, b, lane) vmulxd_laneq_f64((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulxd_laneq_f64
  #define vmulxd_laneq_f64(a, b, lane) simde_vmulxd_laneq_f64(a, b, lane)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vmulx_lane_f16(simde_float16x4_t a, simde_float16x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_float16x4_private
    r_,
    a_ = simde_float16x4_to_private(a),
    b_ = simde_float16x4_to_private(b);
  simde_float32_t b_lane_ = simde_float16_to_float32(b_.values[lane]);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_float16_from_float32(
        simde_float16_to_float32(a_.values[i]) * b_lane_);
  }

  return simde_float16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vmulx_lane_f16(a, b, lane) vmulx_lane_f16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulx_lane_f16
  #define vmulx_lane_f16(a, b, lane) simde_vmulx_lane_f16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vmulx_lane_f32(simde_float32x2_t a, simde_float32x2_t b, const int lane)
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
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmulx_lane_f32(a, b, lane) vmulx_lane_f32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulx_lane_f32
  #define vmulx_lane_f32(a, b, lane) simde_vmulx_lane_f32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vmulx_lane_f64(simde_float64x1_t a, simde_float64x1_t b, const int lane)
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
  #define simde_vmulx_lane_f64(a, b, lane) vmulx_lane_f64((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulx_lane_f64
  #define vmulx_lane_f64(a, b, lane) simde_vmulx_lane_f64((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vmulxq_lane_f16(simde_float16x8_t a, simde_float16x4_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_float16x8_private
    r_,
    a_ = simde_float16x8_to_private(a);
  simde_float16x4_private b_ = simde_float16x4_to_private(b);
  simde_float32_t b_lane_ = simde_float16_to_float32(b_.values[lane]);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_float16_from_float32(
        simde_float16_to_float32(a_.values[i]) * b_lane_);
  }

  return simde_float16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vmulxq_lane_f16(a, b, lane) vmulxq_lane_f16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulxq_lane_f16
  #define vmulxq_lane_f16(a, b, lane) simde_vmulxq_lane_f16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vmulxq_lane_f32(simde_float32x4_t a, simde_float32x2_t b, const int lane)
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
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmulxq_lane_f32(a, b, lane) vmulxq_lane_f32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulxq_lane_f32
  #define vmulxq_lane_f32(a, b, lane) simde_vmulxq_lane_f32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vmulxq_lane_f64(simde_float64x2_t a, simde_float64x1_t b, const int lane)
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
  #define simde_vmulxq_lane_f64(a, b, lane) vmulxq_lane_f64((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulxq_lane_f64
  #define vmulxq_lane_f64(a, b, lane) simde_vmulxq_lane_f64((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vmulxq_laneq_f16(simde_float16x8_t a, simde_float16x8_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_float16x8_private
    r_,
    a_ = simde_float16x8_to_private(a),
    b_ = simde_float16x8_to_private(b);
  simde_float32_t b_lane_ = simde_float16_to_float32(b_.values[lane]);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_float16_from_float32(
        simde_float16_to_float32(a_.values[i]) * b_lane_);
  }

  return simde_float16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vmulxq_laneq_f16(a, b, lane) vmulxq_laneq_f16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulxq_laneq_f16
  #define vmulxq_laneq_f16(a, b, lane) simde_vmulxq_laneq_f16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vmulxq_laneq_f32(simde_float32x4_t a, simde_float32x4_t b, const int lane)
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
  #define simde_vmulxq_laneq_f32(a, b, lane) vmulxq_laneq_f32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulxq_laneq_f32
  #define vmulxq_laneq_f32(a, b, lane) simde_vmulxq_laneq_f32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vmulxq_laneq_f64(simde_float64x2_t a, simde_float64x2_t b, const int lane)
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
  #define simde_vmulxq_laneq_f64(a, b, lane) vmulxq_laneq_f64((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulxq_laneq_f64
  #define vmulxq_laneq_f64(a, b, lane) simde_vmulxq_laneq_f64((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vmulx_laneq_f16(simde_float16x4_t a, simde_float16x8_t b, const int lane)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_float16x4_private
    r_,
    a_ = simde_float16x4_to_private(a);
  simde_float16x8_private b_ = simde_float16x8_to_private(b);
  simde_float32_t b_lane_ = simde_float16_to_float32(b_.values[lane]);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_float16_from_float32(
        simde_float16_to_float32(a_.values[i]) * b_lane_);
  }

  return simde_float16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
  #define simde_vmulx_laneq_f16(a, b, lane) vmulx_laneq_f16((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulx_laneq_f16
  #define vmulx_laneq_f16(a, b, lane) simde_vmulx_laneq_f16((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vmulx_laneq_f32(simde_float32x2_t a, simde_float32x4_t b, const int lane)
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
  #define simde_vmulx_laneq_f32(a, b, lane) vmulx_laneq_f32((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulx_laneq_f32
  #define vmulx_laneq_f32(a, b, lane) simde_vmulx_laneq_f32((a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vmulx_laneq_f64(simde_float64x1_t a, simde_float64x2_t b, const int lane)
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
  #define simde_vmulx_laneq_f64(a, b, lane) vmulx_laneq_f64((a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmulx_laneq_f64
  #define vmulx_laneq_f64(a, b, lane) simde_vmulx_laneq_f64((a), (b), (lane))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_MULX_LANE_H) */
