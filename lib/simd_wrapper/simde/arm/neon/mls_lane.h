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

#if !defined(SIMDE_ARM_NEON_MLS_LANE_H)
#define SIMDE_ARM_NEON_MLS_LANE_H

#include "mls.h"
#include "dup_lane.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmls_lane_f32(a, b, v, lane) vmls_lane_f32((a), (b), (v), (lane))
#else
  #define simde_vmls_lane_f32(a, b, v, lane) simde_vmls_f32((a), (b), simde_vdup_lane_f32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmls_lane_f32
  #define vmls_lane_f32(a, b, v, lane) simde_vmls_lane_f32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmls_laneq_f32(a, b, v, lane) vmls_laneq_f32((a), (b), (v), (lane))
#else
  #define simde_vmls_laneq_f32(a, b, v, lane) simde_vmls_f32((a), (b), simde_vdup_laneq_f32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmls_laneq_f32
  #define vmls_laneq_f32(a, b, v, lane) simde_vmls_laneq_f32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmlsq_laneq_f32(a, b, v, lane) vmlsq_laneq_f32((a), (b), (v), (lane))
#else
  #define simde_vmlsq_laneq_f32(a, b, v, lane) simde_vmlsq_f32((a), (b), simde_vdupq_laneq_f32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmlsq_laneq_f32
  #define vmlsq_laneq_f32(a, b, v, lane) simde_vmlsq_laneq_f32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmls_lane_s16(a, b, v, lane) vmls_lane_s16((a), (b), (v), (lane))
#else
  #define simde_vmls_lane_s16(a, b, v, lane) simde_vmls_s16((a), (b), simde_vdup_lane_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmls_lane_s16
  #define vmls_lane_s16(a, b, v, lane) simde_vmls_lane_s16((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmls_laneq_s16(a, b, v, lane) vmls_laneq_s16((a), (b), (v), (lane))
#else
  #define simde_vmls_laneq_s16(a, b, v, lane) simde_vmls_s16((a), (b), simde_vdup_laneq_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmls_laneq_s16
  #define vmls_laneq_s16(a, b, v, lane) simde_vmls_laneq_s16((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmlsq_laneq_s16(a, b, v, lane) vmlsq_laneq_s16((a), (b), (v), (lane))
#else
  #define simde_vmlsq_laneq_s16(a, b, v, lane) simde_vmlsq_s16((a), (b), simde_vdupq_laneq_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmlsq_laneq_s16
  #define vmlsq_laneq_s16(a, b, v, lane) simde_vmlsq_laneq_s16((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmls_lane_s32(a, b, v, lane) vmls_lane_s32((a), (b), (v), (lane))
#else
  #define simde_vmls_lane_s32(a, b, v, lane) simde_vmls_s32((a), (b), simde_vdup_lane_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmls_lane_s32
  #define vmls_lane_s32(a, b, v, lane) simde_vmls_lane_s32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmls_laneq_s32(a, b, v, lane) vmls_laneq_s32((a), (b), (v), (lane))
#else
  #define simde_vmls_laneq_s32(a, b, v, lane) simde_vmls_s32((a), (b), simde_vdup_laneq_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmls_laneq_s32
  #define vmls_laneq_s32(a, b, v, lane) simde_vmls_laneq_s32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmlsq_laneq_s32(a, b, v, lane) vmlsq_laneq_s32((a), (b), (v), (lane))
#else
  #define simde_vmlsq_laneq_s32(a, b, v, lane) simde_vmlsq_s32((a), (b), simde_vdupq_laneq_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmlsq_laneq_s32
  #define vmlsq_laneq_s32(a, b, v, lane) simde_vmlsq_laneq_s32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmls_lane_u16(a, b, v, lane) vmls_lane_u16((a), (b), (v), (lane))
#else
  #define simde_vmls_lane_u16(a, b, v, lane) simde_vmls_u16((a), (b), simde_vdup_lane_u16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmls_lane_u16
  #define vmls_lane_u16(a, b, v, lane) simde_vmls_lane_u16((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmls_laneq_u16(a, b, v, lane) vmls_laneq_u16((a), (b), (v), (lane))
