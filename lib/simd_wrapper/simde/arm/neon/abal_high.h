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

#if !defined(SIMDE_ARM_NEON_ABAL_HIGH_H)
#define SIMDE_ARM_NEON_ABAL_HIGH_H

#include "abdl.h"
#include "add.h"
#include "movl_high.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vabal_high_s8(simde_int16x8_t a, simde_int8x16_t b, simde_int8x16_t c) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vabal_high_s8(a, b, c);
  #else
    return simde_vaddq_s16(simde_vabdl_s8(simde_vget_high_s8(b), simde_vget_high_s8(c)), a);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vabal_high_s8
  #define vabal_high_s8(a, b, c) simde_vabal_high_s8((a), (b), (c))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vabal_high_s16(simde_int32x4_t a, simde_int16x8_t b, simde_int16x8_t c) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vabal_high_s16(a, b, c);
  #else
    return simde_vaddq_s32(simde_vabdl_s16(simde_vget_high_s16(b), simde_vget_high_s16(c)), a);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vabal_high_s16
  #define vabal_high_s16(a, b, c) simde_vabal_high_s16((a), (b), (c))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vabal_high_s32(simde_int64x2_t a, simde_int32x4_t b, simde_int32x4_t c) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vabal_high_s32(a, b, c);
  #else
    return simde_vaddq_s64(simde_vabdl_s32(simde_vget_high_s32(b), simde_vget_high_s32(c)), a);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vabal_high_s32
  #define vabal_high_s32(a, b, c) simde_vabal_high_s32((a), (b), (c))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vabal_high_u8(simde_uint16x8_t a, simde_uint8x16_t b, simde_uint8x16_t c) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vabal_high_u8(a, b, c);
  #else
    return simde_vaddq_u16(simde_vabdl_u8(simde_vget_high_u8(b), simde_vget_high_u8(c)), a);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vabal_high_u8
  #define vabal_high_u8(a, b, c) simde_vabal_high_u8((a), (b), (c))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vabal_high_u16(simde_uint32x4_t a, simde_uint16x8_t b, simde_uint16x8_t c) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vabal_high_u16(a, b, c);
  #else
    return simde_vaddq_u32(simde_vabdl_u16(simde_vget_high_u16(b), simde_vget_high_u16(c)), a);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vabal_high_u16
  #define vabal_high_u16(a, b, c) simde_vabal_high_u16((a), (b), (c))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vabal_high_u32(simde_uint64x2_t a, simde_uint32x4_t b, simde_uint32x4_t c) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vabal_high_u32(a, b, c);
  #else
    return simde_vaddq_u64(simde_vabdl_u32(simde_vget_high_u32(b), simde_vget_high_u32(c)), a);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vabal_high_u32
  #define vabal_high_u32(a, b, c) simde_vabal_high_u32((a), (b), (c))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_abal_H) */
