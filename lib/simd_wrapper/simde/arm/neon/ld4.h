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
 *   2020      Sean Maher <seanptmaher@gmail.com>
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 *   2023      Chi-Wei Chu <wewe5215@gapp.nthu.edu.tw> (Copyright owned by NTHU pllab)
 */

#if !defined(SIMDE_ARM_NEON_LD4_H)
#define SIMDE_ARM_NEON_LD4_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
#if HEDLEY_GCC_VERSION_CHECK(7,0,0)
  SIMDE_DIAGNOSTIC_DISABLE_MAYBE_UNINITIAZILED_
#endif
SIMDE_BEGIN_DECLS_

#if !defined(SIMDE_BUG_INTEL_857088)

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4x4_t
simde_vld4_f16(simde_float16_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vld4_f16(ptr);
  #else
    simde_float16x4_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE) && SIMDE_ARCH_RISCV_ZVFH && (SIMDE_NATURAL_VECTOR_SIZE >= 128)
      vfloat16m1x4_t dest = __riscv_vlseg4e16_v_f16m1x4((_Float16 *)&ptr[0], 4);
      a_[0].sv64 = __riscv_vget_v_f16m1x4_f16m1(dest, 0);
      a_[1].sv64 = __riscv_vget_v_f16m1x4_f16m1(dest, 1);
      a_[2].sv64 = __riscv_vget_v_f16m1x4_f16m1(dest, 2);
      a_[3].sv64 = __riscv_vget_v_f16m1x4_f16m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_float16x4_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_float16x4x4_t s_ = { { simde_float16x4_from_private(a_[0]), simde_float16x4_from_private(a_[1]),
                                 simde_float16x4_from_private(a_[2]), simde_float16x4_from_private(a_[3]) } };
    return (s_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4_f16
  #define vld4_f16(a) simde_vld4_f16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2x4_t
simde_vld4_f32(simde_float32 const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4_f32(ptr);
  #else
    simde_float32x2_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vfloat32m1x4_t dest = __riscv_vlseg4e32_v_f32m1x4(&ptr[0], 2);
      a_[0].sv64 = __riscv_vget_v_f32m1x4_f32m1(dest, 0);
      a_[1].sv64 = __riscv_vget_v_f32m1x4_f32m1(dest, 1);
      a_[2].sv64 = __riscv_vget_v_f32m1x4_f32m1(dest, 2);
      a_[3].sv64 = __riscv_vget_v_f32m1x4_f32m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_float32x2_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_float32x2x4_t s_ = { { simde_float32x2_from_private(a_[0]), simde_float32x2_from_private(a_[1]),
                                 simde_float32x2_from_private(a_[2]), simde_float32x2_from_private(a_[3]) } };
    return (s_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4_f32
  #define vld4_f32(a) simde_vld4_f32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1x4_t
simde_vld4_f64(simde_float64 const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld4_f64(ptr);
  #else
    simde_float64x1_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vfloat64m1x4_t dest = __riscv_vlseg4e64_v_f64m1x4(&ptr[0], 1);
      a_[0].sv64 = __riscv_vget_v_f64m1x4_f64m1(dest, 0);
      a_[1].sv64 = __riscv_vget_v_f64m1x4_f64m1(dest, 1);
      a_[2].sv64 = __riscv_vget_v_f64m1x4_f64m1(dest, 2);
      a_[3].sv64 = __riscv_vget_v_f64m1x4_f64m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_float64x1_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_float64x1x4_t s_ = { { simde_float64x1_from_private(a_[0]), simde_float64x1_from_private(a_[1]),
                                 simde_float64x1_from_private(a_[2]), simde_float64x1_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld4_f64
  #define vld4_f64(a) simde_vld4_f64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8x4_t
simde_vld4_s8(int8_t const ptr[HEDLEY_ARRAY_PARAM(32)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4_s8(ptr);
  #else
    simde_int8x8_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint8m1x4_t dest = __riscv_vlseg4e8_v_i8m1x4(&ptr[0], 8);
      a_[0].sv64 = __riscv_vget_v_i8m1x4_i8m1(dest, 0);
      a_[1].sv64 = __riscv_vget_v_i8m1x4_i8m1(dest, 1);
      a_[2].sv64 = __riscv_vget_v_i8m1x4_i8m1(dest, 2);
      a_[3].sv64 = __riscv_vget_v_i8m1x4_i8m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_int8x8_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_int8x8x4_t s_ = { { simde_int8x8_from_private(a_[0]), simde_int8x8_from_private(a_[1]),
                              simde_int8x8_from_private(a_[2]), simde_int8x8_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4_s8
  #define vld4_s8(a) simde_vld4_s8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4x4_t
simde_vld4_s16(int16_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4_s16(ptr);
  #else
    simde_int16x4_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint16m1x4_t dest = __riscv_vlseg4e16_v_i16m1x4(&ptr[0], 4);
      a_[0].sv64 = __riscv_vget_v_i16m1x4_i16m1(dest, 0);
      a_[1].sv64 = __riscv_vget_v_i16m1x4_i16m1(dest, 1);
      a_[2].sv64 = __riscv_vget_v_i16m1x4_i16m1(dest, 2);
      a_[3].sv64 = __riscv_vget_v_i16m1x4_i16m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_int16x4_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_int16x4x4_t s_ = { { simde_int16x4_from_private(a_[0]), simde_int16x4_from_private(a_[1]),
                               simde_int16x4_from_private(a_[2]), simde_int16x4_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4_s16
  #define vld4_s16(a) simde_vld4_s16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2x4_t
simde_vld4_s32(int32_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4_s32(ptr);
  #else
    simde_int32x2_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint32m1x4_t dest = __riscv_vlseg4e32_v_i32m1x4(&ptr[0], 2);
      a_[0].sv64 = __riscv_vget_v_i32m1x4_i32m1(dest, 0);
      a_[1].sv64 = __riscv_vget_v_i32m1x4_i32m1(dest, 1);
      a_[2].sv64 = __riscv_vget_v_i32m1x4_i32m1(dest, 2);
      a_[3].sv64 = __riscv_vget_v_i32m1x4_i32m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_int32x2_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_int32x2x4_t s_ = { { simde_int32x2_from_private(a_[0]), simde_int32x2_from_private(a_[1]),
                               simde_int32x2_from_private(a_[2]), simde_int32x2_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4_s32
  #define vld4_s32(a) simde_vld4_s32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1x4_t
simde_vld4_s64(int64_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4_s64(ptr);
  #else
    simde_int64x1_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint64m1x4_t dest = __riscv_vlseg4e64_v_i64m1x4(&ptr[0], 1);
      a_[0].sv64 = __riscv_vget_v_i64m1x4_i64m1(dest, 0);
      a_[1].sv64 = __riscv_vget_v_i64m1x4_i64m1(dest, 1);
      a_[2].sv64 = __riscv_vget_v_i64m1x4_i64m1(dest, 2);
      a_[3].sv64 = __riscv_vget_v_i64m1x4_i64m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_int64x1_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_int64x1x4_t s_ = { { simde_int64x1_from_private(a_[0]), simde_int64x1_from_private(a_[1]),
                               simde_int64x1_from_private(a_[2]), simde_int64x1_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4_s64
  #define vld4_s64(a) simde_vld4_s64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8x4_t
simde_vld4_u8(uint8_t const ptr[HEDLEY_ARRAY_PARAM(32)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4_u8(ptr);
  #else
    simde_uint8x8_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint8m1x4_t dest = __riscv_vlseg4e8_v_u8m1x4(&ptr[0], 8);
      a_[0].sv64 = __riscv_vget_v_u8m1x4_u8m1(dest, 0);
      a_[1].sv64 = __riscv_vget_v_u8m1x4_u8m1(dest, 1);
      a_[2].sv64 = __riscv_vget_v_u8m1x4_u8m1(dest, 2);
      a_[3].sv64 = __riscv_vget_v_u8m1x4_u8m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_uint8x8_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_uint8x8x4_t s_ = { { simde_uint8x8_from_private(a_[0]), simde_uint8x8_from_private(a_[1]),
                               simde_uint8x8_from_private(a_[2]), simde_uint8x8_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4_u8
  #define vld4_u8(a) simde_vld4_u8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4x4_t
simde_vld4_u16(uint16_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4_u16(ptr);
  #else
    simde_uint16x4_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint16m1x4_t dest = __riscv_vlseg4e16_v_u16m1x4(&ptr[0], 4);
      a_[0].sv64 = __riscv_vget_v_u16m1x4_u16m1(dest, 0);
      a_[1].sv64 = __riscv_vget_v_u16m1x4_u16m1(dest, 1);
      a_[2].sv64 = __riscv_vget_v_u16m1x4_u16m1(dest, 2);
      a_[3].sv64 = __riscv_vget_v_u16m1x4_u16m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_uint16x4_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_uint16x4x4_t s_ = { { simde_uint16x4_from_private(a_[0]), simde_uint16x4_from_private(a_[1]),
                                simde_uint16x4_from_private(a_[2]), simde_uint16x4_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4_u16
  #define vld4_u16(a) simde_vld4_u16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2x4_t
simde_vld4_u32(uint32_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4_u32(ptr);
  #else
    simde_uint32x2_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint32m1x4_t dest = __riscv_vlseg4e32_v_u32m1x4(&ptr[0], 2);
      a_[0].sv64 = __riscv_vget_v_u32m1x4_u32m1(dest, 0);
      a_[1].sv64 = __riscv_vget_v_u32m1x4_u32m1(dest, 1);
      a_[2].sv64 = __riscv_vget_v_u32m1x4_u32m1(dest, 2);
      a_[3].sv64 = __riscv_vget_v_u32m1x4_u32m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_uint32x2_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_uint32x2x4_t s_ = { { simde_uint32x2_from_private(a_[0]), simde_uint32x2_from_private(a_[1]),
                                simde_uint32x2_from_private(a_[2]), simde_uint32x2_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4_u32
  #define vld4_u32(a) simde_vld4_u32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1x4_t
simde_vld4_u64(uint64_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4_u64(ptr);
  #else
    simde_uint64x1_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint64m1x4_t dest = __riscv_vlseg4e64_v_u64m1x4(&ptr[0], 1);
      a_[0].sv64 = __riscv_vget_v_u64m1x4_u64m1(dest, 0);
      a_[1].sv64 = __riscv_vget_v_u64m1x4_u64m1(dest, 1);
      a_[2].sv64 = __riscv_vget_v_u64m1x4_u64m1(dest, 2);
      a_[3].sv64 = __riscv_vget_v_u64m1x4_u64m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_uint64x1_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_uint64x1x4_t s_ = { { simde_uint64x1_from_private(a_[0]), simde_uint64x1_from_private(a_[1]),
                                simde_uint64x1_from_private(a_[2]), simde_uint64x1_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4_u64
  #define vld4_u64(a) simde_vld4_u64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8x4_t
simde_vld4q_f16(simde_float16_t const ptr[HEDLEY_ARRAY_PARAM(32)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vld4q_f16(ptr);
  #else
    simde_float16x8_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE) && SIMDE_ARCH_RISCV_ZVFH && (SIMDE_NATURAL_VECTOR_SIZE >= 128)
      vfloat16m1x4_t dest = __riscv_vlseg4e16_v_f16m1x4((_Float16 *)&ptr[0], 8);
      a_[0].sv128 = __riscv_vget_v_f16m1x4_f16m1(dest, 0);
      a_[1].sv128 = __riscv_vget_v_f16m1x4_f16m1(dest, 1);
      a_[2].sv128 = __riscv_vget_v_f16m1x4_f16m1(dest, 2);
      a_[3].sv128 = __riscv_vget_v_f16m1x4_f16m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_float16x8_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_float16x8x4_t s_ = { { simde_float16x8_from_private(a_[0]), simde_float16x8_from_private(a_[1]),
                                 simde_float16x8_from_private(a_[2]), simde_float16x8_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4q_f16
  #define vld4q_f16(a) simde_vld4q_f16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4x4_t
simde_vld4q_f32(simde_float32 const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4q_f32(ptr);
  #else
    simde_float32x4_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vfloat32m1x4_t dest = __riscv_vlseg4e32_v_f32m1x4(&ptr[0], 4);
      a_[0].sv128 = __riscv_vget_v_f32m1x4_f32m1(dest, 0);
      a_[1].sv128 = __riscv_vget_v_f32m1x4_f32m1(dest, 1);
      a_[2].sv128 = __riscv_vget_v_f32m1x4_f32m1(dest, 2);
      a_[3].sv128 = __riscv_vget_v_f32m1x4_f32m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_float32x4_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_float32x4x4_t s_ = { { simde_float32x4_from_private(a_[0]), simde_float32x4_from_private(a_[1]),
                                 simde_float32x4_from_private(a_[2]), simde_float32x4_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4q_f32
  #define vld4q_f32(a) simde_vld4q_f32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2x4_t
simde_vld4q_f64(simde_float64 const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld4q_f64(ptr);
  #else
    simde_float64x2_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vfloat64m1x4_t dest = __riscv_vlseg4e64_v_f64m1x4(&ptr[0], 2);
      a_[0].sv128 = __riscv_vget_v_f64m1x4_f64m1(dest, 0);
      a_[1].sv128 = __riscv_vget_v_f64m1x4_f64m1(dest, 1);
      a_[2].sv128 = __riscv_vget_v_f64m1x4_f64m1(dest, 2);
      a_[3].sv128 = __riscv_vget_v_f64m1x4_f64m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_float64x2_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_float64x2x4_t s_ = { { simde_float64x2_from_private(a_[0]), simde_float64x2_from_private(a_[1]),
                                 simde_float64x2_from_private(a_[2]), simde_float64x2_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld4q_f64
  #define vld4q_f64(a) simde_vld4q_f64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16x4_t
simde_vld4q_s8(int8_t const ptr[HEDLEY_ARRAY_PARAM(64)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4q_s8(ptr);
  #else
    simde_int8x16_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint8m1x4_t dest = __riscv_vlseg4e8_v_i8m1x4(&ptr[0], 16);
      a_[0].sv128 = __riscv_vget_v_i8m1x4_i8m1(dest, 0);
      a_[1].sv128 = __riscv_vget_v_i8m1x4_i8m1(dest, 1);
      a_[2].sv128 = __riscv_vget_v_i8m1x4_i8m1(dest, 2);
      a_[3].sv128 = __riscv_vget_v_i8m1x4_i8m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_int8x16_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_int8x16x4_t s_ = { { simde_int8x16_from_private(a_[0]), simde_int8x16_from_private(a_[1]),
                               simde_int8x16_from_private(a_[2]), simde_int8x16_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4q_s8
  #define vld4q_s8(a) simde_vld4q_s8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8x4_t
simde_vld4q_s16(int16_t const ptr[HEDLEY_ARRAY_PARAM(32)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4q_s16(ptr);
  #else
    simde_int16x8_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint16m1x4_t dest = __riscv_vlseg4e16_v_i16m1x4(&ptr[0], 8);
      a_[0].sv128 = __riscv_vget_v_i16m1x4_i16m1(dest, 0);
      a_[1].sv128 = __riscv_vget_v_i16m1x4_i16m1(dest, 1);
      a_[2].sv128 = __riscv_vget_v_i16m1x4_i16m1(dest, 2);
      a_[3].sv128 = __riscv_vget_v_i16m1x4_i16m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_int16x8_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_int16x8x4_t s_ = { { simde_int16x8_from_private(a_[0]), simde_int16x8_from_private(a_[1]),
                               simde_int16x8_from_private(a_[2]), simde_int16x8_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4q_s16
  #define vld4q_s16(a) simde_vld4q_s16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4x4_t
simde_vld4q_s32(int32_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4q_s32(ptr);
  #else
    simde_int32x4_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint32m1x4_t dest = __riscv_vlseg4e32_v_i32m1x4(&ptr[0], 4);
      a_[0].sv128 = __riscv_vget_v_i32m1x4_i32m1(dest, 0);
      a_[1].sv128 = __riscv_vget_v_i32m1x4_i32m1(dest, 1);
      a_[2].sv128 = __riscv_vget_v_i32m1x4_i32m1(dest, 2);
      a_[3].sv128 = __riscv_vget_v_i32m1x4_i32m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_int32x4_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_int32x4x4_t s_ = { { simde_int32x4_from_private(a_[0]), simde_int32x4_from_private(a_[1]),
                               simde_int32x4_from_private(a_[2]), simde_int32x4_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4q_s32
  #define vld4q_s32(a) simde_vld4q_s32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2x4_t
simde_vld4q_s64(int64_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld4q_s64(ptr);
  #else
    simde_int64x2_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint64m1x4_t dest = __riscv_vlseg4e64_v_i64m1x4(&ptr[0], 2);
      a_[0].sv128 = __riscv_vget_v_i64m1x4_i64m1(dest, 0);
      a_[1].sv128 = __riscv_vget_v_i64m1x4_i64m1(dest, 1);
      a_[2].sv128 = __riscv_vget_v_i64m1x4_i64m1(dest, 2);
      a_[3].sv128 = __riscv_vget_v_i64m1x4_i64m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_int64x2_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_int64x2x4_t s_ = { { simde_int64x2_from_private(a_[0]), simde_int64x2_from_private(a_[1]),
                               simde_int64x2_from_private(a_[2]), simde_int64x2_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld4q_s64
  #define vld4q_s64(a) simde_vld4q_s64((a))
#endif
SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16x4_t
simde_vld4q_u8(uint8_t const ptr[HEDLEY_ARRAY_PARAM(64)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4q_u8(ptr);
  #elif defined(SIMDE_WASM_SIMD128_NATIVE)
    // Let a, b, c, d be the 4 uint8x16 to return, they are laid out in memory:
    // [a0, b0, c0, d0, a1, b1, c1, d1, a2, b2, c2, d2, a3, b3, c3, d3,
    //  a4, b4, c4, d4, a5, b5, c5, d5, a6, b6, c6, d6, a7, b7, c7, d7,
    //  a8, b8, c8, d8, a9, b9, c9, d9, a10, b10, c10, d10, a11, b11, c11, d11,
    //  a12, b12, c12, d12, a13, b13, c13, d13, a14, b14, c14, d14, a15, b15, c15, d15]
    v128_t a_ = wasm_v128_load(&ptr[0]);
    v128_t b_ = wasm_v128_load(&ptr[16]);
    v128_t c_ = wasm_v128_load(&ptr[32]);
    v128_t d_ = wasm_v128_load(&ptr[48]);

    v128_t a_low_b_low = wasm_i8x16_shuffle(a_, b_, 0, 4, 8, 12, 16, 20, 24, 28,
                                            1, 5, 9, 13, 17, 21, 25, 29);
    v128_t a_high_b_high = wasm_i8x16_shuffle(c_, d_, 0, 4, 8, 12, 16, 20, 24,
                                              28, 1, 5, 9, 13, 17, 21, 25, 29);
    v128_t a = wasm_i8x16_shuffle(a_low_b_low, a_high_b_high, 0, 1, 2, 3, 4, 5,
                                  6, 7, 16, 17, 18, 19, 20, 21, 22, 23);
    v128_t b = wasm_i8x16_shuffle(a_low_b_low, a_high_b_high, 8, 9, 10, 11, 12,
                                  13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31);

    v128_t c_low_d_low = wasm_i8x16_shuffle(a_, b_, 2, 6, 10, 14, 18, 22, 26,
                                            30, 3, 7, 11, 15, 19, 23, 27, 31);
    v128_t c_high_d_high = wasm_i8x16_shuffle(c_, d_, 2, 6, 10, 14, 18, 22, 26,
                                              30, 3, 7, 11, 15, 19, 23, 27, 31);
    v128_t c = wasm_i8x16_shuffle(c_low_d_low, c_high_d_high, 0, 1, 2, 3, 4, 5,
                                  6, 7, 16, 17, 18, 19, 20, 21, 22, 23);
    v128_t d = wasm_i8x16_shuffle(c_low_d_low, c_high_d_high, 8, 9, 10, 11, 12,
                                  13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31);

    simde_uint8x16_private r_[4];
    r_[0].v128 = a;
    r_[1].v128 = b;
    r_[2].v128 = c;
    r_[3].v128 = d;
    simde_uint8x16x4_t s_ = {{simde_uint8x16_from_private(r_[0]),
                              simde_uint8x16_from_private(r_[1]),
                              simde_uint8x16_from_private(r_[2]),
                              simde_uint8x16_from_private(r_[3])}};
    return s_;
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint8x16_private r_[4];
    vuint8m1x4_t dest = __riscv_vlseg4e8_v_u8m1x4(&ptr[0], 16);
    r_[0].sv128 = __riscv_vget_v_u8m1x4_u8m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_u8m1x4_u8m1(dest, 1);
    r_[2].sv128 = __riscv_vget_v_u8m1x4_u8m1(dest, 2);
    r_[3].sv128 = __riscv_vget_v_u8m1x4_u8m1(dest, 3);
    simde_uint8x16x4_t r = { {
      simde_uint8x16_from_private(r_[0]),
      simde_uint8x16_from_private(r_[1]),
      simde_uint8x16_from_private(r_[2]),
      simde_uint8x16_from_private(r_[3])
    } };
    return r;
  #else
    simde_uint8x16_private a_[4];
    for (size_t i = 0; i < (sizeof(simde_uint8x16_t) / sizeof(*ptr)) * 4 ; i++) {
      a_[i % 4].values[i / 4] = ptr[i];
    }
    simde_uint8x16x4_t s_ = { { simde_uint8x16_from_private(a_[0]), simde_uint8x16_from_private(a_[1]),
                                simde_uint8x16_from_private(a_[2]), simde_uint8x16_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4q_u8
  #define vld4q_u8(a) simde_vld4q_u8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8x4_t
simde_vld4q_u16(uint16_t const ptr[HEDLEY_ARRAY_PARAM(32)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4q_u16(ptr);
  #else
    simde_uint16x8_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint16m1x4_t dest = __riscv_vlseg4e16_v_u16m1x4(&ptr[0], 8);
      a_[0].sv128 = __riscv_vget_v_u16m1x4_u16m1(dest, 0);
      a_[1].sv128 = __riscv_vget_v_u16m1x4_u16m1(dest, 1);
      a_[2].sv128 = __riscv_vget_v_u16m1x4_u16m1(dest, 2);
      a_[3].sv128 = __riscv_vget_v_u16m1x4_u16m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_uint16x8_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_uint16x8x4_t s_ = { { simde_uint16x8_from_private(a_[0]), simde_uint16x8_from_private(a_[1]),
                                simde_uint16x8_from_private(a_[2]), simde_uint16x8_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4q_u16
  #define vld4q_u16(a) simde_vld4q_u16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4x4_t
simde_vld4q_u32(uint32_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4q_u32(ptr);
  #else
    simde_uint32x4_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint32m1x4_t dest = __riscv_vlseg4e32_v_u32m1x4(&ptr[0], 4);
      a_[0].sv128 = __riscv_vget_v_u32m1x4_u32m1(dest, 0);
      a_[1].sv128 = __riscv_vget_v_u32m1x4_u32m1(dest, 1);
      a_[2].sv128 = __riscv_vget_v_u32m1x4_u32m1(dest, 2);
      a_[3].sv128 = __riscv_vget_v_u32m1x4_u32m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_uint32x4_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_uint32x4x4_t s_ = { { simde_uint32x4_from_private(a_[0]), simde_uint32x4_from_private(a_[1]),
                                simde_uint32x4_from_private(a_[2]), simde_uint32x4_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4q_u32
  #define vld4q_u32(a) simde_vld4q_u32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2x4_t
simde_vld4q_u64(uint64_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld4q_u64(ptr);
  #else
    simde_uint64x2_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint64m1x4_t dest = __riscv_vlseg4e64_v_u64m1x4(&ptr[0], 2);
      a_[0].sv128 = __riscv_vget_v_u64m1x4_u64m1(dest, 0);
      a_[1].sv128 = __riscv_vget_v_u64m1x4_u64m1(dest, 1);
      a_[2].sv128 = __riscv_vget_v_u64m1x4_u64m1(dest, 2);
      a_[3].sv128 = __riscv_vget_v_u64m1x4_u64m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_uint64x2_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_uint64x2x4_t s_ = { { simde_uint64x2_from_private(a_[0]), simde_uint64x2_from_private(a_[1]),
                                simde_uint64x2_from_private(a_[2]), simde_uint64x2_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld4q_u64
  #define vld4q_u64(a) simde_vld4q_u64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8x4_t
simde_vld4_p8(simde_poly8_t const ptr[HEDLEY_ARRAY_PARAM(32)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4_p8(ptr);
  #else
    simde_poly8x8_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint8m1x4_t dest = __riscv_vlseg4e8_v_u8m1x4(&ptr[0], 8);
      a_[0].sv64 = __riscv_vget_v_u8m1x4_u8m1(dest, 0);
      a_[1].sv64 = __riscv_vget_v_u8m1x4_u8m1(dest, 1);
      a_[2].sv64 = __riscv_vget_v_u8m1x4_u8m1(dest, 2);
      a_[3].sv64 = __riscv_vget_v_u8m1x4_u8m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_poly8x8_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_poly8x8x4_t s_ = { { simde_poly8x8_from_private(a_[0]), simde_poly8x8_from_private(a_[1]),
                               simde_poly8x8_from_private(a_[2]), simde_poly8x8_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4_p8
  #define vld4_p8(a) simde_vld4_p8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4x4_t
simde_vld4_p16(simde_poly16_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4_p16(ptr);
  #else
    simde_poly16x4_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint16m1x4_t dest = __riscv_vlseg4e16_v_u16m1x4(&ptr[0], 4);
      a_[0].sv64 = __riscv_vget_v_u16m1x4_u16m1(dest, 0);
      a_[1].sv64 = __riscv_vget_v_u16m1x4_u16m1(dest, 1);
      a_[2].sv64 = __riscv_vget_v_u16m1x4_u16m1(dest, 2);
      a_[3].sv64 = __riscv_vget_v_u16m1x4_u16m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_poly16x4_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_poly16x4x4_t s_ = { { simde_poly16x4_from_private(a_[0]), simde_poly16x4_from_private(a_[1]),
                                simde_poly16x4_from_private(a_[2]), simde_poly16x4_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4_p16
  #define vld4_p16(a) simde_vld4_p16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1x4_t
simde_vld4_p64(simde_poly64_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vld4_p64(ptr);
  #else
    simde_poly64x1_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint64m1x4_t dest = __riscv_vlseg4e64_v_u64m1x4(&ptr[0], 1);
      a_[0].sv64 = __riscv_vget_v_u64m1x4_u64m1(dest, 0);
      a_[1].sv64 = __riscv_vget_v_u64m1x4_u64m1(dest, 1);
      a_[2].sv64 = __riscv_vget_v_u64m1x4_u64m1(dest, 2);
      a_[3].sv64 = __riscv_vget_v_u64m1x4_u64m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_poly64x1_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_poly64x1x4_t s_ = { { simde_poly64x1_from_private(a_[0]), simde_poly64x1_from_private(a_[1]),
                                simde_poly64x1_from_private(a_[2]), simde_poly64x1_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld4_p64
  #define vld4_p64(a) simde_vld4_p64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16x4_t
simde_vld4q_p8(simde_poly8_t const ptr[HEDLEY_ARRAY_PARAM(64)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4q_p8(ptr);
  #else
    simde_poly8x16_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint8m1x4_t dest = __riscv_vlseg4e8_v_u8m1x4(&ptr[0], 16);
      a_[0].sv128 = __riscv_vget_v_u8m1x4_u8m1(dest, 0);
      a_[1].sv128 = __riscv_vget_v_u8m1x4_u8m1(dest, 1);
      a_[2].sv128 = __riscv_vget_v_u8m1x4_u8m1(dest, 2);
      a_[3].sv128 = __riscv_vget_v_u8m1x4_u8m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_poly8x16_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_poly8x16x4_t s_ = { { simde_poly8x16_from_private(a_[0]), simde_poly8x16_from_private(a_[1]),
                                simde_poly8x16_from_private(a_[2]), simde_poly8x16_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4q_p8
  #define vld4q_p8(a) simde_vld4q_p8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8x4_t
simde_vld4q_p16(simde_poly16_t const ptr[HEDLEY_ARRAY_PARAM(32)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld4q_p16(ptr);
  #else
    simde_poly16x8_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint16m1x4_t dest = __riscv_vlseg4e16_v_u16m1x4(&ptr[0], 8);
      a_[0].sv128 = __riscv_vget_v_u16m1x4_u16m1(dest, 0);
      a_[1].sv128 = __riscv_vget_v_u16m1x4_u16m1(dest, 1);
      a_[2].sv128 = __riscv_vget_v_u16m1x4_u16m1(dest, 2);
      a_[3].sv128 = __riscv_vget_v_u16m1x4_u16m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_poly16x8_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_poly16x8x4_t s_ = { { simde_poly16x8_from_private(a_[0]), simde_poly16x8_from_private(a_[1]),
                                simde_poly16x8_from_private(a_[2]), simde_poly16x8_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld4q_p16
  #define vld4q_p16(a) simde_vld4q_p16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2x4_t
simde_vld4q_p64(simde_poly64_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld4q_p64(ptr);
  #else
    simde_poly64x2_private a_[4];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint64m1x4_t dest = __riscv_vlseg4e64_v_u64m1x4(&ptr[0], 2);
      a_[0].sv128 = __riscv_vget_v_u64m1x4_u64m1(dest, 0);
      a_[1].sv128 = __riscv_vget_v_u64m1x4_u64m1(dest, 1);
      a_[2].sv128 = __riscv_vget_v_u64m1x4_u64m1(dest, 2);
      a_[3].sv128 = __riscv_vget_v_u64m1x4_u64m1(dest, 3);
    #else
      for (size_t i = 0; i < (sizeof(simde_poly64x2_t) / sizeof(*ptr)) * 4 ; i++) {
        a_[i % 4].values[i / 4] = ptr[i];
      }
    #endif
    simde_poly64x2x4_t s_ = { { simde_poly64x2_from_private(a_[0]), simde_poly64x2_from_private(a_[1]),
                                simde_poly64x2_from_private(a_[2]), simde_poly64x2_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld4q_p64
  #define vld4q_p64(a) simde_vld4q_p64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4x4_t
simde_vld4_bf16(simde_bfloat16 const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vld4_bf16(ptr);
  #else
    simde_bfloat16x4_private a_[4];
    for (size_t i = 0; i < (sizeof(simde_bfloat16x4_t) / sizeof(*ptr)) * 4 ; i++) {
      a_[i % 4].values[i / 4] = ptr[i];
    }
    simde_bfloat16x4x4_t s_ = { { simde_bfloat16x4_from_private(a_[0]), simde_bfloat16x4_from_private(a_[1]),
                                 simde_bfloat16x4_from_private(a_[2]), simde_bfloat16x4_from_private(a_[3]) } };
    return (s_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld4_bf16
  #define vld4_bf16(a) simde_vld4_bf16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8x4_t
simde_vld4q_bf16(simde_bfloat16 const ptr[HEDLEY_ARRAY_PARAM(32)]) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vld4q_bf16(ptr);
  #else
    simde_bfloat16x8_private a_[4];
    for (size_t i = 0; i < (sizeof(simde_bfloat16x8_t) / sizeof(*ptr)) * 4 ; i++) {
      a_[i % 4].values[i / 4] = ptr[i];
    }
    simde_bfloat16x8x4_t s_ = { { simde_bfloat16x8_from_private(a_[0]), simde_bfloat16x8_from_private(a_[1]),
                                 simde_bfloat16x8_from_private(a_[2]), simde_bfloat16x8_from_private(a_[3]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld4q_bf16
  #define vld4q_bf16(a) simde_vld4q_bf16((a))
#endif

#endif /* !defined(SIMDE_BUG_INTEL_857088) */

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_LD4_H) */
