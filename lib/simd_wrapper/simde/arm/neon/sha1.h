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

#if !defined(SIMDE_ARM_NEON_SHA1_H)
#define SIMDE_ARM_NEON_SHA1_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#define ROL(operand, N, shift) (((operand) >> (N-shift)) | ((operand) << (shift)))

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde_vsha1h_u32(uint32_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_SHA2)
    return vsha1h_u32(a);
  #else
    return ROL(a, 32, 30);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vsha1h_u32
  #define vsha1h_u32(a) simde_vsha1h_u32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsha1cq_u32(simde_uint32x4_t hash_abcd, uint32_t hash_e, simde_uint32x4_t wk) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_SHA2)
    return vsha1cq_u32(hash_abcd, hash_e, wk);
  #else
    simde_uint32x4_private
      x_ = simde_uint32x4_to_private(hash_abcd),
      w_ = simde_uint32x4_to_private(wk);
    uint32_t y_ = hash_e;
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(x_.values) / sizeof(x_.values[0])) ; i++) {
      uint32_t t = (((x_.values[2] ^ x_.values[3]) & x_.values[1]) ^ x_.values[3]);
      y_ = y_ + ROL(x_.values[0], 32, 5) + t + w_.values[i];
      x_.values[1] = ROL(x_.values[1], 32, 30);
      uint32_t tmp = y_;
      y_ = 0x0 | x_.values[3];
      x_.values[3] = 0x0 | x_.values[2];
      x_.values[2] = 0x0 | x_.values[1];
      x_.values[1] = 0x0 | x_.values[0];
      x_.values[0] = tmp | 0x0;
    }
    return simde_uint32x4_from_private(x_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vsha1cq_u32
  #define vsha1cq_u32(hash_abcd, hash_e, wk) simde_vsha1cq_u32((hash_abcd), (hash_e), (wk))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsha1mq_u32(simde_uint32x4_t hash_abcd, uint32_t hash_e, simde_uint32x4_t wk) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_SHA2)
    return vsha1mq_u32(hash_abcd, hash_e, wk);
  #else
    simde_uint32x4_private
      x_ = simde_uint32x4_to_private(hash_abcd),
      w_ = simde_uint32x4_to_private(wk);
    uint32_t y_ = hash_e;
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(x_.values) / sizeof(x_.values[0])) ; i++) {
      uint32_t t = ((x_.values[1] & x_.values[2]) | ((x_.values[1] | x_.values[2]) & x_.values[3]));
      y_ = y_ + ROL(x_.values[0], 32, 5) + t + w_.values[i];
      x_.values[1] = ROL(x_.values[1], 32, 30);
      uint32_t tmp = y_;
      y_ = 0x0 | x_.values[3];
      x_.values[3] = 0x0 | x_.values[2];
      x_.values[2] = 0x0 | x_.values[1];
      x_.values[1] = 0x0 | x_.values[0];
      x_.values[0] = tmp | 0x0;
    }
    return simde_uint32x4_from_private(x_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vsha1mq_u32
  #define vsha1mq_u32(hash_abcd, hash_e, wk) simde_vsha1mq_u32((hash_abcd), (hash_e), (wk))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsha1pq_u32(simde_uint32x4_t hash_abcd, uint32_t hash_e, simde_uint32x4_t wk) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_SHA2)
    return vsha1pq_u32(hash_abcd, hash_e, wk);
  #else
    simde_uint32x4_private
      x_ = simde_uint32x4_to_private(hash_abcd),
      w_ = simde_uint32x4_to_private(wk);
    uint32_t y_ = hash_e;
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(x_.values) / sizeof(x_.values[0])) ; i++) {
      uint32_t t = (x_.values[1] ^ x_.values[2] ^ x_.values[3]);
      y_ = y_ + ROL(x_.values[0], 32, 5) + t + w_.values[i];
      x_.values[1] = ROL(x_.values[1], 32, 30);
      uint32_t tmp = y_;
      y_ = 0x0 | x_.values[3];
      x_.values[3] = 0x0 | x_.values[2];
      x_.values[2] = 0x0 | x_.values[1];
      x_.values[1] = 0x0 | x_.values[0];
      x_.values[0] = tmp | 0x0;
    }
    return simde_uint32x4_from_private(x_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vsha1pq_u32
  #define vsha1pq_u32(hash_abcd, hash_e, wk) simde_vsha1pq_u32((hash_abcd), (hash_e), (wk))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsha1su0q_u32(simde_uint32x4_t w0_3, simde_uint32x4_t w4_7, simde_uint32x4_t w8_11) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_SHA2)
    return vsha1su0q_u32(w0_3, w4_7, w8_11);
  #else
    simde_uint32x4_private
      r_,
      x_ = simde_uint32x4_to_private(w0_3),
      y_ = simde_uint32x4_to_private(w4_7),
      z_ = simde_uint32x4_to_private(w8_11);
    r_.values[3] = y_.values[1];
    r_.values[2] = y_.values[0];
    r_.values[1] = x_.values[3];
    r_.values[0] = x_.values[2];
    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(x_.values) / sizeof(x_.values[0])) ; i++) {
      r_.values[i] = r_.values[i] ^ x_.values[i] ^ z_.values[i];
    }
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vsha1su0q_u32
  #define vsha1su0q_u32(w0_3, w4_7, w8_11) simde_vsha1su0q_u32((w0_3), (w4_7), (w8_11))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsha1su1q_u32(simde_uint32x4_t tw0_3, simde_uint32x4_t tw12_15) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_SHA2)
    return vsha1su1q_u32(tw0_3, tw12_15);
  #else
    simde_uint32x4_private
      r_,
      T_,
      x_ = simde_uint32x4_to_private(tw0_3),
      y_ = simde_uint32x4_to_private(tw12_15);
    T_.values[0] = x_.values[0] ^ y_.values[1];
    T_.values[1] = x_.values[1] ^ y_.values[2];
    T_.values[2] = x_.values[2] ^ y_.values[3];
    T_.values[3] = x_.values[3] ^ 0x0;
    r_.values[0] = ROL(T_.values[0], 32, 1);
    r_.values[1] = ROL(T_.values[1], 32, 1);
    r_.values[2] = ROL(T_.values[2], 32, 1);
    r_.values[3] = ROL(T_.values[3], 32, 1) ^ ROL(T_.values[0], 32, 2);

    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vsha1su1q_u32
  #define vsha1su1q_u32(tw0_3, tw12_15) simde_vsha1su1q_u32((tw0_3), (tw12_15))
#endif

#undef ROL

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_SHA1_H) */
