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

#if !defined(SIMDE_ARM_NEON_SUBHN_HIGH_H)
#define SIMDE_ARM_NEON_SUBHN_HIGH_H

#include "subhn.h"
#include "combine.h"

#include "reinterpret.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vsubhn_high_s16(r, a, b) vsubhn_high_s16((r), (a), (b))
#else
  #define simde_vsubhn_high_s16(r, a, b) simde_vcombine_s8(r, simde_vsubhn_s16(a, b))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsubhn_high_s16
  #define vsubhn_high_s16(r, a, b) simde_vsubhn_high_s16((r), (a), (b))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vsubhn_high_s32(r, a, b) vsubhn_high_s32((r), (a), (b))
#else
  #define simde_vsubhn_high_s32(r, a, b) simde_vcombine_s16(r, simde_vsubhn_s32(a, b))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsubhn_high_s32
  #define vsubhn_high_s32(r, a, b) simde_vsubhn_high_s32((r), (a), (b))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vsubhn_high_s64(r, a, b) vsubhn_high_s64((r), (a), (b))
#else
  #define simde_vsubhn_high_s64(r, a, b) simde_vcombine_s32(r, simde_vsubhn_s64(a, b))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsubhn_high_s64
  #define vsubhn_high_s64(r, a, b) simde_vsubhn_high_s64((r), (a), (b))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vsubhn_high_u16(r, a, b) vsubhn_high_u16((r), (a), (b))
#else
  #define simde_vsubhn_high_u16(r, a, b) simde_vcombine_u8(r, simde_vsubhn_u16(a, b))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsubhn_high_u16
  #define vsubhn_high_u16(r, a, b) simde_vsubhn_high_u16((r), (a), (b))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vsubhn_high_u32(r, a, b) vsubhn_high_u32((r), (a), (b))
#else
  #define simde_vsubhn_high_u32(r, a, b) simde_vcombine_u16(r, simde_vsubhn_u32(a, b))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsubhn_high_u32
  #define vsubhn_high_u32(r, a, b) simde_vsubhn_high_u32((r), (a), (b))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vsubhn_high_u64(r, a, b) vsubhn_high_u64((r), (a), (b))
#else
  #define simde_vsubhn_high_u64(r, a, b) simde_vcombine_u32(r, simde_vsubhn_u64(a, b))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsubhn_high_u64
  #define vsubhn_high_u64(r, a, b) simde_vsubhn_high_u64((r), (a), (b))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_SUBHN_HIGH_H) */
