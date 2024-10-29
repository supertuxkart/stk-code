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

#if !defined(SIMDE_ARM_NEON_SM3_H)
#define SIMDE_ARM_NEON_SM3_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#define ROR32(operand, shift) (((operand) >> (shift)) | ((operand) << (32-shift)))
#define ROL32(operand, shift) (((operand) >> (32-shift)) | ((operand) << (shift)))
#define LSR(operand, shift) ((operand) >> (shift))
#define LSL(operand, shift) ((operand) << (shift))

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsm3ss1q_u32(simde_uint32x4_t n, simde_uint32x4_t m, simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_SM3)
    return vsm3ss1q_u32(n, m, a);
  #else
    simde_uint32x4_private
      r_,
      n_ = simde_uint32x4_to_private(n),
      m_ = simde_uint32x4_to_private(m),
      a_ = simde_uint32x4_to_private(a);
    r_.values[3] = ROL32((ROL32(n_.values[3], 12) + m_.values[3] + a_.values[3]), 7);
    r_.values[2] = 0;
    r_.values[1] = 0;
    r_.values[0] = 0;
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsm3ss1q_u32
  #define vsm3ss1q_u32(n, m, a) simde_vsm3ss1q_u32((n), (m), (a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsm3tt1aq_u32(simde_uint32x4_t a, simde_uint32x4_t b, simde_uint32x4_t c, const int imm2)
  SIMDE_REQUIRE_CONSTANT_RANGE(imm2, 0, 3)
{
    simde_uint32x4_private
      r_,
      a_ = simde_uint32x4_to_private(a),
      b_ = simde_uint32x4_to_private(b),
      c_ = simde_uint32x4_to_private(c);
    uint32_t WjPrime, TT1, SS2;

    WjPrime = c_.values[imm2];
    SS2 = b_.values[3] ^ ROL32(a_.values[3], 12);
    TT1 = a_.values[1] ^ (a_.values[3] ^ a_.values[2]);
    TT1 = (TT1 + a_.values[0] + SS2 + WjPrime);
    r_.values[0] = a_.values[1];
    r_.values[1] = ROL32(a_.values[2], 9);
    r_.values[2] = a_.values[3];
    r_.values[3] = TT1;
    return simde_uint32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_SM3)
  #define simde_vsm3tt1aq_u32(a, b, c, imm2) vsm3tt1aq_u32((a), (b), (c), (imm2));
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsm3tt1aq_u32
  #define vsm3tt1aq_u32(a, b, c, imm2) simde_vsm3tt1aq_u32((a), (b), (c), (imm2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsm3tt1bq_u32(simde_uint32x4_t a, simde_uint32x4_t b, simde_uint32x4_t c, const int imm2)
  SIMDE_REQUIRE_CONSTANT_RANGE(imm2, 0, 3)
{
    simde_uint32x4_private
      r_,
      a_ = simde_uint32x4_to_private(a),
      b_ = simde_uint32x4_to_private(b),
      c_ = simde_uint32x4_to_private(c);
    uint32_t WjPrime, TT1, SS2;

    WjPrime = c_.values[imm2];
    SS2 = b_.values[3] ^ ROL32(a_.values[3], 12);
    TT1 = (a_.values[3] & a_.values[1]) | (a_.values[3] & a_.values[2]) | (a_.values[1] & a_.values[2]);
    TT1 = (TT1 + a_.values[0] + SS2 + WjPrime);
    r_.values[0] = a_.values[1];
    r_.values[1] = ROL32(a_.values[2], 9);
    r_.values[2] = a_.values[3];
    r_.values[3] = TT1;
    return simde_uint32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_SM3)
  #define simde_vsm3tt1bq_u32(a, b, c, imm2) vsm3tt1bq_u32((a), (b), (c), (imm2));
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsm3tt1bq_u32
  #define vsm3tt1bq_u32(a, b, c, imm2) simde_vsm3tt1bq_u32((a), (b), (c), (imm2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsm3tt2aq_u32(simde_uint32x4_t a, simde_uint32x4_t b, simde_uint32x4_t c, const int imm2)
  SIMDE_REQUIRE_CONSTANT_RANGE(imm2, 0, 3)
{
    simde_uint32x4_private
      r_,
      a_ = simde_uint32x4_to_private(a),
      b_ = simde_uint32x4_to_private(b),
      c_ = simde_uint32x4_to_private(c);
    uint32_t Wj, TT2;

    Wj = c_.values[imm2];
    TT2 = a_.values[1] ^ (a_.values[3] ^ a_.values[2]);
    TT2 = (TT2 + a_.values[0] + b_.values[3] + Wj);
    r_.values[0] = a_.values[1];
    r_.values[1] = ROL32(a_.values[2], 19);
    r_.values[2] = a_.values[3];
    r_.values[3] = TT2 ^ ROL32(TT2, 9) ^ ROL32(TT2, 17);
    return simde_uint32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_SM3)
  #define simde_vsm3tt2aq_u32(a, b, c, imm2) vsm3tt2aq_u32((a), (b), (c), (imm2));
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsm3tt2aq_u32
  #define vsm3tt2aq_u32(a, b, c, imm2) simde_vsm3tt2aq_u32((a), (b), (c), (imm2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsm3tt2bq_u32(simde_uint32x4_t a, simde_uint32x4_t b, simde_uint32x4_t c, const int imm2)
  SIMDE_REQUIRE_CONSTANT_RANGE(imm2, 0, 3)
{
    simde_uint32x4_private
      r_,
      a_ = simde_uint32x4_to_private(a),
      b_ = simde_uint32x4_to_private(b),
      c_ = simde_uint32x4_to_private(c);
    uint32_t Wj, TT2;

    Wj = c_.values[imm2];
    TT2 = (a_.values[3] & a_.values[2]) | (~(a_.values[3]) & a_.values[1]);
    TT2 = (TT2 + a_.values[0] + b_.values[3] + Wj);
    r_.values[0] = a_.values[1];
    r_.values[1] = ROL32(a_.values[2], 19);
    r_.values[2] = a_.values[3];
    r_.values[3] = TT2 ^ ROL32(TT2, 9) ^ ROL32(TT2, 17);
    return simde_uint32x4_from_private(r_);
}
#if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_SM3)
  #define simde_vsm3tt2bq_u32(a, b, c, imm2) vsm3tt2bq_u32((a), (b), (c), (imm2));
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsm3tt2bq_u32
  #define vsm3tt2bq_u32(a, b, c, imm2) simde_vsm3tt2bq_u32((a), (b), (c), (imm2))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsm3partw1q_u32(simde_uint32x4_t a, simde_uint32x4_t b, simde_uint32x4_t c) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_SM3)
    return vsm3partw1q_u32(a, b, c);
  #else
    simde_uint32x4_private
      r_,
      a_ = simde_uint32x4_to_private(a),
      b_ = simde_uint32x4_to_private(b),
      c_ = simde_uint32x4_to_private(c);
    r_.values[2] = (a_.values[2] ^ b_.values[2]) ^ (ROL32(c_.values[3], 15));
    r_.values[1] = (a_.values[1] ^ b_.values[1]) ^ (ROL32(c_.values[2], 15));
    r_.values[0] = (a_.values[0] ^ b_.values[0]) ^ (ROL32(c_.values[1], 15));
    for(int i = 0; i < 4; ++i) {
      if (i == 3) {
        r_.values[3] = (a_.values[3] ^ b_.values[3]) ^ (ROL32(r_.values[0], 15));
      }
      r_.values[i] = r_.values[i] ^ ROL32(r_.values[i], 15) ^ ROL32(r_.values[i], 23);
    }
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsm3partw1q_u32
  #define vsm3partw1q_u32(a, b, c) simde_vsm3partw1q_u32((a), (b), (c))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsm3partw2q_u32(simde_uint32x4_t a, simde_uint32x4_t b, simde_uint32x4_t c) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_SM3)
    return vsm3partw2q_u32(a, b, c);
  #else
    simde_uint32x4_private
      r_,
      tmp_,
      a_ = simde_uint32x4_to_private(a),
      b_ = simde_uint32x4_to_private(b),
      c_ = simde_uint32x4_to_private(c);
    uint32_t tmp2;
    tmp_.values[3] = b_.values[3] ^ (ROL32(c_.values[3], 7));
    tmp_.values[2] = b_.values[2] ^ (ROL32(c_.values[2], 7));
    tmp_.values[1] = b_.values[1] ^ (ROL32(c_.values[1], 7));
    tmp_.values[0] = b_.values[0] ^ (ROL32(c_.values[0], 7));
    r_.values[3] = a_.values[3] ^ tmp_.values[3];
    r_.values[2] = a_.values[2] ^ tmp_.values[2];
    r_.values[1] = a_.values[1] ^ tmp_.values[1];
    r_.values[0] = a_.values[0] ^ tmp_.values[0];
    tmp2 = ROL32(tmp_.values[0], 15);
    tmp2 = tmp2 ^ ROL32(tmp2, 15) ^ ROL32(tmp2, 23);
    r_.values[3] = r_.values[3] ^ tmp2;

    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsm3partw2q_u32
  #define vsm3partw2q_u32(a, b, c) simde_vsm3partw2q_u32((a), (b), (c))
#endif

#undef ROR32
#undef ROL32
#undef LSR
#undef LSL

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_SM3_H) */
