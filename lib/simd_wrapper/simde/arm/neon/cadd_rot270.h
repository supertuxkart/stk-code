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
 *   2023      Chi-Wei Chu <wewe5215@gapp.nthu.edu.tw>
 */

#if !defined(SIMDE_ARM_NEON_CADD_ROT270_H)
#define SIMDE_ARM_NEON_CADD_ROT270_H

#include "add.h"
#include "types.h"
HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#if defined(__clang__) && SIMDE_FLOAT16_API == SIMDE_FLOAT16_API_FP16
SIMDE_DIAGNOSTIC_DISABLE_DOUBLE_PROMOTION_
_Pragma("clang diagnostic ignored \"-Wimplicit-float-conversion\"")
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t simde_vcadd_rot270_f16(simde_float16x4_t a, simde_float16x4_t b)
{
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && SIMDE_ARCH_ARM_CHECK(8, 3) &&                                                   \
      (!defined(HEDLEY_GCC_VERSION) || HEDLEY_GCC_VERSION_CHECK(9, 0, 0)) &&                                                  \
      (!defined(__clang__) || SIMDE_DETECT_CLANG_VERSION_CHECK(12, 0, 0))
    return vcadd_rot270_f16(a, b);
  #else
    simde_float16x4_private r_, a_ = simde_float16x4_to_private(a), b_ = simde_float16x4_to_private(b);
    #if defined(SIMDE_SHUFFLE_VECTOR_) && !defined(SIMDE_BUG_GCC_100760) &&                                                     \
        ((SIMDE_FLOAT16_API == SIMDE_FLOAT16_API_FP16) || (SIMDE_FLOAT16_API == SIMDE_FLOAT16_API_FLOAT16))
      b_.values = SIMDE_SHUFFLE_VECTOR_(16, 4, -b_.values, b_.values, 5, 0, 7, 2);
      r_.values = b_.values + a_.values;
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0; i < (sizeof(r_.values) / (2 * sizeof(r_.values[0]))); i++)
      {
        r_.values[2 * i] = simde_vaddh_f16(b_.values[2 * i + 1], a_.values[2 * i]);
        r_.values[2 * i + 1] =
            simde_vaddh_f16(simde_float16_from_float32(-simde_float16_to_float32(b_.values[2 * i])), a_.values[2 * i + 1]);
      }
    #endif
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcadd_rot270_f16
  #define vcadd_rot270_f16(a, b) simde_vcadd_rot270_f16(a, b)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t simde_vcaddq_rot270_f16(simde_float16x8_t a, simde_float16x8_t b)
{
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && SIMDE_ARCH_ARM_CHECK(8, 3) &&                                                   \
      (!defined(HEDLEY_GCC_VERSION) || HEDLEY_GCC_VERSION_CHECK(9, 0, 0)) &&                                                  \
      (!defined(__clang__) || SIMDE_DETECT_CLANG_VERSION_CHECK(12, 0, 0))
    return vcaddq_rot270_f16(a, b);
  #else
    simde_float16x8_private r_, a_ = simde_float16x8_to_private(a), b_ = simde_float16x8_to_private(b);
    #if defined(SIMDE_SHUFFLE_VECTOR_) &&                                                                                       \
        ((SIMDE_FLOAT16_API == SIMDE_FLOAT16_API_FP16) || (SIMDE_FLOAT16_API == SIMDE_FLOAT16_API_FLOAT16))
      b_.values = SIMDE_SHUFFLE_VECTOR_(16, 8, -b_.values, b_.values, 9, 0, 11, 2, 13, 4, 15, 6);
      r_.values = b_.values + a_.values;
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0; i < (sizeof(r_.values) / (2 * sizeof(r_.values[0]))); i++)
      {
        r_.values[2 * i] = simde_vaddh_f16(b_.values[2 * i + 1], a_.values[2 * i]);
        r_.values[2 * i + 1] =
            simde_vaddh_f16(simde_float16_from_float32(-simde_float16_to_float32(b_.values[2 * i])), a_.values[2 * i + 1]);
      }
    #endif
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcaddq_rot270_f16
  #define vcaddq_rot270_f16(a, b) simde_vcaddq_rot270_f16(a, b)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t simde_vcadd_rot270_f32(simde_float32x2_t a, simde_float32x2_t b)
{
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && SIMDE_ARCH_ARM_CHECK(8, 3) &&                                                   \
      (!defined(HEDLEY_GCC_VERSION) || HEDLEY_GCC_VERSION_CHECK(9, 0, 0)) &&                                                  \
      (!defined(__clang__) || SIMDE_DETECT_CLANG_VERSION_CHECK(12, 0, 0))
    return vcadd_rot270_f32(a, b);
  #else
    simde_float32x2_private r_, a_ = simde_float32x2_to_private(a), b_ = simde_float32x2_to_private(b);
    #if defined(SIMDE_SHUFFLE_VECTOR_) && !defined(SIMDE_BUG_GCC_100760)
      b_.values = SIMDE_SHUFFLE_VECTOR_(32, 8, -b_.values, b_.values, 3, 0);
      r_.values = b_.values + a_.values;
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0; i < (sizeof(r_.values) / (2 * sizeof(r_.values[0]))); i++)
      {
        r_.values[2 * i] = b_.values[2 * i + 1] + a_.values[2 * i];
        r_.values[2 * i + 1] = -(b_.values[2 * i]) + a_.values[2 * i + 1];
      }
    #endif
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcadd_rot270_f32
  #define vcadd_rot270_f32(a, b) simde_vcadd_rot270_f32(a, b)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t simde_vcaddq_rot270_f32(simde_float32x4_t a, simde_float32x4_t b)
{
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && SIMDE_ARCH_ARM_CHECK(8, 3) &&                                                   \
      (!defined(HEDLEY_GCC_VERSION) || HEDLEY_GCC_VERSION_CHECK(9, 0, 0)) &&                                                  \
      (!defined(__clang__) || SIMDE_DETECT_CLANG_VERSION_CHECK(12, 0, 0))
    return vcaddq_rot270_f32(a, b);
  #else
    simde_float32x4_private r_, a_ = simde_float32x4_to_private(a), b_ = simde_float32x4_to_private(b);
    #if defined(SIMDE_SHUFFLE_VECTOR_)
      b_.values = SIMDE_SHUFFLE_VECTOR_(32, 16, -b_.values, b_.values, 5, 0, 7, 2);
      r_.values = b_.values + a_.values;
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0; i < (sizeof(r_.values) / (2 * sizeof(r_.values[0]))); i++)
      {
        r_.values[2 * i] = b_.values[2 * i + 1] + a_.values[2 * i];
        r_.values[2 * i + 1] = -(b_.values[2 * i]) + a_.values[2 * i + 1];
      }
    #endif
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcaddq_rot270_f32
  #define vcaddq_rot270_f32(a, b) simde_vcaddq_rot270_f32(a, b)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t simde_vcaddq_rot270_f64(simde_float64x2_t a, simde_float64x2_t b)
{
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && SIMDE_ARCH_ARM_CHECK(8, 3) &&                                                   \
      (!defined(HEDLEY_GCC_VERSION) || HEDLEY_GCC_VERSION_CHECK(9, 0, 0)) &&                                                  \
      (!defined(__clang__) || SIMDE_DETECT_CLANG_VERSION_CHECK(12, 0, 0))
    return vcaddq_rot270_f64(a, b);
  #else
    simde_float64x2_private r_, a_ = simde_float64x2_to_private(a), b_ = simde_float64x2_to_private(b);
    #if defined(SIMDE_SHUFFLE_VECTOR_)
      b_.values = SIMDE_SHUFFLE_VECTOR_(64, 16, -b_.values, b_.values, 3, 0);
      r_.values = b_.values + a_.values;
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0; i < (sizeof(r_.values) / (2 * sizeof(r_.values[0]))); i++)
      {
        r_.values[2 * i] = b_.values[2 * i + 1] + a_.values[2 * i];
        r_.values[2 * i + 1] = -(b_.values[2 * i]) + a_.values[2 * i + 1];
      }
    #endif
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vcaddq_rot270_f64
  #define vcaddq_rot270_f64(a, b) simde_vcaddq_rot270_f64(a, b)
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_CADD_ROT270_H) */
