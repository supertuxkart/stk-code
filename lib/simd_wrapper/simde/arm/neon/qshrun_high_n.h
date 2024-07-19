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

#if !defined(SIMDE_ARM_NEON_QSHRUN_HIGH_N_H)
#define SIMDE_ARM_NEON_QSHRUN_HIGH_N_H

#include "combine.h"
#include "qmovn.h"
#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vqshrun_high_n_s16(simde_uint8x8_t r, simde_int16x8_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 8) {
  simde_int16x8_private a_ = simde_int16x8_to_private(a);
  simde_uint16x8_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    int16_t tmp = (a_.values[i]) >> n;
    if (tmp > UINT8_MAX) tmp = UINT8_MAX;
    else if (tmp < 0) tmp = 0;
    r_.values[i] = HEDLEY_STATIC_CAST(uint8_t, tmp);
  }
  return simde_vcombine_u8(r, simde_vqmovn_u16(simde_uint16x8_from_private(r_)));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71365)
  #define simde_vqshrun_high_n_s16(r, a, n) vqshrun_high_n_s16((r), (a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshrun_high_n_s16
  #define vqshrun_high_n_s16(r, a, n) simde_vqshrun_high_n_s16((r), (a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vqshrun_high_n_s32(simde_uint16x4_t r, simde_int32x4_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 16) {
  simde_int32x4_private a_ = simde_int32x4_to_private(a);
  simde_uint32x4_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    int32_t tmp = (a_.values[i] >> n);
    if (tmp > UINT16_MAX) tmp = UINT16_MAX;
    else if (tmp < 0) tmp = 0;
    r_.values[i] = HEDLEY_STATIC_CAST(uint16_t, tmp);
  }
  return simde_vcombine_u16(r, simde_vqmovn_u32(simde_uint32x4_from_private(r_)));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71365)
  #define simde_vqshrun_high_n_s32(r, a, n) vqshrun_high_n_s32((r), (a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshrun_high_n_s32
  #define vqshrun_high_n_s32(r, a, n) simde_vqshrun_high_n_s32((r), (a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vqshrun_high_n_s64(simde_uint32x2_t r, simde_int64x2_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  simde_int64x2_private a_ = simde_int64x2_to_private(a);
  simde_uint64x2_private r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    int64_t tmp = (a_.values[i] >> n);
    if (tmp > UINT32_MAX) tmp = UINT32_MAX;
    else if (tmp < 0) tmp = 0;
    r_.values[i] = HEDLEY_STATIC_CAST(uint32_t, tmp);
  }
  return simde_vcombine_u32(r, simde_vqmovn_u64(simde_uint64x2_from_private(r_)));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && !defined(SIMDE_BUG_CLANG_71365)
  #define simde_vqshrun_high_n_s64(r, a, n) vqshrun_high_n_s64((r), (a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshrun_high_n_s64
  #define vqshrun_high_n_s64(r, a, n) simde_vqshrun_high_n_s64((r), (a), (n))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_QSHRUN_HIGH_N_H) */
