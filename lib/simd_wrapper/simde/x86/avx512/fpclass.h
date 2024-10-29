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
 *   2023      Michael R. Crusoe <crusoe@debian.org>
 */

#if !defined(SIMDE_X86_AVX512_FPCLASS_H)
#define SIMDE_X86_AVX512_FPCLASS_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde__mmask8
simde_mm256_fpclass_ps_mask(simde__m256 a, int imm8)
  SIMDE_REQUIRE_CONSTANT_RANGE(imm8, 0, 0x88) {
  simde__mmask8 r = 0;
  simde__m256_private a_ = simde__m256_to_private(a);

  for (size_t i = 0 ; i < (sizeof(a_.f32) / sizeof(a_.f32[0])) ; i++) {
    r |= simde_math_fpclassf(a_.f32[i], imm8) ? (UINT8_C(1) << i) : 0;
  }
  return r;
}
#if defined(SIMDE_X86_AVX512DQ_NATIVE) && defined(SIMDE_X86_AVX512VL_NATIVE)
#  define simde_mm256_fpclass_ps_mask(a, imm8) _mm256_fpclass_ps_mask((a), (imm8))
#endif
#if defined(SIMDE_X86_AVX512DQ_ENABLE_NATIVE_ALIASES) && defined(SIMDE_X86_AVX512VL_ENABLE_NATIVE_ALIASES)
#  undef _mm256_fpclass_ps_mask
#  define _mm256_fpclass_ps_mask(a, imm8) simde_mm256_fpclass_ps_mask((a), (imm8))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde__mmask32
simde_mm512_fpclass_ph_mask(simde__m512h a, int imm8)
  SIMDE_REQUIRE_CONSTANT_RANGE(imm8, 0, 0x88) {
  simde__mmask32 r = 0;
  simde__m512h_private a_ = simde__m512h_to_private(a);

  for (size_t i = 0 ; i < (sizeof(a_.f16) / sizeof(a_.f16[0])) ; i++) {
    r |= simde_fpclasshf(a_.f16[i], imm8) ? (UINT8_C(1) << i) : 0;
  }
  return r;
}
#if defined(SIMDE_X86_AVX512FP16_NATIVE)
#  define simde_mm512_fpclass_ph_mask(a, imm8) _mm512_fpclass_ph_mask((a), (imm8))
#endif
#if defined(SIMDE_X86_AVX512FP16_ENABLE_NATIVE_ALIASES)
#  undef _mm512_fpclass_ph_mask
#  define _mm512_fpclass_ph_mask(a, imm8) simde_mm512_fpclass_ph_mask((a), (imm8))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde__mmask8
simde_mm512_fpclass_pd_mask(simde__m512d a, int imm8)
  SIMDE_REQUIRE_CONSTANT_RANGE(imm8, 0, 0x88) {
  simde__mmask8 r = 0;
  simde__m512d_private a_ = simde__m512d_to_private(a);

  for (size_t i = 0 ; i < (sizeof(a_.f64) / sizeof(a_.f64[0])) ; i++) {
    r |= simde_math_fpclass(a_.f64[i], imm8) ? (UINT8_C(1) << i) : 0;
  }
  return r;
}
#if defined(SIMDE_X86_AVX512DQ_NATIVE)
#  define simde_mm512_fpclass_pd_mask(a, imm8) _mm512_fpclass_pd_mask((a), (imm8))
#endif
#if defined(SIMDE_X86_AVX512DQ_ENABLE_NATIVE_ALIASES)
#  undef _mm512_fpclass_pd_mask
#  define _mm512_fpclass_pd_mask(a, imm8) simde_mm512_fpclass_pd_mask((a), (imm8))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_X86_AVX512_FPCLASS_H) */