#else
  #define simde_vmls_laneq_u16(a, b, v, lane) simde_vmls_u16((a), (b), simde_vdup_laneq_u16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmls_laneq_u16
  #define vmls_laneq_u16(a, b, v, lane) simde_vmls_laneq_u16((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmlsq_laneq_u16(a, b, v, lane) vmlsq_laneq_u16((a), (b), (v), (lane))
#else
  #define simde_vmlsq_laneq_u16(a, b, v, lane) simde_vmlsq_u16((a), (b), simde_vdupq_laneq_u16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmlsq_laneq_u16
  #define vmlsq_laneq_u16(a, b, v, lane) simde_vmlsq_laneq_u16((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmls_lane_u32(a, b, v, lane) vmls_lane_u32((a), (b), (v), (lane))
#else
  #define simde_vmls_lane_u32(a, b, v, lane) simde_vmls_u32((a), (b), simde_vdup_lane_u32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmls_lane_u32
  #define vmls_lane_u32(a, b, v, lane) simde_vmls_lane_u32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmls_laneq_u32(a, b, v, lane) vmls_laneq_u32((a), (b), (v), (lane))
#else
  #define simde_vmls_laneq_u32(a, b, v, lane) simde_vmls_u32((a), (b), simde_vdup_laneq_u32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmls_laneq_u32
  #define vmls_laneq_u32(a, b, v, lane) simde_vmls_laneq_u32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vmlsq_laneq_u32(a, b, v, lane) vmlsq_laneq_u32((a), (b), (v), (lane))
#else
  #define simde_vmlsq_laneq_u32(a, b, v, lane) simde_vmlsq_u32((a), (b), simde_vdupq_laneq_u32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmlsq_laneq_u32
  #define vmlsq_laneq_u32(a, b, v, lane) simde_vmlsq_laneq_u32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmlsq_lane_f32(a, b, v, lane) vmlsq_lane_f32((a), (b), (v), (lane))
#else
  #define simde_vmlsq_lane_f32(a, b, v, lane) simde_vmlsq_f32((a), (b), simde_vdupq_lane_f32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmlsq_lane_f32
  #define vmlsq_lane_f32(a, b, v, lane) simde_vmlsq_lane_f32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmlsq_lane_s16(a, b, v, lane) vmlsq_lane_s16((a), (b), (v), (lane))
#else
  #define simde_vmlsq_lane_s16(a, b, v, lane) simde_vmlsq_s16((a), (b), simde_vdupq_lane_s16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmlsq_lane_s16
  #define vmlsq_lane_s16(a, b, v, lane) simde_vmlsq_lane_s16((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmlsq_lane_s32(a, b, v, lane) vmlsq_lane_s32((a), (b), (v), (lane))
#else
  #define simde_vmlsq_lane_s32(a, b, v, lane) simde_vmlsq_s32((a), (b), simde_vdupq_lane_s32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmlsq_lane_s32
  #define vmlsq_lane_s32(a, b, v, lane) simde_vmlsq_lane_s32((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmlsq_lane_u16(a, b, v, lane) vmlsq_lane_u16((a), (b), (v), (lane))
#else
  #define simde_vmlsq_lane_u16(a, b, v, lane) simde_vmlsq_u16((a), (b), simde_vdupq_lane_u16((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmlsq_lane_u16
  #define vmlsq_lane_u16(a, b, v, lane) simde_vmlsq_lane_u16((a), (b), (v), (lane))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vmlsq_lane_u32(a, b, v, lane) vmlsq_lane_u32((a), (b), (v), (lane))
#else
  #define simde_vmlsq_lane_u32(a, b, v, lane) simde_vmlsq_u32((a), (b), simde_vdupq_lane_u32((v), (lane)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vmlsq_lane_u32
  #define vmlsq_lane_u32(a, b, v, lane) simde_vmlsq_lane_u32((a), (b), (v), (lane))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_MLS_LANE_H) */
