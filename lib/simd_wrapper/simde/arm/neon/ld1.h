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
 *   2021      Zhi An Ng <zhin@google.com> (Copyright owned by Google, LLC)
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 *   2023      Chi-Wei Chu <wewe5215@gapp.nthu.edu.tw> (Copyright owned by NTHU pllab)
 */

#if !defined(SIMDE_ARM_NEON_LD1_H)
#define SIMDE_ARM_NEON_LD1_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vld1_f16(simde_float16_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vld1_f16(ptr);
  #else
    simde_float16x4_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE) && defined(SIMDE_ARCH_RISCV_ZVFH)
      r_.sv64 = __riscv_vle16_v_f16m1((_Float16 *)ptr , 4);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_f16
  #define vld1_f16(a) simde_vld1_f16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vld1_f32(simde_float32 const ptr[HEDLEY_ARRAY_PARAM(2)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1_f32(ptr);
  #else
    simde_float32x2_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv64 = __riscv_vle32_v_f32m1(ptr , 2);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_f32
  #define vld1_f32(a) simde_vld1_f32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vld1_f64(simde_float64 const ptr[HEDLEY_ARRAY_PARAM(1)]) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld1_f64(ptr);
  #else
    simde_float64x1_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv64 = __riscv_vle64_v_f64m1(ptr , 1);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld1_f64
  #define vld1_f64(a) simde_vld1_f64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vld1_s8(int8_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1_s8(ptr);
  #else
    simde_int8x8_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv64 = __riscv_vle8_v_i8m1(ptr , 8);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_s8
  #define vld1_s8(a) simde_vld1_s8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vld1_s16(int16_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1_s16(ptr);
  #else
    simde_int16x4_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv64 = __riscv_vle16_v_i16m1(ptr , 4);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_s16
  #define vld1_s16(a) simde_vld1_s16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vld1_s32(int32_t const ptr[HEDLEY_ARRAY_PARAM(2)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1_s32(ptr);
  #else
    simde_int32x2_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv64 = __riscv_vle32_v_i32m1(ptr , 2);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_s32
  #define vld1_s32(a) simde_vld1_s32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vld1_s64(int64_t const ptr[HEDLEY_ARRAY_PARAM(1)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1_s64(ptr);
  #else
    simde_int64x1_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv64 = __riscv_vle64_v_i64m1(ptr , 1);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_s64
  #define vld1_s64(a) simde_vld1_s64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vld1_u8(uint8_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1_u8(ptr);
  #else
    simde_uint8x8_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv64 = __riscv_vle8_v_u8m1(ptr , 8);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_u8
  #define vld1_u8(a) simde_vld1_u8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vld1_u16(uint16_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1_u16(ptr);
  #else
    simde_uint16x4_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv64 = __riscv_vle16_v_u16m1(ptr , 4);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_u16
  #define vld1_u16(a) simde_vld1_u16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vld1_u32(uint32_t const ptr[HEDLEY_ARRAY_PARAM(2)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1_u32(ptr);
  #else
    simde_uint32x2_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv64 = __riscv_vle32_v_u32m1(ptr , 2);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_u32
  #define vld1_u32(a) simde_vld1_u32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vld1_u64(uint64_t const ptr[HEDLEY_ARRAY_PARAM(1)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1_u64(ptr);
  #else
    simde_uint64x1_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv64 = __riscv_vle64_v_u64m1(ptr , 1);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_u64
  #define vld1_u64(a) simde_vld1_u64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vld1q_f16(simde_float16_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vld1q_f16(ptr);
  #else
    simde_float16x8_private r_;
    #if defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_load(ptr);
    #elif defined(SIMDE_RISCV_V_NATIVE) && defined(SIMDE_ARCH_RISCV_ZVFH)
      r_.sv128 = __riscv_vle16_v_f16m1((_Float16 *)ptr , 8);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1q_f16
  #define vld1q_f16(a) simde_vld1q_f16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vld1q_f32(simde_float32 const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1q_f32(ptr);
  #else
    simde_float32x4_private r_;
    #if defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_load(ptr);
    #elif defined(SIMDE_RISCV_V_NATIVE)
      r_.sv128 = __riscv_vle32_v_f32m1(ptr , 4);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1q_f32
  #define vld1q_f32(a) simde_vld1q_f32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vld1q_f64(simde_float64 const ptr[HEDLEY_ARRAY_PARAM(2)]) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld1q_f64(ptr);
  #else
    simde_float64x2_private r_;
    #if defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_load(ptr);
    #elif defined(SIMDE_RISCV_V_NATIVE)
      r_.sv128 = __riscv_vle64_v_f64m1(ptr , 2);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld1q_f64
  #define vld1q_f64(a) simde_vld1q_f64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vld1q_s8(int8_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1q_s8(ptr);
  #else
    simde_int8x16_private r_;
    #if defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_load(ptr);
    #elif defined(SIMDE_RISCV_V_NATIVE)
      r_.sv128 = __riscv_vle8_v_i8m1(ptr , 16);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1q_s8
  #define vld1q_s8(a) simde_vld1q_s8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vld1q_s16(int16_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1q_s16(ptr);
  #else
    simde_int16x8_private r_;
    #if defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_load(ptr);
    #elif defined(SIMDE_RISCV_V_NATIVE)
      r_.sv128 = __riscv_vle16_v_i16m1(ptr , 8);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1q_s16
  #define vld1q_s16(a) simde_vld1q_s16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vld1q_s32(int32_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1q_s32(ptr);
  #else
    simde_int32x4_private r_;
    #if defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_load(ptr);
    #elif defined(SIMDE_RISCV_V_NATIVE)
      r_.sv128 = __riscv_vle32_v_i32m1(ptr , 4);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1q_s32
  #define vld1q_s32(a) simde_vld1q_s32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vld1q_s64(int64_t const ptr[HEDLEY_ARRAY_PARAM(2)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1q_s64(ptr);
  #else
    simde_int64x2_private r_;
    #if defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_load(ptr);
    #elif defined(SIMDE_RISCV_V_NATIVE)
      r_.sv128 = __riscv_vle64_v_i64m1(ptr , 2);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1q_s64
  #define vld1q_s64(a) simde_vld1q_s64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vld1q_u8(uint8_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1q_u8(ptr);
  #else
    simde_uint8x16_private r_;
    #if defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_load(ptr);
    #elif defined(SIMDE_RISCV_V_NATIVE)
      r_.sv128 = __riscv_vle8_v_u8m1(ptr , 16);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1q_u8
  #define vld1q_u8(a) simde_vld1q_u8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vld1q_u16(uint16_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1q_u16(ptr);
  #else
    simde_uint16x8_private r_;
    #if defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_load(ptr);
    #elif defined(SIMDE_RISCV_V_NATIVE)
      r_.sv128 = __riscv_vle16_v_u16m1(ptr , 8);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1q_u16
  #define vld1q_u16(a) simde_vld1q_u16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vld1q_u32(uint32_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1q_u32(ptr);
  #else
    simde_uint32x4_private r_;
    #if defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_load(ptr);
    #elif defined(SIMDE_RISCV_V_NATIVE)
      r_.sv128 = __riscv_vle32_v_u32m1(ptr , 4);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1q_u32
  #define vld1q_u32(a) simde_vld1q_u32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vld1q_u64(uint64_t const ptr[HEDLEY_ARRAY_PARAM(2)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1q_u64(ptr);
  #else
    simde_uint64x2_private r_;
    #if defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_load(ptr);
    #elif defined(SIMDE_RISCV_V_NATIVE)
      r_.sv128 = __riscv_vle64_v_u64m1(ptr , 2);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1q_u64
  #define vld1q_u64(a) simde_vld1q_u64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vld1_p8(simde_poly8_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1_p8(ptr);
  #else
    simde_poly8x8_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv64 = __riscv_vle8_v_u8m1(ptr , 8);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_p8
  #define vld1_p8(a) simde_vld1_p8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vld1_p16(simde_poly16_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1_p16(ptr);
  #else
    simde_poly16x4_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv64 = __riscv_vle16_v_u16m1(ptr , 4);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_p16
  #define vld1_p16(a) simde_vld1_p16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vld1_p64(simde_poly64_t const ptr[HEDLEY_ARRAY_PARAM(1)]) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vld1_p64(ptr);
  #else
    simde_poly64x1_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv64 = __riscv_vle64_v_u64m1(ptr , 1);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_poly64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld1_p64
  #define vld1_p64(a) simde_vld1_p64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vld1q_p8(simde_poly8_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vld1q_p8(ptr);
  #else
    simde_poly8x16_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv128 = __riscv_vle8_v_u8m1(ptr , 16);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1q_p8
  #define vld1q_p8(a) simde_vld1q_p8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vld1q_p16(simde_poly16_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld1q_p16(ptr);
  #else
    simde_poly16x8_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv128 = __riscv_vle16_v_u16m1(ptr , 8);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1q_p16
  #define vld1q_p16(a) simde_vld1q_p16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vld1q_p64(simde_poly64_t const ptr[HEDLEY_ARRAY_PARAM(2)]) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vld1q_p64(ptr);
  #else
    simde_poly64x2_private r_;
    #if defined(SIMDE_RISCV_V_NATIVE)
      r_.sv128 = __riscv_vle64_v_u64m1(ptr , 2);
    #else
      simde_memcpy(&r_, ptr, sizeof(r_));
    #endif
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld1q_p64
  #define vld1q_p64(a) simde_vld1q_p64((a))
#endif

#if !defined(SIMDE_TARGET_NOT_SUPPORT_INT128_TYPE)
SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vldrq_p128(simde_poly128_t const ptr[HEDLEY_ARRAY_PARAM(1)]) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vldrq_p128(ptr);
  #else
    simde_poly128_t r_;
    simde_memcpy(&r_, ptr, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vldrq_p128
  #define vldrq_p128(a) simde_vldrq_p128((a))
#endif

#endif /* !defined(SIMDE_TARGET_NOT_SUPPORT_INT128_TYPE) */

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vld1_bf16(simde_bfloat16_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vld1_bf16(ptr);
  #else
    simde_bfloat16x4_private r_;
    simde_memcpy(&r_, ptr, sizeof(r_));
    return simde_bfloat16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld1_bf16
  #define vld1_bf16(a) simde_vld1_bf16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vld1q_bf16(simde_bfloat16_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vld1q_bf16(ptr);
  #else
    simde_bfloat16x8_private r_;
    simde_memcpy(&r_, ptr, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld1q_bf16
  #define vld1q_bf16(a) simde_vld1q_bf16((a))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_LD1_H) */
