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

#if !defined(SIMDE_ARM_NEON_QDMLAL_H)
#define SIMDE_ARM_NEON_QDMLAL_H

#include "add.h"
#include "mul.h"
#include "mul_n.h"
#include "movl.h"
#include "qadd.h"
#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
int32_t
simde_vqdmlalh_s16(int32_t a, int16_t b, int16_t c) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqdmlalh_s16(a, b, c);
  #else
    return HEDLEY_STATIC_CAST(int32_t, b) * HEDLEY_STATIC_CAST(int32_t, c) * 2 + a;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmlalh_s16
  #define vqdmlalh_s16(a, b, c) simde_vqdmlalh_s16((a), (b), (c))
#endif

SIMDE_FUNCTION_ATTRIBUTES
int64_t
simde_vqdmlals_s32(int64_t a, int32_t b, int32_t c) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqdmlals_s32(a, b, c);
  #else
    return HEDLEY_STATIC_CAST(int64_t, b) * HEDLEY_STATIC_CAST(int64_t, c) * 2 + a;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqdmlals_s32
  #define vqdmlals_s32(a, b, c) simde_vqdmlals_s32((a), (b), (c))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vqdmlal_s16(simde_int32x4_t a, simde_int16x4_t b, simde_int16x4_t c) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqdmlal_s16(a, b, c);
  #else
    simde_int32x4_t temp = simde_vmulq_s32(simde_vmovl_s16(b), simde_vmovl_s16(c));
    return simde_vqaddq_s32(simde_vqaddq_s32(temp, temp), a);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqdmlal_s16
  #define vqdmlal_s16(a, b, c) simde_vqdmlal_s16((a), (b), (c))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vqdmlal_s32(simde_int64x2_t a, simde_int32x2_t b, simde_int32x2_t c) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vqdmlal_s32(a, b, c);
  #else
    simde_int64x2_t r = simde_x_vmulq_s64(
          simde_vmovl_s32(b),
          simde_vmovl_s32(c));
    return simde_vqaddq_s64(a, simde_vqaddq_s64(r, r));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vqdmlal_s32
  #define vqdmlal_s32(a, b, c) simde_vqdmlal_s32((a), (b), (c))
#endif


SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_QDMLAL_H) */
