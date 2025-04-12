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

#if !defined(SIMDE_ARM_NEON_QSHRN_HIGH_N_H)
#define SIMDE_ARM_NEON_QSHRN_HIGH_N_H

#include "types.h"
#include "shr_n.h"
#include "qmovn.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqshrn_high_n_s16(r, a, n) vqshrn_high_n_s16((r), (a), (n))
#else
  #define simde_vqshrn_high_n_s16(r, a, n) simde_vcombine_s8(r, simde_vqmovn_s16(simde_vshrq_n_s16(a, n)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshrn_high_n_s16
  #define vqshrn_high_n_s16(r, a, n) simde_vqshrn_high_n_s16((r), (a), (n))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqshrn_high_n_s32(r, a, n) vqshrn_high_n_s32((r), (a), (n))
#else
  #define simde_vqshrn_high_n_s32(r, a, n) simde_vcombine_s16(r, simde_vqmovn_s32(simde_vshrq_n_s32(a, n)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshrn_high_n_s32
  #define vqshrn_high_n_s32(r, a, n) simde_vqshrn_high_n_s32((r), (a), (n))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqshrn_high_n_s64(r, a, n) vqshrn_high_n_s64((r), (a), (n))
#else
  #define simde_vqshrn_high_n_s64(r, a, n) simde_vcombine_s32(r, simde_vqmovn_s64(simde_vshrq_n_s64(a, n)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshrn_high_n_s64
  #define vqshrn_high_n_s64(r, a, n) simde_vqshrn_high_n_s64((r), (a), (n))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqshrn_high_n_u16(r, a, n) vqshrn_high_n_u16((r), (a), (n))
#else
  #define simde_vqshrn_high_n_u16(r, a, n) simde_vcombine_u8(r, simde_vqmovn_u16(simde_vshrq_n_u16(a, n)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshrn_high_n_u16
  #define vqshrn_high_n_u16(r, a, n) simde_vqshrn_high_n_u16((r), (a), (n))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqshrn_high_n_u32(r, a, n) vqshrn_high_n_u32((r), (a), (n))
#else
  #define simde_vqshrn_high_n_u32(r, a, n) simde_vcombine_u16(r, simde_vqmovn_u32(simde_vshrq_n_u32(a, n)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshrn_high_n_u32
  #define vqshrn_high_n_u32(r, a, n) simde_vqshrn_high_n_u32((r), (a), (n))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqshrn_high_n_u64(r, a, n) vqshrn_high_n_u64((r), (a), (n))
#else
  #define simde_vqshrn_high_n_u64(r, a, n) simde_vcombine_u32(r, simde_vqmovn_u64(simde_vshrq_n_u64(a, n)))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshrn_high_n_u64
  #define vqshrn_high_n_u64(r, a, n) simde_vqshrn_high_n_u64((r), (a), (n))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_QSHRN_HIGH_N_H) */
