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
 *   2020      Evan Nemerson <evan@nemerson.com>
 *   2021      Zhi An Ng <zhin@google.com> (Copyright owned by Google, LLC)
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 *   2023      Chi-Wei Chu <wewe5215@gapp.nthu.edu.tw> (Copyright owned by NTHU pllab)
 */

#if !defined(SIMDE_ARM_NEON_ST2_H)
#define SIMDE_ARM_NEON_ST2_H

#include "types.h"
#include "zip.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#if !defined(SIMDE_BUG_INTEL_857088)

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_f16(simde_float16_t *ptr, simde_float16x4x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    vst2_f16(ptr, val);
  #else
    simde_float16x4_private a_[2] = {simde_float16x4_to_private(val.val[0]),
                                     simde_float16x4_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE) && SIMDE_ARCH_RISCV_ZVFH && (SIMDE_NATURAL_VECTOR_SIZE >= 128)
      vfloat16m1x2_t dest = __riscv_vlseg2e16_v_f16m1x2((_Float16 *)ptr, 4);
      dest = __riscv_vset_v_f16m1_f16m1x2 (dest, 0, a_[0].sv64);
      dest = __riscv_vset_v_f16m1_f16m1x2 (dest, 1, a_[1].sv64);
      __riscv_vsseg2e16_v_f16m1x2 ((_Float16 *)ptr, dest, 4);
    #else
      simde_float16_t buf[8];
      for (size_t i = 0; i < 8 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2_f16
  #define vst2_f16(a, b) simde_vst2_f16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_f32(simde_float32_t *ptr, simde_float32x2x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2_f32(ptr, val);
  #else
    simde_float32x2_private a_[2] = {simde_float32x2_to_private(val.val[0]),
                                     simde_float32x2_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vfloat32m1x2_t dest = __riscv_vlseg2e32_v_f32m1x2(ptr, 2);
      dest = __riscv_vset_v_f32m1_f32m1x2 (dest, 0, a_[0].sv64);
      dest = __riscv_vset_v_f32m1_f32m1x2 (dest, 1, a_[1].sv64);
      __riscv_vsseg2e32_v_f32m1x2 (ptr, dest, 2);
    #else
      simde_float32_t buf[4];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2_f32
  #define vst2_f32(a, b) simde_vst2_f32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_f64(simde_float64_t *ptr, simde_float64x1x2_t val) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    vst2_f64(ptr, val);
  #else
    simde_float64x1_private a_[2] = {simde_float64x1_to_private(val.val[0]),
                                     simde_float64x1_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vfloat64m1x2_t dest = __riscv_vlseg2e64_v_f64m1x2(ptr, 1);
      dest = __riscv_vset_v_f64m1_f64m1x2 (dest, 0, a_[0].sv64);
      dest = __riscv_vset_v_f64m1_f64m1x2 (dest, 1, a_[1].sv64);
      __riscv_vsseg2e64_v_f64m1x2 (ptr, dest, 1);
    #else
      simde_float64_t buf[2];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vst2_f64
  #define vst2_f64(a, b) simde_vst2_f64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_s8(int8_t *ptr, simde_int8x8x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2_s8(ptr, val);
  #else
    simde_int8x8_private a_[2] = {simde_int8x8_to_private(val.val[0]),
                                  simde_int8x8_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint8m1x2_t dest = __riscv_vlseg2e8_v_i8m1x2(ptr, 8);
      dest = __riscv_vset_v_i8m1_i8m1x2 (dest, 0, a_[0].sv64);
      dest = __riscv_vset_v_i8m1_i8m1x2 (dest, 1, a_[1].sv64);
      __riscv_vsseg2e8_v_i8m1x2 (ptr, dest, 8);
    #else
      int8_t buf[16];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2_s8
  #define vst2_s8(a, b) simde_vst2_s8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_s16(int16_t *ptr, simde_int16x4x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2_s16(ptr, val);
  #else
    simde_int16x4_private a_[2] = {simde_int16x4_to_private(val.val[0]),
                                   simde_int16x4_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint16m1x2_t dest = __riscv_vlseg2e16_v_i16m1x2(ptr, 4);
      dest = __riscv_vset_v_i16m1_i16m1x2 (dest, 0, a_[0].sv64);
      dest = __riscv_vset_v_i16m1_i16m1x2 (dest, 1, a_[1].sv64);
      __riscv_vsseg2e16_v_i16m1x2 (ptr, dest, 4);
    #else
      int16_t buf[8];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2_s16
  #define vst2_s16(a, b) simde_vst2_s16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_s32(int32_t *ptr, simde_int32x2x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2_s32(ptr, val);
  #else
    simde_int32x2_private a_[2] = {simde_int32x2_to_private(val.val[0]),
                                   simde_int32x2_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint32m1x2_t dest = __riscv_vlseg2e32_v_i32m1x2(ptr, 2);
      dest = __riscv_vset_v_i32m1_i32m1x2 (dest, 0, a_[0].sv64);
      dest = __riscv_vset_v_i32m1_i32m1x2 (dest, 1, a_[1].sv64);
      __riscv_vsseg2e32_v_i32m1x2 (ptr, dest, 2);
    #else
      int32_t buf[4];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2_s32
  #define vst2_s32(a, b) simde_vst2_s32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_s64(int64_t *ptr, simde_int64x1x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2_s64(ptr, val);
  #else
    simde_int64x1_private a_[2] = {simde_int64x1_to_private(val.val[0]),
                                   simde_int64x1_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint64m1x2_t dest = __riscv_vlseg2e64_v_i64m1x2(ptr, 1);
      dest = __riscv_vset_v_i64m1_i64m1x2 (dest, 0, a_[0].sv64);
      dest = __riscv_vset_v_i64m1_i64m1x2 (dest, 1, a_[1].sv64);
      __riscv_vsseg2e64_v_i64m1x2 (ptr, dest, 1);
    #else
      int64_t buf[2];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2_s64
  #define vst2_s64(a, b) simde_vst2_s64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_u8(uint8_t *ptr, simde_uint8x8x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2_u8(ptr, val);
  #else
    simde_uint8x8_private a_[2] = {simde_uint8x8_to_private(val.val[0]),
                                   simde_uint8x8_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint8m1x2_t dest = __riscv_vlseg2e8_v_u8m1x2(ptr, 8);
      dest = __riscv_vset_v_u8m1_u8m1x2 (dest, 0, a_[0].sv64);
      dest = __riscv_vset_v_u8m1_u8m1x2 (dest, 1, a_[1].sv64);
      __riscv_vsseg2e8_v_u8m1x2 (ptr, dest, 8);
    #else
      uint8_t buf[16];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2_u8
  #define vst2_u8(a, b) simde_vst2_u8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_u16(uint16_t *ptr, simde_uint16x4x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2_u16(ptr, val);
  #else
    simde_uint16x4_private a_[2] = {simde_uint16x4_to_private(val.val[0]),
                                    simde_uint16x4_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint16m1x2_t dest = __riscv_vlseg2e16_v_u16m1x2(ptr, 4);
      dest = __riscv_vset_v_u16m1_u16m1x2 (dest, 0, a_[0].sv64);
      dest = __riscv_vset_v_u16m1_u16m1x2 (dest, 1, a_[1].sv64);
      __riscv_vsseg2e16_v_u16m1x2 (ptr, dest, 4);
    #else
      uint16_t buf[8];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2_u16
  #define vst2_u16(a, b) simde_vst2_u16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_u32(uint32_t *ptr, simde_uint32x2x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2_u32(ptr, val);
  #else
    simde_uint32x2_private a_[2] = {simde_uint32x2_to_private(val.val[0]),
                                    simde_uint32x2_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint32m1x2_t dest = __riscv_vlseg2e32_v_u32m1x2(ptr, 2);
      dest = __riscv_vset_v_u32m1_u32m1x2 (dest, 0, a_[0].sv64);
      dest = __riscv_vset_v_u32m1_u32m1x2 (dest, 1, a_[1].sv64);
      __riscv_vsseg2e32_v_u32m1x2 (ptr, dest, 2);
    #else
      uint32_t buf[4];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2_u32
  #define vst2_u32(a, b) simde_vst2_u32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_u64(uint64_t *ptr, simde_uint64x1x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2_u64(ptr, val);
  #else
    simde_uint64x1_private a_[2] = {simde_uint64x1_to_private(val.val[0]),
                                   simde_uint64x1_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint64m1x2_t dest = __riscv_vlseg2e64_v_u64m1x2(ptr, 1);
      dest = __riscv_vset_v_u64m1_u64m1x2 (dest, 0, a_[0].sv64);
      dest = __riscv_vset_v_u64m1_u64m1x2 (dest, 1, a_[1].sv64);
      __riscv_vsseg2e64_v_u64m1x2 (ptr, dest, 1);
    #else
      uint64_t buf[2];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2_u64
  #define vst2_u64(a, b) simde_vst2_u64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_f16(simde_float16_t *ptr, simde_float16x8x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    vst2q_f16(ptr, val);
  #elif defined(SIMDE_RISCV_V_NATIVE) && SIMDE_ARCH_RISCV_ZVFH && (SIMDE_NATURAL_VECTOR_SIZE >= 128)
    simde_float16x8_private a_[2] = {simde_float16x8_to_private(val.val[0]),
                                     simde_float16x8_to_private(val.val[1])};
    vfloat16m1x2_t dest = __riscv_vlseg2e16_v_f16m1x2((_Float16 *)ptr, 8);
    dest = __riscv_vset_v_f16m1_f16m1x2 (dest, 0, a_[0].sv128);
    dest = __riscv_vset_v_f16m1_f16m1x2 (dest, 1, a_[1].sv128);
    __riscv_vsseg2e16_v_f16m1x2 ((_Float16 *)ptr, dest, 8);
  #else
    simde_float16x8x2_t r = simde_vzipq_f16(val.val[0], val.val[1]);
    simde_vst1q_f16(ptr, r.val[0]);
    simde_vst1q_f16(ptr+8, r.val[1]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2q_f16
  #define vst2q_f16(a, b) simde_vst2q_f16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_f32(simde_float32_t *ptr, simde_float32x4x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2q_f32(ptr, val);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_float32x4_private a_[2] = {simde_float32x4_to_private(val.val[0]),
                                     simde_float32x4_to_private(val.val[1])};
    vfloat32m1x2_t dest = __riscv_vlseg2e32_v_f32m1x2(ptr, 4);
    dest = __riscv_vset_v_f32m1_f32m1x2 (dest, 0, a_[0].sv128);
    dest = __riscv_vset_v_f32m1_f32m1x2 (dest, 1, a_[1].sv128);
    __riscv_vsseg2e32_v_f32m1x2 (ptr, dest, 4);
  #else
    simde_float32x4x2_t r = simde_vzipq_f32(val.val[0], val.val[1]);
    simde_vst1q_f32(ptr, r.val[0]);
    simde_vst1q_f32(ptr+4, r.val[1]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2q_f32
  #define vst2q_f32(a, b) simde_vst2q_f32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_f64(simde_float64_t *ptr, simde_float64x2x2_t val) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    vst2q_f64(ptr, val);
  #else
    simde_float64x2_private a_[2] = {simde_float64x2_to_private(val.val[0]),
                                   simde_float64x2_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vfloat64m1x2_t dest = __riscv_vlseg2e64_v_f64m1x2(ptr, 2);
      dest = __riscv_vset_v_f64m1_f64m1x2 (dest, 0, a_[0].sv128);
      dest = __riscv_vset_v_f64m1_f64m1x2 (dest, 1, a_[1].sv128);
      __riscv_vsseg2e64_v_f64m1x2 (ptr, dest, 2);
    #else
      simde_float64_t buf[4];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vst2q_f64
  #define vst2q_f64(a, b) simde_vst2q_f64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_s8(int8_t *ptr, simde_int8x16x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2q_s8(ptr, val);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int8x16_private a_[2] = {simde_int8x16_to_private(val.val[0]),
                                  simde_int8x16_to_private(val.val[1])};
    vint8m1x2_t dest = __riscv_vlseg2e8_v_i8m1x2(ptr, 16);
      dest = __riscv_vset_v_i8m1_i8m1x2 (dest, 0, a_[0].sv128);
      dest = __riscv_vset_v_i8m1_i8m1x2 (dest, 1, a_[1].sv128);
      __riscv_vsseg2e8_v_i8m1x2 (ptr, dest, 16);
  #else
    simde_int8x16x2_t r = simde_vzipq_s8(val.val[0], val.val[1]);
    simde_vst1q_s8(ptr, r.val[0]);
    simde_vst1q_s8(ptr+16, r.val[1]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2q_s8
  #define vst2q_s8(a, b) simde_vst2q_s8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_s16(int16_t *ptr, simde_int16x8x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2q_s16(ptr, val);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int16x8_private a_[2] = {simde_int16x8_to_private(val.val[0]),
                                   simde_int16x8_to_private(val.val[1])};
    vint16m1x2_t dest = __riscv_vlseg2e16_v_i16m1x2(ptr, 8);
      dest = __riscv_vset_v_i16m1_i16m1x2 (dest, 0, a_[0].sv128);
      dest = __riscv_vset_v_i16m1_i16m1x2 (dest, 1, a_[1].sv128);
      __riscv_vsseg2e16_v_i16m1x2 (ptr, dest, 8);
  #else
    simde_int16x8x2_t r = simde_vzipq_s16(val.val[0], val.val[1]);
    simde_vst1q_s16(ptr, r.val[0]);
    simde_vst1q_s16(ptr+8, r.val[1]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2q_s16
  #define vst2q_s16(a, b) simde_vst2q_s16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_s32(int32_t *ptr, simde_int32x4x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2q_s32(ptr, val);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int32x4_private a_[2] = {simde_int32x4_to_private(val.val[0]),
                                    simde_int32x4_to_private(val.val[1])};
    vint32m1x2_t dest = __riscv_vlseg2e32_v_i32m1x2(ptr, 4);
    dest = __riscv_vset_v_i32m1_i32m1x2 (dest, 0, a_[0].sv128);
    dest = __riscv_vset_v_i32m1_i32m1x2 (dest, 1, a_[1].sv128);
    __riscv_vsseg2e32_v_i32m1x2 (ptr, dest, 4);
  #else
    simde_int32x4x2_t r = simde_vzipq_s32(val.val[0], val.val[1]);
    simde_vst1q_s32(ptr, r.val[0]);
    simde_vst1q_s32(ptr+4, r.val[1]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2q_s32
  #define vst2q_s32(a, b) simde_vst2q_s32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_s64(int64_t *ptr, simde_int64x2x2_t val) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    vst2q_s64(ptr, val);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int64x2_private a_[2] = {simde_int64x2_to_private(val.val[0]),
                                   simde_int64x2_to_private(val.val[1])};
    vint64m1x2_t dest = __riscv_vlseg2e64_v_i64m1x2(ptr, 2);
    dest = __riscv_vset_v_i64m1_i64m1x2 (dest, 0, a_[0].sv128);
    dest = __riscv_vset_v_i64m1_i64m1x2 (dest, 1, a_[1].sv128);
    __riscv_vsseg2e64_v_i64m1x2 (ptr, dest, 2);
  #else
    int64_t buf[4];
    simde_int64x2_private a_[2] = {simde_int64x2_to_private(val.val[0]),
                                   simde_int64x2_to_private(val.val[1])};
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
      buf[i] = a_[i % 2].values[i / 2];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vst2q_s64
  #define vst2q_s64(a, b) simde_vst2q_s64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_u8(uint8_t *ptr, simde_uint8x16x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2q_u8(ptr, val);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint8x16_private a_[2] = {simde_uint8x16_to_private(val.val[0]),
                                   simde_uint8x16_to_private(val.val[1])};
    vuint8m1x2_t dest = __riscv_vlseg2e8_v_u8m1x2(ptr, 16);
    dest = __riscv_vset_v_u8m1_u8m1x2 (dest, 0, a_[0].sv128);
    dest = __riscv_vset_v_u8m1_u8m1x2 (dest, 1, a_[1].sv128);
    __riscv_vsseg2e8_v_u8m1x2 (ptr, dest, 16);
  #else
    simde_uint8x16x2_t r = simde_vzipq_u8(val.val[0], val.val[1]);
    simde_vst1q_u8(ptr, r.val[0]);
    simde_vst1q_u8(ptr+16, r.val[1]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2q_u8
  #define vst2q_u8(a, b) simde_vst2q_u8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_u16(uint16_t *ptr, simde_uint16x8x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2q_u16(ptr, val);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint16x8_private a_[2] = {simde_uint16x8_to_private(val.val[0]),
                                    simde_uint16x8_to_private(val.val[1])};
    vuint16m1x2_t dest = __riscv_vlseg2e16_v_u16m1x2(ptr, 8);
    dest = __riscv_vset_v_u16m1_u16m1x2 (dest, 0, a_[0].sv128);
    dest = __riscv_vset_v_u16m1_u16m1x2 (dest, 1, a_[1].sv128);
    __riscv_vsseg2e16_v_u16m1x2 (ptr, dest, 8);
  #else
    simde_uint16x8x2_t r = simde_vzipq_u16(val.val[0], val.val[1]);
    simde_vst1q_u16(ptr, r.val[0]);
    simde_vst1q_u16(ptr+8, r.val[1]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2q_u16
  #define vst2q_u16(a, b) simde_vst2q_u16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_u32(uint32_t *ptr, simde_uint32x4x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2q_u32(ptr, val);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint32x4_private a_[2] = {simde_uint32x4_to_private(val.val[0]),
                                    simde_uint32x4_to_private(val.val[1])};
    vuint32m1x2_t dest = __riscv_vlseg2e32_v_u32m1x2(ptr, 4);
    dest = __riscv_vset_v_u32m1_u32m1x2 (dest, 0, a_[0].sv128);
    dest = __riscv_vset_v_u32m1_u32m1x2 (dest, 1, a_[1].sv128);
    __riscv_vsseg2e32_v_u32m1x2 (ptr, dest, 4);
  #else
    simde_uint32x4x2_t r = simde_vzipq_u32(val.val[0], val.val[1]);
    simde_vst1q_u32(ptr, r.val[0]);
    simde_vst1q_u32(ptr+4, r.val[1]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2q_u32
  #define vst2q_u32(a, b) simde_vst2q_u32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_u64(uint64_t *ptr, simde_uint64x2x2_t val) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    vst2q_u64(ptr, val);
  #else
    simde_uint64x2_private a_[2] = {simde_uint64x2_to_private(val.val[0]),
                                   simde_uint64x2_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint64m1x2_t dest = __riscv_vlseg2e64_v_u64m1x2(ptr, 2);
      dest = __riscv_vset_v_u64m1_u64m1x2 (dest, 0, a_[0].sv128);
      dest = __riscv_vset_v_u64m1_u64m1x2 (dest, 1, a_[1].sv128);
      __riscv_vsseg2e64_v_u64m1x2 (ptr, dest, 2);
    #else
      uint64_t buf[4];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vst2q_u64
  #define vst2q_u64(a, b) simde_vst2q_u64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_p8(simde_poly8_t *ptr, simde_poly8x8x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2_p8(ptr, val);
  #else
    simde_poly8x8_private a_[2] = {simde_poly8x8_to_private(val.val[0]),
                                   simde_poly8x8_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint8m1x2_t dest = __riscv_vlseg2e8_v_u8m1x2(ptr, 8);
      dest = __riscv_vset_v_u8m1_u8m1x2 (dest, 0, a_[0].sv64);
      dest = __riscv_vset_v_u8m1_u8m1x2 (dest, 1, a_[1].sv64);
      __riscv_vsseg2e8_v_u8m1x2 (ptr, dest, 8);
    #else
      simde_poly8_t buf[16];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2_p8
  #define vst2_p8(a, b) simde_vst2_p8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_p16(simde_poly16_t *ptr, simde_poly16x4x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2_p16(ptr, val);
  #else
    simde_poly16x4_private a_[2] = {simde_poly16x4_to_private(val.val[0]),
                                    simde_poly16x4_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint16m1x2_t dest = __riscv_vlseg2e16_v_u16m1x2(ptr, 4);
      dest = __riscv_vset_v_u16m1_u16m1x2 (dest, 0, a_[0].sv64);
      dest = __riscv_vset_v_u16m1_u16m1x2 (dest, 1, a_[1].sv64);
      __riscv_vsseg2e16_v_u16m1x2 (ptr, dest, 4);
    #else
      simde_poly16_t buf[8];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2_p16
  #define vst2_p16(a, b) simde_vst2_p16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_p64(simde_poly64_t *ptr, simde_poly64x1x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    vst2_p64(ptr, val);
  #else
    simde_poly64x1_private a_[2] = {simde_poly64x1_to_private(val.val[0]),
                                   simde_poly64x1_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint64m1x2_t dest = __riscv_vlseg2e64_v_u64m1x2(ptr, 1);
      dest = __riscv_vset_v_u64m1_u64m1x2 (dest, 0, a_[0].sv64);
      dest = __riscv_vset_v_u64m1_u64m1x2 (dest, 1, a_[1].sv64);
      __riscv_vsseg2e64_v_u64m1x2 (ptr, dest, 1);
    #else
      simde_poly64_t buf[2];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vst2_p64
  #define vst2_p64(a, b) simde_vst2_p64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_p8(simde_poly8_t *ptr, simde_poly8x16x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2q_p8(ptr, val);
  #else
    simde_poly8x16_private a_[2] = {simde_poly8x16_to_private(val.val[0]),
                                   simde_poly8x16_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint8m1x2_t dest = __riscv_vlseg2e8_v_u8m1x2(ptr, 16);
      dest = __riscv_vset_v_u8m1_u8m1x2 (dest, 0, a_[0].sv128);
      dest = __riscv_vset_v_u8m1_u8m1x2 (dest, 1, a_[1].sv128);
      __riscv_vsseg2e8_v_u8m1x2 (ptr, dest, 16);
    #else
      simde_poly8_t buf[32];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2q_p8
  #define vst2q_p8(a, b) simde_vst2q_p8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_p16(simde_poly16_t *ptr, simde_poly16x8x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst2q_p16(ptr, val);
  #else
    simde_poly16x8_private a_[2] = {simde_poly16x8_to_private(val.val[0]),
                                   simde_poly16x8_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint16m1x2_t dest = __riscv_vlseg2e16_v_u16m1x2(ptr, 8);
      dest = __riscv_vset_v_u16m1_u16m1x2 (dest, 0, a_[0].sv128);
      dest = __riscv_vset_v_u16m1_u16m1x2 (dest, 1, a_[1].sv128);
      __riscv_vsseg2e16_v_u16m1x2 (ptr, dest, 8);
    #else
      simde_poly16_t buf[16];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst2q_p16
  #define vst2q_p16(a, b) simde_vst2q_p16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_p64(simde_poly64_t *ptr, simde_poly64x2x2_t val) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    vst2q_p64(ptr, val);
  #else
    simde_poly64x2_private a_[2] = {simde_poly64x2_to_private(val.val[0]),
                                   simde_poly64x2_to_private(val.val[1])};
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint64m1x2_t dest = __riscv_vlseg2e64_v_u64m1x2(ptr, 2);
      dest = __riscv_vset_v_u64m1_u64m1x2 (dest, 0, a_[0].sv128);
      dest = __riscv_vset_v_u64m1_u64m1x2 (dest, 1, a_[1].sv128);
      __riscv_vsseg2e64_v_u64m1x2 (ptr, dest, 2);
    #else
      simde_poly64_t buf[4];
      for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
        buf[i] = a_[i % 2].values[i / 2];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vst2q_p64
  #define vst2q_p64(a, b) simde_vst2q_p64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2_bf16(simde_bfloat16_t *ptr, simde_bfloat16x4x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    vst2_bf16(ptr, val);
  #else
    simde_bfloat16x4_private a_[2] = {simde_bfloat16x4_to_private(val.val[0]),
                                     simde_bfloat16x4_to_private(val.val[1])};
    simde_bfloat16_t buf[8];
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
      buf[i] = a_[i % 2].values[i / 2];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vst2_bf16
  #define vst2_bf16(a, b) simde_vst2_bf16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst2q_bf16(simde_bfloat16_t *ptr, simde_bfloat16x8x2_t val) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    vst2q_bf16(ptr, val);
  #else
    simde_bfloat16x8_private a_[2] = {simde_bfloat16x8_to_private(val.val[0]),
                                     simde_bfloat16x8_to_private(val.val[1])};
    simde_bfloat16_t buf[16];
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 2 ; i++) {
      buf[i] = a_[i % 2].values[i / 2];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vst2q_bf16
  #define vst2q_bf16(a, b) simde_vst2q_bf16((a), (b))
#endif

#endif /* !defined(SIMDE_BUG_INTEL_857088) */

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_ST2_H) */
