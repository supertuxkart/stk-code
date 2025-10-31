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
 *   2023      Chi-Wei Chu <wewe5215@gapp.nthu.edu.tw> (Copyright owned by NTHU pllab)
 */

#if !defined(SIMDE_ARM_NEON_ST1Q_X3_H)
#define SIMDE_ARM_NEON_ST1Q_X3_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#if !defined(SIMDE_BUG_INTEL_857088)

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_f16_x3(simde_float16_t ptr[HEDLEY_ARRAY_PARAM(24)], simde_float16x8x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    vst1q_f16_x3(ptr, val);
  #else
    simde_float16x8_private a[3] = { simde_float16x8_to_private(val.val[0]),
                                      simde_float16x8_to_private(val.val[1]),
                                      simde_float16x8_to_private(val.val[2]) };
    #if defined(SIMDE_RISCV_V_NATIVE) && SIMDE_ARCH_RISCV_ZVFH
      __riscv_vse16_v_f16m1((_Float16 *)ptr , a[0].sv128 , 8);
      __riscv_vse16_v_f16m1((_Float16 *)ptr+8 , a[1].sv128 , 8);
      __riscv_vse16_v_f16m1((_Float16 *)ptr+16 , a[2].sv128 , 8);
    #else
      simde_float16_t buf[24];
      for (size_t i = 0; i < 24 ; i++) {
        buf[i] = a[i / 8].values[i % 8];
      }
      simde_memcpy(ptr, buf, sizeof(buf));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1q_f16_x3
  #define vst1q_f16_x3(a, b) simde_vst1q_f16_x3((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_f32_x3(simde_float32 ptr[HEDLEY_ARRAY_PARAM(12)], simde_float32x4x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1q_f32_x3(ptr, val);
  #else
    simde_vst1q_f32(ptr, val.val[0]);
    simde_vst1q_f32(ptr+4, val.val[1]);
    simde_vst1q_f32(ptr+8, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1q_f32_x3
  #define vst1q_f32_x3(ptr, val) simde_vst1q_f32_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_f64_x3(simde_float64 ptr[HEDLEY_ARRAY_PARAM(6)], simde_float64x2x3_t val) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    vst1q_f64_x3(ptr, val);
  #else
    simde_vst1q_f64(ptr, val.val[0]);
    simde_vst1q_f64(ptr+2, val.val[1]);
    simde_vst1q_f64(ptr+4, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vst1q_f64_x3
  #define vst1q_f64_x3(ptr, val) simde_vst1q_f64_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_s8_x3(int8_t ptr[HEDLEY_ARRAY_PARAM(48)], simde_int8x16x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1q_s8_x3(ptr, val);
  #else
    simde_vst1q_s8(ptr, val.val[0]);
    simde_vst1q_s8(ptr+16, val.val[1]);
    simde_vst1q_s8(ptr+32, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1q_s8_x3
  #define vst1q_s8_x3(ptr, val) simde_vst1q_s8_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_s16_x3(int16_t ptr[HEDLEY_ARRAY_PARAM(24)], simde_int16x8x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1q_s16_x3(ptr, val);
  #else
    simde_vst1q_s16(ptr, val.val[0]);
    simde_vst1q_s16(ptr+8, val.val[1]);
    simde_vst1q_s16(ptr+16, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1q_s16_x3
  #define vst1q_s16_x3(ptr, val) simde_vst1q_s16_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_s32_x3(int32_t ptr[HEDLEY_ARRAY_PARAM(12)], simde_int32x4x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1q_s32_x3(ptr, val);
  #else
    simde_vst1q_s32(ptr, val.val[0]);
    simde_vst1q_s32(ptr+4, val.val[1]);
    simde_vst1q_s32(ptr+8, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1q_s32_x3
  #define vst1q_s32_x3(ptr, val) simde_vst1q_s32_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_s64_x3(int64_t ptr[HEDLEY_ARRAY_PARAM(6)], simde_int64x2x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1q_s64_x3(ptr, val);
  #else
    simde_vst1q_s64(ptr, val.val[0]);
    simde_vst1q_s64(ptr+2, val.val[1]);
    simde_vst1q_s64(ptr+4, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1q_s64_x3
  #define vst1q_s64_x3(ptr, val) simde_vst1q_s64_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_u8_x3(uint8_t ptr[HEDLEY_ARRAY_PARAM(48)], simde_uint8x16x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1q_u8_x3(ptr, val);
  #else
    simde_vst1q_u8(ptr, val.val[0]);
    simde_vst1q_u8(ptr+16, val.val[1]);
    simde_vst1q_u8(ptr+32, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1q_u8_x3
  #define vst1q_u8_x3(ptr, val) simde_vst1q_u8_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_u16_x3(uint16_t ptr[HEDLEY_ARRAY_PARAM(24)], simde_uint16x8x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1q_u16_x3(ptr, val);
  #else
    simde_vst1q_u16(ptr, val.val[0]);
    simde_vst1q_u16(ptr+8, val.val[1]);
    simde_vst1q_u16(ptr+16, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1q_u16_x3
  #define vst1q_u16_x3(ptr, val) simde_vst1q_u16_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_u32_x3(uint32_t ptr[HEDLEY_ARRAY_PARAM(12)], simde_uint32x4x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1q_u32_x3(ptr, val);
  #else
    simde_vst1q_u32(ptr, val.val[0]);
    simde_vst1q_u32(ptr+4, val.val[1]);
    simde_vst1q_u32(ptr+8, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1q_u32_x3
  #define vst1q_u32_x3(ptr, val) simde_vst1q_u32_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_u64_x3(uint64_t ptr[HEDLEY_ARRAY_PARAM(6)], simde_uint64x2x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && !defined(SIMDE_BUG_GCC_REV_260989)
    vst1q_u64_x3(ptr, val);
  #else
    simde_vst1q_u64(ptr, val.val[0]);
    simde_vst1q_u64(ptr+2, val.val[1]);
    simde_vst1q_u64(ptr+4, val.val[2]);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1q_u64_x3
  #define vst1q_u64_x3(ptr, val) simde_vst1q_u64_x3((ptr), (val))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_p8_x3(simde_poly8_t ptr[HEDLEY_ARRAY_PARAM(48)], simde_poly8x16x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,5,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    vst1q_p8_x3(ptr, val);
  #else
    simde_poly8x16_private val_[3];
    for (size_t i = 0; i < 3; i++) {
      val_[i] = simde_poly8x16_to_private(val.val[i]);
    }
    #if defined(SIMDE_RISCV_V_NATIVE)
      __riscv_vse8_v_u8m1(ptr , val_[0].sv128 , 16);
      __riscv_vse8_v_u8m1(ptr+16 , val_[1].sv128 , 16);
      __riscv_vse8_v_u8m1(ptr+32 , val_[2].sv128 , 16);
    #else
      simde_memcpy(ptr, &val_, sizeof(val_));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1q_p8_x3
  #define vst1q_p8_x3(a, b) simde_vst1q_p8_x3((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_p16_x3(simde_poly16_t ptr[HEDLEY_ARRAY_PARAM(24)], simde_poly16x8x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,5,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    vst1q_p16_x3(ptr, val);
  #else
    simde_poly16x8_private val_[3];
    for (size_t i = 0; i < 3; i++) {
      val_[i] = simde_poly16x8_to_private(val.val[i]);
    }
    #if defined(SIMDE_RISCV_V_NATIVE)
      __riscv_vse16_v_u16m1(ptr , val_[0].sv128 , 8);
      __riscv_vse16_v_u16m1(ptr+8 , val_[1].sv128 , 8);
      __riscv_vse16_v_u16m1(ptr+16 , val_[2].sv128 , 8);
    #else
      simde_memcpy(ptr, &val_, sizeof(val_));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst1q_p16_x3
  #define vst1q_p16_x3(a, b) simde_vst1q_p16_x3((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_p64_x3(simde_poly64_t ptr[HEDLEY_ARRAY_PARAM(6)], simde_poly64x2x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,5,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    vst1q_p64_x3(ptr, val);
  #else
    simde_poly64x2_private val_[3];
    for (size_t i = 0; i < 3; i++) {
      val_[i] = simde_poly64x2_to_private(val.val[i]);
    }
    #if defined(SIMDE_RISCV_V_NATIVE)
      __riscv_vse64_v_u64m1(ptr , val_[0].sv128 , 2);
      __riscv_vse64_v_u64m1(ptr+2 , val_[1].sv128 , 2);
      __riscv_vse64_v_u64m1(ptr+4 , val_[2].sv128 , 2);
    #else
      simde_memcpy(ptr, &val_, sizeof(val_));
    #endif
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vst1q_p64_x3
  #define vst1q_p64_x3(a, b) simde_vst1q_p64_x3((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst1q_bf16_x3(simde_bfloat16_t ptr[HEDLEY_ARRAY_PARAM(24)], simde_bfloat16x8x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    vst1q_bf16_x3(ptr, val);
  #else
    simde_bfloat16x8_private val_[3];
    for (size_t i = 0; i < 3; i++) {
      val_[i] = simde_bfloat16x8_to_private(val.val[i]);
    }
    simde_memcpy(ptr, &val_, sizeof(val_));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vst1q_bf16_x3
  #define vst1q_bf16_x3(a, b) simde_vst1q_bf16_x3((a), (b))
#endif

#endif /* !defined(SIMDE_BUG_INTEL_857088) */

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_ST1Q_X3_H) */
