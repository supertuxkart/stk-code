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

#if !defined(SIMDE_ARM_NEON_PMINNM_H)
#define SIMDE_ARM_NEON_PMINNM_H

#include "types.h"
#include "min.h"
#include "uzp1.h"
#include "uzp2.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_float32_t
simde_vpminnms_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vpminnms_f32(a);
  #else
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    return (a_.values[0] < a_.values[1]) ? a_.values[0] : a_.values[1];
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vpminnms_f32
  #define vpminnms_f32(a) simde_vpminnms_f32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64_t
simde_vpminnmqd_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vpminnmqd_f64(a);
  #else
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    return (a_.values[0] < a_.values[1]) ? a_.values[0] : a_.values[1];
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vpminnmqd_f64
  #define vpminnmqd_f64(a) simde_vpminnmqd_f64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vpminnm_f16(simde_float16x4_t a, simde_float16x4_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vpminnm_f16(a, b);
  #else
    return simde_vmin_f16(simde_vuzp1_f16(a, b), simde_vuzp2_f16(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vpminnm_f16
  #define vpminnm_f16(a, b) simde_vpminnm_f16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vpminnm_f32(simde_float32x2_t a, simde_float32x2_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vpminnm_f32(a, b);
  #else
    return simde_vmin_f32(simde_vuzp1_f32(a, b), simde_vuzp2_f32(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vpminnm_f32
  #define vpminnm_f32(a, b) simde_vpminnm_f32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vpminnmq_f16(simde_float16x8_t a, simde_float16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vpminnmq_f16(a, b);
  #else
    return simde_vminq_f16(simde_vuzp1q_f16(a, b), simde_vuzp2q_f16(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vpminnmq_f16
  #define vpminnmq_f16(a, b) simde_vpminnmq_f16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vpminnmq_f32(simde_float32x4_t a, simde_float32x4_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vpminnmq_f32(a, b);
  #else
    return simde_vminq_f32(simde_vuzp1q_f32(a, b), simde_vuzp2q_f32(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vpminnmq_f32
  #define vpminnmq_f32(a, b) simde_vpminnmq_f32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vpminnmq_f64(simde_float64x2_t a, simde_float64x2_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vpminnmq_f64(a, b);
  #else
    return simde_vminq_f64(simde_vuzp1q_f64(a, b), simde_vuzp2q_f64(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vpminnmq_f64
  #define vpminnmq_f64(a, b) simde_vpminnmq_f64((a), (b))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_PMINNM_H) */
