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

#if !defined(SIMDE_X86_AVX512_CVTUS_H)
#define SIMDE_X86_AVX512_CVTUS_H

#include "types.h"
#include "mov.h"
#include "storeu.h"
#include "loadu.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
void
simde_mm512_mask_cvtusepi32_storeu_epi8 (void* base_addr, simde__mmask16 k, simde__m512i a) {
  #if defined(SIMDE_X86_AVX512F_NATIVE)
    _mm512_mask_cvtusepi32_storeu_epi8(base_addr, k, a);
  #else
    simde__m256i_private r_ = simde__m256i_to_private(simde_mm256_loadu_epi8(base_addr));
    simde__m512i_private a_ = simde__m512i_to_private(a);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(a_.u32) / sizeof(a_.u32[0])) ; i++) {
      r_.i8[i] = ((k>>i) &1 ) ?
        ((a_.u32[i] > UINT8_MAX)
          ? (HEDLEY_STATIC_CAST(int8_t, UINT8_MAX))
          : HEDLEY_STATIC_CAST(int8_t, a_.u32[i])) : r_.i8[i];
    }

    simde_mm256_storeu_epi8(base_addr, simde__m256i_from_private(r_));
  #endif
}
#if defined(SIMDE_X86_AVX512F_ENABLE_NATIVE_ALIASES)
  #undef _mm512_mask_cvtusepi32_storeu_epi8
  #define _mm512_mask_cvtusepi32_storeu_epi8(base_addr, k, a) simde_mm512_mask_cvtusepi32_storeu_epi8((base_addr), (k), (a))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_X86_AVX512_CVTUS_H) */
