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

#if !defined(SIMDE_ARM_NEON_CRC32_H)
#define SIMDE_ARM_NEON_CRC32_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
uint64_t simde_crc32_reverseBits(uint64_t num, int num_of_bits)
{
  uint64_t reverse_num = 0;
  for (int i = 0; i < num_of_bits; i++) {
    if (num & (1ULL << i))
      reverse_num |= 1ULL << (num_of_bits - 1 - i);
  }
  return reverse_num;
}

SIMDE_FUNCTION_ATTRIBUTES
uint32_t simde_crc32_eor_mask(uint32_t a, uint32_t b, uint32_t mask) {
  uint32_t part_a = a & mask;
  uint32_t part_result = part_a ^ b;
  uint32_t result = (a & ~mask) | part_result;
  return result;
}

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde___crc32b(uint32_t a, uint8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(__ARM_ACLE)
    return __crc32b(a, b);
  #else
    uint32_t r_acc = HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(a, 32));
    uint32_t r_val = HEDLEY_STATIC_CAST(uint32_t, (simde_crc32_reverseBits(b, 8) << 24));
    uint32_t head = r_acc ^ r_val;
    uint32_t tail = 0;
    const uint32_t poly = 0x04C11DB7;
    for(int i = 31; i >= 24; --i) {
      if ((head>>i) & 1) {
        head = simde_crc32_eor_mask(head, poly >> (32-i), (1u << (i)) - 1);
        tail = simde_crc32_eor_mask(tail, poly << i, 0xFFFFFFFF);
      }
    }
    uint32_t result = ((head & 0x00FFFFFF) << 8) | ((tail & 0xFF000000) >> 24);
    return HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(result, 32));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef __crc32b
  #define __crc32b(a, b) simde___crc32b((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde___crc32h(uint32_t a, uint16_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(__ARM_ACLE)
    return __crc32h(a, b);
  #else
    uint32_t r_acc = HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(a, 32));
    uint32_t r_val = HEDLEY_STATIC_CAST(uint32_t, (simde_crc32_reverseBits(b, 16) << 16));
    uint32_t head = r_acc ^ r_val;
    uint32_t tail = 0;
    const uint32_t poly = 0x04C11DB7;
    for(int i = 31; i >= 16; --i) {
      if ((head>>i) & 1) {
        head = simde_crc32_eor_mask(head, poly >> (32-i), (1u << (i)) - 1);
        tail = simde_crc32_eor_mask(tail, poly << i, 0xFFFFFFFF);
      }
    }
    uint32_t result = ((head & 0x0000FFFF) << 16) | ((tail & 0xFFFF0000) >> 16);
    return HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(result, 32));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef __crc32h
  #define __crc32h(a, b) simde___crc32h((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde___crc32w(uint32_t a, uint32_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(__ARM_ACLE)
    return __crc32w(a, b);
  #else
    uint32_t r_acc = HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(a, 32));
    uint32_t r_val = HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(b, 32));
    uint32_t head = r_acc ^ r_val;
    uint32_t tail = 0;
    const uint32_t poly = 0x04C11DB7;
    for(int i = 31; i >= 0; --i) {
      if ((head>>i) & 1) {
        head = simde_crc32_eor_mask(head, poly >> (32-i), (1u << (i)) - 1);
        tail = simde_crc32_eor_mask(tail, poly << i, 0xFFFFFFFF);
      }
    }
    return HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(tail, 32));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef __crc32w
  #define __crc32w(a, b) simde___crc32w((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde___crc32d(uint32_t a, uint64_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(__ARM_ACLE)
    return __crc32d(a, b);
  #else
    uint32_t r_acc = HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(a, 32));
    uint64_t r_val = simde_crc32_reverseBits(b, 64);
    uint32_t val_head = HEDLEY_STATIC_CAST(uint32_t, r_val >> 32);
    uint32_t val_mid = HEDLEY_STATIC_CAST(uint32_t, r_val & 0x00000000FFFFFFFF);
    uint32_t head = r_acc ^ val_head;
    uint32_t mid = 0u ^ val_mid;
    uint32_t tail = 0u;
    const uint32_t poly = 0x04C11DB7;
    for(int i = 31; i >= 0; --i) {
      if ((head>>i) & 1) {
        head = simde_crc32_eor_mask(head, poly >> (32-i), (1u << (i)) - 1);
        mid = simde_crc32_eor_mask(mid, poly << i, 0xFFFFFFFF);
        tail = simde_crc32_eor_mask(tail, 0x0, 0xFFFFFFFF);
      }
    }
    for(int i = 31; i >= 0; --i) {
      if ((mid>>i) & 1) {
        mid = simde_crc32_eor_mask(mid, poly >> (32-i), (1u << (i)) - 1);
        tail = simde_crc32_eor_mask(tail, poly << i, 0xFFFFFFFF);
      }
    }
    return HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(tail, 32));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef __crc32d
  #define __crc32d(a, b) simde___crc32d((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde___crc32cb(uint32_t a, uint8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(__ARM_ACLE)
    return __crc32cb(a, b);
  #else
    uint32_t r_acc = HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(a, 32));
    uint32_t r_val = HEDLEY_STATIC_CAST(uint32_t, (simde_crc32_reverseBits(b, 8) << 24));
    uint32_t head = r_acc ^ r_val;
    uint32_t tail = 0;
    const uint32_t poly = 0x1EDC6F41;
    for(int i = 31; i >= 24; --i) {
      if ((head>>i) & 1) {
        head = simde_crc32_eor_mask(head, poly >> (32-i), (1u << (i)) - 1);
        tail = simde_crc32_eor_mask(tail, poly << i, 0xFFFFFFFF);
      }
    }
    uint32_t result = ((head & 0x00FFFFFF) << 8) | ((tail & 0xFF000000) >> 24);
    return HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(result, 32));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef __crc32cb
  #define __crc32cb(a, b) simde___crc32cb((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde___crc32ch(uint32_t a, uint16_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(__ARM_ACLE)
    return __crc32ch(a, b);
  #else
    uint32_t r_acc = HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(a, 32));
    uint32_t r_val = HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(b, 16) << 16);
    uint32_t head = r_acc ^ r_val;
    uint32_t tail = 0;
    const uint32_t poly = 0x1EDC6F41;
    for(int i = 31; i >= 16; --i) {
      if ((head>>i) & 1) {
        head = simde_crc32_eor_mask(head, poly >> (32-i), (1u << (i)) - 1);
        tail = simde_crc32_eor_mask(tail, poly << i, 0xFFFFFFFF);
      }
    }
    uint32_t result = ((head & 0x0000FFFF) << 16) | ((tail & 0xFFFF0000) >> 16);
    return HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(result, 32));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef __crc32ch
  #define __crc32ch(a, b) simde___crc32ch((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde___crc32cw(uint32_t a, uint32_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(__ARM_ACLE)
    return __crc32cw(a, b);
  #else
    uint32_t r_acc = HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(a, 32));
    uint32_t r_val = HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(b, 32));
    uint32_t head = r_acc ^ r_val;
    uint32_t tail = 0;
    const uint32_t poly = 0x1EDC6F41;
    for(int i = 31; i >= 0; --i) {
      if ((head>>i) & 1) {
        head = simde_crc32_eor_mask(head, poly >> (32-i), (1u << (i)) - 1);
        tail = simde_crc32_eor_mask(tail, poly << i, 0xFFFFFFFF);
      }
    }
    return HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(tail, 32));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef __crc32cw
  #define __crc32cw(a, b) simde___crc32cw((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde___crc32cd(uint32_t a, uint64_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(__ARM_ACLE)
    return __crc32cd(a, b);
  #else
    uint32_t r_acc = HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(a, 32));
    uint64_t r_val = simde_crc32_reverseBits(b, 64);
    uint32_t val_head = HEDLEY_STATIC_CAST(uint32_t, r_val >> 32);
    uint32_t val_mid = HEDLEY_STATIC_CAST(uint32_t, r_val & 0x00000000FFFFFFFF);
    uint32_t head = r_acc ^ val_head;
    uint32_t mid = 0u ^ val_mid;
    uint32_t tail = 0u;
    const uint32_t poly = 0x1EDC6F41;
    for(int i = 31; i >= 0; --i) {
      if ((head>>i) & 1) {
        head = simde_crc32_eor_mask(head, poly >> (32-i), (1u << (i)) - 1);
        mid = simde_crc32_eor_mask(mid, poly << i, 0xFFFFFFFF);
        tail = simde_crc32_eor_mask(tail, 0x0, 0xFFFFFFFF);
      }
    }
    for(int i = 31; i >= 0; --i) {
      if ((mid>>i) & 1) {
        mid = simde_crc32_eor_mask(mid, poly >> (32-i), (1u << (i)) - 1);
        tail = simde_crc32_eor_mask(tail, poly << i, 0xFFFFFFFF);
      }
    }
    return HEDLEY_STATIC_CAST(uint32_t, simde_crc32_reverseBits(tail, 32));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef __crc32cd
  #define __crc32cd(a, b) simde___crc32cd((a), (b))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_CRC32_H) */
