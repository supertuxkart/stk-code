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

#if !defined(SIMDE_ARM_NEON_PMAXNM_H)
#define SIMDE_ARM_NEON_PMAXNM_H

#include "types.h"
#include "max.h"
#include "uzp1.h"
#include "uzp2.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_float32_t
simde_vpmaxnms_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vpmaxnms_f32(a);
  #else
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    return (a_.values[0] > a_.values[1]) ? a_.values[0] : a_.values[1];
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vpmaxnms_f32
  #define vpmaxnms_f32(a) simde_vpmaxnms_f32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64_t
simde_vpmaxnmqd_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vpmaxnmqd_f64(a);
  #else
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    return (a_.values[0] > a_.values[1]) ? a_.values[0] : a_.values[1];
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vpmaxnmqd_f64
  #define vpmaxnmqd_f64(a) simde_vpmaxnmqd_f64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vpmaxnm_f16(simde_float16x4_t a, simde_float16x4_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vpmaxnm_f16(a, b);
  #else
    return simde_vmax_f16(simde_vuzp1_f16(a, b), simde_vuzp2_f16(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vpmaxnm_f16
  #define vpmaxnm_f16(a, b) simde_vpmaxnm_f16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vpmaxnm_f32(simde_float32x2_t a, simde_float32x2_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vpmaxnm_f32(a, b);
  #else
    return simde_vmax_f32(simde_vuzp1_f32(a, b), simde_vuzp2_f32(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vpmaxnm_f32
  #define vpmaxnm_f32(a, b) simde_vpmaxnm_f32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vpmaxnmq_f16(simde_float16x8_t a, simde_float16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vpmaxnmq_f16(a, b);
  #else
    return simde_vmaxq_f16(simde_vuzp1q_f16(a, b), simde_vuzp2q_f16(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vpmaxnmq_f16
  #define vpmaxnmq_f16(a, b) simde_vpmaxnmq_f16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vpmaxnmq_f32(simde_float32x4_t a, simde_float32x4_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vpmaxnmq_f32(a, b);
  #else
    return simde_vmaxq_f32(simde_vuzp1q_f32(a, b), simde_vuzp2q_f32(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vpmaxnmq_f32
  #define vpmaxnmq_f32(a, b) simde_vpmaxnmq_f32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vpmaxnmq_f64(simde_float64x2_t a, simde_float64x2_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vpmaxnmq_f64(a, b);
  #else
    return simde_vmaxq_f64(simde_vuzp1q_f64(a, b), simde_vuzp2q_f64(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vpmaxnmq_f64
  #define vpmaxnmq_f64(a, b) simde_vpmaxnmq_f64((a), (b))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_PMAXNM_H) */
