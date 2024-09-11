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

#if !defined(SIMDE_ARM_NEON_SM4_H)
#define SIMDE_ARM_NEON_SM4_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#define ROR32(operand, shift) (((operand) >> (shift)) | ((operand) << (32-shift)))
#define ROL32(operand, shift) (((operand) >> (32-shift)) | ((operand) << (shift)))
#define LSR(operand, shift) ((operand) >> (shift))
#define LSL(operand, shift) ((operand) << (shift))

static const uint8_t simde_sbox_sm4[256] = {
  0xd6,0x90,0xe9,0xfe,0xcc,0xe1,0x3d,0xb7,0x16,0xb6,0x14,0xc2,0x28,0xfb,0x2c,0x05,
  0x2b,0x67,0x9a,0x76,0x2a,0xbe,0x04,0xc3,0xaa,0x44,0x13,0x26,0x49,0x86,0x06,0x99,
  0x9c,0x42,0x50,0xf4,0x91,0xef,0x98,0x7a,0x33,0x54,0x0b,0x43,0xed,0xcf,0xac,0x62,
  0xe4,0xb3,0x1c,0xa9,0xc9,0x08,0xe8,0x95,0x80,0xdf,0x94,0xfa,0x75,0x8f,0x3f,0xa6,
  0x47,0x07,0xa7,0xfc,0xf3,0x73,0x17,0xba,0x83,0x59,0x3c,0x19,0xe6,0x85,0x4f,0xa8,
  0x68,0x6b,0x81,0xb2,0x71,0x64,0xda,0x8b,0xf8,0xeb,0x0f,0x4b,0x70,0x56,0x9d,0x35,
  0x1e,0x24,0x0e,0x5e,0x63,0x58,0xd1,0xa2,0x25,0x22,0x7c,0x3b,0x01,0x21,0x78,0x87,
  0xd4,0x00,0x46,0x57,0x9f,0xd3,0x27,0x52,0x4c,0x36,0x02,0xe7,0xa0,0xc4,0xc8,0x9e,
  0xea,0xbf,0x8a,0xd2,0x40,0xc7,0x38,0xb5,0xa3,0xf7,0xf2,0xce,0xf9,0x61,0x15,0xa1,
  0xe0,0xae,0x5d,0xa4,0x9b,0x34,0x1a,0x55,0xad,0x93,0x32,0x30,0xf5,0x8c,0xb1,0xe3,
  0x1d,0xf6,0xe2,0x2e,0x82,0x66,0xca,0x60,0xc0,0x29,0x23,0xab,0x0d,0x53,0x4e,0x6f,
  0xd5,0xdb,0x37,0x45,0xde,0xfd,0x8e,0x2f,0x03,0xff,0x6a,0x72,0x6d,0x6c,0x5b,0x51,
  0x8d,0x1b,0xaf,0x92,0xbb,0xdd,0xbc,0x7f,0x11,0xd9,0x5c,0x41,0x1f,0x10,0x5a,0xd8,
  0x0a,0xc1,0x31,0x88,0xa5,0xcd,0x7b,0xbd,0x2d,0x74,0xd0,0x12,0xb8,0xe5,0xb4,0xb0,
  0x89,0x69,0x97,0x4a,0x0c,0x96,0x77,0x7e,0x65,0xb9,0xf1,0x09,0xc5,0x6e,0xc6,0x84,
  0x18,0xf0,0x7d,0xec,0x3a,0xdc,0x4d,0x20,0x79,0xee,0x5f,0x3e,0xd7,0xcb,0x39,0x48
};

static void simde_u32_to_u8x4(uint32_t src, uint8_t* dst) {
  for(int i = 0; i < 4; ++i) {
    *(dst + i) = HEDLEY_STATIC_CAST(uint8_t, ((src << (i * 8)) >> 24));
  }
}

static void simde_u32_from_u8x4(uint8_t* src, uint32_t* dst) {
  *dst = 0;
  for(int i = 0; i < 4; ++i) {
    *dst = *dst | (HEDLEY_STATIC_CAST(uint32_t, src[i]) << (24 - i * 8));
  }
}

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsm4eq_u32(simde_uint32x4_t a, simde_uint32x4_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_SM4)
    return vsm4eq_u32(a, b);
  #else
    simde_uint32x4_private
      a_ = simde_uint32x4_to_private(a),
      b_ = simde_uint32x4_to_private(b);
    uint32_t intval, roundkey;
    uint8_t _intval[4];
    for(int index = 0; index < 4; ++index) {
      roundkey = b_.values[index];

      intval = a_.values[3] ^ a_.values[2] ^ a_.values[1] ^ roundkey;

      simde_u32_to_u8x4(intval, _intval);
      for(int i = 0; i < 4; ++i) {
        _intval[i] = simde_sbox_sm4[_intval[i]];
      }
      simde_u32_from_u8x4(_intval, &intval);
      intval = intval ^ ROL32(intval, 2) ^ ROL32(intval, 10) ^ ROL32(intval, 18) ^ ROL32(intval, 24);
      intval = intval ^ a_.values[0];

      a_.values[0] = a_.values[1];
      a_.values[1] = a_.values[2];
      a_.values[2] = a_.values[3];
      a_.values[3] = intval;
    }
    return simde_uint32x4_from_private(a_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsm4eq_u32
  #define vsm4eq_u32(a, b) simde_vsm4eq_u32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vsm4ekeyq_u32(simde_uint32x4_t a, simde_uint32x4_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_SM4)
    return vsm4ekeyq_u32(a, b);
  #else
    simde_uint32x4_private
      a_ = simde_uint32x4_to_private(a),
      b_ = simde_uint32x4_to_private(b);
    uint32_t intval, constval;
    uint8_t _intval[4];
    for(int index = 0; index < 4; ++index) {
      constval = b_.values[index];

      intval = a_.values[3] ^ a_.values[2] ^ a_.values[1] ^ constval;

      simde_u32_to_u8x4(intval, _intval);
      for(int i = 0; i < 4; ++i) {
        _intval[i] = simde_sbox_sm4[_intval[i]];
      }
      simde_u32_from_u8x4(_intval, &intval);
      intval = intval ^ ROL32(intval, 13) ^ ROL32(intval, 23);
      intval = intval ^ a_.values[0];

      a_.values[0] = a_.values[1];
      a_.values[1] = a_.values[2];
      a_.values[2] = a_.values[3];
      a_.values[3] = intval;
    }
    return simde_uint32x4_from_private(a_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vsm4ekeyq_u32
  #define vsm4ekeyq_u32(a, b) simde_vsm4ekeyq_u32((a), (b))
#endif

#undef ROR32
#undef ROL32
#undef LSR
#undef LSL

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_SM4_H) */
