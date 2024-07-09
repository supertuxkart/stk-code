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
 */

#if !defined(SIMDE_ARM_NEON_RECPX_H)
#define SIMDE_ARM_NEON_RECPX_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_float16_t
simde_vrecpxh_f16(simde_float16_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vrecpxh_f16(a);
  #else
    if (simde_isnanhf(a)) {
      return SIMDE_NANHF;
    }
    uint16_t n;
    simde_memcpy(&n, &a, sizeof(a));
    uint16_t sign = n & 0x8000;
    uint16_t exp = n & 0x7c00;
    uint16_t result;
    if (exp == 0) {
      uint16_t max_exp = 0x7b00;
      result = sign|max_exp;
    }
    else {
      exp = ~(exp) & 0x7c00;
      result = sign|exp;
    }
    simde_memcpy(&a, &result, sizeof(result));
    return a;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vrecpxh_f16
  #define vrecpxh_f16(a) simde_vrecpxh_f16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32_t
simde_vrecpxs_f32(simde_float32_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vrecpxs_f32(a);
  #else
    if (simde_math_isnanf(a)) {
      return SIMDE_MATH_NANF;
    }
    uint32_t n;
    simde_memcpy(&n, &a, sizeof(a));
    uint32_t sign = n & 0x80000000;
    uint32_t exp = n & 0x7f800000;
    uint32_t result;
    if (exp == 0) {
      uint32_t max_exp = 0x7f000000;
      result = sign|max_exp;
    }
    else {
      exp = ~(exp) & 0x7f800000;
      result = sign|exp;
    }
    simde_memcpy(&a, &result, sizeof(result));
    return a;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vrecpxs_f32
  #define vrecpxs_f32(a) simde_vrecpxs_f32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64_t
simde_vrecpxd_f64(simde_float64_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vrecpxd_f64(a);
  #else
    if (simde_math_isnan(a)) {
      return SIMDE_MATH_NAN;
    }
    uint64_t n;
    simde_memcpy(&n, &a, sizeof(a));
    uint64_t sign = n & 0x8000000000000000ull;
    uint64_t exp = n & 0x7ff0000000000000ull;
    uint64_t result;
    if (exp == 0) {
      uint64_t max_exp = 0x7fe0000000000000ull;
      result = sign|max_exp;
    }
    else {
      exp = ~(exp) & 0x7ff0000000000000ull;
      result = sign|exp;
    }
    simde_memcpy(&a, &result, sizeof(result));
    return a;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vrecpxd_f64
  #define vrecpxd_f64(a) simde_vrecpxd_f64((a))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP
#endif /* !defined(SIMDE_ARM_NEON_RECPX_H) */
