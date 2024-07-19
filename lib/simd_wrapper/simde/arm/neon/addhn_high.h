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

#if !defined(SIMDE_ARM_NEON_ADDHN_HIGH_H)
#define SIMDE_ARM_NEON_ADDHN_HIGH_H

#include "addhn.h"
#include "combine.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vaddhn_high_s16(simde_int8x8_t r, simde_int16x8_t a, simde_int16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vaddhn_high_s16(r, a, b);
  #else
    return simde_vcombine_s8(r, simde_vaddhn_s16(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vaddhn_high_s16
  #define vaddhn_high_s16(r, a, b) simde_vaddhn_high_s16((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vaddhn_high_s32(simde_int16x4_t r, simde_int32x4_t a, simde_int32x4_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vaddhn_high_s32(r, a, b);
  #else
    return simde_vcombine_s16(r, simde_vaddhn_s32(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vaddhn_high_s32
  #define vaddhn_high_s32(r, a, b) simde_vaddhn_high_s32((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vaddhn_high_s64(simde_int32x2_t r, simde_int64x2_t a, simde_int64x2_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vaddhn_high_s64(r, a, b);
  #else
    return simde_vcombine_s32(r, simde_vaddhn_s64(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vaddhn_high_s64
  #define vaddhn_high_s64(r, a, b) simde_vaddhn_high_s64((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vaddhn_high_u16(simde_uint8x8_t r, simde_uint16x8_t a, simde_uint16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vaddhn_high_u16(r, a, b);
  #else
    return simde_vcombine_u8(r, simde_vaddhn_u16(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vaddhn_high_u16
  #define vaddhn_high_u16(r, a, b) simde_vaddhn_high_u16((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vaddhn_high_u32(simde_uint16x4_t r, simde_uint32x4_t a, simde_uint32x4_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vaddhn_high_u32(r, a, b);
  #else
    return simde_vcombine_u16(r, simde_vaddhn_u32(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vaddhn_high_u32
  #define vaddhn_high_u32(r, a, b) simde_vaddhn_high_u32((r), (a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vaddhn_high_u64(simde_uint32x2_t r, simde_uint64x2_t a, simde_uint64x2_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vaddhn_high_u64(r, a, b);
  #else
    return simde_vcombine_u32(r, simde_vaddhn_u64(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vaddhn_high_u64
  #define vaddhn_high_u64(r, a, b) simde_vaddhn_high_u64((r), (a), (b))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_ADDHN_HIGH_H) */
