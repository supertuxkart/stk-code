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

#if !defined(SIMDE_ARM_NEON_AES_H)
#define SIMDE_ARM_NEON_AES_H

#include "types.h"
#include "../../simde-aes.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

static uint8_t simde_xtime(uint8_t x)
{
  return HEDLEY_STATIC_CAST(uint8_t, (x<<1) ^ (((x>>7) & 1) * 0x1b));
}

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vaeseq_u8(simde_uint8x16_t data, simde_uint8x16_t key) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_AES)
    return vaeseq_u8(data, key);
  #else
    /* ref: https://github.com/kokke/tiny-AES-c/blob/master/aes.c */
    simde_uint8x16_private
      r_,
      a_ = simde_uint8x16_to_private(data),
      b_ = simde_uint8x16_to_private(key);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] ^ b_.values[i];
    }
    // AESShiftRows
    uint8_t tmp;
    tmp = r_.values[1];
    r_.values[1] = r_.values[5];
    r_.values[5] = r_.values[9];
    r_.values[9] = r_.values[13];
    r_.values[13] = tmp;

    tmp = r_.values[2];
    r_.values[2] = r_.values[10];
    r_.values[10] = tmp;

    tmp = r_.values[6];
    r_.values[6] = r_.values[14];
    r_.values[14] = tmp;

    tmp = r_.values[3];
    r_.values[3] = r_.values[15];
    r_.values[15] = r_.values[11];
    r_.values[11] = r_.values[7];
    r_.values[7] = tmp;

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_x_aes_s_box[r_.values[i]];
    }
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vaeseq_u8
  #define vaeseq_u8(data, key) simde_vaeseq_u8((data), (key))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vaesdq_u8(simde_uint8x16_t data, simde_uint8x16_t key) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_AES)
    return vaesdq_u8(data, key);
  #else
    /* ref: https://github.com/kokke/tiny-AES-c/blob/master/aes.c */
    simde_uint8x16_private
      r_,
      a_ = simde_uint8x16_to_private(data),
      b_ = simde_uint8x16_to_private(key);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = a_.values[i] ^ b_.values[i];
    }
    // AESInvShiftRows
    uint8_t tmp;
    tmp = r_.values[13];
    r_.values[13] = r_.values[9];
    r_.values[9] = r_.values[5];
    r_.values[5] = r_.values[1];
    r_.values[1] = tmp;

    tmp = r_.values[2];
    r_.values[2] = r_.values[10];
    r_.values[10] = tmp;

    tmp = r_.values[6];
    r_.values[6] = r_.values[14];
    r_.values[14] = tmp;

    tmp = r_.values[3];
    r_.values[3] = r_.values[7];
    r_.values[7] = r_.values[11];
    r_.values[11] = r_.values[15];
    r_.values[15] = tmp;
    for(int i = 0; i < 16; ++i) {
      r_.values[i] = simde_x_aes_inv_s_box[r_.values[i]];
    }
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vaesdq_u8
  #define vaesdq_u8(data, key) simde_vaesdq_u8((data), (key))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vaesmcq_u8(simde_uint8x16_t data) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_AES)
    return vaesmcq_u8(data);
  #else
    /* ref: https://github.com/kokke/tiny-AES-c/blob/master/aes.c */
    simde_uint8x16_private
      a_ = simde_uint8x16_to_private(data);
    uint8_t i;
    uint8_t Tmp, Tm, t;
    for (i = 0; i < 4; ++i)
    {
      t   = a_.values[i*4+0];
      Tmp = a_.values[i*4+0] ^ a_.values[i*4+1] ^ a_.values[i*4+2] ^ a_.values[i*4+3] ;
      Tm  = a_.values[i*4+0] ^ a_.values[i*4+1] ; Tm = simde_xtime(Tm);  a_.values[i*4+0] ^= Tm ^ Tmp ;
      Tm  = a_.values[i*4+1] ^ a_.values[i*4+2] ; Tm = simde_xtime(Tm);  a_.values[i*4+1] ^= Tm ^ Tmp ;
      Tm  = a_.values[i*4+2] ^ a_.values[i*4+3] ; Tm = simde_xtime(Tm);  a_.values[i*4+2] ^= Tm ^ Tmp ;
      Tm  = a_.values[i*4+3] ^ t ;        Tm = simde_xtime(Tm);  a_.values[i*4+3] ^= Tm ^ Tmp ;
    }
    return simde_uint8x16_from_private(a_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vaesmcq_u8
  #define vaesmcq_u8(data) simde_vaesmcq_u8((data))
#endif

static uint8_t Multiply(uint8_t x, uint8_t y)
{
  return (((y & 1) * x) ^
       ((y>>1 & 1) * simde_xtime(x)) ^
       ((y>>2 & 1) * simde_xtime(simde_xtime(x))) ^
       ((y>>3 & 1) * simde_xtime(simde_xtime(simde_xtime(x)))) ^
       ((y>>4 & 1) * simde_xtime(simde_xtime(simde_xtime(simde_xtime(x)))))); /* this last call to simde_xtime() can be omitted */
}

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vaesimcq_u8(simde_uint8x16_t data) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_AES)
    return vaesimcq_u8(data);
  #else
    simde_uint8x16_private
      a_ = simde_uint8x16_to_private(data),
      r_;
    /* ref: simde/simde/x86/aes.h */
    #if defined(SIMDE_X86_AES_NATIVE)
      r_.m128i = _mm_aesimc_si128(a_.m128i);
    #else
      int Nb = simde_x_aes_Nb;
      // uint8_t k[] = {0x0e, 0x09, 0x0d, 0x0b}; // a(x) = {0e} + {09}x + {0d}x2 + {0b}x3
      uint8_t i, j, col[4], res[4];

      for (j = 0; j < Nb; j++) {
        for (i = 0; i < 4; i++) {
          col[i] = a_.values[Nb*j+i];
        }

        //coef_mult(k, col, res);
        simde_x_aes_coef_mult_lookup(4, col, res);

        for (i = 0; i < 4; i++) {
          r_.values[Nb*j+i] = res[i];
        }
      }
    #endif
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vaesimcq_u8
  #define vaesimcq_u8(data) simde_vaesimcq_u8((data))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_AES_H) */
