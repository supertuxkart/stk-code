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

#if !defined(SIMDE_ARM_NEON_QRDMLAH_LANE_H)
#define SIMDE_ARM_NEON_QRDMLAH_LANE_H

#include "types.h"
#include "qrdmlah.h"
#include "dup_lane.h"
#include "get_lane.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_QRDMX)
  #define simde_vqrdmlahh_lane_s16(a, b, v, lane) vqrdmlahh_lane_s16((a), (b), (v), (lane))
#else
  #define simde_vqrdmlahh_lane_s16(a, b, v, lane) simde_vqrdmlahh_s16((a), (b), simde_vget_lane_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrdmlahh_lane_s16
  #define vqrdmlahh_lane_s16(a, b, v, lane) simde_vqrdmlahh_lane_s16((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_QRDMX)
  #define simde_vqrdmlahh_laneq_s16(a, b, v, lane) vqrdmlahh_laneq_s16((a), (b), (v), (lane))
#else
  #define simde_vqrdmlahh_laneq_s16(a, b, v, lane) simde_vqrdmlahh_s16((a), (b), simde_vgetq_lane_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrdmlahh_laneq_s16
  #define vqrdmlahh_laneq_s16(a, b, v, lane) simde_vqrdmlahh_laneq_s16((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_QRDMX)
  #define simde_vqrdmlahs_lane_s32(a, b, v, lane) vqrdmlahs_lane_s32((a), (b), (v), (lane))
#else
  #define simde_vqrdmlahs_lane_s32(a, b, v, lane) simde_vqrdmlahs_s32((a), (b), simde_vget_lane_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrdmlahs_lane_s32
  #define vqrdmlahs_lane_s32(a, b, v, lane) simde_vqrdmlahs_lane_s32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_QRDMX)
  #define simde_vqrdmlahs_laneq_s32(a, b, v, lane) vqrdmlahs_laneq_s32((a), (b), (v), (lane))
#else
  #define simde_vqrdmlahs_laneq_s32(a, b, v, lane) simde_vqrdmlahs_s32((a), (b), simde_vgetq_lane_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrdmlahs_laneq_s32
  #define vqrdmlahs_laneq_s32(a, b, v, lane) simde_vqrdmlahs_laneq_s32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARCH_ARM_QRDMX)
  #define simde_vqrdmlah_lane_s16(a, b, v, lane) vqrdmlah_lane_s16((a), (b), (v), (lane))
#else
  #define simde_vqrdmlah_lane_s16(a, b, v, lane) simde_vqrdmlah_s16((a), (b), simde_vdup_lane_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrdmlah_lane_s16
  #define vqrdmlah_lane_s16(a, b, v, lane) simde_vqrdmlah_lane_s16((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARCH_ARM_QRDMX)
  #define simde_vqrdmlah_lane_s32(a, b, v, lane) vqrdmlah_lane_s32((a), (b), (v), (lane))
#else
  #define simde_vqrdmlah_lane_s32(a, b, v, lane) simde_vqrdmlah_s32((a), (b), simde_vdup_lane_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrdmlah_lane_s32
  #define vqrdmlah_lane_s32(a, b, v, lane) simde_vqrdmlah_lane_s32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARCH_ARM_QRDMX)
  #define simde_vqrdmlahq_lane_s16(a, b, v, lane) vqrdmlahq_lane_s16((a), (b), (v), (lane))
#else
  #define simde_vqrdmlahq_lane_s16(a, b, v, lane) simde_vqrdmlahq_s16((a), (b), simde_vdupq_lane_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrdmlahq_lane_s16
  #define vqrdmlahq_lane_s16(a, b, v, lane) simde_vqrdmlahq_lane_s16((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARCH_ARM_QRDMX)
  #define simde_vqrdmlahq_lane_s32(a, b, v, lane) vqrdmlahq_lane_s32((a), (b), (v), (lane))
#else
  #define simde_vqrdmlahq_lane_s32(a, b, v, lane) simde_vqrdmlahq_s32((a), (b), simde_vdupq_lane_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrdmlahq_lane_s32
  #define vqrdmlahq_lane_s32(a, b, v, lane) simde_vqrdmlahq_lane_s32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_QRDMX)
  #define simde_vqrdmlah_laneq_s16(a, b, v, lane) vqrdmlah_laneq_s16((a), (b), (v), (lane))
#else
  #define simde_vqrdmlah_laneq_s16(a, b, v, lane) simde_vqrdmlah_s16((a), (b), simde_vdup_laneq_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrdmlah_laneq_s16
  #define vqrdmlah_laneq_s16(a, b, v, lane) simde_vqrdmlah_laneq_s16((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_QRDMX)
  #define simde_vqrdmlah_laneq_s32(a, b, v, lane) vqrdmlah_laneq_s32((a), (b), (v), (lane))
#else
  #define simde_vqrdmlah_laneq_s32(a, b, v, lane) simde_vqrdmlah_s32((a), (b), simde_vdup_laneq_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrdmlah_laneq_s32
  #define vqrdmlah_laneq_s32(a, b, v, lane) simde_vqrdmlah_laneq_s32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_QRDMX)
  #define simde_vqrdmlahq_laneq_s16(a, b, v, lane) vqrdmlahq_laneq_s16((a), (b), (v), (lane))
#else
  #define simde_vqrdmlahq_laneq_s16(a, b, v, lane) simde_vqrdmlahq_s16((a), (b), simde_vdupq_laneq_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrdmlahq_laneq_s16
  #define vqrdmlahq_laneq_s16(a, b, v, lane) simde_vqrdmlahq_laneq_s16((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_QRDMX)
  #define simde_vqrdmlahq_laneq_s32(a, b, v, lane) vqrdmlahq_laneq_s32((a), (b), (v), (lane))
#else
  #define simde_vqrdmlahq_laneq_s32(a, b, v, lane) simde_vqrdmlahq_s32((a), (b), simde_vdupq_laneq_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrdmlahq_laneq_s32
  #define vqrdmlahq_laneq_s32(a, b, v, lane) simde_vqrdmlahq_laneq_s32((a), (b), (v), (lane))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_QRDMLAH_LANE_H) */
