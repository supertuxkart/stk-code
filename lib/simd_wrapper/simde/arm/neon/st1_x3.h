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
 *   2021      Décio Luiz Gazzoni Filho <decio@decpp.net>
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 *   2023      Chi-Wei Chu <wewe5215@gapp.nthu.edu.tw> (Copyright owned by NTHU pllab)
 */

#if !defined(SIMDE_ARM_NEON_ST1_X3_H)
#define SIMDE_ARM_NEON_ST1_X3_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#if !defined(SIMDE_BUG_INTEL_857088)

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_f16_x3(simde_float16_t ptr[HEDLEY_ARRAY_PARAM(12)], simde_float16x4x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    vst1_f16_x3(ptr, val);
  #else
    simde_float16x4_private a[3] = { simde_float16x4_to_private(val.val[0]),
                                      simde_float16x4_to_private(val.val[1]),
                                      simde_float16x4_to_private(val.val[2]) };
    #if defined(SIMDE_RISCV_V_NATIVE) && SIMDE_ARCH_RISCV_ZVFH
      __riscv_vse16_v_f16m1((_Float16 *)ptr , a[0].sv64 , 4);
      __riscv_vse16_v_f16m1((_Float16 *)ptr+4 , a[1].sv64 , 4);
      __riscv_vse16_v_f16m1((_Float16 *)ptr+8 , a[2].sv64 , 4);
    #else
      simde_float16_t buf[12];
      for (size_t i = 0; i < 12 ; i++) {
        buf[i] = a[i / 4].values[i % 4];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1_f16_x3
  #define vst1_f16_x3(a, b) simde_vst1_f16_x3((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_f32_x3(simde_float32 ptr[HEDLEY_ARRAY_PARAM(6)], simde_float32x2x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1_f32_x3(ptr, val);
  #else
    simde_vst1_f32(ptr, val.val[0]);
    simde_vst1_f32(ptr+2, val.val[1]);
    simde_vst1_f32(ptr+4, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1_f32_x3
  #define vst1_f32_x3(ptr, val) simde_vst1_f32_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_f64_x3(simde_float64 ptr[HEDLEY_ARRAY_PARAM(3)], simde_float64x1x3_t val) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    vst1_f64_x3(ptr, val);
  #else
    simde_vst1_f64(ptr, val.val[0]);
    simde_vst1_f64(ptr+1, val.val[1]);
    simde_vst1_f64(ptr+2, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vst1_f64_x3
  #define vst1_f64_x3(ptr, val) simde_vst1_f64_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_s8_x3(int8_t ptr[HEDLEY_ARRAY_PARAM(24)], simde_int8x8x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1_s8_x3(ptr, val);
  #else
    simde_vst1_s8(ptr, val.val[0]);
    simde_vst1_s8(ptr+8, val.val[1]);
    simde_vst1_s8(ptr+16, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1_s8_x3
  #define vst1_s8_x3(ptr, val) simde_vst1_s8_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_s16_x3(int16_t ptr[HEDLEY_ARRAY_PARAM(12)], simde_int16x4x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1_s16_x3(ptr, val);
  #else
    simde_vst1_s16(ptr, val.val[0]);
    simde_vst1_s16(ptr+4, val.val[1]);
    simde_vst1_s16(ptr+8, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1_s16_x3
  #define vst1_s16_x3(ptr, val) simde_vst1_s16_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_s32_x3(int32_t ptr[HEDLEY_ARRAY_PARAM(6)], simde_int32x2x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1_s32_x3(ptr, val);
  #else
    simde_vst1_s32(ptr, val.val[0]);
    simde_vst1_s32(ptr+2, val.val[1]);
    simde_vst1_s32(ptr+4, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1_s32_x3
  #define vst1_s32_x3(ptr, val) simde_vst1_s32_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_s64_x3(int64_t ptr[HEDLEY_ARRAY_PARAM(3)], simde_int64x1x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1_s64_x3(ptr, val);
  #else
    simde_vst1_s64(ptr, val.val[0]);
    simde_vst1_s64(ptr+1, val.val[1]);
    simde_vst1_s64(ptr+2, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1_s64_x3
  #define vst1_s64_x3(ptr, val) simde_vst1_s64_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_u8_x3(uint8_t ptr[HEDLEY_ARRAY_PARAM(24)], simde_uint8x8x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1_u8_x3(ptr, val);
  #else
    simde_vst1_u8(ptr, val.val[0]);
    simde_vst1_u8(ptr+8, val.val[1]);
    simde_vst1_u8(ptr+16, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1_u8_x3
  #define vst1_u8_x3(ptr, val) simde_vst1_u8_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_u16_x3(uint16_t ptr[HEDLEY_ARRAY_PARAM(12)], simde_uint16x4x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1_u16_x3(ptr, val);
  #else
    simde_vst1_u16(ptr, val.val[0]);
    simde_vst1_u16(ptr+4, val.val[1]);
    simde_vst1_u16(ptr+8, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1_u16_x3
  #define vst1_u16_x3(ptr, val) simde_vst1_u16_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_u32_x3(uint32_t ptr[HEDLEY_ARRAY_PARAM(6)], simde_uint32x2x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1_u32_x3(ptr, val);
  #else
    simde_vst1_u32(ptr, val.val[0]);
    simde_vst1_u32(ptr+2, val.val[1]);
    simde_vst1_u32(ptr+4, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1_u32_x3
  #define vst1_u32_x3(ptr, val) simde_vst1_u32_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_u64_x3(uint64_t ptr[HEDLEY_ARRAY_PARAM(3)], simde_uint64x1x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1_u64_x3(ptr, val);
  #else
    simde_vst1_u64(ptr, val.val[0]);
    simde_vst1_u64(ptr+1, val.val[1]);
    simde_vst1_u64(ptr+2, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1_u64_x3
  #define vst1_u64_x3(ptr, val) simde_vst1_u64_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_p8_x3(simde_poly8_t ptr[HEDLEY_ARRAY_PARAM(24)], simde_poly8x8x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,5,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    vst1_p8_x3(ptr, val);
  #else
    simde_poly8x8_private val_[3];
    for (size_t i = 0; i < 3; i++) {
      val_[i] = simde_poly8x8_to_private(val.val[i]);
    }
    #if defined(SIMDE_RISCV_V_NATIVE)
      __riscv_vse8_v_u8m1(ptr , val_[0].sv64 , 8);
      __riscv_vse8_v_u8m1(ptr+8 , val_[1].sv64 , 8);
      __riscv_vse8_v_u8m1(ptr+16 , val_[2].sv64 , 8);
    #else
      simde_memcpy(ptr, &val_, sizeof(val_));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1_p8_x3
  #define vst1_p8_x3(a, b) simde_vst1_p8_x3((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_p16_x3(simde_poly16_t ptr[HEDLEY_ARRAY_PARAM(12)], simde_poly16x4x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,5,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    vst1_p16_x3(ptr, val);
  #else
    simde_poly16x4_private val_[3];
    for (size_t i = 0; i < 3; i++) {
      val_[i] = simde_poly16x4_to_private(val.val[i]);
    }
    #if defined(SIMDE_RISCV_V_NATIVE)
      __riscv_vse16_v_u16m1(ptr , val_[0].sv64 , 4);
      __riscv_vse16_v_u16m1(ptr+4 , val_[1].sv64 , 4);
      __riscv_vse16_v_u16m1(ptr+8 , val_[2].sv64 , 4);
    #else
      simde_memcpy(ptr, &val_, sizeof(val_));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1_p16_x3
  #define vst1_p16_x3(a, b) simde_vst1_p16_x3((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_p64_x3(simde_poly64_t ptr[HEDLEY_ARRAY_PARAM(3)], simde_poly64x1x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,5,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    vst1_p64_x3(ptr, val);
  #else
    simde_poly64x1_private val_[3];
    for (size_t i = 0; i < 3; i++) {
      val_[i] = simde_poly64x1_to_private(val.val[i]);
    }
    #if defined(SIMDE_RISCV_V_NATIVE)
      __riscv_vse64_v_u64m1(ptr , val_[0].sv64 , 1);
      __riscv_vse64_v_u64m1(ptr+1 , val_[1].sv64 , 1);
      __riscv_vse64_v_u64m1(ptr+2 , val_[2].sv64 , 1);
    #else
      simde_memcpy(ptr, &val_, sizeof(val_));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vst1_p64_x3
  #define vst1_p64_x3(a, b) simde_vst1_p64_x3((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1_bf16_x3(simde_bfloat16_t ptr[HEDLEY_ARRAY_PARAM(12)], simde_bfloat16x4x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    vst1_bf16_x3(ptr, val);
  #else
    simde_bfloat16x4_private val_[3];
    for (size_t i = 0; i < 3; i++) {
      val_[i] = simde_bfloat16x4_to_private(val.val[i]);
    }
    simde_memcpy(ptr, &val_, sizeof(val_));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vst1_bf16_x3
  #define vst1_bf16_x3(a, b) simde_vst1_bf16_x3((a), (b))
#endif

#endif /* !defined(SIMDE_BUG_INTEL_857088) */

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_ST1_X3_H) */
