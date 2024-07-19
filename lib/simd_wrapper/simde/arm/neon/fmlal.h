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

#if !defined(SIMDE_ARM_NEON_FMLAL_H)
#define SIMDE_ARM_NEON_FMLAL_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vfmlal_low_f16(simde_float32x2_t r, simde_float16x4_t a, simde_float16x4_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && \
      defined(SIMDE_ARCH_ARM_FP16_FML)
    return vfmlal_low_f16(r, a, b);
  #else
    simde_float32x2_private
      ret_,
      r_ = simde_float32x2_to_private(r);
    simde_float16x4_private
      a_ = simde_float16x4_to_private(a),
      b_ = simde_float16x4_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(ret_.values) / sizeof(ret_.values[0])) ; i++) {
      ret_.values[i] = r_.values[i] +
        simde_float16_to_float32(a_.values[i]) * simde_float16_to_float32(b_.values[i]);
    }
    return simde_float32x2_from_private(ret_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vfmlal_low_f16
  #define vfmlal_low_f16(r, a, b) simde_vfmlal_low_f16((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vfmlalq_low_f16(simde_float32x4_t r, simde_float16x8_t a, simde_float16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && \
      defined(SIMDE_ARCH_ARM_FP16_FML)
    return vfmlalq_low_f16(r, a, b);
  #else
    simde_float32x4_private
      ret_,
      r_ = simde_float32x4_to_private(r);
    simde_float16x8_private
      a_ = simde_float16x8_to_private(a),
      b_ = simde_float16x8_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(ret_.values) / sizeof(ret_.values[0])) ; i++) {
      ret_.values[i] = r_.values[i] +
        simde_float16_to_float32(a_.values[i]) * simde_float16_to_float32(b_.values[i]);
    }
    return simde_float32x4_from_private(ret_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vfmlalq_low_f16
  #define vfmlalq_low_f16(r, a, b) simde_vfmlalq_low_f16((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vfmlal_high_f16(simde_float32x2_t r, simde_float16x4_t a, simde_float16x4_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && \
      defined(SIMDE_ARCH_ARM_FP16_FML)
    return vfmlal_high_f16(r, a, b);
  #else
    simde_float32x2_private
      ret_,
      r_ = simde_float32x2_to_private(r);
    simde_float16x4_private
      a_ = simde_float16x4_to_private(a),
      b_ = simde_float16x4_to_private(b);
    size_t high_offset = sizeof(a_.values) / sizeof(a_.values[0]) / 2;

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(ret_.values) / sizeof(ret_.values[0])) ; i++) {
      ret_.values[i] = r_.values[i] +
        simde_float16_to_float32(a_.values[i+high_offset]) * simde_float16_to_float32(b_.values[i+high_offset]);
    }
    return simde_float32x2_from_private(ret_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vfmlal_high_f16
  #define vfmlal_high_f16(r, a, b) simde_vfmlal_high_f16((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vfmlalq_high_f16(simde_float32x4_t r, simde_float16x8_t a, simde_float16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && \
      defined(SIMDE_ARCH_ARM_FP16_FML)
    return vfmlalq_high_f16(r, a, b);
  #else
    simde_float32x4_private
      ret_,
      r_ = simde_float32x4_to_private(r);
    simde_float16x8_private
      a_ = simde_float16x8_to_private(a),
      b_ = simde_float16x8_to_private(b);
    size_t high_offset = sizeof(a_.values) / sizeof(a_.values[0]) / 2;

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(ret_.values) / sizeof(ret_.values[0])) ; i++) {
      ret_.values[i] = r_.values[i] +
        simde_float16_to_float32(a_.values[i+high_offset]) * simde_float16_to_float32(b_.values[i+high_offset]);
    }
    return simde_float32x4_from_private(ret_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vfmlalq_high_f16
  #define vfmlalq_high_f16(r, a, b) simde_vfmlalq_high_f16((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vfmlal_lane_low_f16(simde_float32x2_t r, simde_float16x4_t a, simde_float16x4_t b, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_float32x2_private
    ret_,
    r_ = simde_float32x2_to_private(r);
  simde_float16x4_private
    a_ = simde_float16x4_to_private(a),
    b_ = simde_float16x4_to_private(b);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(ret_.values) / sizeof(ret_.values[0])) ; i++) {
    ret_.values[i] = r_.values[i] +
      simde_float16_to_float32(a_.values[i]) * simde_float16_to_float32(b_.values[lane]);
  }
  return simde_float32x2_from_private(ret_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && \
    defined(SIMDE_ARCH_ARM_FP16_FML)
  #define simde_vfmlal_lane_low_f16(r, a, b, lane) vfmlal_lane_low_f16((r), (a), (b), (lane));
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vfmlal_lane_low_f16
  #define vfmlal_lane_low_f16(r, a, b, lane) simde_vfmlal_lane_low_f16((r), (a), (b), (lane));
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vfmlal_laneq_low_f16(simde_float32x2_t r, simde_float16x4_t a, simde_float16x8_t b, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_float32x2_private
    ret_,
    r_ = simde_float32x2_to_private(r);
  simde_float16x4_private
    a_ = simde_float16x4_to_private(a);
  simde_float16x8_private
    b_ = simde_float16x8_to_private(b);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(ret_.values) / sizeof(ret_.values[0])) ; i++) {
    ret_.values[i] = r_.values[i] +
      simde_float16_to_float32(a_.values[i]) * simde_float16_to_float32(b_.values[lane]);
  }
  return simde_float32x2_from_private(ret_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && \
    defined(SIMDE_ARCH_ARM_FP16_FML)
  #define simde_vfmlal_laneq_low_f16(r, a, b, lane) vfmlal_laneq_low_f16((r), (a), (b), (lane));
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vfmlal_laneq_low_f16
  #define vfmlal_laneq_low_f16(r, a, b, lane) simde_vfmlal_laneq_low_f16((r), (a), (b), (lane));
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vfmlalq_lane_low_f16(simde_float32x4_t r, simde_float16x8_t a, simde_float16x4_t b, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_float32x4_private
    ret_,
    r_ = simde_float32x4_to_private(r);
  simde_float16x4_private
    b_ = simde_float16x4_to_private(b);
  simde_float16x8_private
    a_ = simde_float16x8_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(ret_.values) / sizeof(ret_.values[0])) ; i++) {
    ret_.values[i] = r_.values[i] +
      simde_float16_to_float32(a_.values[i]) * simde_float16_to_float32(b_.values[lane]);
  }
  return simde_float32x4_from_private(ret_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && \
    defined(SIMDE_ARCH_ARM_FP16_FML)
  #define simde_vfmlalq_lane_low_f16(r, a, b, lane) vfmlalq_lane_low_f16((r), (a), (b), (lane));
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vfmlalq_lane_low_f16
  #define vfmlalq_lane_low_f16(r, a, b, lane) simde_vfmlalq_lane_low_f16((r), (a), (b), (lane));
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vfmlalq_laneq_low_f16(simde_float32x4_t r, simde_float16x8_t a, simde_float16x8_t b, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_float32x4_private
    ret_,
    r_ = simde_float32x4_to_private(r);
  simde_float16x8_private
    a_ = simde_float16x8_to_private(a),
    b_ = simde_float16x8_to_private(b);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(ret_.values) / sizeof(ret_.values[0])) ; i++) {
    ret_.values[i] = r_.values[i] +
      simde_float16_to_float32(a_.values[i]) * simde_float16_to_float32(b_.values[lane]);
  }
  return simde_float32x4_from_private(ret_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && \
    defined(SIMDE_ARCH_ARM_FP16_FML)
  #define simde_vfmlalq_laneq_low_f16(r, a, b, lane) vfmlalq_laneq_low_f16((r), (a), (b), (lane));
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vfmlalq_laneq_low_f16
  #define vfmlalq_laneq_low_f16(r, a, b, lane) simde_vfmlalq_laneq_low_f16((r), (a), (b), (lane));
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vfmlal_lane_high_f16(simde_float32x2_t r, simde_float16x4_t a, simde_float16x4_t b, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_float32x2_private
    ret_,
    r_ = simde_float32x2_to_private(r);
  simde_float16x4_private
    a_ = simde_float16x4_to_private(a),
    b_ = simde_float16x4_to_private(b);
  size_t high_offset = sizeof(a_.values) / sizeof(a_.values[0]) / 2;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(ret_.values) / sizeof(ret_.values[0])) ; i++) {
    ret_.values[i] = r_.values[i] +
      simde_float16_to_float32(a_.values[i+high_offset]) * simde_float16_to_float32(b_.values[lane]);
  }
  return simde_float32x2_from_private(ret_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && \
    defined(SIMDE_ARCH_ARM_FP16_FML)
  #define simde_vfmlal_lane_high_f16(r, a, b, lane) vfmlal_lane_high_f16((r), (a), (b), (lane));
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vfmlal_lane_high_f16
  #define vfmlal_lane_high_f16(r, a, b, lane) simde_vfmlal_lane_high_f16((r), (a), (b), (lane));
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vfmlal_laneq_high_f16(simde_float32x2_t r, simde_float16x4_t a, simde_float16x8_t b, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_float32x2_private
    ret_,
    r_ = simde_float32x2_to_private(r);
  simde_float16x4_private
    a_ = simde_float16x4_to_private(a);
  simde_float16x8_private
    b_ = simde_float16x8_to_private(b);
  size_t high_offset = sizeof(a_.values) / sizeof(a_.values[0]) / 2;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(ret_.values) / sizeof(ret_.values[0])) ; i++) {
    ret_.values[i] = r_.values[i] +
      simde_float16_to_float32(a_.values[i+high_offset]) * simde_float16_to_float32(b_.values[lane]);
  }
  return simde_float32x2_from_private(ret_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && \
    defined(SIMDE_ARCH_ARM_FP16_FML)
  #define simde_vfmlal_laneq_high_f16(r, a, b, lane) vfmlal_laneq_high_f16((r), (a), (b), (lane));
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vfmlal_laneq_high_f16
  #define vfmlal_laneq_high_f16(r, a, b, lane) simde_vfmlal_laneq_high_f16((r), (a), (b), (lane));
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vfmlalq_lane_high_f16(simde_float32x4_t r, simde_float16x8_t a, simde_float16x4_t b, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_float32x4_private
    ret_,
    r_ = simde_float32x4_to_private(r);
  simde_float16x4_private
    b_ = simde_float16x4_to_private(b);
  simde_float16x8_private
    a_ = simde_float16x8_to_private(a);
  size_t high_offset = sizeof(a_.values) / sizeof(a_.values[0]) / 2;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(ret_.values) / sizeof(ret_.values[0])) ; i++) {
    ret_.values[i] = r_.values[i] +
      simde_float16_to_float32(a_.values[i+high_offset]) * simde_float16_to_float32(b_.values[lane]);
  }
  return simde_float32x4_from_private(ret_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && \
    defined(SIMDE_ARCH_ARM_FP16_FML)
  #define simde_vfmlalq_lane_high_f16(r, a, b, lane) vfmlalq_lane_high_f16((r), (a), (b), (lane));
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vfmlalq_lane_high_f16
  #define vfmlalq_lane_high_f16(r, a, b, lane) simde_vfmlalq_lane_high_f16((r), (a), (b), (lane));
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vfmlalq_laneq_high_f16(simde_float32x4_t r, simde_float16x8_t a, simde_float16x8_t b, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_float32x4_private
    ret_,
    r_ = simde_float32x4_to_private(r);
  simde_float16x8_private
    a_ = simde_float16x8_to_private(a),
    b_ = simde_float16x8_to_private(b);
  size_t high_offset = sizeof(a_.values) / sizeof(a_.values[0]) / 2;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(ret_.values) / sizeof(ret_.values[0])) ; i++) {
    ret_.values[i] = r_.values[i] +
      simde_float16_to_float32(a_.values[i+high_offset]) * simde_float16_to_float32(b_.values[lane]);
  }
  return simde_float32x4_from_private(ret_);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && \
    defined(SIMDE_ARCH_ARM_FP16_FML)
  #define simde_vfmlalq_laneq_high_f16(r, a, b, lane) vfmlalq_laneq_high_f16((r), (a), (b), (lane));
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vfmlalq_laneq_high_f16
  #define vfmlalq_laneq_high_f16(r, a, b, lane) simde_vfmlalq_laneq_high_f16((r), (a), (b), (lane));
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vbfmlalbq_f32(simde_float32x4_t r, simde_bfloat16x8_t a, simde_bfloat16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vbfmlalbq_f32(r, a, b);
  #else
    simde_float32x4_private
      ret,
      r_ = simde_float32x4_to_private(r);
    simde_bfloat16x8_private
      a_ = simde_bfloat16x8_to_private(a),
      b_ = simde_bfloat16x8_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(ret.values) / sizeof(ret.values[0])) ; i++) {
      ret.values[i] = r_.values[i] +
        simde_bfloat16_to_float32(a_.values[i * 2]) * simde_bfloat16_to_float32(b_.values[i * 2]);
    }
    return simde_float32x4_from_private(ret);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vbfmlalbq_f32
  #define vbfmlalbq_f32(r, a, b) simde_vbfmlalbq_f32((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vbfmlaltq_f32(simde_float32x4_t r, simde_bfloat16x8_t a, simde_bfloat16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vbfmlaltq_f32(r, a, b);
  #else
    simde_float32x4_private
      ret,
      r_ = simde_float32x4_to_private(r);
    simde_bfloat16x8_private
      a_ = simde_bfloat16x8_to_private(a),
      b_ = simde_bfloat16x8_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(ret.values) / sizeof(ret.values[0])) ; i++) {
      ret.values[i] = r_.values[i] +
        simde_bfloat16_to_float32(a_.values[i * 2 + 1]) * simde_bfloat16_to_float32(b_.values[i * 2 + 1]);
    }
    return simde_float32x4_from_private(ret);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vbfmlaltq_f32
  #define vbfmlaltq_f32(r, a, b) simde_vbfmlaltq_f32((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vbfmlalbq_lane_f32(simde_float32x4_t r, simde_bfloat16x8_t a, simde_bfloat16x4_t b, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_float32x4_private
    ret,
    r_ = simde_float32x4_to_private(r);
  simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
  simde_bfloat16x4_private b_ = simde_bfloat16x4_to_private(b);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(ret.values) / sizeof(ret.values[0])) ; i++) {
    ret.values[i] = r_.values[i] +
      simde_bfloat16_to_float32(a_.values[i * 2]) * simde_bfloat16_to_float32(b_.values[lane]);
  }
  return simde_float32x4_from_private(ret);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
  #define simde_vbfmlalbq_lane_f32(r, a, b, lane) vbfmlalbq_lane_f32((r), (a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vbfmlalbq_lane_f32
  #define vbfmlalbq_lane_f32(r, a, b, lane) simde_vbfmlalbq_lane_f32((r), (a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vbfmlalbq_laneq_f32(simde_float32x4_t r, simde_bfloat16x8_t a, simde_bfloat16x8_t b, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_float32x4_private
    ret,
    r_ = simde_float32x4_to_private(r);
  simde_bfloat16x8_private
    a_ = simde_bfloat16x8_to_private(a),
    b_ = simde_bfloat16x8_to_private(b);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(ret.values) / sizeof(ret.values[0])) ; i++) {
    ret.values[i] = r_.values[i] +
      simde_bfloat16_to_float32(a_.values[i * 2]) * simde_bfloat16_to_float32(b_.values[lane]);
  }
  return simde_float32x4_from_private(ret);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
  #define simde_vbfmlalbq_laneq_f32(r, a, b, lane) vbfmlalbq_laneq_f32((r), (a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vbfmlalbq_laneq_f32
  #define vbfmlalbq_laneq_f32(r, a, b, lane) simde_vbfmlalbq_laneq_f32((r), (a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vbfmlaltq_lane_f32(simde_float32x4_t r, simde_bfloat16x8_t a, simde_bfloat16x4_t b, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_float32x4_private
    ret,
    r_ = simde_float32x4_to_private(r);
  simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
  simde_bfloat16x4_private b_ = simde_bfloat16x4_to_private(b);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(ret.values) / sizeof(ret.values[0])) ; i++) {
    ret.values[i] = r_.values[i] +
      simde_bfloat16_to_float32(a_.values[i * 2 + 1]) * simde_bfloat16_to_float32(b_.values[lane]);
  }
  return simde_float32x4_from_private(ret);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
  #define simde_vbfmlaltq_lane_f32(r, a, b, lane) vbfmlaltq_lane_f32((r), (a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vbfmlaltq_lane_f32
  #define vbfmlaltq_lane_f32(r, a, b, lane) simde_vbfmlaltq_lane_f32((r), (a), (b), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vbfmlaltq_laneq_f32(simde_float32x4_t r, simde_bfloat16x8_t a, simde_bfloat16x8_t b, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_float32x4_private
    ret,
    r_ = simde_float32x4_to_private(r);
  simde_bfloat16x8_private
    a_ = simde_bfloat16x8_to_private(a),
    b_ = simde_bfloat16x8_to_private(b);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(ret.values) / sizeof(ret.values[0])) ; i++) {
    ret.values[i] = r_.values[i] +
      simde_bfloat16_to_float32(a_.values[i * 2 + 1]) * simde_bfloat16_to_float32(b_.values[lane]);
  }
  return simde_float32x4_from_private(ret);
}
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
  #define simde_vbfmlaltq_laneq_f32(r, a, b, lane) vbfmlaltq_laneq_f32((r), (a), (b), (lane))
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vbfmlaltq_laneq_f32
  #define vbfmlaltq_laneq_f32(r, a, b, lane) simde_vbfmlaltq_laneq_f32((r), (a), (b), (lane))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_FMLAL_H) */
