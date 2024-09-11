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

#if !defined(SIMDE_ARM_NEON_QDMLAL_LANE_H)
#define SIMDE_ARM_NEON_QDMLAL_LANE_H

#include "qdmlal.h"
#include "dup_lane.h"
#include "get_lane.h"
#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqdmlal_lane_s16(a, b, v, lane) vqdmlal_lane_s16((a), (b), (v), (lane))
#else
  #define simde_vqdmlal_lane_s16(a, b, v, lane) simde_vqdmlal_s16((a), (b), simde_vdup_lane_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqdmlal_lane_s16
  #define vqdmlal_lane_s16(a, b, c, lane) simde_vqdmlal_lane_s16((a), (b), (c), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqdmlal_lane_s32(a, b, v, lane) vqdmlal_lane_s32((a), (b), (v), (lane))
#else
  #define simde_vqdmlal_lane_s32(a, b, v, lane) simde_vqdmlal_s32((a), (b), simde_vdup_lane_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqdmlal_lane_s32
  #define vqdmlal_lane_s32(a, b, c, lane) simde_vqdmlal_lane_s32((a), (b), (c), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmlal_laneq_s16(a, b, v, lane) vqdmlal_laneq_s16((a), (b), (v), (lane))
#else
  #define simde_vqdmlal_laneq_s16(a, b, v, lane) simde_vqdmlal_s16((a), (b), simde_vdup_laneq_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmlal_laneq_s16
  #define vqdmlal_laneq_s16(a, b, c, lane) simde_vqdmlal_laneq_s16((a), (b), (c), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmlal_laneq_s32(a, b, v, lane) vqdmlal_laneq_s32((a), (b), (v), (lane))
#else
  #define simde_vqdmlal_laneq_s32(a, b, v, lane) simde_vqdmlal_s32((a), (b), simde_vdup_laneq_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmlal_laneq_s32
  #define vqdmlal_laneq_s32(a, b, c, lane) simde_vqdmlal_laneq_s32((a), (b), (c), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmlalh_lane_s16(a, b, v, lane) vqdmlalh_lane_s16((a), (b), (v), (lane))
#else
  #define simde_vqdmlalh_lane_s16(a, b, v, lane) simde_vqdmlalh_s16((a), (b), simde_vget_lane_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmlalh_lane_s16
  #define vqdmlalh_lane_s16(a, b, c, lane) simde_vqdmlalh_lane_s16((a), (b), (c), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmlalh_laneq_s16(a, b, v, lane) vqdmlalh_laneq_s16((a), (b), (v), (lane))
#else
  #define simde_vqdmlalh_laneq_s16(a, b, v, lane) simde_vqdmlalh_s16((a), (b), simde_vgetq_lane_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmlalh_laneq_s16
  #define vqdmlalh_laneq_s16(a, b, c, lane) simde_vqdmlalh_laneq_s16((a), (b), (c), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmlals_lane_s32(a, b, v, lane) vqdmlals_lane_s32((a), (b), (v), (lane))
#else
  #define simde_vqdmlals_lane_s32(a, b, v, lane) simde_vqdmlals_s32((a), (b), simde_vget_lane_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmlals_lane_s32
  #define vqdmlals_lane_s32(a, b, c, lane) simde_vqdmlals_lane_s32((a), (b), (c), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmlals_laneq_s32(a, b, v, lane) vqdmlals_laneq_s32((a), (b), (v), (lane))
#else
  #define simde_vqdmlals_laneq_s32(a, b, v, lane) simde_vqdmlals_s32((a), (b), simde_vgetq_lane_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmlals_laneq_s32
  #define vqdmlals_laneq_s32(a, b, c, lane) simde_vqdmlals_laneq_s32((a), (b), (c), (lane))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_QDMLAL_LANE_H) */
