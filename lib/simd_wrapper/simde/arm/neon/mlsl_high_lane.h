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

#if !defined(SIMDE_ARM_NEON_MLSL_HIGH_LANE_H)
#define SIMDE_ARM_NEON_MLSL_HIGH_LANE_H

#include "movl_high.h"
#include "mlsl_high.h"
#include "dup_n.h"
#include "mls.h"
#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vmlsl_high_lane_s16(simde_int32x4_t a, simde_int16x8_t b, simde_int16x4_t v, const int lane) SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
    return simde_vmlsl_high_s16(a, b, simde_vdupq_n_s16(simde_int16x4_to_private(v).values[lane]));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmlsl_high_lane_s16(a, b, v, lane) vmlsl_high_lane_s16(a, b, v, lane)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmlsl_high_lane_s16
  #define vmlsl_high_lane_s16(a, b, v, lane) simde_vmlsl_high_lane_s16((a), (b), (v), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vmlsl_high_laneq_s16(simde_int32x4_t a, simde_int16x8_t b, simde_int16x8_t v, const int lane) SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
    return simde_vmlsl_high_s16(a, b, simde_vdupq_n_s16(simde_int16x8_to_private(v).values[lane]));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmlsl_high_laneq_s16(a, b, v, lane) vmlsl_high_laneq_s16(a, b, v, lane)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmlsl_high_laneq_s16
  #define vmlsl_high_laneq_s16(a, b, v, lane) simde_vmlsl_high_laneq_s16((a), (b), (v), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vmlsl_high_lane_s32(simde_int64x2_t a, simde_int32x4_t b, simde_int32x2_t v, const int lane) SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
    return simde_vmlsl_high_s32(a, b, simde_vdupq_n_s32(simde_int32x2_to_private(v).values[lane]));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmlsl_high_lane_s32(a, b, v, lane) vmlsl_high_lane_s32(a, b, v, lane)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmlsl_high_lane_s32
  #define vmlsl_high_lane_s32(a, b, v, lane) simde_vmlsl_high_lane_s32((a), (b), (v), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vmlsl_high_laneq_s32(simde_int64x2_t a, simde_int32x4_t b, simde_int32x4_t v, const int lane) SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
    return simde_vmlsl_high_s32(a, b, simde_vdupq_n_s32(simde_int32x4_to_private(v).values[lane]));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmlsl_high_laneq_s32(a, b, v, lane) vmlsl_high_laneq_s32(a, b, v, lane)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmlsl_high_laneq_s32
  #define vmlsl_high_laneq_s32(a, b, v, lane) simde_vmlsl_high_laneq_s32((a), (b), (v), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vmlsl_high_lane_u16(simde_uint32x4_t a, simde_uint16x8_t b, simde_uint16x4_t v, const int lane) SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
    return simde_vmlsl_high_u16(a, b, simde_vdupq_n_u16(simde_uint16x4_to_private(v).values[lane]));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmlsl_high_lane_u16(a, b, v, lane) vmlsl_high_lane_u16(a, b, v, lane)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmlsl_high_lane_u16
  #define vmlsl_high_lane_u16(a, b, v, lane) simde_vmlsl_high_lane_u16((a), (b), (v), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vmlsl_high_laneq_u16(simde_uint32x4_t a, simde_uint16x8_t b, simde_uint16x8_t v, const int lane) SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
    return simde_vmlsl_high_u16(a, b, simde_vdupq_n_u16(simde_uint16x8_to_private(v).values[lane]));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmlsl_high_laneq_u16(a, b, v, lane) vmlsl_high_laneq_u16(a, b, v, lane)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmlsl_high_laneq_u16
  #define vmlsl_high_laneq_u16(a, b, v, lane) simde_vmlsl_high_laneq_u16((a), (b), (v), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vmlsl_high_lane_u32(simde_uint64x2_t a, simde_uint32x4_t b, simde_uint32x2_t v, const int lane) SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
    return simde_vmlsl_high_u32(a, b, simde_vdupq_n_u32(simde_uint32x2_to_private(v).values[lane]));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmlsl_high_lane_u32(a, b, v, lane) vmlsl_high_lane_u32(a, b, v, lane)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmlsl_high_lane_u32
  #define vmlsl_high_lane_u32(a, b, v, lane) simde_vmlsl_high_lane_u32((a), (b), (v), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vmlsl_high_laneq_u32(simde_uint64x2_t a, simde_uint32x4_t b, simde_uint32x4_t v, const int lane) SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
    return simde_vmlsl_high_u32(a, b, simde_vdupq_n_u32(simde_uint32x4_to_private(v).values[lane]));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmlsl_high_laneq_u32(a, b, v, lane) vmlsl_high_laneq_u32(a, b, v, lane)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmlsl_high_laneq_u32
  #define vmlsl_high_laneq_u32(a, b, v, lane) simde_vmlsl_high_laneq_u32((a), (b), (v), (lane))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_MLSL_HIGH_LANE_H) */
