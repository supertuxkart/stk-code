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

#if !defined(SIMDE_ARM_NEON_QRSHRN_HIGH_N_H)
#define SIMDE_ARM_NEON_QRSHRN_HIGH_N_H

#include "combine.h"
#include "qmovn.h"
#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vqrshrn_high_n_s16(simde_int8x8_t r, simde_int16x8_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 8) {
  simde_int16x8_private
    r_,
    a_ = simde_int16x8_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    int16_t tmp = HEDLEY_STATIC_CAST(int16_t, (a_.values[i] + (1 << (n - 1))) >> n);
    if (tmp > INT8_MAX) tmp = INT8_MAX;
    else if (tmp < INT8_MIN) tmp = INT8_MIN;
    r_.values[i] = HEDLEY_STATIC_CAST(int8_t, tmp);
  }
  return simde_vcombine_s8(r, simde_vqmovn_s16(simde_int16x8_from_private(r_)));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqrshrn_high_n_s16(r, a, n) vqrshrn_high_n_s16((r), (a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrshrn_high_n_s16
  #define vqrshrn_high_n_s16(r, a, n) simde_vqrshrn_high_n_s16((r), (a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vqrshrn_high_n_s32(simde_int16x4_t r, simde_int32x4_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 16) {
  simde_int32x4_private
    r_,
    a_ = simde_int32x4_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    int32_t tmp = (a_.values[i] >> ((n == 32) ? 31 : n)) + ((a_.values[i] & HEDLEY_STATIC_CAST(int32_t, UINT32_C(1) << (n - 1))) != 0);
    if (tmp > INT16_MAX) tmp = INT16_MAX;
    else if (tmp < INT16_MIN) tmp = INT16_MIN;
    r_.values[i] = HEDLEY_STATIC_CAST(int16_t, tmp);
  }
  return simde_vcombine_s16(r, simde_vqmovn_s32(simde_int32x4_from_private(r_)));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqrshrn_high_n_s32(r, a, n) vqrshrn_high_n_s32((r), (a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrshrn_high_n_s32
  #define vqrshrn_high_n_s32(r, a, n) simde_vqrshrn_high_n_s32((r), (a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vqrshrn_high_n_s64(simde_int32x2_t r, simde_int64x2_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  simde_int64x2_private
    r_,
    a_ = simde_int64x2_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    int64_t tmp = (a_.values[i] >> ((n == 64) ? 63 : n)) + ((a_.values[i] & HEDLEY_STATIC_CAST(int64_t, UINT64_C(1) << (n - 1))) != 0);
    if (tmp > INT32_MAX) tmp = INT32_MAX;
    else if (tmp < INT32_MIN) tmp = INT32_MIN;
    r_.values[i] = HEDLEY_STATIC_CAST(int32_t, tmp);
  }
  return simde_vcombine_s32(r, simde_vqmovn_s64(simde_int64x2_from_private(r_)));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqrshrn_high_n_s64(r, a, n) vqrshrn_high_n_s64((r), (a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrshrn_high_n_s64
  #define vqrshrn_high_n_s64(r, a, n) simde_vqrshrn_high_n_s64((r), (a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vqrshrn_high_n_u16(simde_uint8x8_t r, simde_uint16x8_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 8) {
  simde_uint16x8_private
    r_,
    a_ = simde_uint16x8_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    uint16_t tmp = HEDLEY_STATIC_CAST(uint16_t, (a_.values[i] + (1 << (n - 1))) >> n);
    if (tmp > UINT8_MAX) tmp = UINT8_MAX;
    r_.values[i] = HEDLEY_STATIC_CAST(uint8_t, tmp);
  }
  return simde_vcombine_u8(r, simde_vqmovn_u16(simde_uint16x8_from_private(r_)));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqrshrn_high_n_u16(r, a, n) vqrshrn_high_n_u16((r), (a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrshrn_high_n_u16
  #define vqrshrn_high_n_u16(r, a, n) simde_vqrshrn_high_n_u16((r), (a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vqrshrn_high_n_u32(simde_uint16x4_t r, simde_uint32x4_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 16) {
  simde_uint32x4_private
    r_,
    a_ = simde_uint32x4_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    uint32_t tmp = (a_.values[i] >> ((n == 32) ? 31 : n)) + ((a_.values[i] & HEDLEY_STATIC_CAST(uint32_t, UINT32_C(1) << (n - 1))) != 0);
    if (tmp > UINT16_MAX) tmp = UINT16_MAX;
    r_.values[i] = HEDLEY_STATIC_CAST(uint16_t, tmp);
  }
  return simde_vcombine_u16(r, simde_vqmovn_u32(simde_uint32x4_from_private(r_)));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqrshrn_high_n_u32(r, a, n) vqrshrn_high_n_u32((r), (a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrshrn_high_n_u32
  #define vqrshrn_high_n_u32(r, a, n) simde_vqrshrn_high_n_u32((r), (a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vqrshrn_high_n_u64(simde_uint32x2_t r, simde_uint64x2_t a, const int n)
   SIMDE_REQUIRE_CONSTANT_RANGE(n, 1, 32) {
  simde_uint64x2_private
    r_,
    a_ = simde_uint64x2_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    uint64_t tmp = (a_.values[i] >> ((n == 64) ? 63 : n)) + ((a_.values[i] & HEDLEY_STATIC_CAST(uint64_t, UINT64_C(1) << (n - 1))) != 0);
    if (tmp > UINT32_MAX) tmp = UINT32_MAX;
    r_.values[i] = HEDLEY_STATIC_CAST(uint32_t, tmp);
  }
  return simde_vcombine_u32(r, simde_vqmovn_u64(simde_uint64x2_from_private(r_)));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqrshrn_high_n_u64(r, a, n) vqrshrn_high_n_u64((r), (a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrshrn_high_n_u64
  #define vqrshrn_high_n_u64(r, a, n) simde_vqrshrn_high_n_u64((r), (a), (n))
#endif


SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_RSHRN_HIGH_N_H) */
