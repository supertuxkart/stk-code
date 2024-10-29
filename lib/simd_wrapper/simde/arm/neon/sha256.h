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

#if !defined(SIMDE_ARM_NEON_SHA256_H)
#define SIMDE_ARM_NEON_SHA256_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#define ROR32(operand, shift) (((operand) >> (shift)) | ((operand) << (32-shift)))
#define ROL32(operand, shift) (((operand) >> (32-shift)) | ((operand) << (shift)))
#define LSR(operand, shift) ((operand) >> (shift))
#define LSL(operand, shift) ((operand) << (shift))

static uint32_t simde_SHAchoose(uint32_t x, uint32_t y, uint32_t z) {
  return (((y ^ z) & x) ^ z);
}

static uint32_t simde_SHAmajority(uint32_t x, uint32_t y, uint32_t z) {
  return ((x & y) | ((x | y) & z));
}

static uint32_t simde_SHAhashSIGMA0(uint32_t x) {
  return ROR32(x, 2) ^ ROR32(x, 13) ^ ROR32(x, 22);
}

static uint32_t simde_SHAhashSIGMA1(uint32_t x) {
  return ROR32(x, 6) ^ ROR32(x, 11) ^ ROR32(x, 25);
}

static simde_uint32x4_t
x_simde_sha256hash(simde_uint32x4_t x, simde_uint32x4_t y, simde_uint32x4_t w, int part1) {
  uint32_t chs, maj, t;
  simde_uint32x4_private
    x_ = simde_uint32x4_to_private(x),
    y_ = simde_uint32x4_to_private(y),
    w_ = simde_uint32x4_to_private(w);

  for(int i = 0; i < 4; ++i) {
    chs = simde_SHAchoose(y_.values[0], y_.values[1], y_.values[2]);
    maj = simde_SHAmajority(x_.values[0], x_.values[1], x_.values[2]);
    t = y_.values[3] + simde_SHAhashSIGMA1(y_.values[0]) + chs + w_.values[i];
    x_.values[3] = t + x_.values[3];
    y_.values[3] = t + simde_SHAhashSIGMA0(x_.values[0]) + maj;
    uint32_t tmp = y_.values[3];
    y_.values[3] = 0x0 | y_.values[2];
    y_.values[2] = 0x0 | y_.values[1];
    y_.values[1] = 0x0 | y_.values[0];
    y_.values[0] = 0x0 | x_.values[3];
    x_.values[3] = 0x0 | x_.values[2];
    x_.values[2] = 0x0 | x_.values[1];
    x_.values[1] = 0x0 | x_.values[0];
    x_.values[0] = tmp | 0x0;
  }
  return (part1 == 1) ? simde_uint32x4_from_private(x_) : simde_uint32x4_from_private(y_);
}

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsha256hq_u32(simde_uint32x4_t hash_efgh, simde_uint32x4_t hash_abcd, simde_uint32x4_t wk) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_SHA2)
    return vsha256hq_u32(hash_efgh, hash_abcd, wk);
  #else
    return x_simde_sha256hash(hash_efgh, hash_abcd, wk, 1);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vsha256hq_u32
  #define vsha256hq_u32(hash_efgh, hash_abcd, wk) simde_vsha256hq_u32((hash_efgh), (hash_abcd), (wk))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsha256h2q_u32(simde_uint32x4_t hash_efgh, simde_uint32x4_t hash_abcd, simde_uint32x4_t wk) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_SHA2)
    return vsha256h2q_u32(hash_efgh, hash_abcd, wk);
  #else
    return x_simde_sha256hash(hash_abcd, hash_efgh, wk, 0);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vsha256h2q_u32
  #define vsha256h2q_u32(hash_efgh, hash_abcd, wk) simde_vsha256h2q_u32((hash_efgh), (hash_abcd), (wk))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsha256su0q_u32(simde_uint32x4_t w0_3, simde_uint32x4_t w4_7) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_SHA2)
    return vsha256su0q_u32(w0_3, w4_7);
  #else
    simde_uint32x4_private
      r_,
      T_,
      x_ = simde_uint32x4_to_private(w0_3),
      y_ = simde_uint32x4_to_private(w4_7);
    T_.values[3] = y_.values[0];
    T_.values[2] = x_.values[3];
    T_.values[1] = x_.values[2];
    T_.values[0] = x_.values[1];
    uint32_t elt;
    for(int i = 0; i < 4; ++i) {
      elt = T_.values[i];
      elt = ROR32(elt, 7) ^ ROR32(elt, 18) ^ LSR(elt, 3);
      r_.values[i] = elt + x_.values[i];
    }
    return simde_uint32x4_from_private(r_);

  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vsha256su0q_u32
  #define vsha256su0q_u32(w0_3, w4_7) simde_vsha256su0q_u32((w0_3), (w4_7))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsha256su1q_u32(simde_uint32x4_t tw0_3, simde_uint32x4_t w8_11, simde_uint32x4_t w12_15) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_SHA2)
    return vsha256su1q_u32(tw0_3, w8_11, w12_15);
  #else
    simde_uint32x4_private
      r_,
      T0_,
      x_ = simde_uint32x4_to_private(tw0_3),
      y_ = simde_uint32x4_to_private(w8_11),
      z_ = simde_uint32x4_to_private(w12_15);
    simde_uint32x2_private T1_;
    T0_.values[3] = z_.values[0];
    T0_.values[2] = y_.values[3];
    T0_.values[1] = y_.values[2];
    T0_.values[0] = y_.values[1];
    uint32_t elt;
    T1_.values[1] = z_.values[3];
    T1_.values[0] = z_.values[2];
    for(int i = 0; i < 2; ++i) {
      elt = T1_.values[i];
      elt = ROR32(elt, 17) ^ ROR32(elt, 19) ^ LSR(elt, 10);
      elt = elt + x_.values[i] + T0_.values[i];
      r_.values[i] = elt;
    }
    T1_.values[1] = r_.values[1];
    T1_.values[0] = r_.values[0];
    for(int i = 2; i < 4; ++i) {
      elt = T1_.values[i-2];
      elt = ROR32(elt, 17) ^ ROR32(elt, 19) ^ LSR(elt, 10);
      elt = elt + x_.values[i] + T0_.values[i];
      r_.values[i] = elt;
    }
    return simde_uint32x4_from_private(r_);

  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vsha256su1q_u32
  #define vsha256su1q_u32(tw0_3, w8_11, w12_15) simde_vsha256su1q_u32((tw0_3), (w8_11), (w12_15))
#endif

#undef ROR32
#undef ROL32
#undef LSR
#undef LSL

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_SHA256_H) */
