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
 *   2021      Zhi An Ng <zhin@google.com> (Copyright owned by Google, LLC)
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 *   2023      Chi-Wei Chu <wewe5215@gapp.nthu.edu.tw> (Copyright owned by NTHU pllab)
 */

#if !defined(SIMDE_ARM_NEON_LD2_H)
#define SIMDE_ARM_NEON_LD2_H

#include "get_low.h"
#include "get_high.h"
#include "ld1.h"
#include "uzp.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
#if HEDLEY_GCC_VERSION_CHECK(7,0,0)
  SIMDE_DIAGNOSTIC_DISABLE_MAYBE_UNINITIAZILED_
#endif
SIMDE_BEGIN_DECLS_

#if !defined(SIMDE_BUG_INTEL_857088)

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8x2_t
simde_vld2_s8(int8_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_s8(ptr);
  #elif defined(SIMDE_WASM_SIMD128_NATIVE)
    v128_t a = wasm_v128_load(ptr);
    simde_int8x16_private q_;
    q_.v128 = wasm_i8x16_shuffle(a, a, 0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15);
    simde_int8x16_t q = simde_int8x16_from_private(q_);

    simde_int8x8x2_t u = {
      simde_vget_low_s8(q),
      simde_vget_high_s8(q)
    };
    return u;
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int8x8_private a_[2];
    vint8m1x2_t dest = __riscv_vlseg2e8_v_i8m1x2(&ptr[0], 8);
    a_[0].sv64 = __riscv_vget_v_i8m1x2_i8m1(dest, 0);
    a_[1].sv64 = __riscv_vget_v_i8m1x2_i8m1(dest, 1);
    simde_int8x8x2_t r = { {
      simde_int8x8_from_private(a_[0]),
      simde_int8x8_from_private(a_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128) && defined(SIMDE_SHUFFLE_VECTOR_)
    simde_int8x16_private a_ = simde_int8x16_to_private(simde_vld1q_s8(ptr));
    a_.values = SIMDE_SHUFFLE_VECTOR_(8, 16, a_.values, a_.values, 0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15);
    simde_int8x8x2_t r;
    simde_memcpy(&r, &a_, sizeof(r));
    return r;
  #else
    simde_int8x8_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_int8x8x2_t r = { {
      simde_int8x8_from_private(r_[0]),
      simde_int8x8_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_s8
  #define vld2_s8(a) simde_vld2_s8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4x2_t
simde_vld2_s16(int16_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_s16(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int16x4_private a_[2];
    vint16m1x2_t dest = __riscv_vlseg2e16_v_i16m1x2(&ptr[0], 4);
    a_[0].sv64 = __riscv_vget_v_i16m1x2_i16m1(dest, 0);
    a_[1].sv64 = __riscv_vget_v_i16m1x2_i16m1(dest, 1);
    simde_int16x4x2_t r = { {
      simde_int16x4_from_private(a_[0]),
      simde_int16x4_from_private(a_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128) && defined(SIMDE_SHUFFLE_VECTOR_)
    simde_int16x8_private a_ = simde_int16x8_to_private(simde_vld1q_s16(ptr));
    a_.values = SIMDE_SHUFFLE_VECTOR_(16, 16, a_.values, a_.values, 0, 2, 4, 6, 1, 3, 5, 7);
    simde_int16x4x2_t r;
    simde_memcpy(&r, &a_, sizeof(r));
    return r;
  #else
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0)
      HEDLEY_DIAGNOSTIC_PUSH
      SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_
    #endif
    simde_int16x4_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0)
      HEDLEY_DIAGNOSTIC_POP
    #endif

    simde_int16x4x2_t r = { {
      simde_int16x4_from_private(r_[0]),
      simde_int16x4_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_s16
  #define vld2_s16(a) simde_vld2_s16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2x2_t
simde_vld2_s32(int32_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_s32(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int32x2_private a_[2];
    vint32m1x2_t dest = __riscv_vlseg2e32_v_i32m1x2(&ptr[0], 2);
    a_[0].sv64 = __riscv_vget_v_i32m1x2_i32m1(dest, 0);
    a_[1].sv64 = __riscv_vget_v_i32m1x2_i32m1(dest, 1);
    simde_int32x2x2_t r = { {
      simde_int32x2_from_private(a_[0]),
      simde_int32x2_from_private(a_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128) && defined(SIMDE_SHUFFLE_VECTOR_)
    simde_int32x4_private a_ = simde_int32x4_to_private(simde_vld1q_s32(ptr));
    a_.values = SIMDE_SHUFFLE_VECTOR_(32, 16, a_.values, a_.values, 0, 2, 1, 3);
    simde_int32x2x2_t r;
    simde_memcpy(&r, &a_, sizeof(r));
    return r;
  #else
    simde_int32x2_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_int32x2x2_t r = { {
      simde_int32x2_from_private(r_[0]),
      simde_int32x2_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_s32
  #define vld2_s32(a) simde_vld2_s32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1x2_t
simde_vld2_s64(int64_t const ptr[HEDLEY_ARRAY_PARAM(2)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_s64(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int64x1_private a_[2];
    vint64m1x2_t dest = __riscv_vlseg2e64_v_i64m1x2(&ptr[0], 1);
    a_[0].sv64 = __riscv_vget_v_i64m1x2_i64m1(dest, 0);
    a_[1].sv64 = __riscv_vget_v_i64m1x2_i64m1(dest, 1);
    simde_int64x1x2_t r = { {
      simde_int64x1_from_private(a_[0]),
      simde_int64x1_from_private(a_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128) && defined(SIMDE_SHUFFLE_VECTOR_)
    simde_int64x2_private a_ = simde_int64x2_to_private(simde_vld1q_s64(ptr));
    a_.values = SIMDE_SHUFFLE_VECTOR_(64, 16, a_.values, a_.values, 0, 1);
    simde_int64x1x2_t r;
    simde_memcpy(&r, &a_, sizeof(r));
    return r;
  #else
    simde_int64x1_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_int64x1x2_t r = { {
      simde_int64x1_from_private(r_[0]),
      simde_int64x1_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_s64
  #define vld2_s64(a) simde_vld2_s64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8x2_t
simde_vld2_u8(uint8_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_u8(ptr);
  #elif defined(SIMDE_WASM_SIMD128_NATIVE)
    v128_t a = wasm_v128_load(ptr);
    simde_uint8x16_private q_;
    q_.v128 = wasm_i8x16_shuffle(a, a, 0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15);
    simde_uint8x16_t q = simde_uint8x16_from_private(q_);

    simde_uint8x8x2_t u = {
      simde_vget_low_u8(q),
      simde_vget_high_u8(q)
    };
    return u;
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint8x8_private a_[2];
    vuint8m1x2_t dest = __riscv_vlseg2e8_v_u8m1x2(&ptr[0], 8);
    a_[0].sv64 = __riscv_vget_v_u8m1x2_u8m1(dest, 0);
    a_[1].sv64 = __riscv_vget_v_u8m1x2_u8m1(dest, 1);
    simde_uint8x8x2_t r = { {
      simde_uint8x8_from_private(a_[0]),
      simde_uint8x8_from_private(a_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128) && defined(SIMDE_SHUFFLE_VECTOR_)
    simde_uint8x16_private a_ = simde_uint8x16_to_private(simde_vld1q_u8(ptr));
    a_.values = SIMDE_SHUFFLE_VECTOR_(8, 16, a_.values, a_.values, 0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15);
    simde_uint8x8x2_t r;
    simde_memcpy(&r, &a_, sizeof(r));
    return r;
  #else
    simde_uint8x8_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_uint8x8x2_t r = { {
      simde_uint8x8_from_private(r_[0]),
      simde_uint8x8_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_u8
  #define vld2_u8(a) simde_vld2_u8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4x2_t
simde_vld2_u16(uint16_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_u16(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint16x4_private a_[2];
    vuint16m1x2_t dest = __riscv_vlseg2e16_v_u16m1x2(&ptr[0], 4);
    a_[0].sv64 = __riscv_vget_v_u16m1x2_u16m1(dest, 0);
    a_[1].sv64 = __riscv_vget_v_u16m1x2_u16m1(dest, 1);
    simde_uint16x4x2_t r = { {
      simde_uint16x4_from_private(a_[0]),
      simde_uint16x4_from_private(a_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128) && defined(SIMDE_SHUFFLE_VECTOR_)
    simde_uint16x8_private a_ = simde_uint16x8_to_private(simde_vld1q_u16(ptr));
    a_.values = SIMDE_SHUFFLE_VECTOR_(16, 16, a_.values, a_.values, 0, 2, 4, 6, 1, 3, 5, 7);
    simde_uint16x4x2_t r;
    simde_memcpy(&r, &a_, sizeof(r));
    return r;
  #else
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0)
      HEDLEY_DIAGNOSTIC_PUSH
      SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_
    #endif
    simde_uint16x4_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0)
      HEDLEY_DIAGNOSTIC_POP
    #endif

    simde_uint16x4x2_t r = { {
      simde_uint16x4_from_private(r_[0]),
      simde_uint16x4_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_u16
  #define vld2_u16(a) simde_vld2_u16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2x2_t
simde_vld2_u32(uint32_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_u32(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint32x2_private a_[2];
    vuint32m1x2_t dest = __riscv_vlseg2e32_v_u32m1x2(&ptr[0], 2);
    a_[0].sv64 = __riscv_vget_v_u32m1x2_u32m1(dest, 0);
    a_[1].sv64 = __riscv_vget_v_u32m1x2_u32m1(dest, 1);
    simde_uint32x2x2_t r = { {
      simde_uint32x2_from_private(a_[0]),
      simde_uint32x2_from_private(a_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128) && defined(SIMDE_SHUFFLE_VECTOR_)
    simde_uint32x4_private a_ = simde_uint32x4_to_private(simde_vld1q_u32(ptr));
    a_.values = SIMDE_SHUFFLE_VECTOR_(32, 16, a_.values, a_.values, 0, 2, 1, 3);
    simde_uint32x2x2_t r;
    simde_memcpy(&r, &a_, sizeof(r));
    return r;
  #else
    simde_uint32x2_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_uint32x2x2_t r = { {
      simde_uint32x2_from_private(r_[0]),
      simde_uint32x2_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_u32
  #define vld2_u32(a) simde_vld2_u32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1x2_t
simde_vld2_u64(uint64_t const ptr[HEDLEY_ARRAY_PARAM(2)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_u64(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint64x1_private a_[2];
    vuint64m1x2_t dest = __riscv_vlseg2e64_v_u64m1x2(&ptr[0], 1);
    a_[0].sv64 = __riscv_vget_v_u64m1x2_u64m1(dest, 0);
    a_[1].sv64 = __riscv_vget_v_u64m1x2_u64m1(dest, 1);
    simde_uint64x1x2_t r = { {
      simde_uint64x1_from_private(a_[0]),
      simde_uint64x1_from_private(a_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128) && defined(SIMDE_SHUFFLE_VECTOR_)
    simde_uint64x2_private a_ = simde_uint64x2_to_private(simde_vld1q_u64(ptr));
    a_.values = SIMDE_SHUFFLE_VECTOR_(64, 16, a_.values, a_.values, 0, 1);
    simde_uint64x1x2_t r;
    simde_memcpy(&r, &a_, sizeof(r));
    return r;
  #else
    simde_uint64x1_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_uint64x1x2_t r = { {
      simde_uint64x1_from_private(r_[0]),
      simde_uint64x1_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_u64
  #define vld2_u64(a) simde_vld2_u64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4x2_t
simde_vld2_f16(simde_float16_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vld2_f16(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE) && SIMDE_ARCH_RISCV_ZVFH && (SIMDE_NATURAL_VECTOR_SIZE >= 128)
    simde_float16x4_private r_[2];
    vfloat16m1x2_t dest = __riscv_vlseg2e16_v_f16m1x2((_Float16 *)&ptr[0], 4);
    r_[0].sv64 = __riscv_vget_v_f16m1x2_f16m1(dest, 0);
    r_[1].sv64 = __riscv_vget_v_f16m1x2_f16m1(dest, 1);
    simde_float16x4x2_t r = { {
      simde_float16x4_from_private(r_[0]),
      simde_float16x4_from_private(r_[1]),
    } };
    return r;
  #else
    simde_float16x4_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_float16x4x2_t r = { {
      simde_float16x4_from_private(r_[0]),
      simde_float16x4_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_f16
  #define vld2_f16(a) simde_vld2_f16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2x2_t
simde_vld2_f32(simde_float32_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_f32(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_float32x2_private r_[2];
    vfloat32m1x2_t dest = __riscv_vlseg2e32_v_f32m1x2(&ptr[0], 2);
    r_[0].sv64 = __riscv_vget_v_f32m1x2_f32m1(dest, 0);
    r_[1].sv64 = __riscv_vget_v_f32m1x2_f32m1(dest, 1);
    simde_float32x2x2_t r = { {
      simde_float32x2_from_private(r_[0]),
      simde_float32x2_from_private(r_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128) && defined(SIMDE_SHUFFLE_VECTOR_)
    simde_float32x4_private a_ = simde_float32x4_to_private(simde_vld1q_f32(ptr));
    a_.values = SIMDE_SHUFFLE_VECTOR_(32, 16, a_.values, a_.values, 0, 2, 1, 3);
    simde_float32x2x2_t r;
    simde_memcpy(&r, &a_, sizeof(r));
    return r;
  #else
    simde_float32x2_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_float32x2x2_t r = { {
      simde_float32x2_from_private(r_[0]),
      simde_float32x2_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_f32
  #define vld2_f32(a) simde_vld2_f32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1x2_t
simde_vld2_f64(simde_float64_t const ptr[HEDLEY_ARRAY_PARAM(2)]) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2_f64(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_float64x1_private r_[2];
    vfloat64m1x2_t dest = __riscv_vlseg2e64_v_f64m1x2(&ptr[0], 1);
    r_[0].sv64 = __riscv_vget_v_f64m1x2_f64m1(dest, 0);
    r_[1].sv64 = __riscv_vget_v_f64m1x2_f64m1(dest, 1);
    simde_float64x1x2_t r = { {
      simde_float64x1_from_private(r_[0]),
      simde_float64x1_from_private(r_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128) && defined(SIMDE_SHUFFLE_VECTOR_)
    simde_float64x2_private a_ = simde_float64x2_to_private(simde_vld1q_f64(ptr));
    a_.values = SIMDE_SHUFFLE_VECTOR_(64, 16, a_.values, a_.values, 0, 1);
    simde_float64x1x2_t r;
    simde_memcpy(&r, &a_, sizeof(r));
    return r;
  #else
    simde_float64x1_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_float64x1x2_t r = { {
      simde_float64x1_from_private(r_[0]),
      simde_float64x1_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2_f64
  #define vld2_f64(a) simde_vld2_f64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16x2_t
simde_vld2q_s8(int8_t const ptr[HEDLEY_ARRAY_PARAM(32)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2q_s8(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int8x16_private a_[2];
    vint8m1x2_t dest = __riscv_vlseg2e8_v_i8m1x2(&ptr[0], 16);
    a_[0].sv128 = __riscv_vget_v_i8m1x2_i8m1(dest, 0);
    a_[1].sv128 = __riscv_vget_v_i8m1x2_i8m1(dest, 1);
    simde_int8x16x2_t r = { {
      simde_int8x16_from_private(a_[0]),
      simde_int8x16_from_private(a_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128)
    return
      simde_vuzpq_s8(
        simde_vld1q_s8(&(ptr[0])),
        simde_vld1q_s8(&(ptr[16]))
      );
  #else
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0) && defined(SIMDE_ARCH_RISCV64)
      HEDLEY_DIAGNOSTIC_PUSH
      SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_
    #endif
    simde_int8x16_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_int8x16x2_t r = { {
      simde_int8x16_from_private(r_[0]),
      simde_int8x16_from_private(r_[1]),
    } };

    return r;
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0) && defined(SIMDE_ARCH_RISCV64)
      HEDLEY_DIAGNOSTIC_POP
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2q_s8
  #define vld2q_s8(a) simde_vld2q_s8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4x2_t
simde_vld2q_s32(int32_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2q_s32(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int32x4_private a_[2];
    vint32m1x2_t dest = __riscv_vlseg2e32_v_i32m1x2(&ptr[0], 4);
    a_[0].sv128 = __riscv_vget_v_i32m1x2_i32m1(dest, 0);
    a_[1].sv128 = __riscv_vget_v_i32m1x2_i32m1(dest, 1);
    simde_int32x4x2_t r = { {
      simde_int32x4_from_private(a_[0]),
      simde_int32x4_from_private(a_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128)
    return
      simde_vuzpq_s32(
        simde_vld1q_s32(&(ptr[0])),
        simde_vld1q_s32(&(ptr[4]))
      );
  #else
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0)
      HEDLEY_DIAGNOSTIC_PUSH
      SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_
    #endif
    simde_int32x4_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0)
      HEDLEY_DIAGNOSTIC_POP
    #endif

    simde_int32x4x2_t r = { {
      simde_int32x4_from_private(r_[0]),
      simde_int32x4_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2q_s32
  #define vld2q_s32(a) simde_vld2q_s32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8x2_t
simde_vld2q_s16(int16_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2q_s16(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int16x8_private r_[2];
    vint16m1x2_t dest = __riscv_vlseg2e16_v_i16m1x2(&ptr[0], 8);
    r_[0].sv128 = __riscv_vget_v_i16m1x2_i16m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_i16m1x2_i16m1(dest, 1);
    simde_int16x8x2_t r = { {
      simde_int16x8_from_private(r_[0]),
      simde_int16x8_from_private(r_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128)
    return
      simde_vuzpq_s16(
        simde_vld1q_s16(&(ptr[0])),
        simde_vld1q_s16(&(ptr[8]))
      );
  #else
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0) && defined(SIMDE_ARCH_RISCV64)
      HEDLEY_DIAGNOSTIC_PUSH
      SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_
    #endif
    simde_int16x8_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_int16x8x2_t r = { {
      simde_int16x8_from_private(r_[0]),
      simde_int16x8_from_private(r_[1]),
    } };

    return r;
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0) && defined(SIMDE_ARCH_RISCV64)
      HEDLEY_DIAGNOSTIC_POP
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2q_s16
  #define vld2q_s16(a) simde_vld2q_s16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2x2_t
simde_vld2q_s64(int64_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_s64(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int64x2_private r_[2];
    vint64m1x2_t dest = __riscv_vlseg2e64_v_i64m1x2(&ptr[0], 2);
    r_[0].sv128 = __riscv_vget_v_i64m1x2_i64m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_i64m1x2_i64m1(dest, 1);
    simde_int64x2x2_t r = { {
      simde_int64x2_from_private(r_[0]),
      simde_int64x2_from_private(r_[1]),
    } };
    return r;
  #else
    simde_int64x2_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_int64x2x2_t r = { {
      simde_int64x2_from_private(r_[0]),
      simde_int64x2_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_s64
  #define vld2q_s64(a) simde_vld2q_s64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16x2_t
simde_vld2q_u8(uint8_t const ptr[HEDLEY_ARRAY_PARAM(32)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2q_u8(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint8x16_private r_[2];
    vuint8m1x2_t dest = __riscv_vlseg2e8_v_u8m1x2(&ptr[0], 16);
    r_[0].sv128 = __riscv_vget_v_u8m1x2_u8m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_u8m1x2_u8m1(dest, 1);
    simde_uint8x16x2_t r = { {
      simde_uint8x16_from_private(r_[0]),
      simde_uint8x16_from_private(r_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128)
    return
      simde_vuzpq_u8(
        simde_vld1q_u8(&(ptr[ 0])),
        simde_vld1q_u8(&(ptr[16]))
      );
  #else
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0) && defined(SIMDE_ARCH_RISCV64)
      HEDLEY_DIAGNOSTIC_PUSH
      SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_
    #endif
    simde_uint8x16_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_uint8x16x2_t r = { {
      simde_uint8x16_from_private(r_[0]),
      simde_uint8x16_from_private(r_[1]),
    } };

    return r;
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0) && defined(SIMDE_ARCH_RISCV64)
      HEDLEY_DIAGNOSTIC_POP
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2q_u8
  #define vld2q_u8(a) simde_vld2q_u8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8x2_t
simde_vld2q_u16(uint16_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2q_u16(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint16x8_private r_[2];
    vuint16m1x2_t dest = __riscv_vlseg2e16_v_u16m1x2(&ptr[0], 8);
    r_[0].sv128 = __riscv_vget_v_u16m1x2_u16m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_u16m1x2_u16m1(dest, 1);
    simde_uint16x8x2_t r = { {
      simde_uint16x8_from_private(r_[0]),
      simde_uint16x8_from_private(r_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128)
    return
      simde_vuzpq_u16(
        simde_vld1q_u16(&(ptr[0])),
        simde_vld1q_u16(&(ptr[8]))
      );
  #else
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0) && defined(SIMDE_ARCH_RISCV64)
      HEDLEY_DIAGNOSTIC_PUSH
      SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_
    #endif
    simde_uint16x8_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_uint16x8x2_t r = { {
      simde_uint16x8_from_private(r_[0]),
      simde_uint16x8_from_private(r_[1]),
    } };

    return r;
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0) && defined(SIMDE_ARCH_RISCV64)
      HEDLEY_DIAGNOSTIC_POP
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2q_u16
  #define vld2q_u16(a) simde_vld2q_u16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4x2_t
simde_vld2q_u32(uint32_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2q_u32(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint32x4_private r_[2];
    vuint32m1x2_t dest = __riscv_vlseg2e32_v_u32m1x2(&ptr[0], 4);
    r_[0].sv128 = __riscv_vget_v_u32m1x2_u32m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_u32m1x2_u32m1(dest, 1);
    simde_uint32x4x2_t r = { {
      simde_uint32x4_from_private(r_[0]),
      simde_uint32x4_from_private(r_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128)
    return
      simde_vuzpq_u32(
        simde_vld1q_u32(&(ptr[0])),
        simde_vld1q_u32(&(ptr[4]))
      );
  #else
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0)
      HEDLEY_DIAGNOSTIC_PUSH
      SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_
    #endif
    simde_uint32x4_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0)
      HEDLEY_DIAGNOSTIC_POP
    #endif

    simde_uint32x4x2_t r = { {
      simde_uint32x4_from_private(r_[0]),
      simde_uint32x4_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2q_u32
  #define vld2q_u32(a) simde_vld2q_u32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2x2_t
simde_vld2q_u64(uint64_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_u64(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint64x2_private r_[2];
    vuint64m1x2_t dest = __riscv_vlseg2e64_v_u64m1x2(&ptr[0], 2);
    r_[0].sv128 = __riscv_vget_v_u64m1x2_u64m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_u64m1x2_u64m1(dest, 1);
    simde_uint64x2x2_t r = { {
      simde_uint64x2_from_private(r_[0]),
      simde_uint64x2_from_private(r_[1]),
    } };
    return r;
  #else
    simde_uint64x2_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_uint64x2x2_t r = { {
      simde_uint64x2_from_private(r_[0]),
      simde_uint64x2_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_u64
  #define vld2q_u64(a) simde_vld2q_u64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8x2_t
simde_vld2q_f16(simde_float16_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vld2q_f16(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE) && SIMDE_ARCH_RISCV_ZVFH && (SIMDE_NATURAL_VECTOR_SIZE >= 128)
    simde_float16x8_private r_[2];
    vfloat16m1x2_t dest = __riscv_vlseg2e16_v_f16m1x2((_Float16 *)&ptr[0], 8);
    r_[0].sv128 = __riscv_vget_v_f16m1x2_f16m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_f16m1x2_f16m1(dest, 1);
    simde_float16x8x2_t r = { {
      simde_float16x8_from_private(r_[0]),
      simde_float16x8_from_private(r_[1]),
    } };
    return r;
  #else
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0)
      HEDLEY_DIAGNOSTIC_PUSH
      SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_
    #endif
    simde_float16x8_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0)
      HEDLEY_DIAGNOSTIC_POP
    #endif

    simde_float16x8x2_t r = { {
      simde_float16x8_from_private(r_[0]),
      simde_float16x8_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2q_f16
  #define vld2q_f16(a) simde_vld2q_f16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4x2_t
simde_vld2q_f32(simde_float32_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2q_f32(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_float32x4_private r_[2];
    vfloat32m1x2_t dest = __riscv_vlseg2e32_v_f32m1x2(&ptr[0], 4);
    r_[0].sv128 = __riscv_vget_v_f32m1x2_f32m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_f32m1x2_f32m1(dest, 1);
    simde_float32x4x2_t r = { {
      simde_float32x4_from_private(r_[0]),
      simde_float32x4_from_private(r_[1]),
    } };
    return r;
  #elif SIMDE_NATURAL_VECTOR_SIZE_GE(128)
    return
      simde_vuzpq_f32(
        simde_vld1q_f32(&(ptr[0])),
        simde_vld1q_f32(&(ptr[4]))
      );
  #else
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0)
      HEDLEY_DIAGNOSTIC_PUSH
      SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_
    #endif
    simde_float32x4_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0)
      HEDLEY_DIAGNOSTIC_POP
    #endif

    simde_float32x4x2_t r = { {
      simde_float32x4_from_private(r_[0]),
      simde_float32x4_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2q_f32
  #define vld2q_f32(a) simde_vld2q_f32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2x2_t
simde_vld2q_f64(simde_float64_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_f64(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_float64x2_private r_[2];
    vfloat64m1x2_t dest = __riscv_vlseg2e64_v_f64m1x2(&ptr[0], 2);
    r_[0].sv128 = __riscv_vget_v_f64m1x2_f64m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_f64m1x2_f64m1(dest, 1);
    simde_float64x2x2_t r = { {
      simde_float64x2_from_private(r_[0]),
      simde_float64x2_from_private(r_[1]),
    } };
    return r;
  #else
    simde_float64x2_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_float64x2x2_t r = { {
      simde_float64x2_from_private(r_[0]),
      simde_float64x2_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_f64
  #define vld2q_f64(a) simde_vld2q_f64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8x2_t
simde_vld2_p8(simde_poly8_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_p8(ptr);
  #else
    simde_poly8x8_private r_[2];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint8m1x2_t dest = __riscv_vlseg2e8_v_u8m1x2(&ptr[0], 8);
      r_[0].sv64 = __riscv_vget_v_u8m1x2_u8m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_u8m1x2_u8m1(dest, 1);
    #else
      for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif
    simde_poly8x8x2_t r = { {
      simde_poly8x8_from_private(r_[0]),
      simde_poly8x8_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_p8
  #define vld2_p8(a) simde_vld2_p8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4x2_t
simde_vld2_p16(simde_poly16_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2_p16(ptr);
  #else
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0)
      HEDLEY_DIAGNOSTIC_PUSH
      SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_
    #endif
    simde_poly16x4_private r_[2];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint16m1x2_t dest = __riscv_vlseg2e16_v_u16m1x2(&ptr[0], 4);
      r_[0].sv64 = __riscv_vget_v_u16m1x2_u16m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_u16m1x2_u16m1(dest, 1);
    #else
      for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0)
      HEDLEY_DIAGNOSTIC_POP
    #endif

    simde_poly16x4x2_t r = { {
      simde_poly16x4_from_private(r_[0]),
      simde_poly16x4_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2_p16
  #define vld2_p16(a) simde_vld2_p16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1x2_t
simde_vld2_p64(simde_poly64_t const ptr[HEDLEY_ARRAY_PARAM(2)]) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vld2_p64(ptr);
  #else
    simde_poly64x1_private r_[2];

    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint64m1x2_t dest = __riscv_vlseg2e64_v_u64m1x2(&ptr[0], 1);
      r_[0].sv64 = __riscv_vget_v_u64m1x2_u64m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_u64m1x2_u64m1(dest, 1);
    #else
      for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif

    simde_poly64x1x2_t r = { {
      simde_poly64x1_from_private(r_[0]),
      simde_poly64x1_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld2_p64
  #define vld2_p64(a) simde_vld2_p64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16x2_t
simde_vld2q_p8(simde_poly8_t const ptr[HEDLEY_ARRAY_PARAM(32)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2q_p8(ptr);
  #else
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0) && defined(SIMDE_ARCH_RISCV64)
      HEDLEY_DIAGNOSTIC_PUSH
      SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_
    #endif
    simde_poly8x16_private r_[2];

    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint8m1x2_t dest = __riscv_vlseg2e8_v_u8m1x2(&ptr[0], 16);
      r_[0].sv128 = __riscv_vget_v_u8m1x2_u8m1(dest, 0);
      r_[1].sv128 = __riscv_vget_v_u8m1x2_u8m1(dest, 1);
    #else
      for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif

    simde_poly8x16x2_t r = { {
      simde_poly8x16_from_private(r_[0]),
      simde_poly8x16_from_private(r_[1]),
    } };

    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0) && defined(SIMDE_ARCH_RISCV64)
      HEDLEY_DIAGNOSTIC_POP
    #endif
    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2q_p8
  #define vld2q_p8(a) simde_vld2q_p8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8x2_t
simde_vld2q_p16(simde_poly16_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld2q_p16(ptr);
  #else
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0) && defined(SIMDE_ARCH_RISCV64)
      HEDLEY_DIAGNOSTIC_PUSH
      SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_
    #endif
    simde_poly16x8_private r_[2];

    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint16m1x2_t dest = __riscv_vlseg2e16_v_u16m1x2(&ptr[0], 8);
      r_[0].sv128 = __riscv_vget_v_u16m1x2_u16m1(dest, 0);
      r_[1].sv128 = __riscv_vget_v_u16m1x2_u16m1(dest, 1);
    #else
      for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif

    simde_poly16x8x2_t r = { {
      simde_poly16x8_from_private(r_[0]),
      simde_poly16x8_from_private(r_[1]),
    } };
    #if defined(SIMDE_DIAGNOSTIC_DISABLE_UNINITIALIZED_) && HEDLEY_GCC_VERSION_CHECK(12,0,0) && defined(SIMDE_ARCH_RISCV64)
      HEDLEY_DIAGNOSTIC_POP
    #endif

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld2q_p16
  #define vld2q_p16(a) simde_vld2q_p16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2x2_t
simde_vld2q_p64(simde_poly64_t const ptr[HEDLEY_ARRAY_PARAM(4)]) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld2q_p64(ptr);
  #else
    simde_poly64x2_private r_[2];

    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint64m1x2_t dest = __riscv_vlseg2e64_v_u64m1x2(&ptr[0], 2);
      r_[0].sv128 = __riscv_vget_v_u64m1x2_u64m1(dest, 0);
      r_[1].sv128 = __riscv_vget_v_u64m1x2_u64m1(dest, 1);
    #else
      for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif

    simde_poly64x2x2_t r = { {
      simde_poly64x2_from_private(r_[0]),
      simde_poly64x2_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_p64
  #define vld2q_p64(a) simde_vld2q_p64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4x2_t
simde_vld2_bf16(simde_bfloat16_t const ptr[HEDLEY_ARRAY_PARAM(8)]) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vld2_bf16(ptr);
  #else
    simde_bfloat16x4_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])) ; i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_bfloat16x4x2_t r = { {
      simde_bfloat16x4_from_private(r_[0]),
      simde_bfloat16x4_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld2_bf16
  #define vld2_bf16(a) simde_vld2_bf16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8x2_t
simde_vld2q_bf16(simde_bfloat16_t const ptr[HEDLEY_ARRAY_PARAM(16)]) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vld2q_bf16(ptr);
  #else
    simde_bfloat16x8_private r_[2];

    for (size_t i = 0 ; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_bfloat16x8x2_t r = { {
      simde_bfloat16x8_from_private(r_[0]),
      simde_bfloat16x8_from_private(r_[1]),
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld2q_bf16
  #define vld2q_bf16(a) simde_vld2q_bf16((a))
#endif

#endif /* !defined(SIMDE_BUG_INTEL_857088) */

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_LD2_H) */
