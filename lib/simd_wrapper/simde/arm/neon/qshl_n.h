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

#if !defined(SIMDE_ARM_NEON_QSHL_N_H)
#define SIMDE_ARM_NEON_QSHL_N_H

#include "types.h"
#include "cls.h"
#include "qshl.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
int8_t
simde_vqshlb_n_s8(int8_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 7) {
  return simde_vqshlb_s8(a, HEDLEY_STATIC_CAST(int8_t, n));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqshlb_n_s8(a, n) vqshlb_n_s8((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshlb_n_s8
  #define vqshlb_n_s8(a, n) simde_vqshlb_n_s8((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
int16_t
simde_vqshlh_n_s16(int16_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 15) {
  return simde_vqshlh_s16(a, HEDLEY_STATIC_CAST(int16_t, n));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqshlh_n_s16(a, n) vqshlh_n_s16((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshlh_n_s16
  #define vqshlh_n_s16(a, n) simde_vqshlh_n_s16((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
int32_t
simde_vqshls_n_s32(int32_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 31) {
  return simde_vqshls_s32(a, n);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqshls_n_s32(a, n) vqshls_n_s32((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshls_n_s32
  #define vqshls_n_s32(a, n) simde_vqshls_n_s32((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
int64_t
simde_vqshld_n_s64(int64_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 63) {
  return simde_vqshld_s64(a, n);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqshld_n_s64(a, n) vqshld_n_s64((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshld_n_s64
  #define vqshld_n_s64(a, n) simde_vqshld_n_s64((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint8_t
simde_vqshlb_n_u8(uint8_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 7) {
  return simde_vqshlb_u8(a, HEDLEY_STATIC_CAST(int8_t, n));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqshlb_n_u8(a, n) vqshlb_n_u8((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshlb_n_u8
  #define vqshlb_n_u8(a, n) simde_vqshlb_n_u8((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint16_t
simde_vqshlh_n_u16(uint16_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 15) {
  return simde_vqshlh_u16(a, HEDLEY_STATIC_CAST(int16_t, n));
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqshlh_n_u16(a, n) vqshlh_n_u16((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshlh_n_u16
  #define vqshlh_n_u16(a, n) simde_vqshlh_n_u16((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde_vqshls_n_u32(uint32_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 31) {
  return simde_vqshls_u32(a, n);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqshls_n_u32(a, n) vqshls_n_u32((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshls_n_u32
  #define vqshls_n_u32(a, n) simde_vqshls_n_u32((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint64_t
simde_vqshld_n_u64(uint64_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 63) {
  return simde_vqshld_u64(a, n);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vqshld_n_u64(a, n) vqshld_n_u64((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqshld_n_u64
  #define vqshld_n_u64(a, n) simde_vqshld_n_u64((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vqshl_n_s8 (const simde_int8x8_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 7) {
  simde_int8x8_private
    r_,
    a_ = simde_int8x8_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    SIMDE_CONSTIFY_8_(simde_vqshlb_n_s8, r_.values[i], (HEDLEY_UNREACHABLE(), 0), n, a_.values[i]);
  }
  return simde_int8x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshl_n_s8(a, n) vqshl_n_s8((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshl_n_s8
  #define vqshl_n_s8(a, n) simde_vqshl_n_s8((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vqshl_n_s16 (const simde_int16x4_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 15) {
  simde_int16x4_private
    r_,
    a_ = simde_int16x4_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    SIMDE_CONSTIFY_16_(simde_vqshlh_n_s16, r_.values[i], (HEDLEY_UNREACHABLE(), 0), n, a_.values[i]);
  }
  return simde_int16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshl_n_s16(a, n) vqshl_n_s16((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshl_n_s16
  #define vqshl_n_s16(a, n) simde_vqshl_n_s16((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vqshl_n_s32 (const simde_int32x2_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 31) {
  simde_int32x2_private
    r_,
    a_ = simde_int32x2_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vqshls_s32(a_.values[i], n);
  }
  return simde_int32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshl_n_s32(a, n) vqshl_n_s32((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshl_n_s32
  #define vqshl_n_s32(a, n) simde_vqshl_n_s32((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vqshl_n_s64 (const simde_int64x1_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 63) {
  simde_int64x1_private
    r_,
    a_ = simde_int64x1_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vqshld_s64(a_.values[i], n);
  }
  return simde_int64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshl_n_s64(a, n) vqshl_n_s64((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshl_n_s64
  #define vqshl_n_s64(a, n) simde_vqshl_n_s64((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vqshl_n_u8 (const simde_uint8x8_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 7) {
  simde_uint8x8_private
    r_,
    a_ = simde_uint8x8_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    SIMDE_CONSTIFY_8_(simde_vqshlb_n_u8, r_.values[i], (HEDLEY_UNREACHABLE(), 0), n, a_.values[i]);
  }
  return simde_uint8x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshl_n_u8(a, n) vqshl_n_u8((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshl_n_u8
  #define vqshl_n_u8(a, n) simde_vqshl_n_u8((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vqshl_n_u16 (const simde_uint16x4_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 15) {
  simde_uint16x4_private
    r_,
    a_ = simde_uint16x4_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    SIMDE_CONSTIFY_16_(simde_vqshlh_n_u16, r_.values[i], (HEDLEY_UNREACHABLE(), 0), n, a_.values[i]);
  }
  return simde_uint16x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshl_n_u16(a, n) vqshl_n_u16((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshl_n_u16
  #define vqshl_n_u16(a, n) simde_vqshl_n_u16((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vqshl_n_u32 (const simde_uint32x2_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 31) {
  simde_uint32x2_private
    r_,
    a_ = simde_uint32x2_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vqshls_u32(a_.values[i], n);
  }
  return simde_uint32x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshl_n_u32(a, n) vqshl_n_u32((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshl_n_u32
  #define vqshl_n_u32(a, n) simde_vqshl_n_u32((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vqshl_n_u64 (const simde_uint64x1_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 63) {
  simde_uint64x1_private
    r_,
    a_ = simde_uint64x1_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vqshld_u64(a_.values[i], n);
  }
  return simde_uint64x1_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshl_n_u64(a, n) vqshl_n_u64((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshl_n_u64
  #define vqshl_n_u64(a, n) simde_vqshl_n_u64((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vqshlq_n_s8 (const simde_int8x16_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 7) {
  simde_int8x16_private
    r_,
    a_ = simde_int8x16_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    SIMDE_CONSTIFY_8_(simde_vqshlb_n_s8, r_.values[i], (HEDLEY_UNREACHABLE(), 0), n, a_.values[i]);
  }

  return simde_int8x16_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshlq_n_s8(a, n) vqshlq_n_s8((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshlq_n_s8
  #define vqshlq_n_s8(a, n) simde_vqshlq_n_s8((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vqshlq_n_s16 (const simde_int16x8_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 15) {
  simde_int16x8_private
    r_,
    a_ = simde_int16x8_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    SIMDE_CONSTIFY_16_(simde_vqshlh_n_s16, r_.values[i], (HEDLEY_UNREACHABLE(), 0), n, a_.values[i]);
  }

  return simde_int16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshlq_n_s16(a, n) vqshlq_n_s16((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshlq_n_s16
  #define vqshlq_n_s16(a, n) simde_vqshlq_n_s16((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vqshlq_n_s32 (const simde_int32x4_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 31) {
  simde_int32x4_private
    r_,
    a_ = simde_int32x4_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vqshls_s32(a_.values[i], n);
  }

  return simde_int32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshlq_n_s32(a, n) vqshlq_n_s32((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshlq_n_s32
  #define vqshlq_n_s32(a, n) simde_vqshlq_n_s32((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vqshlq_n_s64 (const simde_int64x2_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 63) {
  simde_int64x2_private
    r_,
    a_ = simde_int64x2_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vqshld_s64(a_.values[i], n);
  }

  return simde_int64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshlq_n_s64(a, n) vqshlq_n_s64((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshlq_n_s64
  #define vqshlq_n_s64(a, n) simde_vqshlq_n_s64((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vqshlq_n_u8 (const simde_uint8x16_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 7) {
  simde_uint8x16_private
    r_,
    a_ = simde_uint8x16_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    SIMDE_CONSTIFY_8_(simde_vqshlb_n_u8, r_.values[i], (HEDLEY_UNREACHABLE(), 0), n, a_.values[i]);
  }

  return simde_uint8x16_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshlq_n_u8(a, n) vqshlq_n_u8((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshlq_n_u8
  #define vqshlq_n_u8(a, n) simde_vqshlq_n_u8((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vqshlq_n_u16 (const simde_uint16x8_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 15) {
  simde_uint16x8_private
    r_,
    a_ = simde_uint16x8_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    SIMDE_CONSTIFY_16_(simde_vqshlh_n_u16, r_.values[i], (HEDLEY_UNREACHABLE(), 0), n, a_.values[i]);
  }

  return simde_uint16x8_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshlq_n_u16(a, n) vqshlq_n_u16((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshlq_n_u16
  #define vqshlq_n_u16(a, n) simde_vqshlq_n_u16((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vqshlq_n_u32 (const simde_uint32x4_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 31) {
  simde_uint32x4_private
    r_,
    a_ = simde_uint32x4_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vqshls_u32(a_.values[i], n);
  }

  return simde_uint32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshlq_n_u32(a, n) vqshlq_n_u32((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshlq_n_u32
  #define vqshlq_n_u32(a, n) simde_vqshlq_n_u32((a), (n))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vqshlq_n_u64 (const simde_uint64x2_t a, const int n)
    SIMDE_REQUIRE_CONSTANT_RANGE(n, 0, 63) {
  simde_uint64x2_private
    r_,
    a_ = simde_uint64x2_to_private(a);

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
    r_.values[i] = simde_vqshld_u64(a_.values[i], n);
  }

  return simde_uint64x2_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vqshlq_n_u64(a, n) vqshlq_n_u64((a), (n))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqshlq_n_u64
  #define vqshlq_n_u64(a, n) simde_vqshlq_n_u64((a), (n))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_QSHL_N_H) */
