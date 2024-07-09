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

#if !defined(SIMDE_ARM_NEON_COPY_LANE_H)
#define SIMDE_ARM_NEON_COPY_LANE_H

#include "types.h"
#include "cvt.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vcopy_lane_s8(simde_int8x8_t a, const int lane1, simde_int8x8_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 7)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 7) {
  simde_int8x8_private
    b_ = simde_int8x8_to_private(b),
    r_ = simde_int8x8_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_int8x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_lane_s8(a, lane1, b, lane2) vcopy_lane_s8((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_lane_s8
  #define vcopy_lane_s8(a, lane1, b, lane2) simde_vcopy_lane_s8((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vcopy_lane_s16(simde_int16x4_t a, const int lane1, simde_int16x4_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 3)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 3) {
  simde_int16x4_private
    b_ = simde_int16x4_to_private(b),
    r_ = simde_int16x4_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_int16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_lane_s16(a, lane1, b, lane2) vcopy_lane_s16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_lane_s16
  #define vcopy_lane_s16(a, lane1, b, lane2) simde_vcopy_lane_s16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vcopy_lane_s32(simde_int32x2_t a, const int lane1, simde_int32x2_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 1)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 1) {
  simde_int32x2_private
    b_ = simde_int32x2_to_private(b),
    r_ = simde_int32x2_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_int32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_lane_s32(a, lane1, b, lane2) vcopy_lane_s32((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_lane_s32
  #define vcopy_lane_s32(a, lane1, b, lane2) simde_vcopy_lane_s32((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vcopy_lane_s64(simde_int64x1_t a, const int lane1, simde_int64x1_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 0)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 0) {
  simde_int64x1_private
    b_ = simde_int64x1_to_private(b),
    r_ = simde_int64x1_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_int64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_lane_s64(a, lane1, b, lane2) vcopy_lane_s64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_lane_s64
  #define vcopy_lane_s64(a, lane1, b, lane2) simde_vcopy_lane_s64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vcopy_lane_u8(simde_uint8x8_t a, const int lane1, simde_uint8x8_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 7)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 7) {
  simde_uint8x8_private
    b_ = simde_uint8x8_to_private(b),
    r_ = simde_uint8x8_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint8x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_lane_u8(a, lane1, b, lane2) vcopy_lane_u8((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_lane_u8
  #define vcopy_lane_u8(a, lane1, b, lane2) simde_vcopy_lane_u8((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vcopy_lane_u16(simde_uint16x4_t a, const int lane1, simde_uint16x4_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 3)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 3) {
  simde_uint16x4_private
    b_ = simde_uint16x4_to_private(b),
    r_ = simde_uint16x4_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_lane_u16(a, lane1, b, lane2) vcopy_lane_u16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_lane_u16
  #define vcopy_lane_u16(a, lane1, b, lane2) simde_vcopy_lane_u16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vcopy_lane_u32(simde_uint32x2_t a, const int lane1, simde_uint32x2_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 1)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 1) {
  simde_uint32x2_private
    b_ = simde_uint32x2_to_private(b),
    r_ = simde_uint32x2_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_lane_u32(a, lane1, b, lane2) vcopy_lane_u32((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_lane_u32
  #define vcopy_lane_u32(a, lane1, b, lane2) simde_vcopy_lane_u32((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vcopy_lane_u64(simde_uint64x1_t a, const int lane1, simde_uint64x1_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 0)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 0) {
  simde_uint64x1_private
    b_ = simde_uint64x1_to_private(b),
    r_ = simde_uint64x1_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_lane_u64(a, lane1, b, lane2) vcopy_lane_u64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_lane_u64
  #define vcopy_lane_u64(a, lane1, b, lane2) simde_vcopy_lane_u64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vcopy_lane_f32(simde_float32x2_t a, const int lane1, simde_float32x2_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 1)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 1) {
  simde_float32x2_private
    b_ = simde_float32x2_to_private(b),
    r_ = simde_float32x2_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_float32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_lane_f32(a, lane1, b, lane2) vcopy_lane_f32((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_lane_f32
  #define vcopy_lane_f32(a, lane1, b, lane2) simde_vcopy_lane_f32((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vcopy_lane_f64(simde_float64x1_t a, const int lane1, simde_float64x1_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 0)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 0) {
  simde_float64x1_private
    b_ = simde_float64x1_to_private(b),
    r_ = simde_float64x1_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_float64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_lane_f64(a, lane1, b, lane2) vcopy_lane_f64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_lane_f64
  #define vcopy_lane_f64(a, lane1, b, lane2) simde_vcopy_lane_f64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vcopy_laneq_s8(simde_int8x8_t a, const int lane1, simde_int8x16_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 7)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 15) {
  simde_int8x8_private
    r_ = simde_int8x8_to_private(a);
  simde_int8x16_private
    b_ = simde_int8x16_to_private(b);

  r_.values[lane1] = b_.values[lane2];
  return simde_int8x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_laneq_s8(a, lane1, b, lane2) vcopy_laneq_s8((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_laneq_s8
  #define vcopy_laneq_s8(a, lane1, b, lane2) simde_vcopy_laneq_s8((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vcopy_laneq_s16(simde_int16x4_t a, const int lane1, simde_int16x8_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 3)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 7) {
  simde_int16x4_private
    r_ = simde_int16x4_to_private(a);
  simde_int16x8_private
    b_ = simde_int16x8_to_private(b);

  r_.values[lane1] = b_.values[lane2];
  return simde_int16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_laneq_s16(a, lane1, b, lane2) vcopy_laneq_s16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_laneq_s16
  #define vcopy_laneq_s16(a, lane1, b, lane2) simde_vcopy_laneq_s16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vcopy_laneq_s32(simde_int32x2_t a, const int lane1, simde_int32x4_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 1)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 3) {
  simde_int32x2_private
    r_ = simde_int32x2_to_private(a);
  simde_int32x4_private
    b_ = simde_int32x4_to_private(b);

  r_.values[lane1] = b_.values[lane2];
  return simde_int32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_laneq_s32(a, lane1, b, lane2) vcopy_laneq_s32((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_laneq_s32
  #define vcopy_laneq_s32(a, lane1, b, lane2) simde_vcopy_laneq_s32((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vcopy_laneq_s64(simde_int64x1_t a, const int lane1, simde_int64x2_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 0)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 1) {
  simde_int64x1_private
    r_ = simde_int64x1_to_private(a);
  simde_int64x2_private
    b_ = simde_int64x2_to_private(b);

  r_.values[lane1] = b_.values[lane2];
  return simde_int64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_laneq_s64(a, lane1, b, lane2) vcopy_laneq_s64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_laneq_s64
  #define vcopy_laneq_s64(a, lane1, b, lane2) simde_vcopy_laneq_s64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vcopy_laneq_u8(simde_uint8x8_t a, const int lane1, simde_uint8x16_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 7)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 15) {
  simde_uint8x8_private
    r_ = simde_uint8x8_to_private(a);
  simde_uint8x16_private
    b_ = simde_uint8x16_to_private(b);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint8x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_laneq_u8(a, lane1, b, lane2) vcopy_laneq_u8((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_laneq_u8
  #define vcopy_laneq_u8(a, lane1, b, lane2) simde_vcopy_laneq_u8((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vcopy_laneq_u16(simde_uint16x4_t a, const int lane1, simde_uint16x8_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 3)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 7) {
  simde_uint16x4_private
    r_ = simde_uint16x4_to_private(a);
  simde_uint16x8_private
    b_ = simde_uint16x8_to_private(b);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_laneq_u16(a, lane1, b, lane2) vcopy_laneq_u16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_laneq_u16
  #define vcopy_laneq_u16(a, lane1, b, lane2) simde_vcopy_laneq_u16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vcopy_laneq_u32(simde_uint32x2_t a, const int lane1, simde_uint32x4_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 1)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 3) {
  simde_uint32x2_private
    r_ = simde_uint32x2_to_private(a);
  simde_uint32x4_private
    b_ = simde_uint32x4_to_private(b);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_laneq_u32(a, lane1, b, lane2) vcopy_laneq_u32((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_laneq_u32
  #define vcopy_laneq_u32(a, lane1, b, lane2) simde_vcopy_laneq_u32((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vcopy_laneq_u64(simde_uint64x1_t a, const int lane1, simde_uint64x2_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 0)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 1) {
  simde_uint64x1_private
    r_ = simde_uint64x1_to_private(a);
  simde_uint64x2_private
    b_ = simde_uint64x2_to_private(b);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_laneq_u64(a, lane1, b, lane2) vcopy_laneq_u64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_laneq_u64
  #define vcopy_laneq_u64(a, lane1, b, lane2) simde_vcopy_laneq_u64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vcopy_laneq_f32(simde_float32x2_t a, const int lane1, simde_float32x4_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 1)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 3) {
  simde_float32x2_private
    r_ = simde_float32x2_to_private(a);
  simde_float32x4_private
    b_ = simde_float32x4_to_private(b);

  r_.values[lane1] = b_.values[lane2];
  return simde_float32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_laneq_f32(a, lane1, b, lane2) vcopy_laneq_f32((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_laneq_f32
  #define vcopy_laneq_f32(a, lane1, b, lane2) simde_vcopy_laneq_f32((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vcopy_laneq_f64(simde_float64x1_t a, const int lane1, simde_float64x2_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 0)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 1) {
  simde_float64x1_private
    r_ = simde_float64x1_to_private(a);
  simde_float64x2_private
    b_ = simde_float64x2_to_private(b);

  r_.values[lane1] = b_.values[lane2];
  return simde_float64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopy_laneq_f64(a, lane1, b, lane2) vcopy_laneq_f64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_laneq_f64
  #define vcopy_laneq_f64(a, lane1, b, lane2) simde_vcopy_laneq_f64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vcopyq_lane_s8(simde_int8x16_t a, const int lane1, simde_int8x8_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 15)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 7) {
  simde_int8x8_private
    b_ = simde_int8x8_to_private(b);
  simde_int8x16_private
    r_ = simde_int8x16_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_int8x16_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_lane_s8(a, lane1, b, lane2) vcopyq_lane_s8((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_lane_s8
  #define vcopyq_lane_s8(a, lane1, b, lane2) simde_vcopyq_lane_s8((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vcopyq_lane_s16(simde_int16x8_t a, const int lane1, simde_int16x4_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 7)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 3) {
  simde_int16x4_private
    b_ = simde_int16x4_to_private(b);
  simde_int16x8_private
    r_ = simde_int16x8_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_int16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_lane_s16(a, lane1, b, lane2) vcopyq_lane_s16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_lane_s16
  #define vcopyq_lane_s16(a, lane1, b, lane2) simde_vcopyq_lane_s16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vcopyq_lane_s32(simde_int32x4_t a, const int lane1, simde_int32x2_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 3)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 1) {
  simde_int32x2_private
    b_ = simde_int32x2_to_private(b);
  simde_int32x4_private
    r_ = simde_int32x4_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_int32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_lane_s32(a, lane1, b, lane2) vcopyq_lane_s32((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_lane_s32
  #define vcopyq_lane_s32(a, lane1, b, lane2) simde_vcopyq_lane_s32((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vcopyq_lane_s64(simde_int64x2_t a, const int lane1, simde_int64x1_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 1)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 0) {
  simde_int64x1_private
    b_ = simde_int64x1_to_private(b);
  simde_int64x2_private
    r_ = simde_int64x2_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_int64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_lane_s64(a, lane1, b, lane2) vcopyq_lane_s64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_lane_s64
  #define vcopyq_lane_s64(a, lane1, b, lane2) simde_vcopyq_lane_s64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vcopyq_lane_u8(simde_uint8x16_t a, const int lane1, simde_uint8x8_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 15)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 7) {
  simde_uint8x8_private
    b_ = simde_uint8x8_to_private(b);
  simde_uint8x16_private
    r_ = simde_uint8x16_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint8x16_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_lane_u8(a, lane1, b, lane2) vcopyq_lane_u8((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_lane_u8
  #define vcopyq_lane_u8(a, lane1, b, lane2) simde_vcopyq_lane_u8((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vcopyq_lane_u16(simde_uint16x8_t a, const int lane1, simde_uint16x4_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 7)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 3) {
  simde_uint16x4_private
    b_ = simde_uint16x4_to_private(b);
  simde_uint16x8_private
    r_ = simde_uint16x8_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_lane_u16(a, lane1, b, lane2) vcopyq_lane_u16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_lane_u16
  #define vcopyq_lane_u16(a, lane1, b, lane2) simde_vcopyq_lane_u16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vcopyq_lane_u32(simde_uint32x4_t a, const int lane1, simde_uint32x2_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 3)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 1) {
  simde_uint32x2_private
    b_ = simde_uint32x2_to_private(b);
  simde_uint32x4_private
    r_ = simde_uint32x4_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_lane_u32(a, lane1, b, lane2) vcopyq_lane_u32((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_lane_u32
  #define vcopyq_lane_u32(a, lane1, b, lane2) simde_vcopyq_lane_u32((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vcopyq_lane_u64(simde_uint64x2_t a, const int lane1, simde_uint64x1_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 1)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 0) {
  simde_uint64x1_private
    b_ = simde_uint64x1_to_private(b);
  simde_uint64x2_private
    r_ = simde_uint64x2_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_lane_u64(a, lane1, b, lane2) vcopyq_lane_u64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_lane_u64
  #define vcopyq_lane_u64(a, lane1, b, lane2) simde_vcopyq_lane_u64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vcopyq_lane_f32(simde_float32x4_t a, const int lane1, simde_float32x2_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 3)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 1) {
  simde_float32x2_private
    b_ = simde_float32x2_to_private(b);
  simde_float32x4_private
    r_ = simde_float32x4_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_float32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_lane_f32(a, lane1, b, lane2) vcopyq_lane_f32((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_lane_f32
  #define vcopyq_lane_f32(a, lane1, b, lane2) simde_vcopyq_lane_f32((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vcopyq_lane_f64(simde_float64x2_t a, const int lane1, simde_float64x1_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 1)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 0) {
  simde_float64x1_private
    b_ = simde_float64x1_to_private(b);
  simde_float64x2_private
    r_ = simde_float64x2_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_float64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_lane_f64(a, lane1, b, lane2) vcopyq_lane_f64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_lane_f64
  #define vcopyq_lane_f64(a, lane1, b, lane2) simde_vcopyq_lane_f64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vcopyq_laneq_s8(simde_int8x16_t a, const int lane1, simde_int8x16_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 15)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 15) {
  simde_int8x16_private
    b_ = simde_int8x16_to_private(b),
    r_ = simde_int8x16_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_int8x16_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_laneq_s8(a, lane1, b, lane2) vcopyq_laneq_s8((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_laneq_s8
  #define vcopyq_laneq_s8(a, lane1, b, lane2) simde_vcopyq_laneq_s8((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vcopyq_laneq_s16(simde_int16x8_t a, const int lane1, simde_int16x8_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 7)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 7) {
  simde_int16x8_private
    b_ = simde_int16x8_to_private(b),
    r_ = simde_int16x8_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_int16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_laneq_s16(a, lane1, b, lane2) vcopyq_laneq_s16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_laneq_s16
  #define vcopyq_laneq_s16(a, lane1, b, lane2) simde_vcopyq_laneq_s16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vcopyq_laneq_s32(simde_int32x4_t a, const int lane1, simde_int32x4_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 3)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 3) {
  simde_int32x4_private
    b_ = simde_int32x4_to_private(b),
    r_ = simde_int32x4_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_int32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_laneq_s32(a, lane1, b, lane2) vcopyq_laneq_s32((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_laneq_s32
  #define vcopyq_laneq_s32(a, lane1, b, lane2) simde_vcopyq_laneq_s32((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vcopyq_laneq_s64(simde_int64x2_t a, const int lane1, simde_int64x2_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 1)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 1) {
  simde_int64x2_private
    b_ = simde_int64x2_to_private(b),
    r_ = simde_int64x2_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_int64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_laneq_s64(a, lane1, b, lane2) vcopyq_laneq_s64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_laneq_s64
  #define vcopyq_laneq_s64(a, lane1, b, lane2) simde_vcopyq_laneq_s64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vcopyq_laneq_u8(simde_uint8x16_t a, const int lane1, simde_uint8x16_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 15)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 15) {
  simde_uint8x16_private
    b_ = simde_uint8x16_to_private(b),
    r_ = simde_uint8x16_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint8x16_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_laneq_u8(a, lane1, b, lane2) vcopyq_laneq_u8((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_laneq_u8
  #define vcopyq_laneq_u8(a, lane1, b, lane2) simde_vcopyq_laneq_u8((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vcopyq_laneq_u16(simde_uint16x8_t a, const int lane1, simde_uint16x8_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 7)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 7) {
  simde_uint16x8_private
    b_ = simde_uint16x8_to_private(b),
    r_ = simde_uint16x8_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_laneq_u16(a, lane1, b, lane2) vcopyq_laneq_u16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_laneq_u16
  #define vcopyq_laneq_u16(a, lane1, b, lane2) simde_vcopyq_laneq_u16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vcopyq_laneq_u32(simde_uint32x4_t a, const int lane1, simde_uint32x4_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 3)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 3) {
  simde_uint32x4_private
    b_ = simde_uint32x4_to_private(b),
    r_ = simde_uint32x4_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_laneq_u32(a, lane1, b, lane2) vcopyq_laneq_u32((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_laneq_u32
  #define vcopyq_laneq_u32(a, lane1, b, lane2) simde_vcopyq_laneq_u32((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vcopyq_laneq_u64(simde_uint64x2_t a, const int lane1, simde_uint64x2_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 1)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 1) {
  simde_uint64x2_private
    b_ = simde_uint64x2_to_private(b),
    r_ = simde_uint64x2_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_uint64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_laneq_u64(a, lane1, b, lane2) vcopyq_laneq_u64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_laneq_u64
  #define vcopyq_laneq_u64(a, lane1, b, lane2) simde_vcopyq_laneq_u64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vcopyq_laneq_f32(simde_float32x4_t a, const int lane1, simde_float32x4_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 3)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 3) {
  simde_float32x4_private
    b_ = simde_float32x4_to_private(b),
    r_ = simde_float32x4_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_float32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_laneq_f32(a, lane1, b, lane2) vcopyq_laneq_f32((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_laneq_f32
  #define vcopyq_laneq_f32(a, lane1, b, lane2) simde_vcopyq_laneq_f32((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vcopyq_laneq_f64(simde_float64x2_t a, const int lane1, simde_float64x2_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 1)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 1) {
  simde_float64x2_private
    b_ = simde_float64x2_to_private(b),
    r_ = simde_float64x2_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_float64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vcopyq_laneq_f64(a, lane1, b, lane2) vcopyq_laneq_f64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_laneq_f64
  #define vcopyq_laneq_f64(a, lane1, b, lane2) simde_vcopyq_laneq_f64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vcopy_lane_p8(simde_poly8x8_t a, const int lane1, simde_poly8x8_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 7)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 7) {
  simde_poly8x8_private
    b_ = simde_poly8x8_to_private(b),
    r_ = simde_poly8x8_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_poly8x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71362)
  #define simde_vcopy_lane_p8(a, lane1, b, lane2) vcopy_lane_p8((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_lane_p8
  #define vcopy_lane_p8(a, lane1, b, lane2) simde_vcopy_lane_p8((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vcopy_lane_p16(simde_poly16x4_t a, const int lane1, simde_poly16x4_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 3)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 3) {
  simde_poly16x4_private
    b_ = simde_poly16x4_to_private(b),
    r_ = simde_poly16x4_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_poly16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71362)
  #define simde_vcopy_lane_p16(a, lane1, b, lane2) vcopy_lane_p16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_lane_p16
  #define vcopy_lane_p16(a, lane1, b, lane2) simde_vcopy_lane_p16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vcopy_lane_p64(simde_poly64x1_t a, const int lane1, simde_poly64x1_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 0)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 0) {
  simde_poly64x1_private
    b_ = simde_poly64x1_to_private(b),
    r_ = simde_poly64x1_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_poly64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71362)
  #define simde_vcopy_lane_p64(a, lane1, b, lane2) vcopy_lane_p64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_lane_p64
  #define vcopy_lane_p64(a, lane1, b, lane2) simde_vcopy_lane_p64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vcopy_laneq_p8(simde_poly8x8_t a, const int lane1, simde_poly8x16_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 7)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 15) {
  simde_poly8x8_private
    r_ = simde_poly8x8_to_private(a);
  simde_poly8x16_private
    b_ = simde_poly8x16_to_private(b);

  r_.values[lane1] = b_.values[lane2];
  return simde_poly8x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71362)
  #define simde_vcopy_laneq_p8(a, lane1, b, lane2) vcopy_laneq_p8((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_laneq_p8
  #define vcopy_laneq_p8(a, lane1, b, lane2) simde_vcopy_laneq_p8((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vcopy_laneq_p16(simde_poly16x4_t a, const int lane1, simde_poly16x8_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 3)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 7) {
  simde_poly16x4_private
    r_ = simde_poly16x4_to_private(a);
  simde_poly16x8_private
    b_ = simde_poly16x8_to_private(b);

  r_.values[lane1] = b_.values[lane2];
  return simde_poly16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71362)
  #define simde_vcopy_laneq_p16(a, lane1, b, lane2) vcopy_laneq_p16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_laneq_p16
  #define vcopy_laneq_p16(a, lane1, b, lane2) simde_vcopy_laneq_p16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vcopy_laneq_p64(simde_poly64x1_t a, const int lane1, simde_poly64x2_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 0)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 1) {
  simde_poly64x1_private
    r_ = simde_poly64x1_to_private(a);
  simde_poly64x2_private
    b_ = simde_poly64x2_to_private(b);

  r_.values[lane1] = b_.values[lane2];
  return simde_poly64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71362)
  #define simde_vcopy_laneq_p64(a, lane1, b, lane2) vcopy_laneq_p64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_laneq_p64
  #define vcopy_laneq_p64(a, lane1, b, lane2) simde_vcopy_laneq_p64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vcopyq_lane_p8(simde_poly8x16_t a, const int lane1, simde_poly8x8_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 15)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 7) {
  simde_poly8x8_private
    b_ = simde_poly8x8_to_private(b);
  simde_poly8x16_private
    r_ = simde_poly8x16_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_poly8x16_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71362)
  #define simde_vcopyq_lane_p8(a, lane1, b, lane2) vcopyq_lane_p8((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_lane_p8
  #define vcopyq_lane_p8(a, lane1, b, lane2) simde_vcopyq_lane_p8((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vcopyq_lane_p16(simde_poly16x8_t a, const int lane1, simde_poly16x4_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 7)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 3) {
  simde_poly16x4_private
    b_ = simde_poly16x4_to_private(b);
  simde_poly16x8_private
    r_ = simde_poly16x8_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_poly16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71362)
  #define simde_vcopyq_lane_p16(a, lane1, b, lane2) vcopyq_lane_p16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_lane_p16
  #define vcopyq_lane_p16(a, lane1, b, lane2) simde_vcopyq_lane_p16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vcopyq_lane_p64(simde_poly64x2_t a, const int lane1, simde_poly64x1_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 1)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 0) {
  simde_poly64x1_private
    b_ = simde_poly64x1_to_private(b);
  simde_poly64x2_private
    r_ = simde_poly64x2_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_poly64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71362)
  #define simde_vcopyq_lane_p64(a, lane1, b, lane2) vcopyq_lane_p64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_lane_p64
  #define vcopyq_lane_p64(a, lane1, b, lane2) simde_vcopyq_lane_p64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vcopyq_laneq_p8(simde_poly8x16_t a, const int lane1, simde_poly8x16_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 15)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 15) {
  simde_poly8x16_private
    b_ = simde_poly8x16_to_private(b),
    r_ = simde_poly8x16_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_poly8x16_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71362)
  #define simde_vcopyq_laneq_p8(a, lane1, b, lane2) vcopyq_laneq_p8((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_laneq_p8
  #define vcopyq_laneq_p8(a, lane1, b, lane2) simde_vcopyq_laneq_p8((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vcopyq_laneq_p16(simde_poly16x8_t a, const int lane1, simde_poly16x8_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 7)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 7) {
  simde_poly16x8_private
    b_ = simde_poly16x8_to_private(b),
    r_ = simde_poly16x8_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_poly16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71362)
  #define simde_vcopyq_laneq_p16(a, lane1, b, lane2) vcopyq_laneq_p16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_laneq_p16
  #define vcopyq_laneq_p16(a, lane1, b, lane2) simde_vcopyq_laneq_p16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vcopyq_laneq_p64(simde_poly64x2_t a, const int lane1, simde_poly64x2_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 1)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 1) {
  simde_poly64x2_private
    b_ = simde_poly64x2_to_private(b),
    r_ = simde_poly64x2_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_poly64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71362)
  #define simde_vcopyq_laneq_p64(a, lane1, b, lane2) vcopyq_laneq_p64((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_laneq_p64
  #define vcopyq_laneq_p64(a, lane1, b, lane2) simde_vcopyq_laneq_p64((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vcopy_lane_bf16(simde_bfloat16x4_t a, const int lane1, simde_bfloat16x4_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 3)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 3) {
  simde_bfloat16x4_private
    b_ = simde_bfloat16x4_to_private(b),
    r_ = simde_bfloat16x4_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_bfloat16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
  #define simde_vcopy_lane_bf16(a, lane1, b, lane2) vcopy_lane_bf16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_lane_bf16
  #define vcopy_lane_bf16(a, lane1, b, lane2) simde_vcopy_lane_bf16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vcopy_laneq_bf16(simde_bfloat16x4_t a, const int lane1, simde_bfloat16x8_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 3)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 7) {
  simde_bfloat16x4_private r_ = simde_bfloat16x4_to_private(a);
  simde_bfloat16x8_private b_ = simde_bfloat16x8_to_private(b);

  r_.values[lane1] = b_.values[lane2];
  return simde_bfloat16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
  #define simde_vcopy_laneq_bf16(a, lane1, b, lane2) vcopy_laneq_bf16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopy_laneq_bf16
  #define vcopy_laneq_bf16(a, lane1, b, lane2) simde_vcopy_laneq_bf16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vcopyq_lane_bf16(simde_bfloat16x8_t a, const int lane1, simde_bfloat16x4_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 7)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 3) {
  simde_bfloat16x4_private b_ = simde_bfloat16x4_to_private(b);
  simde_bfloat16x8_private r_ = simde_bfloat16x8_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_bfloat16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
  #define simde_vcopyq_lane_bf16(a, lane1, b, lane2) vcopyq_lane_bf16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_lane_bf16
  #define vcopyq_lane_bf16(a, lane1, b, lane2) simde_vcopyq_lane_bf16((a), (lane1), (b), (lane2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vcopyq_laneq_bf16(simde_bfloat16x8_t a, const int lane1, simde_bfloat16x8_t b, const int lane2)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane1, 0, 7)
    SIMDE_REQUIRE_CONSTANT_RANGE(lane2, 0, 7) {
  simde_bfloat16x8_private
    b_ = simde_bfloat16x8_to_private(b),
    r_ = simde_bfloat16x8_to_private(a);

  r_.values[lane1] = b_.values[lane2];
  return simde_bfloat16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
  #define simde_vcopyq_laneq_bf16(a, lane1, b, lane2) vcopyq_laneq_bf16((a), (lane1), (b), (lane2))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcopyq_laneq_bf16
  #define vcopyq_laneq_bf16(a, lane1, b, lane2) simde_vcopyq_laneq_bf16((a), (lane1), (b), (lane2))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* SIMDE_ARM_NEON_COPY_LANE_H */
