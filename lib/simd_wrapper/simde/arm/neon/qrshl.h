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

#if !defined(SIMDE_ARM_NEON_QRSHL_H)
#define SIMDE_ARM_NEON_QRSHL_H
#include "../../x86/avx.h"
#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
int8_t
simde_vqrshlb_s8(int8_t a, int8_t b) {
  int8_t r;

  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    r = vqrshlb_s8(a, b);
  #else
    if (b < -8) {
      r = 0;
    } else if (b < 0) {
      r = HEDLEY_STATIC_CAST(int8_t, a <= 0
            ? ((a + (1 << (-b - 1))) >> -b)
            : HEDLEY_STATIC_CAST(int8_t, ((HEDLEY_STATIC_CAST(uint8_t,
              (a + (1 << (-b - 1)))) >> -b) & 0x7FUL)));
    } else if (b == 0) {
      r = a;
    } else if (b < 7) {
      r = HEDLEY_STATIC_CAST(int8_t, a << b);
      if ((r >> b) != a) {
        r = (a < 0) ? INT8_MIN : INT8_MAX;
      }
    } else if (a == 0) {
      r = 0;
    } else {
      r = (a < 0) ? INT8_MIN : INT8_MAX;
    }
  #endif

  return r;
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrshlb_s8
  #define vqrshlb_s8(a, b) simde_vqrshlb_s8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
int16_t
simde_vqrshlh_s16(int16_t a, int16_t b) {
  int16_t r;

  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    r = vqrshlh_s16(a, b);
  #else
    int8_t b8 = HEDLEY_STATIC_CAST(int8_t, b);

    if (b8 <= -16) {
      r = 0;
    } else if (b8 < 0) {
      r = HEDLEY_STATIC_CAST(int16_t, a <= 0
            ? ((a + (1 << (-b8 - 1))) >> -b8)
            : HEDLEY_STATIC_CAST(int16_t, ((HEDLEY_STATIC_CAST(uint16_t,
              (a + (1 << (-b8 - 1)))) >> -b8) & 0x7FFFUL)));
    } else if (b8 == 0) {
      r = a;
    } else if (b8 < 15) {
      r = HEDLEY_STATIC_CAST(int16_t, a << b8);
      if ((r >> b8) != a) {
        r = (a < 0) ? INT16_MIN : INT16_MAX;
      }
    } else if (a == 0) {
      r = 0;
    } else {
      r = (a < 0) ? INT16_MIN : INT16_MAX;
    }
  #endif

  return r;
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrshlh_s16
  #define vqrshlh_s16(a, b) simde_vqrshlh_s16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
int32_t
simde_vqrshls_s32(int32_t a, int32_t b) {
  int32_t r;

  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    r = vqrshls_s32(a, b);
  #else
    int8_t b8 = HEDLEY_STATIC_CAST(int8_t, b);

    if (b8 <= -32) {
      r = 0;
    } else if (b8 < 0) {
      r = a <= 0
            ? ((a + (1 << (-b8 - 1))) >> -b8)
            : HEDLEY_STATIC_CAST(int32_t, ((HEDLEY_STATIC_CAST(uint32_t,
              (a + (1 << (-b8 - 1)))) >> -b8) & 0x7FFFFFFFUL));
    } else if (b8 == 0) {
      r = a;
    } else if (b8 < 31) {
      r = HEDLEY_STATIC_CAST(int32_t, a << b8);
      if ((r >> b8) != a) {
        r = (a < 0) ? INT32_MIN : INT32_MAX;
      }
    } else if (a == 0) {
      r = 0;
    } else {
      r = (a < 0) ? INT32_MIN : INT32_MAX;
    }
  #endif

  return r;
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrshls_s32
  #define vqrshls_s32(a, b) simde_vqrshls_s32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
int64_t
simde_vqrshld_s64(int64_t a, int64_t b) {
  int64_t r;

  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    r = vqrshld_s64(a, b);
  #else
    int8_t b8 = HEDLEY_STATIC_CAST(int8_t, b);

    if (b8 <= -64) {
      r = 0;
    } else if (b8 < 0) {
      r = a <= 0
            ? ((a + (INT64_C(1) << (-b8 - 1))) >> -b8)
            : HEDLEY_STATIC_CAST(int64_t, ((HEDLEY_STATIC_CAST(uint64_t,
              (a + (INT64_C(1) << (-b8 - 1)))) >> -b8) & 0x7FFFFFFFFFFFFFFFUL));
    } else if (b8 == 0) {
      r = a;
    } else if (b8 < 63) {
      r = HEDLEY_STATIC_CAST(int64_t, a << b8);
      if ((r >> b8) != a) {
        r = (a < 0) ? INT64_MIN : INT64_MAX;
      }
    } else if (a == 0) {
      r = 0;
    } else {
      r = (a < 0) ? INT64_MIN : INT64_MAX;
    }
  #endif

  return r;
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrshld_s64
  #define vqrshld_s64(a, b) simde_vqrshld_s64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint8_t
simde_vqrshlb_u8(uint8_t a, int8_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    #if defined(HEDLEY_GCC_VERSION) && !HEDLEY_GCC_VERSION_CHECK(11,0,0)
      return vqrshlb_u8(a, HEDLEY_STATIC_CAST(uint8_t, b));
    #elif HEDLEY_HAS_WARNING("-Wsign-conversion")
      /* https://github.com/llvm/llvm-project/commit/f0a78bdfdc6d56b25e0081884580b3960a3c2429 */
      HEDLEY_DIAGNOSTIC_PUSH
      #pragma clang diagnostic ignored "-Wsign-conversion"
      return vqrshlb_u8(a, b);
      HEDLEY_DIAGNOSTIC_POP
    #else
      return vqrshlb_u8(a, b);
    #endif
  #else
    uint8_t r;

    if (b < -8) {
      r = 0;
    } else if (b < 0) {
      r = (a >> -b) + ((a >> (-b - 1)) & 1);
    } else if (b == 0) {
      r = a;
    } else if (b < 7) {
      r = HEDLEY_STATIC_CAST(uint8_t, a << b);
      if ((r >> b) != a) {
        r = UINT8_MAX;
      }
    } else if (a == 0) {
      r = 0;
    } else {
      r = UINT8_MAX;
    }

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrshlb_u8
  #define vqrshlb_u8(a, b) simde_vqrshlb_u8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint16_t
simde_vqrshlh_u16(uint16_t a, int16_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    #if defined(HEDLEY_GCC_VERSION) && !HEDLEY_GCC_VERSION_CHECK(11,0,0)
      return vqrshlh_u16(a, HEDLEY_STATIC_CAST(uint16_t, b));
    #elif HEDLEY_HAS_WARNING("-Wsign-conversion")
      HEDLEY_DIAGNOSTIC_PUSH
      #pragma clang diagnostic ignored "-Wsign-conversion"
      return vqrshlh_u16(a, b);
      HEDLEY_DIAGNOSTIC_POP
    #else
      return vqrshlh_u16(a, b);
    #endif
  #else
    b = HEDLEY_STATIC_CAST(int8_t, b);
    uint16_t r;

    if (b < -16) {
      r = 0;
    } else if (b < 0) {
      r = (a >> -b) + ((a >> (-b - 1)) & 1);
    } else if (b == 0) {
      r = a;
    } else if (b < 15) {
      r = HEDLEY_STATIC_CAST(uint16_t, a << b);
      if ((r >> b) != a) {
        r = UINT16_MAX;
      }
    } else if (a == 0) {
      r = 0;
    } else {
      r = UINT16_MAX;
    }

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrshlh_u16
  #define vqrshlh_u16(a, b) simde_vqrshlh_u16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde_vqrshls_u32(uint32_t a, int32_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    #if defined(HEDLEY_GCC_VERSION) && !HEDLEY_GCC_VERSION_CHECK(11,0,0)
      return vqrshls_u32(a, HEDLEY_STATIC_CAST(uint16_t, b));
    #elif HEDLEY_HAS_WARNING("-Wsign-conversion")
      HEDLEY_DIAGNOSTIC_PUSH
      #pragma clang diagnostic ignored "-Wsign-conversion"
      return vqrshls_u32(a, b);
      HEDLEY_DIAGNOSTIC_POP
    #else
      return vqrshls_u32(a, b);
    #endif
  #else
    b = HEDLEY_STATIC_CAST(int8_t, b);
    uint32_t r;

    if (b < -32) {
      r = 0;
    } else if (b < 0) {
      r = (a >> -b) + ((a >> (-b - 1)) & 1);
    } else if (b == 0) {
      r = a;
    } else if (b < 31) {
      r = HEDLEY_STATIC_CAST(uint32_t, a << b);
      if ((r >> b) != a) {
        r = UINT32_MAX;
      }
    } else if (a == 0) {
      r = 0;
    } else {
      r = UINT32_MAX;
    }

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrshls_u32
  #define vqrshls_u32(a, b) simde_vqrshls_u32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint64_t
simde_vqrshld_u64(uint64_t a, int64_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    #if defined(HEDLEY_GCC_VERSION) && !HEDLEY_GCC_VERSION_CHECK(11,0,0)
      return vqrshld_u64(a, HEDLEY_STATIC_CAST(uint16_t, b));
    #elif HEDLEY_HAS_WARNING("-Wsign-conversion")
      HEDLEY_DIAGNOSTIC_PUSH
      #pragma clang diagnostic ignored "-Wsign-conversion"
      return vqrshld_u64(a, b);
      HEDLEY_DIAGNOSTIC_POP
    #else
      return vqrshld_u64(a, b);
    #endif
  #else
    b = HEDLEY_STATIC_CAST(int8_t, b);
    uint64_t r;

    if (b < -64) {
      r = 0;
    } else if (b < 0) {
      r = (a >> -b) + ((a >> (-b - 1)) & 1);
    } else if (b == 0) {
      r = a;
    } else if (b < 63) {
      r = HEDLEY_STATIC_CAST(uint64_t, a << b);
      if ((r >> b) != a) {
        r = UINT64_MAX;
      }
    } else if (a == 0) {
      r = 0;
    } else {
      r = UINT64_MAX;
    }

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqrshld_u64
  #define vqrshld_u64(a, b) simde_vqrshld_u64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vqrshl_s8 (const simde_int8x8_t a, const simde_int8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshl_s8(a, b);
  #else
    simde_int8x8_private
      r_,
      a_ = simde_int8x8_to_private(a),
      b_ = simde_int8x8_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshlb_s8(a_.values[i], b_.values[i]);
    }

    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshl_s8
  #define vqrshl_s8(a, b) simde_vqrshl_s8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vqrshl_s16 (const simde_int16x4_t a, const simde_int16x4_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshl_s16(a, b);
  #else
    simde_int16x4_private
      r_,
      a_ = simde_int16x4_to_private(a),
      b_ = simde_int16x4_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshlh_s16(a_.values[i], b_.values[i]);
    }

    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshl_s16
  #define vqrshl_s16(a, b) simde_vqrshl_s16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vqrshl_s32 (const simde_int32x2_t a, const simde_int32x2_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshl_s32(a, b);
  #else
    simde_int32x2_private
      r_,
      a_ = simde_int32x2_to_private(a),
      b_ = simde_int32x2_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshls_s32(a_.values[i], b_.values[i]);
    }

    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshl_s32
  #define vqrshl_s32(a, b) simde_vqrshl_s32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vqrshl_s64 (const simde_int64x1_t a, const simde_int64x1_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshl_s64(a, b);
  #else
    simde_int64x1_private
      r_,
      a_ = simde_int64x1_to_private(a),
      b_ = simde_int64x1_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshld_s64(a_.values[i], b_.values[i]);
    }

    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshl_s64
  #define vqrshl_s64(a, b) simde_vqrshl_s64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vqrshl_u8 (const simde_uint8x8_t a, const simde_int8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshl_u8(a, b);
  #else
    simde_uint8x8_private
      r_,
      a_ = simde_uint8x8_to_private(a);
    simde_int8x8_private b_ = simde_int8x8_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshlb_u8(a_.values[i], b_.values[i]);
    }

    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshl_u8
  #define vqrshl_u8(a, b) simde_vqrshl_u8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vqrshl_u16 (const simde_uint16x4_t a, const simde_int16x4_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshl_u16(a, b);
  #else
    simde_uint16x4_private
      r_,
      a_ = simde_uint16x4_to_private(a);
    simde_int16x4_private b_ = simde_int16x4_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshlh_u16(a_.values[i], b_.values[i]);
    }

    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshl_u16
  #define vqrshl_u16(a, b) simde_vqrshl_u16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vqrshl_u32 (const simde_uint32x2_t a, const simde_int32x2_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshl_u32(a, b);
  #else
    simde_uint32x2_private
      r_,
      a_ = simde_uint32x2_to_private(a);
    simde_int32x2_private b_ = simde_int32x2_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshls_u32(a_.values[i], b_.values[i]);
    }

    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshl_u32
  #define vqrshl_u32(a, b) simde_vqrshl_u32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vqrshl_u64 (const simde_uint64x1_t a, const simde_int64x1_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshl_u64(a, b);
  #else
    simde_uint64x1_private
      r_,
      a_ = simde_uint64x1_to_private(a);
    simde_int64x1_private b_ = simde_int64x1_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshld_u64(a_.values[i], b_.values[i]);
    }

    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshl_u64
  #define vqrshl_u64(a, b) simde_vqrshl_u64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vqrshlq_s8 (const simde_int8x16_t a, const simde_int8x16_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshlq_s8(a, b);
  #else
    simde_int8x16_private
      r_,
      a_ = simde_int8x16_to_private(a),
      b_ = simde_int8x16_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshlb_s8(a_.values[i], b_.values[i]);
    }

    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshlq_s8
  #define vqrshlq_s8(a, b) simde_vqrshlq_s8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vqrshlq_s16 (const simde_int16x8_t a, const simde_int16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshlq_s16(a, b);
  #else
    simde_int16x8_private
      r_,
      a_ = simde_int16x8_to_private(a),
      b_ = simde_int16x8_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshlh_s16(a_.values[i], b_.values[i]);
    }

    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshlq_s16
  #define vqrshlq_s16(a, b) simde_vqrshlq_s16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vqrshlq_s32 (const simde_int32x4_t a, const simde_int32x4_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshlq_s32(a, b);
  #else
    simde_int32x4_private
      r_,
      a_ = simde_int32x4_to_private(a),
      b_ = simde_int32x4_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshls_s32(a_.values[i], b_.values[i]);
    }

    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshlq_s32
  #define vqrshlq_s32(a, b) simde_vqrshlq_s32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vqrshlq_s64 (const simde_int64x2_t a, const simde_int64x2_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshlq_s64(a, b);
  #else
    simde_int64x2_private
      r_,
      a_ = simde_int64x2_to_private(a),
      b_ = simde_int64x2_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshld_s64(a_.values[i], b_.values[i]);
    }

    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshlq_s64
  #define vqrshlq_s64(a, b) simde_vqrshlq_s64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vqrshlq_u8 (const simde_uint8x16_t a, const simde_int8x16_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshlq_u8(a, b);
  #else
    simde_uint8x16_private
      r_,
      a_ = simde_uint8x16_to_private(a);
    simde_int8x16_private b_ = simde_int8x16_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshlb_u8(a_.values[i], b_.values[i]);
    }

    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshlq_u8
  #define vqrshlq_u8(a, b) simde_vqrshlq_u8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vqrshlq_u16 (const simde_uint16x8_t a, const simde_int16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshlq_u16(a, b);
  #else
    simde_uint16x8_private
      r_,
      a_ = simde_uint16x8_to_private(a);
    simde_int16x8_private b_ = simde_int16x8_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshlh_u16(a_.values[i], b_.values[i]);
    }

    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshlq_u16
  #define vqrshlq_u16(a, b) simde_vqrshlq_u16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vqrshlq_u32 (const simde_uint32x4_t a, const simde_int32x4_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshlq_u32(a, b);
  #else
    simde_uint32x4_private
      r_,
      a_ = simde_uint32x4_to_private(a);
    simde_int32x4_private b_ = simde_int32x4_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshls_u32(a_.values[i], b_.values[i]);
    }

    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshlq_u32
  #define vqrshlq_u32(a, b) simde_vqrshlq_u32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vqrshlq_u64 (const simde_uint64x2_t a, const simde_int64x2_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqrshlq_u64(a, b);
  #else
    simde_uint64x2_private
      r_,
      a_ = simde_uint64x2_to_private(a);
    simde_int64x2_private b_ = simde_int64x2_to_private(b);
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vqrshld_u64(a_.values[i], b_.values[i]);
    }

    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqrshlq_u64
  #define vqrshlq_u64(a, b) simde_vqrshlq_u64((a), (b))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_QRSHL_H) */
