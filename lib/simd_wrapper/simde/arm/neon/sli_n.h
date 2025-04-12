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

#if !defined(SIMDE_ARM_NEON_SLI_N_H)
#define SIMDE_ARM_NEON_SLI_N_H

#include "types.h"
#include "shl_n.h"
#include "dup_n.h"
#include "and.h"
#include "orr.h"
#include "reinterpret.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vslid_n_s64(a, b, n) vslid_n_s64(a, b, n)
#else
  #define simde_vslid_n_s64(a, b, n) \
    HEDLEY_STATIC_CAST(int64_t, \
      simde_vslid_n_u64(HEDLEY_STATIC_CAST(uint64_t, a), HEDLEY_STATIC_CAST(uint64_t, b), n))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vslid_n_s64
  #define vslid_n_s64(a, b, n) simde_vslid_n_s64((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
  #define simde_vslid_n_u64(a, b, n) vslid_n_u64(a, b, n)
#else
#define simde_vslid_n_u64(a, b, n) \
    (((a & (UINT64_C(0xffffffffffffffff) >> (64 - n))) | simde_vshld_n_u64((b), (n))))
#endif
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vslid_n_u64
  #define vslid_n_u64(a, b, n) simde_vslid_n_u64((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsli_n_s8(a, b, n) vsli_n_s8((a), (b), (n))
