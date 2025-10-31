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
 *   2020      Evan Nemerson <evan@nemerson.com>
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 */

#if !defined(SIMDE_ARM_NEON_MULL_HIGH_H)
#define SIMDE_ARM_NEON_MULL_HIGH_H

#include "types.h"
#include "mul.h"
#include "movl_high.h"
#include "mull.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vmull_high_s8(simde_int8x16_t a, simde_int8x16_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vmull_high_s8(a, b);
  #else
    return simde_vmulq_s16(simde_vmovl_high_s8(a), simde_vmovl_high_s8(b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmull_high_s8
  #define vmull_high_s8(a, b) simde_vmull_high_s8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vmull_high_s16(simde_int16x8_t a, simde_int16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vmull_high_s16(a, b);
  #else
    return simde_vmulq_s32(simde_vmovl_high_s16(a), simde_vmovl_high_s16(b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmull_high_s16
  #define vmull_high_s16(a, b) simde_vmull_high_s16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vmull_high_s32(simde_int32x4_t a, simde_int32x4_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vmull_high_s32(a, b);
  #else
    return simde_x_vmulq_s64(simde_vmovl_high_s32(a), simde_vmovl_high_s32(b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmull_high_s32
  #define vmull_high_s32(a, b) simde_vmull_high_s32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vmull_high_u8(simde_uint8x16_t a, simde_uint8x16_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vmull_high_u8(a, b);
  #else
    return simde_vmulq_u16(simde_vmovl_high_u8(a), simde_vmovl_high_u8(b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmull_high_u8
  #define vmull_high_u8(a, b) simde_vmull_high_u8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vmull_high_u16(simde_uint16x8_t a, simde_uint16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vmull_high_u16(a, b);
  #else
    return simde_vmulq_u32(simde_vmovl_high_u16(a), simde_vmovl_high_u16(b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmull_high_u16
  #define vmull_high_u16(a, b) simde_vmull_high_u16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vmull_high_u32(simde_uint32x4_t a, simde_uint32x4_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vmull_high_u32(a, b);
  #else
    return simde_x_vmulq_u64(simde_vmovl_high_u32(a), simde_vmovl_high_u32(b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmull_high_u32
  #define vmull_high_u32(a, b) simde_vmull_high_u32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vmull_high_p8(simde_poly8x16_t a, simde_poly8x16_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vmull_high_p8(a, b);
  #else
    simde_uint8x16_private
      a_ = simde_uint8x16_to_private(simde_vreinterpretq_u8_p8(a)),
      b_ = simde_uint8x16_to_private(simde_vreinterpretq_u8_p8(b));
    simde_uint16x8_private r_;

    size_t high_offset = (sizeof(r_.values) / sizeof(r_.values[0]));
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      uint16_t extend_op2 = HEDLEY_STATIC_CAST(uint16_t, b_.values[i+high_offset]);
      uint16_t result = 0;
      for(size_t j = 0; j < 8; ++j) {
        if (a_.values[i+high_offset] & (1 << j)) {
          result = HEDLEY_STATIC_CAST(uint16_t, result ^ (extend_op2 << j));
        }
      }
      r_.values[i] = result;
    }

    return simde_vreinterpretq_p16_u16(simde_uint16x8_from_private(r_));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmull_high_p8
  #define vmull_high_p8(a, b) simde_vmull_high_p8((a), (b))
#endif

#if !defined(SIMDE_TARGET_NOT_SUPPORT_INT128_TYPE)
SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vmull_high_p64(simde_poly64x2_t a, simde_poly64x2_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vmull_high_p64(a, b);
  #else
    simde_poly64x2_private
      a_ = simde_poly64x2_to_private(a),
      b_ = simde_poly64x2_to_private(b);
    return simde_vmull_p64(a_.values[1], b_.values[1]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vmull_high_p64
  #define vmull_high_p64(a, b) simde_vmull_high_p64((a), (b))
#endif
#endif /* !defined(SIMDE_TARGET_NOT_SUPPORT_INT128_TYPE) */

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_MULL_HIGH_H) */
