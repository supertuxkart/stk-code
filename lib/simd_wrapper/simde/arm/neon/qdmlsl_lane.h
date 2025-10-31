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

#if !defined(SIMDE_ARM_NEON_QDMLSL_LANE_H)
#define SIMDE_ARM_NEON_QDMLSL_LANE_H

#include "qdmlsl.h"
#include "dup_lane.h"
#include "get_lane.h"
#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqdmlsl_lane_s16(a, b, v, lane) vqdmlsl_lane_s16((a), (b), (v), (lane))
#else
  #define simde_vqdmlsl_lane_s16(a, b, v, lane) simde_vqdmlsl_s16((a), (b), simde_vdup_lane_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqdmlsl_lane_s16
  #define vqdmlsl_lane_s16(a, b, c, lane) simde_vqdmlsl_lane_s16((a), (b), (c), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqdmlsl_lane_s32(a, b, v, lane) vqdmlsl_lane_s32((a), (b), (v), (lane))
#else
  #define simde_vqdmlsl_lane_s32(a, b, v, lane) simde_vqdmlsl_s32((a), (b), simde_vdup_lane_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqdmlsl_lane_s32
  #define vqdmlsl_lane_s32(a, b, c, lane) simde_vqdmlsl_lane_s32((a), (b), (c), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmlsl_laneq_s16(a, b, v, lane) vqdmlsl_laneq_s16((a), (b), (v), (lane))
#else
  #define simde_vqdmlsl_laneq_s16(a, b, v, lane) simde_vqdmlsl_s16((a), (b), simde_vdup_laneq_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmlsl_laneq_s16
  #define vqdmlsl_laneq_s16(a, b, c, lane) simde_vqdmlsl_laneq_s16((a), (b), (c), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmlsl_laneq_s32(a, b, v, lane) vqdmlsl_laneq_s32((a), (b), (v), (lane))
#else
  #define simde_vqdmlsl_laneq_s32(a, b, v, lane) simde_vqdmlsl_s32((a), (b), simde_vdup_laneq_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmlsl_laneq_s32
  #define vqdmlsl_laneq_s32(a, b, c, lane) simde_vqdmlsl_laneq_s32((a), (b), (c), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmlslh_lane_s16(a, b, v, lane) vqdmlslh_lane_s16((a), (b), (v), (lane))
#else
  #define simde_vqdmlslh_lane_s16(a, b, v, lane) simde_vqdmlslh_s16((a), (b), simde_vget_lane_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmlslh_lane_s16
  #define vqdmlslh_lane_s16(a, b, c, lane) simde_vqdmlslh_lane_s16((a), (b), (c), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmlslh_laneq_s16(a, b, v, lane) vqdmlslh_laneq_s16((a), (b), (v), (lane))
#else
  #define simde_vqdmlslh_laneq_s16(a, b, v, lane) simde_vqdmlslh_s16((a), (b), simde_vgetq_lane_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmlslh_laneq_s16
  #define vqdmlslh_laneq_s16(a, b, c, lane) simde_vqdmlslh_laneq_s16((a), (b), (c), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmlsls_lane_s32(a, b, v, lane) vqdmlsls_lane_s32((a), (b), (v), (lane))
#else
  #define simde_vqdmlsls_lane_s32(a, b, v, lane) simde_vqdmlsls_s32((a), (b), simde_vget_lane_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmlsls_lane_s32
  #define vqdmlsls_lane_s32(a, b, c, lane) simde_vqdmlsls_lane_s32((a), (b), (c), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqdmlsls_laneq_s32(a, b, v, lane) vqdmlsls_laneq_s32((a), (b), (v), (lane))
#else
  #define simde_vqdmlsls_laneq_s32(a, b, v, lane) simde_vqdmlsls_s32((a), (b), simde_vgetq_lane_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmlsls_laneq_s32
  #define vqdmlsls_laneq_s32(a, b, c, lane) simde_vqdmlsls_laneq_s32((a), (b), (c), (lane))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_QDmlsl_LANE_H) */
