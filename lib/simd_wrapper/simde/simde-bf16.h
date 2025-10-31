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

#include "hedley.h"
#include "simde-common.h"
#include "simde-detect-clang.h"

#if !defined(SIMDE_BFLOAT16_H)
#define SIMDE_BFLOAT16_H

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

/* This implementations is based upon simde-f16.h */

/* Portable version which should work on pretty much any compiler.
 * Obviously you can't rely on compiler support for things like
 * conversion to/from 32-bit floats, so make sure you always use the
 * functions and macros in this file!
 */
#define SIMDE_BFLOAT16_API_PORTABLE 1

#define SIMDE_BFLOAT16_API_BF16 2

#if !defined(SIMDE_BFLOAT16_API)
  #if defined(SIMDE_ARM_NEON_BF16)
    #define SIMDE_BFLOAT16_API SIMDE_BFLOAT16_API_BF16
  #else
    #define SIMDE_BFLOAT16_API SIMDE_BFLOAT16_API_PORTABLE
  #endif
#endif

#if SIMDE_BFLOAT16_API == SIMDE_BFLOAT16_API_BF16
  #include <arm_bf16.h>
  typedef __bf16 simde_bfloat16;
#elif SIMDE_BFLOAT16_API == SIMDE_BFLOAT16_API_PORTABLE
  typedef struct { uint16_t value; } simde_bfloat16;
#else
  #error No 16-bit floating point API.
#endif

/* Conversion -- convert between single-precision and brain half-precision
 * floats. */
static HEDLEY_ALWAYS_INLINE HEDLEY_CONST
simde_bfloat16
simde_bfloat16_from_float32 (simde_float32 value) {
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
  return vcvth_bf16_f32(value);
#else
  simde_bfloat16 res;
  char* src = HEDLEY_REINTERPRET_CAST(char*, &value);
  // rounding to nearest bfloat16
  // If the 17th bit of value is 1, set the rounding to 1.
  uint8_t rounding = 0;

  #if SIMDE_ENDIAN_ORDER == SIMDE_ENDIAN_LITTLE
    if (src[1] & UINT8_C(0x80)) rounding = 1;
    src[2] = HEDLEY_STATIC_CAST(char, (HEDLEY_STATIC_CAST(uint8_t, src[2]) + rounding));
    simde_memcpy(&res, src+2, sizeof(res));
  #else
    if (src[2] & UINT8_C(0x80)) rounding = 1;
    src[1] = HEDLEY_STATIC_CAST(char, (HEDLEY_STATIC_CAST(uint8_t, src[1]) + rounding));
    simde_memcpy(&res, src, sizeof(res));
  #endif

  return res;
#endif
}

static HEDLEY_ALWAYS_INLINE HEDLEY_CONST
simde_float32
simde_bfloat16_to_float32 (simde_bfloat16 value) {
#if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
  return vcvtah_f32_bf16(value);
#else
  simde_float32 res = 0.0;
  char* _res = HEDLEY_REINTERPRET_CAST(char*, &res);

  #if SIMDE_ENDIAN_ORDER == SIMDE_ENDIAN_LITTLE
    simde_memcpy(_res+2, &value, sizeof(value));
  #else
    simde_memcpy(_res, &value, sizeof(value));
  #endif

  return res;
#endif
}

SIMDE_DEFINE_CONVERSION_FUNCTION_(simde_uint16_as_bfloat16, simde_bfloat16,      uint16_t)

#define SIMDE_NANBF simde_uint16_as_bfloat16(0xFFC1) // a quiet Not-a-Number
#define SIMDE_INFINITYBF simde_uint16_as_bfloat16(0x7F80)
#define SIMDE_NINFINITYBF simde_uint16_as_bfloat16(0xFF80)

#define SIMDE_BFLOAT16_VALUE(value) simde_bfloat16_from_float32(SIMDE_FLOAT32_C(value))

#if !defined(simde_isinfbf) && defined(simde_math_isinff)
  #define simde_isinfbf(a) simde_math_isinff(simde_bfloat16_to_float32(a))
#endif
#if !defined(simde_isnanbf) && defined(simde_math_isnanf)
  #define simde_isnanbf(a) simde_math_isnanf(simde_bfloat16_to_float32(a))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_BFLOAT16_H) */
