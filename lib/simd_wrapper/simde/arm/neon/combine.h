/* SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the folhighing conditions:
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
 *   2020      Sean Maher <seanptmaher@gmail.com> (Copyright owned by Google, LLC)
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 */

#if !defined(SIMDE_ARM_NEON_COMBINE_H)
#define SIMDE_ARM_NEON_COMBINE_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vcombine_f16(simde_float16x4_t low, simde_float16x4_t high) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vcombine_f16(low, high);
  #else
    simde_float16x8_private r_;
    simde_float16x4_private
      low_ = simde_float16x4_to_private(low),
      high_ = simde_float16x4_to_private(high);

      size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < halfway ; i++) {
        r_.values[i] = low_.values[i];
        r_.values[i + halfway] = high_.values[i];
      }

    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcombine_f16
  #define vcombine_f16(low, high) simde_vcombine_f16((low), (high))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vcombine_f32(simde_float32x2_t low, simde_float32x2_t high) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vcombine_f32(low, high);
  #else
    simde_float32x4_private r_;
    simde_float32x2_private
      low_ = simde_float32x2_to_private(low),
      high_ = simde_float32x2_to_private(high);

    /* Note: __builtin_shufflevector can have a the output contain
     * twice the number of elements, __builtin_shuffle cannot.
     * Using SIMDE_SHUFFLE_VECTOR_ here would not work. */
    #if defined(SIMDE_VECTOR_SUBSCRIPT) && HEDLEY_HAS_BUILTIN(__builtin_shufflevector)
      r_.values = __builtin_shufflevector(low_.values, high_.values, 0, 1, 2, 3);
    #else
      size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < halfway ; i++) {
        r_.values[i] = low_.values[i];
        r_.values[i + halfway] = high_.values[i];
      }
    #endif

    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcombine_f32
  #define vcombine_f32(low, high) simde_vcombine_f32((low), (high))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vcombine_f64(simde_float64x1_t low, simde_float64x1_t high) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcombine_f64(low, high);
  #else
    simde_float64x2_private r_;
    simde_float64x1_private
      low_ = simde_float64x1_to_private(low),
      high_ = simde_float64x1_to_private(high);

    #if defined(SIMDE_VECTOR_SUBSCRIPT) && HEDLEY_HAS_BUILTIN(__builtin_shufflevector)
      r_.values = __builtin_shufflevector(low_.values, high_.values, 0, 1);
    #else
      size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < halfway ; i++) {
        r_.values[i] = low_.values[i];
        r_.values[i + halfway] = high_.values[i];
      }
    #endif

    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcombine_f64
  #define vcombine_f64(low, high) simde_vcombine_f64((low), (high))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vcombine_s8(simde_int8x8_t low, simde_int8x8_t high) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vcombine_s8(low, high);
  #else
    simde_int8x16_private r_;
    simde_int8x8_private
      low_ = simde_int8x8_to_private(low),
      high_ = simde_int8x8_to_private(high);

    #if defined(SIMDE_VECTOR_SUBSCRIPT) && HEDLEY_HAS_BUILTIN(__builtin_shufflevector)
      r_.values = __builtin_shufflevector(low_.values, high_.values, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    #else
      size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < halfway ; i++) {
        r_.values[i] = low_.values[i];
        r_.values[i + halfway] = high_.values[i];
      }
    #endif

    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcombine_s8
  #define vcombine_s8(low, high) simde_vcombine_s8((low), (high))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vcombine_s16(simde_int16x4_t low, simde_int16x4_t high) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vcombine_s16(low, high);
  #else
    simde_int16x8_private r_;
    simde_int16x4_private
      low_ = simde_int16x4_to_private(low),
      high_ = simde_int16x4_to_private(high);

    #if defined(SIMDE_VECTOR_SUBSCRIPT) && HEDLEY_HAS_BUILTIN(__builtin_shufflevector)
      r_.values = __builtin_shufflevector(low_.values, high_.values, 0, 1, 2, 3, 4, 5, 6, 7);
    #else
      size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < halfway ; i++) {
        r_.values[i] = low_.values[i];
        r_.values[i + halfway] = high_.values[i];
      }
    #endif

    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcombine_s16
  #define vcombine_s16(low, high) simde_vcombine_s16((low), (high))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vcombine_s32(simde_int32x2_t low, simde_int32x2_t high) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vcombine_s32(low, high);
  #else
    simde_int32x4_private r_;
    simde_int32x2_private
      low_ = simde_int32x2_to_private(low),
      high_ = simde_int32x2_to_private(high);

    #if defined(SIMDE_VECTOR_SUBSCRIPT) && HEDLEY_HAS_BUILTIN(__builtin_shufflevector)
      r_.values = __builtin_shufflevector(low_.values, high_.values, 0, 1, 2, 3);
    #else
      size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < halfway ; i++) {
        r_.values[i] = low_.values[i];
        r_.values[i + halfway] = high_.values[i];
      }
    #endif

    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcombine_s32
  #define vcombine_s32(low, high) simde_vcombine_s32((low), (high))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vcombine_s64(simde_int64x1_t low, simde_int64x1_t high) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vcombine_s64(low, high);
  #else
    simde_int64x2_private r_;
    simde_int64x1_private
      low_ = simde_int64x1_to_private(low),
      high_ = simde_int64x1_to_private(high);

    #if defined(SIMDE_VECTOR_SUBSCRIPT) && HEDLEY_HAS_BUILTIN(__builtin_shufflevector)
      r_.values = __builtin_shufflevector(low_.values, high_.values, 0, 1);
    #else
      size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < halfway ; i++) {
        r_.values[i] = low_.values[i];
        r_.values[i + halfway] = high_.values[i];
      }
    #endif

    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcombine_s64
  #define vcombine_s64(low, high) simde_vcombine_s64((low), (high))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vcombine_u8(simde_uint8x8_t low, simde_uint8x8_t high) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vcombine_u8(low, high);
  #else
    simde_uint8x16_private r_;
    simde_uint8x8_private
      low_ = simde_uint8x8_to_private(low),
      high_ = simde_uint8x8_to_private(high);

    #if defined(SIMDE_VECTOR_SUBSCRIPT) && HEDLEY_HAS_BUILTIN(__builtin_shufflevector)
      r_.values = __builtin_shufflevector(low_.values, high_.values, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    #else
      size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < halfway ; i++) {
        r_.values[i] = low_.values[i];
        r_.values[i + halfway] = high_.values[i];
      }
    #endif

    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcombine_u8
  #define vcombine_u8(low, high) simde_vcombine_u8((low), (high))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vcombine_u16(simde_uint16x4_t low, simde_uint16x4_t high) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vcombine_u16(low, high);
  #else
    simde_uint16x8_private r_;
    simde_uint16x4_private
      low_ = simde_uint16x4_to_private(low),
      high_ = simde_uint16x4_to_private(high);

    #if defined(SIMDE_VECTOR_SUBSCRIPT) && HEDLEY_HAS_BUILTIN(__builtin_shufflevector)
      r_.values = __builtin_shufflevector(low_.values, high_.values, 0, 1, 2, 3, 4, 5, 6, 7);
    #else
      size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < halfway ; i++) {
        r_.values[i] = low_.values[i];
        r_.values[i + halfway] = high_.values[i];
      }
    #endif

    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcombine_u16
  #define vcombine_u16(low, high) simde_vcombine_u16((low), (high))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vcombine_u32(simde_uint32x2_t low, simde_uint32x2_t high) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vcombine_u32(low, high);
  #else
    simde_uint32x4_private r_;
    simde_uint32x2_private
      low_ = simde_uint32x2_to_private(low),
      high_ = simde_uint32x2_to_private(high);

    #if defined(SIMDE_VECTOR_SUBSCRIPT) && HEDLEY_HAS_BUILTIN(__builtin_shufflevector)
      r_.values = __builtin_shufflevector(low_.values, high_.values, 0, 1, 2, 3);
    #else
      size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < halfway ; i++) {
        r_.values[i] = low_.values[i];
        r_.values[i + halfway] = high_.values[i];
      }
    #endif

    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcombine_u32
  #define vcombine_u32(low, high) simde_vcombine_u32((low), (high))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vcombine_u64(simde_uint64x1_t low, simde_uint64x1_t high) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vcombine_u64(low, high);
  #else
    simde_uint64x2_private r_;
    simde_uint64x1_private
      low_ = simde_uint64x1_to_private(low),
      high_ = simde_uint64x1_to_private(high);

    #if defined(SIMDE_VECTOR_SUBSCRIPT) && HEDLEY_HAS_BUILTIN(__builtin_shufflevector)
      r_.values = __builtin_shufflevector(low_.values, high_.values, 0, 1);
    #else
      size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < halfway ; i++) {
        r_.values[i] = low_.values[i];
        r_.values[i + halfway] = high_.values[i];
      }
    #endif

    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcombine_u64
  #define vcombine_u64(low, high) simde_vcombine_u64((low), (high))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vcombine_p8(simde_poly8x8_t low, simde_poly8x8_t high) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vcombine_p8(low, high);
  #else
    simde_poly8x16_private r_;
    simde_poly8x8_private
      low_ = simde_poly8x8_to_private(low),
      high_ = simde_poly8x8_to_private(high);

    size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < halfway ; i++) {
      r_.values[i] = low_.values[i];
      r_.values[i + halfway] = high_.values[i];
    }

    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcombine_p8
  #define vcombine_p8(low, high) simde_vcombine_p8((low), (high))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vcombine_p16(simde_poly16x4_t low, simde_poly16x4_t high) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vcombine_p16(low, high);
  #else
    simde_poly16x8_private r_;
    simde_poly16x4_private
      low_ = simde_poly16x4_to_private(low),
      high_ = simde_poly16x4_to_private(high);

    size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < halfway ; i++) {
      r_.values[i] = low_.values[i];
      r_.values[i + halfway] = high_.values[i];
    }

    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcombine_p16
  #define vcombine_p16(low, high) simde_vcombine_p16((low), (high))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vcombine_p64(simde_poly64x1_t low, simde_poly64x1_t high) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vcombine_p64(low, high);
  #else
    simde_poly64x2_private r_;
    simde_poly64x1_private
      low_ = simde_poly64x1_to_private(low),
      high_ = simde_poly64x1_to_private(high);

    size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < halfway ; i++) {
      r_.values[i] = low_.values[i];
      r_.values[i + halfway] = high_.values[i];
    }

    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcombine_p64
  #define vcombine_p64(low, high) simde_vcombine_p64((low), (high))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vcombine_bf16(simde_bfloat16x4_t low, simde_bfloat16x4_t high) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vcombine_bf16(low, high);
  #else
    simde_bfloat16x8_private r_;
    simde_bfloat16x4_private
      low_ = simde_bfloat16x4_to_private(low),
      high_ = simde_bfloat16x4_to_private(high);

    size_t halfway = (sizeof(r_.values) / sizeof(r_.values[0])) / 2;
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < halfway ; i++) {
      r_.values[i] = low_.values[i];
      r_.values[i + halfway] = high_.values[i];
    }

    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcombine_bf16
  #define vcombine_bf16(low, high) simde_vcombine_bf16((low), (high))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_COMBINE_H) */
