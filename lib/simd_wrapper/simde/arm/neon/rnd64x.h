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

#if !defined(SIMDE_ARM_NEON_RND64X_H)
#define SIMDE_ARM_NEON_RND64X_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

// src: https://gcc.gnu.org/legacy-ml/gcc-patches/2019-09/msg00053.html
SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vrnd64x_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_FRINT)
    return vrnd64x_f32(a);
  #else
    simde_float32x2_private
      r_,
      a_ = simde_float32x2_to_private(a);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      if (simde_math_isnanf(a_.values[i]) || simde_math_isinff(a_.values[i])) {
        r_.values[i] = HEDLEY_STATIC_CAST(float, INT64_MIN);
      } else {
        r_.values[i] = simde_math_rintf(a_.values[i]);
        if (r_.values[i] > HEDLEY_STATIC_CAST(float, INT64_MAX) || r_.values[i] < HEDLEY_STATIC_CAST(float, INT64_MIN)) {
          r_.values[i] = HEDLEY_STATIC_CAST(float, INT64_MIN);
        }
      }
    }

    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vrnd64x_f32
  #define vrnd64x_f32(a) simde_vrnd64x_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vrnd64x_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_FRINT) && (!defined(__clang__) || SIMDE_DETECT_CLANG_VERSION_CHECK(18, 0, 0))
    return vrnd64x_f64(a);
  #else
    simde_float64x1_private
      r_,
      a_ = simde_float64x1_to_private(a);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      if (simde_math_isnan(a_.values[i]) || simde_math_isinf(a_.values[i])) {
        r_.values[i] = HEDLEY_STATIC_CAST(double, INT64_MIN);
      } else {
        r_.values[i] = simde_math_rint(a_.values[i]);
        if (r_.values[i] > HEDLEY_STATIC_CAST(double, INT64_MAX) || r_.values[i] < HEDLEY_STATIC_CAST(double, INT64_MIN)) {
          r_.values[i] = HEDLEY_STATIC_CAST(double, INT64_MIN);
        }
      }
    }

    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vrnd64x_f64
  #define vrnd64x_f64(a) simde_vrnd64x_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vrnd64xq_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_FRINT)
    return vrnd64xq_f32(a);
  #else
    simde_float32x4_private
      r_,
      a_ = simde_float32x4_to_private(a);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      if (simde_math_isnanf(a_.values[i]) || simde_math_isinff(a_.values[i])) {
        r_.values[i] = HEDLEY_STATIC_CAST(float, INT64_MIN);
      } else {
        r_.values[i] = simde_math_rintf(a_.values[i]);
        if (r_.values[i] > HEDLEY_STATIC_CAST(float, INT64_MAX) || r_.values[i] < HEDLEY_STATIC_CAST(float, INT64_MIN)) {
          r_.values[i] = HEDLEY_STATIC_CAST(float, INT64_MIN);
        }
      }
    }

    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vrnd64xq_f32
  #define vrnd64xq_f32(a) simde_vrnd64xq_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vrnd64xq_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_FRINT) && (!defined(__clang__) || SIMDE_DETECT_CLANG_VERSION_CHECK(18, 0, 0))
    return vrnd64xq_f64(a);
  #else
    simde_float64x2_private
      r_,
      a_ = simde_float64x2_to_private(a);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      if (simde_math_isnan(a_.values[i]) || simde_math_isinf(a_.values[i])) {
        r_.values[i] = HEDLEY_STATIC_CAST(double, INT64_MIN);
      } else {
        r_.values[i] = simde_math_rint(a_.values[i]);
        if (r_.values[i] > HEDLEY_STATIC_CAST(double, INT64_MAX) || r_.values[i] < HEDLEY_STATIC_CAST(double, INT64_MIN)) {
          r_.values[i] = HEDLEY_STATIC_CAST(double, INT64_MIN);
        }
      }
    }

    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vrnd64xq_f64
  #define vrnd64xq_f64(a) simde_vrnd64xq_f64(a)
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_RND64X_H) */
