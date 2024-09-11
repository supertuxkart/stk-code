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

#if !defined(SIMDE_ARM_NEON_LD1_X3_H)
#define SIMDE_ARM_NEON_LD1_X3_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
#if HEDLEY_GCC_VERSION_CHECK(7,0,0)
  SIMDE_DIAGNOSTIC_DISABLE_MAYBE_UNINITIAZILED_
#endif
SIMDE_BEGIN_DECLS_

#if !defined(SIMDE_BUG_INTEL_857088)

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4x3_t
simde_vld1_f16_x3(simde_float16_t const ptr[HEDLEY_ARRAY_PARAM(12)]) {
  #if \
      defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE))) && \
      (!defined(__clang__) || (SIMDE_DETECT_CLANG_VERSION_CHECK(7,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    return vld1_f16_x3(ptr);
  #else
    simde_float16x4_private a_[3];
    #if defined(SIMDE_RISCV_V_NATIVE) && SIMDE_ARCH_RISCV_ZVFH
      a_[0].sv64 = __riscv_vle16_v_f16m1((_Float16 *)ptr , 4);
      a_[1].sv64 = __riscv_vle16_v_f16m1((_Float16 *)(ptr+4) , 4);
      a_[2].sv64 = __riscv_vle16_v_f16m1((_Float16 *)(ptr+8) , 4);
    #else
      for (size_t i = 0; i < 12; i++) {
        a_[i / 4].values[i % 4] = ptr[i];
      }
    #endif
    simde_float16x4x3_t s_ = { { simde_float16x4_from_private(a_[0]),
                                 simde_float16x4_from_private(a_[1]),
                                 simde_float16x4_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_f16_x3
  #define vld1_f16_x3(a) simde_vld1_f16_x3((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2x3_t
simde_vld1_f32_x3(simde_float32 const ptr[HEDLEY_ARRAY_PARAM(6)]) {
  #if \
      defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE))) && \
      (!defined(__clang__) || (SIMDE_DETECT_CLANG_VERSION_CHECK(7,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    return vld1_f32_x3(ptr);
  #else
    simde_float32x2_private a_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      a_[0].sv64 = __riscv_vle32_v_f32m1(ptr , 2);
      a_[1].sv64 = __riscv_vle32_v_f32m1(ptr+2 , 2);
      a_[2].sv64 = __riscv_vle32_v_f32m1(ptr+4 , 2);
    #else
      for (size_t i = 0; i < 6; i++) {
        a_[i / 2].values[i % 2] = ptr[i];
      }
    #endif
    simde_float32x2x3_t s_ = { { simde_float32x2_from_private(a_[0]),
                                 simde_float32x2_from_private(a_[1]),
                                 simde_float32x2_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_f32_x3
  #define vld1_f32_x3(a) simde_vld1_f32_x3((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1x3_t
simde_vld1_f64_x3(simde_float64 const ptr[HEDLEY_ARRAY_PARAM(3)]) {
  #if \
      defined(SIMDE_ARM_NEON_A64V8_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || HEDLEY_GCC_VERSION_CHECK(8,0,0)) && \
      (!defined(__clang__) || SIMDE_DETECT_CLANG_VERSION_CHECK(7,0,0))
    return vld1_f64_x3(ptr);
  #else
    simde_float64x1_private a_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      a_[0].sv64 = __riscv_vle64_v_f64m1(ptr , 1);
      a_[1].sv64 = __riscv_vle64_v_f64m1(ptr+1 , 1);
      a_[2].sv64 = __riscv_vle64_v_f64m1(ptr+2 , 1);
    #else
      for (size_t i = 0; i < 3; i++) {
        a_[i].values[0] = ptr[i];
      }
    #endif
    simde_float64x1x3_t s_ = { { simde_float64x1_from_private(a_[0]),
                                 simde_float64x1_from_private(a_[1]),
                                 simde_float64x1_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld1_f64_x3
  #define vld1_f64_x3(a) simde_vld1_f64_x3((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8x3_t
simde_vld1_s8_x3(int8_t const ptr[HEDLEY_ARRAY_PARAM(24)]) {
  #if \
      defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(12,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE))) && \
      (!defined(__clang__) || (SIMDE_DETECT_CLANG_VERSION_CHECK(7,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    return vld1_s8_x3(ptr);
  #else
    simde_int8x8_private a_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      a_[0].sv64 = __riscv_vle8_v_i8m1(ptr , 8);
      a_[1].sv64 = __riscv_vle8_v_i8m1(ptr+8 , 8);
      a_[2].sv64 = __riscv_vle8_v_i8m1(ptr+16 , 8);
    #else
      for (size_t i = 0; i < 24; i++) {
        a_[i / 8].values[i % 8] = ptr[i];
      }
    #endif
    simde_int8x8x3_t s_ = { { simde_int8x8_from_private(a_[0]),
                              simde_int8x8_from_private(a_[1]),
                              simde_int8x8_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_s8_x3
  #define vld1_s8_x3(a) simde_vld1_s8_x3((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4x3_t
simde_vld1_s16_x3(int16_t const ptr[HEDLEY_ARRAY_PARAM(12)]) {
  #if \
      defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE))) && \
      (!defined(__clang__) || (SIMDE_DETECT_CLANG_VERSION_CHECK(7,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    return vld1_s16_x3(ptr);
  #else
    simde_int16x4_private a_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      a_[0].sv64 = __riscv_vle16_v_i16m1(ptr , 4);
      a_[1].sv64 = __riscv_vle16_v_i16m1(ptr+4 , 4);
      a_[2].sv64 = __riscv_vle16_v_i16m1(ptr+8 , 4);
    #else
      for (size_t i = 0; i < 12; i++) {
        a_[i / 4].values[i % 4] = ptr[i];
      }
    #endif
    simde_int16x4x3_t s_ = { { simde_int16x4_from_private(a_[0]),
                               simde_int16x4_from_private(a_[1]),
                               simde_int16x4_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_s16_x3
  #define vld1_s16_x3(a) simde_vld1_s16_x3((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2x3_t
simde_vld1_s32_x3(int32_t const ptr[HEDLEY_ARRAY_PARAM(6)]) {
  #if \
      defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(12,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE))) && \
      (!defined(__clang__) || (SIMDE_DETECT_CLANG_VERSION_CHECK(7,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    return vld1_s32_x3(ptr);
  #else
    simde_int32x2_private a_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      a_[0].sv64 = __riscv_vle32_v_i32m1(ptr , 2);
      a_[1].sv64 = __riscv_vle32_v_i32m1(ptr+2 , 2);
      a_[2].sv64 = __riscv_vle32_v_i32m1(ptr+4 , 2);
    #else
      for (size_t i = 0; i < 6; i++) {
        a_[i / 2].values[i % 2] = ptr[i];
      }
    #endif
    simde_int32x2x3_t s_ = { { simde_int32x2_from_private(a_[0]),
                               simde_int32x2_from_private(a_[1]),
                               simde_int32x2_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_s32_x3
  #define vld1_s32_x3(a) simde_vld1_s32_x3((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1x3_t
simde_vld1_s64_x3(int64_t const ptr[HEDLEY_ARRAY_PARAM(3)]) {
  #if \
      defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE))) && \
      (!defined(__clang__) || (SIMDE_DETECT_CLANG_VERSION_CHECK(7,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    return vld1_s64_x3(ptr);
  #else
    simde_int64x1_private a_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      a_[0].sv64 = __riscv_vle64_v_i64m1(ptr , 1);
      a_[1].sv64 = __riscv_vle64_v_i64m1(ptr+1 , 1);
      a_[2].sv64 = __riscv_vle64_v_i64m1(ptr+2 , 1);
    #else
      for (size_t i = 0; i < 3; i++) {
        a_[i].values[0] = ptr[i];
      }
    #endif
    simde_int64x1x3_t s_ = { { simde_int64x1_from_private(a_[0]),
                               simde_int64x1_from_private(a_[1]),
                               simde_int64x1_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_s64_x3
  #define vld1_s64_x3(a) simde_vld1_s64_x3((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8x3_t
simde_vld1_u8_x3(uint8_t const ptr[HEDLEY_ARRAY_PARAM(24)]) {
  #if \
      defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE))) && \
      (!defined(__clang__) || (SIMDE_DETECT_CLANG_VERSION_CHECK(7,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    return vld1_u8_x3(ptr);
  #else
    simde_uint8x8_private a_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      a_[0].sv64 = __riscv_vle8_v_u8m1(ptr , 8);
      a_[1].sv64 = __riscv_vle8_v_u8m1(ptr+8 , 8);
      a_[2].sv64 = __riscv_vle8_v_u8m1(ptr+16 , 8);
    #else
      for (size_t i = 0; i < 24; i++) {
        a_[i / 8].values[i % 8] = ptr[i];
      }
    #endif
    simde_uint8x8x3_t s_ = { { simde_uint8x8_from_private(a_[0]),
                               simde_uint8x8_from_private(a_[1]),
                               simde_uint8x8_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_u8_x3
  #define vld1_u8_x3(a) simde_vld1_u8_x3((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4x3_t
simde_vld1_u16_x3(uint16_t const ptr[HEDLEY_ARRAY_PARAM(12)]) {
  #if \
      defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE))) && \
      (!defined(__clang__) || (SIMDE_DETECT_CLANG_VERSION_CHECK(7,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    return vld1_u16_x3(ptr);
  #else
    simde_uint16x4_private a_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      a_[0].sv64 = __riscv_vle16_v_u16m1(ptr , 4);
      a_[1].sv64 = __riscv_vle16_v_u16m1(ptr+4 , 4);
      a_[2].sv64 = __riscv_vle16_v_u16m1(ptr+8 , 4);
    #else
      for (size_t i = 0; i < 12; i++) {
        a_[i / 4].values[i % 4] = ptr[i];
      }
    #endif
    simde_uint16x4x3_t s_ = { { simde_uint16x4_from_private(a_[0]),
                                simde_uint16x4_from_private(a_[1]),
                                simde_uint16x4_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_u16_x3
  #define vld1_u16_x3(a) simde_vld1_u16_x3((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2x3_t
simde_vld1_u32_x3(uint32_t const ptr[HEDLEY_ARRAY_PARAM(6)]) {
  #if \
      defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE))) && \
      (!defined(__clang__) || (SIMDE_DETECT_CLANG_VERSION_CHECK(7,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    return vld1_u32_x3(ptr);
  #else
    simde_uint32x2_private a_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      a_[0].sv64 = __riscv_vle32_v_u32m1(ptr , 2);
      a_[1].sv64 = __riscv_vle32_v_u32m1(ptr+2 , 2);
      a_[2].sv64 = __riscv_vle32_v_u32m1(ptr+4 , 2);
    #else
      for (size_t i = 0; i < 6; i++) {
        a_[i / 2].values[i % 2] = ptr[i];
      }
    #endif
    simde_uint32x2x3_t s_ = { { simde_uint32x2_from_private(a_[0]),
                                simde_uint32x2_from_private(a_[1]),
                                simde_uint32x2_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_u32_x3
  #define vld1_u32_x3(a) simde_vld1_u32_x3((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1x3_t
simde_vld1_u64_x3(uint64_t const ptr[HEDLEY_ARRAY_PARAM(3)]) {
  #if \
      defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE))) && \
      (!defined(__clang__) || (SIMDE_DETECT_CLANG_VERSION_CHECK(7,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    return vld1_u64_x3(ptr);
  #else
    simde_uint64x1_private a_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      a_[0].sv64 = __riscv_vle64_v_u64m1(ptr , 1);
      a_[1].sv64 = __riscv_vle64_v_u64m1(ptr+1 , 1);
      a_[2].sv64 = __riscv_vle64_v_u64m1(ptr+2 , 1);
    #else
      for (size_t i = 0; i < 3; i++) {
        a_[i].values[0] = ptr[i];
      }
    #endif
    simde_uint64x1x3_t s_ = { { simde_uint64x1_from_private(a_[0]),
                                simde_uint64x1_from_private(a_[1]),
                                simde_uint64x1_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_u64_x3
  #define vld1_u64_x3(a) simde_vld1_u64_x3((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8x3_t
simde_vld1_p8_x3(simde_poly8_t const ptr[HEDLEY_ARRAY_PARAM(24)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,5,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    return vld1_p8_x3(ptr);
  #else
    simde_poly8x8_private a_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      a_[0].sv64 = __riscv_vle8_v_u8m1(ptr , 8);
      a_[1].sv64 = __riscv_vle8_v_u8m1(ptr+8 , 8);
      a_[2].sv64 = __riscv_vle8_v_u8m1(ptr+16 , 8);
    #else
      for (size_t i = 0; i < 24; i++) {
        a_[i / 8].values[i % 8] = ptr[i];
      }
    #endif
    simde_poly8x8x3_t s_ = { { simde_poly8x8_from_private(a_[0]),
                               simde_poly8x8_from_private(a_[1]),
                               simde_poly8x8_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_p8_x3
  #define vld1_p8_x3(a) simde_vld1_p8_x3((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4x3_t
simde_vld1_p16_x3(simde_poly16_t const ptr[HEDLEY_ARRAY_PARAM(12)]) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,5,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    return vld1_p16_x3(ptr);
  #else
    simde_poly16x4_private a_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      a_[0].sv64 = __riscv_vle16_v_u16m1(ptr , 4);
      a_[1].sv64 = __riscv_vle16_v_u16m1(ptr+4 , 4);
      a_[2].sv64 = __riscv_vle16_v_u16m1(ptr+8 , 4);
    #else
      for (size_t i = 0; i < 12; i++) {
        a_[i / 4].values[i % 4] = ptr[i];
      }
    #endif
    simde_poly16x4x3_t s_ = { { simde_poly16x4_from_private(a_[0]),
                                simde_poly16x4_from_private(a_[1]),
                                simde_poly16x4_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld1_p16_x3
  #define vld1_p16_x3(a) simde_vld1_p16_x3((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1x3_t
simde_vld1_p64_x3(simde_poly64_t const ptr[HEDLEY_ARRAY_PARAM(3)]) {
  #if \
      defined(SIMDE_ARM_NEON_A32V8_NATIVE) && \
      (!defined(HEDLEY_GCC_VERSION) || (HEDLEY_GCC_VERSION_CHECK(8,5,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE))) && \
      (!defined(__clang__) || (SIMDE_DETECT_CLANG_VERSION_CHECK(9,0,0) && defined(SIMDE_ARM_NEON_A64V8_NATIVE)))
    return vld1_p64_x3(ptr);
  #else
    simde_poly64x1_private a_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      a_[0].sv64 = __riscv_vle64_v_u64m1(ptr , 1);
      a_[1].sv64 = __riscv_vle64_v_u64m1(ptr+1 , 1);
      a_[2].sv64 = __riscv_vle64_v_u64m1(ptr+2 , 1);
    #else
      for (size_t i = 0; i < 3; i++) {
        a_[i].values[0] = ptr[i];
      }
    #endif
    simde_poly64x1x3_t s_ = { { simde_poly64x1_from_private(a_[0]),
                                simde_poly64x1_from_private(a_[1]),
                                simde_poly64x1_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld1_p64_x3
  #define vld1_p64_x3(a) simde_vld1_p64_x3((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4x3_t
simde_vld1_bf16_x3(simde_bfloat16 const ptr[HEDLEY_ARRAY_PARAM(12)]) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vld1_bf16_x3(ptr);
  #else
    simde_bfloat16x4_private a_[3];
    for (size_t i = 0; i < 12; i++) {
      a_[i / 4].values[i % 4] = ptr[i];
    }
    simde_bfloat16x4x3_t s_ = { { simde_bfloat16x4_from_private(a_[0]),
                                 simde_bfloat16x4_from_private(a_[1]),
                                 simde_bfloat16x4_from_private(a_[2]) } };
    return s_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld1_bf16_x3
  #define vld1_bf16_x3(a) simde_vld1_bf16_x3((a))
#endif

#endif /* !defined(SIMDE_BUG_INTEL_857088) */

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_LD1_X3_H) */
