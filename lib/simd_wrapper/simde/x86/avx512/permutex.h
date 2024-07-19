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

#if !defined(SIMDE_X86_AVX512_PERMUTEX_H)
#define SIMDE_X86_AVX512_PERMUTEX_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde__m256i
simde_mm256_permutex_epi64 (simde__m256i a, const int imm8) {
  simde__m256i_private
    a_ = simde__m256i_to_private(a),
    r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.i64) / sizeof(r_.i64[0])) ; i++) {
    r_.i64[i] = a_.i64[(imm8 >> (i*2)) & 3];
  }

  return simde__m256i_from_private(r_);
}
#if defined(SIMDE_X86_AVX512F_NATIVE) && defined(SIMDE_X86_AVX512VL_NATIVE)
  #define simde_mm256_permutex_epi64(a, imm8) _mm256_permutex_epi64((a), (imm8))
#endif
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES) && defined(SIMDE_X86_AVX512VL_ENABLE_NATIVE_ALIASES)
  #undef _mm256_permutex_epi64
  #define _mm256_permutex_epi64(a, imm8) simde_mm256_permutex_epi64((a), (imm8))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde__m512i
simde_mm512_permutex_epi64 (simde__m512i a, const int imm8) {
  simde__m512i_private
    a_ = simde__m512i_to_private(a),
    r_;

  SIMDE_VECTORIZE
  for (size_t i = 0 ; i < (sizeof(r_.m256i_private[0].i64) / sizeof(r_.m256i_private[0].i64[0])) ; i++) {
    r_.m256i_private[0].i64[i] = a_.m256i_private[0].i64[(imm8 >> (i*2)) & 3];
    r_.m256i_private[1].i64[i] = a_.m256i_private[1].i64[(imm8 >> (i*2)) & 3];
  }

  return simde__m512i_from_private(r_);
}
#if defined(SIMDE_X86_AVX512F_NATIVE)
  #define simde_mm512_permutex_epi64(a, imm8)  _mm512_permutex_epi64((a), (imm8))
#elif defined(SIMDE_STATEMENT_EXPR_)
  #define simde_mm512_permutex_epi64(a, imm8) SIMDE_STATEMENT_EXPR_(({\
    simde__m512i_private simde_mm512_permutex_epi64_a_ = simde__m512i_to_private((a)), simde_mm512_permutex_epi64_r_; \
    simde_mm512_permutex_epi64_r_.m256i[0] = simde_mm256_permutex_epi64(simde_mm512_permutex_epi64_a_.m256i[0], (imm8)); \
    simde_mm512_permutex_epi64_r_.m256i[1] = simde_mm256_permutex_epi64(simde_mm512_permutex_epi64_a_.m256i[1], (imm8)); \
    simde__m512i_from_private(simde_mm512_permutex_epi64_r_); \
  }))
#endif
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
  #undef _mm512_permutex_epi64
  #define _mm512_permutex_epi64(a, imm8) simde_mm512_permutex_epi64((a), (imm8))
#endif

#if defined(SIMDE_X86_AVX512F_NATIVE)
  #define simde_mm512_mask_permutex_epi64(src, k, a, imm8) _mm512_mask_permutex_epi64((src), (k), (a), (imm8))
#else
  #define simde_mm512_mask_permutex_epi64(src, k, a, imm8) simde_mm512_mask_mov_epi64((src), (k), simde_mm512_permutex_epi64((a), (imm8)))
#endif
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
  #undef _mm512_mask_permutex_epi64
  #define _mm512_mask_permutex_epi64(src, k, a, imm8) simde_mm512_mask_permutex_epi64((src), (k), (a), (imm8))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_X86_AVX512_PERMUTEX_H) */