#else
  #define simde_vsli_n_s8(a, b, n) \
    simde_vreinterpret_s8_u8(simde_vsli_n_u8( \
        simde_vreinterpret_u8_s8((a)), simde_vreinterpret_u8_s8((b)), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsli_n_s8
  #define vsli_n_s8(a, b, n) simde_vsli_n_s8((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsli_n_u8(a, b, n) vsli_n_u8((a), (b), (n))
#else
  #define simde_vsli_n_u8(a, b, n) \
    simde_vorr_u8( \
        simde_vand_u8((a), simde_vdup_n_u8((UINT8_C(0xff) >> (8 - n)))), \
        simde_vshl_n_u8((b), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsli_n_u8
  #define vsli_n_u8(a, b, n) simde_vsli_n_u8((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsli_n_s16(a, b, n) vsli_n_s16((a), (b), (n))
#else
  #define simde_vsli_n_s16(a, b, n) \
    simde_vreinterpret_s16_u16(simde_vsli_n_u16( \
        simde_vreinterpret_u16_s16((a)), simde_vreinterpret_u16_s16((b)), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsli_n_s16
  #define vsli_n_s16(a, b, n) simde_vsli_n_s16((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsli_n_u16(a, b, n) vsli_n_u16((a), (b), (n))
#else
  #define simde_vsli_n_u16(a, b, n) \
    simde_vorr_u16( \
        simde_vand_u16((a), simde_vdup_n_u16((UINT16_C(0xffff) >> (16 - n)))), \
        simde_vshl_n_u16((b), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsli_n_u16
  #define vsli_n_u16(a, b, n) simde_vsli_n_u16((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsli_n_s32(a, b, n) vsli_n_s32((a), (b), (n))
#else
  #define simde_vsli_n_s32(a, b, n) \
    simde_vreinterpret_s32_u32(simde_vsli_n_u32( \
        simde_vreinterpret_u32_s32((a)), simde_vreinterpret_u32_s32((b)), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsli_n_s32
  #define vsli_n_s32(a, b, n) simde_vsli_n_s32((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsli_n_u32(a, b, n) vsli_n_u32((a), (b), (n))
#else
  #define simde_vsli_n_u32(a, b, n) \
    simde_vorr_u32( \
        simde_vand_u32((a), \
                      simde_vdup_n_u32((UINT32_C(0xffffffff) >> (32 - n)))), \
        simde_vshl_n_u32((b), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsli_n_u32
  #define vsli_n_u32(a, b, n) simde_vsli_n_u32((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsli_n_s64(a, b, n) vsli_n_s64((a), (b), (n))
#else
  #define simde_vsli_n_s64(a, b, n) \
    simde_vreinterpret_s64_u64(simde_vsli_n_u64( \
        simde_vreinterpret_u64_s64((a)), simde_vreinterpret_u64_s64((b)), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsli_n_s64
  #define vsli_n_s64(a, b, n) simde_vsli_n_s64((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsli_n_u64(a, b, n) vsli_n_u64((a), (b), (n))
#else
#define simde_vsli_n_u64(a, b, n) \
    simde_vorr_u64( \
        simde_vand_u64((a), simde_vdup_n_u64( \
                                (UINT64_C(0xffffffffffffffff) >> (64 - n)))), \
        simde_vshl_n_u64((b), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsli_n_u64
  #define vsli_n_u64(a, b, n) simde_vsli_n_u64((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsliq_n_s8(a, b, n) vsliq_n_s8((a), (b), (n))
#else
  #define simde_vsliq_n_s8(a, b, n) \
    simde_vreinterpretq_s8_u8(simde_vsliq_n_u8( \
        simde_vreinterpretq_u8_s8((a)), simde_vreinterpretq_u8_s8((b)), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsliq_n_s8
  #define vsliq_n_s8(a, b, n) simde_vsliq_n_s8((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsliq_n_u8(a, b, n) vsliq_n_u8((a), (b), (n))
#else
  #define simde_vsliq_n_u8(a, b, n) \
    simde_vorrq_u8( \
        simde_vandq_u8((a), simde_vdupq_n_u8((UINT8_C(0xff) >> (8 - n)))), \
        simde_vshlq_n_u8((b), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsliq_n_u8
  #define vsliq_n_u8(a, b, n) simde_vsliq_n_u8((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsliq_n_s16(a, b, n) vsliq_n_s16((a), (b), (n))
#else
  #define simde_vsliq_n_s16(a, b, n) \
    simde_vreinterpretq_s16_u16(simde_vsliq_n_u16( \
        simde_vreinterpretq_u16_s16((a)), simde_vreinterpretq_u16_s16((b)), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsliq_n_s16
  #define vsliq_n_s16(a, b, n) simde_vsliq_n_s16((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsliq_n_u16(a, b, n) vsliq_n_u16((a), (b), (n))
#else
  #define simde_vsliq_n_u16(a, b, n) \
    simde_vorrq_u16( \
        simde_vandq_u16((a), simde_vdupq_n_u16((UINT16_C(0xffff) >> (16 - n)))), \
        simde_vshlq_n_u16((b), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsliq_n_u16
  #define vsliq_n_u16(a, b, n) simde_vsliq_n_u16((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsliq_n_s32(a, b, n) vsliq_n_s32((a), (b), (n))
#else
  #define simde_vsliq_n_s32(a, b, n) \
    simde_vreinterpretq_s32_u32(simde_vsliq_n_u32( \
        simde_vreinterpretq_u32_s32((a)), simde_vreinterpretq_u32_s32((b)), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsliq_n_s32
  #define vsliq_n_s32(a, b, n) simde_vsliq_n_s32((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsliq_n_u32(a, b, n) vsliq_n_u32((a), (b), (n))
#else
  #define simde_vsliq_n_u32(a, b, n) \
    simde_vorrq_u32( \
        simde_vandq_u32((a), \
                      simde_vdupq_n_u32((UINT32_C(0xffffffff) >> (32 - n)))), \
        simde_vshlq_n_u32((b), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsliq_n_u32
  #define vsliq_n_u32(a, b, n) simde_vsliq_n_u32((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsliq_n_s64(a, b, n) vsliq_n_s64((a), (b), (n))
#else
  #define simde_vsliq_n_s64(a, b, n) \
    simde_vreinterpretq_s64_u64(simde_vsliq_n_u64( \
        simde_vreinterpretq_u64_s64((a)), simde_vreinterpretq_u64_s64((b)), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsliq_n_s64
  #define vsliq_n_s64(a, b, n) simde_vsliq_n_s64((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsliq_n_u64(a, b, n) vsliq_n_u64((a), (b), (n))
#else
#define simde_vsliq_n_u64(a, b, n) \
    simde_vorrq_u64( \
        simde_vandq_u64((a), simde_vdupq_n_u64( \
                                (UINT64_C(0xffffffffffffffff) >> (64 - n)))), \
        simde_vshlq_n_u64((b), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsliq_n_u64
  #define vsliq_n_u64(a, b, n) simde_vsliq_n_u64((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsli_n_p8(a, b, n) vsli_n_p8((a), (b), (n))
#else
  #define simde_vsli_n_p8(a, b, n) \
    simde_vreinterpret_p8_u8(simde_vsli_n_u8( \
        simde_vreinterpret_u8_p8((a)), simde_vreinterpret_u8_p8((b)), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsli_n_p8
  #define vsli_n_p8(a, b, n) simde_vsli_n_p8((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsli_n_p16(a, b, n) vsli_n_p16((a), (b), (n))
#else
  #define simde_vsli_n_p16(a, b, n) \
    simde_vreinterpret_p16_u16(simde_vsli_n_u16( \
        simde_vreinterpret_u16_p16((a)), simde_vreinterpret_u16_p16((b)), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsli_n_p16
  #define vsli_n_p16(a, b, n) simde_vsli_n_p16((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
  #define simde_vsli_n_p64(a, b, n) vsli_n_p64((a), (b), (n))
#else
  #define simde_vsli_n_p64(a, b, n) \
    simde_vreinterpret_p64_u64(simde_vsli_n_u64( \
        simde_vreinterpret_u64_p64((a)), simde_vreinterpret_u64_p64((b)), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vsli_n_p64
  #define vsli_n_p64(a, b, n) simde_vsli_n_p64((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsliq_n_p8(a, b, n) vsliq_n_p8((a), (b), (n))
#else
  #define simde_vsliq_n_p8(a, b, n) \
    simde_vreinterpretq_p8_u8(simde_vsliq_n_u8( \
        simde_vreinterpretq_u8_p8((a)), simde_vreinterpretq_u8_p8((b)), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsliq_n_p8
  #define vsliq_n_p8(a, b, n) simde_vsliq_n_p8((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
  #define simde_vsliq_n_p16(a, b, n) vsliq_n_p16((a), (b), (n))
#else
  #define simde_vsliq_n_p16(a, b, n) \
    simde_vreinterpretq_p16_u16(simde_vsliq_n_u16( \
        simde_vreinterpretq_u16_p16((a)), simde_vreinterpretq_u16_p16((b)), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vsliq_n_p16
  #define vsliq_n_p16(a, b, n) simde_vsliq_n_p16((a), (b), (n))
#endif

#if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
  #define simde_vsliq_n_p64(a, b, n) vsliq_n_p64((a), (b), (n))
#else
  #define simde_vsliq_n_p64(a, b, n) \
    simde_vreinterpretq_p64_u64(simde_vsliq_n_u64( \
        simde_vreinterpretq_u64_p64((a)), simde_vreinterpretq_u64_p64((b)), (n)))
#endif
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vsliq_n_p64
  #define vsliq_n_p64(a, b, n) simde_vsliq_n_p64((a), (b), (n))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_SLI_N_H) */
