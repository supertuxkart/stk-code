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

#if !defined(SIMDE_ARM_NEON_LD2_DUP_H)
#define SIMDE_ARM_NEON_LD2_DUP_H

#include "dup_n.h"
#include "reinterpret.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4x2_t
simde_vld2_dup_f16(simde_float16_t const ptr[HEDLEY_ARRAY_PARAM(2)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vld2_dup_f16(ptr);
  #else
    simde_float16x4x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_f16(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_f16
  #define vld2_dup_f16(a) simde_vld2_dup_f16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2x2_t
simde_vld2_dup_f32(simde_float32 const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_dup_f32(ptr);
  #else
    simde_float32x2x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_f32(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_f32
  #define vld2_dup_f32(a) simde_vld2_dup_f32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1x2_t
simde_vld2_dup_f64(simde_float64 const * ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2_dup_f64(ptr);
  #else
    simde_float64x1x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_f64(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_f64
  #define vld2_dup_f64(a) simde_vld2_dup_f64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8x2_t
simde_vld2_dup_s8(int8_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_dup_s8(ptr);
  #else
    simde_int8x8x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_s8(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_s8
  #define vld2_dup_s8(a) simde_vld2_dup_s8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4x2_t
simde_vld2_dup_s16(int16_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_dup_s16(ptr);
  #else
    simde_int16x4x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_s16(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_s16
  #define vld2_dup_s16(a) simde_vld2_dup_s16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2x2_t
simde_vld2_dup_s32(int32_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_dup_s32(ptr);
  #else
    simde_int32x2x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_s32(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_s32
  #define vld2_dup_s32(a) simde_vld2_dup_s32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1x2_t
simde_vld2_dup_s64(int64_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_dup_s64(ptr);
  #else
    simde_int64x1x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_s64(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_s64
  #define vld2_dup_s64(a) simde_vld2_dup_s64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8x2_t
simde_vld2_dup_u8(uint8_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_dup_u8(ptr);
  #else
    simde_uint8x8x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_u8(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_u8
  #define vld2_dup_u8(a) simde_vld2_dup_u8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4x2_t
simde_vld2_dup_u16(uint16_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_dup_u16(ptr);
  #else
    simde_uint16x4x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_u16(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_u16
  #define vld2_dup_u16(a) simde_vld2_dup_u16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2x2_t
simde_vld2_dup_u32(uint32_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_dup_u32(ptr);
  #else
    simde_uint32x2x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_u32(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_u32
  #define vld2_dup_u32(a) simde_vld2_dup_u32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1x2_t
simde_vld2_dup_u64(uint64_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_dup_u64(ptr);
  #else
    simde_uint64x1x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_u64(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_u64
  #define vld2_dup_u64(a) simde_vld2_dup_u64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8x2_t
simde_vld2q_dup_f16(simde_float16_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vld2q_dup_f16(ptr);
  #else
    simde_float16x8x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_f16(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_f16
  #define vld2q_dup_f16(a) simde_vld2q_dup_f16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4x2_t
simde_vld2q_dup_f32(simde_float32 const * ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_dup_f32(ptr);
  #else
    simde_float32x4x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_f32(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_f32
  #define vld2q_dup_f32(a) simde_vld2q_dup_f32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2x2_t
simde_vld2q_dup_f64(simde_float64 const * ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_dup_f64(ptr);
  #else
    simde_float64x2x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_f64(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_f64
  #define vld2q_dup_f64(a) simde_vld2q_dup_f64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16x2_t
simde_vld2q_dup_s8(int8_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_dup_s8(ptr);
  #else
    simde_int8x16x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_s8(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_s8
  #define vld2q_dup_s8(a) simde_vld2q_dup_s8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8x2_t
simde_vld2q_dup_s16(int16_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_dup_s16(ptr);
  #else
    simde_int16x8x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_s16(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_s16
  #define vld2q_dup_s16(a) simde_vld2q_dup_s16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4x2_t
simde_vld2q_dup_s32(int32_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_dup_s32(ptr);
  #else
    simde_int32x4x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_s32(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_s32
  #define vld2q_dup_s32(a) simde_vld2q_dup_s32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2x2_t
simde_vld2q_dup_s64(int64_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_dup_s64(ptr);
  #else
    simde_int64x2x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_s64(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_s64
  #define vld2q_dup_s64(a) simde_vld2q_dup_s64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16x2_t
simde_vld2q_dup_u8(uint8_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_dup_u8(ptr);
  #else
    simde_uint8x16x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_u8(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_u8
  #define vld2q_dup_u8(a) simde_vld2q_dup_u8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8x2_t
simde_vld2q_dup_u16(uint16_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_dup_u16(ptr);
  #else
    simde_uint16x8x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_u16(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_u16
  #define vld2q_dup_u16(a) simde_vld2q_dup_u16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4x2_t
simde_vld2q_dup_u32(uint32_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_dup_u32(ptr);
  #else
    simde_uint32x4x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_u32(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_u32
  #define vld2q_dup_u32(a) simde_vld2q_dup_u32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2x2_t
simde_vld2q_dup_u64(uint64_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_dup_u64(ptr);
  #else
    simde_uint64x2x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_u64(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_u64
  #define vld2q_dup_u64(a) simde_vld2q_dup_u64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8x2_t
simde_vld2_dup_p8(simde_poly8_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_dup_p8(ptr);
  #else
    simde_poly8x8x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_p8(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_p8
  #define vld2_dup_p8(a) simde_vld2_dup_p8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4x2_t
simde_vld2_dup_p16(simde_poly16_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_dup_p16(ptr);
  #else
    simde_poly16x4x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_p16(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_p16
  #define vld2_dup_p16(a) simde_vld2_dup_p16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1x2_t
simde_vld2_dup_p64(simde_poly64_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vld2_dup_p64(ptr);
  #else
    simde_poly64x1x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_p64(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_p64
  #define vld2_dup_p64(a) simde_vld2_dup_p64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16x2_t
simde_vld2q_dup_p8(simde_poly8_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_95399) && \
      !defined(SIMDE_BUG_CLANG_71763)
    return vld2q_dup_p8(ptr);
  #else
    simde_poly8x16x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_p8(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_p8
  #define vld2q_dup_p8(a) simde_vld2q_dup_p8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8x2_t
simde_vld2q_dup_p16(simde_poly16_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_95399) && \
      !defined(SIMDE_BUG_CLANG_71763)
    return vld2q_dup_p16(ptr);
  #else
    simde_poly16x8x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_p16(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_p16
  #define vld2q_dup_p16(a) simde_vld2q_dup_p16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2x2_t
simde_vld2q_dup_p64(simde_poly64_t const * ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_dup_p64(ptr);
  #else
    simde_poly64x2x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_p64(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_p64
  #define vld2q_dup_p64(a) simde_vld2q_dup_p64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4x2_t
simde_vld2_dup_bf16(simde_bfloat16_t const ptr[HEDLEY_ARRAY_PARAM(2)]) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vld2_dup_bf16(ptr);
  #else
    simde_bfloat16x4x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdup_n_bf16(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld2_dup_bf16
  #define vld2_dup_bf16(a) simde_vld2_dup_bf16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8x2_t
simde_vld2q_dup_bf16(simde_bfloat16 const * ptr) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vld2q_dup_bf16(ptr);
  #else
    simde_bfloat16x8x2_t r;

    for (size_t i = 0 ; i < 2 ; i++) {
      r.val[i] = simde_vdupq_n_bf16(ptr[i]);
    }
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_dup_bf16
  #define vld2q_dup_bf16(a) simde_vld2q_dup_bf16((a))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_LD2_DUP_H) */
