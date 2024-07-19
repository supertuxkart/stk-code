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

#if !defined(SIMDE_ARM_NEON_QDMULL_HIGH_LANE_H)
#define SIMDE_ARM_NEON_QDMULL_HIGH_LANE_H

#include "combine.h"
#include "qdmull.h"
#include "dup_n.h"
#include "get_high.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vqdmull_high_lane_s16(simde_int16x8_t a, simde_int16x4_t v, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_int16x4_private
    v_ = simde_int16x4_to_private(v);
  return simde_vqdmull_s16(simde_vget_high_s16(a), simde_vdup_n_s16(v_.values[lane]));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmull_high_lane_s16(a, v, lane) vqdmull_high_lane_s16(a, v, lane)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmull_high_lane_s16
  #define vqdmull_high_lane_s16(a, v, lane) simde_vqdmull_high_lane_s16((a), (v), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vqdmull_high_laneq_s16(simde_int16x8_t a, simde_int16x8_t v, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 7) {
  simde_int16x8_private
    v_ = simde_int16x8_to_private(v);
  return simde_vqdmull_s16(simde_vget_high_s16(a), simde_vdup_n_s16(v_.values[lane]));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmull_high_laneq_s16(a, v, lane) vqdmull_high_laneq_s16(a, v, lane)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmull_high_laneq_s16
  #define vqdmull_high_laneq_s16(a, v, lane) simde_vqdmull_high_laneq_s16((a), (v), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vqdmull_high_lane_s32(simde_int32x4_t a, simde_int32x2_t v, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 1) {
  simde_int32x2_private
    v_ = simde_int32x2_to_private(v);
  return simde_vqdmull_s32(simde_vget_high_s32(a), simde_vdup_n_s32(v_.values[lane]));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmull_high_lane_s32(a, v, lane) vqdmull_high_lane_s32(a, v, lane)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmull_high_lane_s32
  #define vqdmull_high_lane_s32(a, v, lane) simde_vqdmull_high_lane_s32((a), (v), (lane))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vqdmull_high_laneq_s32(simde_int32x4_t a, simde_int32x4_t v, const int lane)
   SIMDE_REQUIRE_CONSTANT_RANGE(lane, 0, 3) {
  simde_int32x4_private
    v_ = simde_int32x4_to_private(v);
  return simde_vqdmull_s32(simde_vget_high_s32(a), simde_vdup_n_s32(v_.values[lane]));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmull_high_laneq_s32(a, v, lane) vqdmull_high_laneq_s32(a, v, lane)
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmull_high_laneq_s32
  #define vqdmull_high_laneq_s32(a, v, lane) simde_vqdmull_high_laneq_s32((a), (v), (lane))
#endif


SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_QDMULL_HIGH_LANE_H) */
