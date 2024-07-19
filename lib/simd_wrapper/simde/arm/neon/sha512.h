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

#if !defined(SIMDE_ARM_NEON_SHA512_H)
#define SIMDE_ARM_NEON_SHA512_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#define ROR64(operand, shift) (((operand) >> (shift)) | ((operand) << (64-shift)))
#define ROL64(operand, shift) (((operand) >> (64-shift)) | ((operand) << (shift)))
#define LSR(operand, shift) ((operand) >> (shift))
#define LSL(operand, shift) ((operand) << (shift))

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vsha512hq_u64(simde_uint64x2_t w, simde_uint64x2_t x, simde_uint64x2_t y) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_SHA512)
    return vsha512hq_u64(w, x, y);
  #else
    simde_uint64x2_private
      r_,
      w_ = simde_uint64x2_to_private(w),
      x_ = simde_uint64x2_to_private(x),
      y_ = simde_uint64x2_to_private(y);
    uint64_t Msigma1;
    uint64_t tmp;
    Msigma1 = ROR64(y_.values[1], 14) ^ ROR64(y_.values[1], 18) ^ ROR64(y_.values[1], 41);
    r_.values[1] = (y_.values[1] & x_.values[0]) ^ (~(y_.values[1]) & x_.values[1]);
    r_.values[1] = (r_.values[1] + Msigma1 + w_.values[1]);
    tmp = r_.values[1] + y_.values[0];
    Msigma1 = ROR64(tmp, 14) ^ ROR64(tmp, 18) ^ ROR64(tmp, 41);
    r_.values[0] = (tmp & y_.values[1]) ^ (~(tmp) & x_.values[0]);
    r_.values[0] = (r_.values[0] + Msigma1 + w_.values[0]);
    return simde_uint64x2_from_private(r_);

  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsha512hq_u64
  #define vsha512hq_u64(w, x, y) simde_vsha512hq_u64((w), (x), (y))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vsha512h2q_u64(simde_uint64x2_t w, simde_uint64x2_t x, simde_uint64x2_t y) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_SHA512)
    return vsha512h2q_u64(w, x, y);
  #else
    simde_uint64x2_private
      r_,
      w_ = simde_uint64x2_to_private(w),
      x_ = simde_uint64x2_to_private(x),
      y_ = simde_uint64x2_to_private(y);
    uint64_t Msigma0;
    Msigma0 = ROR64(y_.values[0], 28) ^ ROR64(y_.values[0], 34) ^ ROR64(y_.values[0], 39);
    r_.values[1] = (y_.values[1] & x_.values[0]) ^ (y_.values[0] & x_.values[0]) ^ (y_.values[1] & y_.values[0]);
    r_.values[1] = (r_.values[1] + Msigma0 + w_.values[1]);
    Msigma0 = ROR64(r_.values[1], 28) ^ ROR64(r_.values[1], 34) ^ ROR64(r_.values[1], 39);
    r_.values[0] = (r_.values[1] & y_.values[0]) ^ (r_.values[1] & y_.values[1]) ^ (y_.values[1] & y_.values[0]);
    r_.values[0] = (r_.values[0] + Msigma0 + w_.values[0]);
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsha512h2q_u64
  #define vsha512h2q_u64(w, x, y) simde_vsha512h2q_u64((w), (x), (y))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vsha512su0q_u64(simde_uint64x2_t w, simde_uint64x2_t x) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_SHA512)
    return vsha512su0q_u64(w, x);
  #else
    simde_uint64x2_private
      r_,
      w_ = simde_uint64x2_to_private(w),
      x_ = simde_uint64x2_to_private(x);
    uint64_t sig0;
    sig0 = ROR64(w_.values[1], 1) ^ ROR64(w_.values[1], 8) ^ (w_.values[1] >> 7);
    r_.values[0] = w_.values[0] + sig0;
    sig0 = ROR64(x_.values[0], 1) ^ ROR64(x_.values[0], 8) ^ (x_.values[0] >> 7);
    r_.values[1] = w_.values[1] + sig0;
    return simde_uint64x2_from_private(r_);

  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsha512su0q_u64
  #define vsha512su0q_u64(w, x) simde_vsha512su0q_u64((w), (x))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vsha512su1q_u64(simde_uint64x2_t w, simde_uint64x2_t x, simde_uint64x2_t y) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_SHA512)
    return vsha512su1q_u64(w, x, y);
  #else
    simde_uint64x2_private
      r_,
      w_ = simde_uint64x2_to_private(w),
      x_ = simde_uint64x2_to_private(x),
      y_ = simde_uint64x2_to_private(y);
    uint64_t sig1;
    sig1 = ROR64(x_.values[1], 19) ^ ROR64(x_.values[1], 61) ^ (x_.values[1] >> 6);
    r_.values[1] = w_.values[1] + sig1 + y_.values[1];
    sig1 = ROR64(x_.values[0], 19) ^ ROR64(x_.values[0], 61) ^ (x_.values[0] >> 6);
    r_.values[0] = w_.values[0] + sig1 + y_.values[0];
    return simde_uint64x2_from_private(r_);

  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsha512su1q_u64
  #define vsha512su1q_u64(w, x, y) simde_vsha512su1q_u64((w), (x), (y))
#endif

#undef ROR64
#undef ROL64
#undef LSR
#undef LSL

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_SHA512_H) */
