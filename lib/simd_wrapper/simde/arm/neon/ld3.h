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
 *   2020      Sean Maher <seanptmaher@gmail.com>
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 *   2023      Chi-Wei Chu <wewe5215@gapp.nthu.edu.tw> (Copyright owned by NTHU pllab)
 */

#if !defined(SIMDE_ARM_NEON_LD3_H)
#define SIMDE_ARM_NEON_LD3_H

#include "types.h"
#include "ld1.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
#if HEDLEY_GCC_VERSION_CHECK(7,0,0)
  SIMDE_DIAGNOSTIC_DISABLE_MAYBE_UNINITIAZILED_
#endif
SIMDE_BEGIN_DECLS_

#if !defined(SIMDE_BUG_INTEL_857088)

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4x3_t
simde_vld3_f16(simde_float16_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vld3_f16(ptr);
  #else
    simde_float16x4_private r_[3];
    #if defined(SIMDE_RISCV_V_NATIVE) && SIMDE_ARCH_RISCV_ZVFH && (SIMDE_NATURAL_VECTOR_SIZE >= 128)
      vfloat16m1x3_t dest = __riscv_vlseg3e16_v_f16m1x3((_Float16 *)&ptr[0], 4);
      r_[0].sv64 = __riscv_vget_v_f16m1x3_f16m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_f16m1x3_f16m1(dest, 1);
      r_[2].sv64 = __riscv_vget_v_f16m1x3_f16m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif
    simde_float16x4x3_t r = { {
      simde_float16x4_from_private(r_[0]),
      simde_float16x4_from_private(r_[1]),
      simde_float16x4_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3_f16
  #define vld3_f16(a) simde_vld3_f16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2x3_t
simde_vld3_f32(simde_float32 const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3_f32(ptr);
  #else
    simde_float32x2_private r_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vfloat32m1x3_t dest = __riscv_vlseg3e32_v_f32m1x3(&ptr[0], 2);
      r_[0].sv64 = __riscv_vget_v_f32m1x3_f32m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_f32m1x3_f32m1(dest, 1);
      r_[2].sv64 = __riscv_vget_v_f32m1x3_f32m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif
    simde_float32x2x3_t r = { {
      simde_float32x2_from_private(r_[0]),
      simde_float32x2_from_private(r_[1]),
      simde_float32x2_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3_f32
  #define vld3_f32(a) simde_vld3_f32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1x3_t
simde_vld3_f64(simde_float64 const *ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld3_f64(ptr);
  #else
    simde_float64x1_private r_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vfloat64m1x3_t dest = __riscv_vlseg3e64_v_f64m1x3(&ptr[0], 1);
      r_[0].sv64 = __riscv_vget_v_f64m1x3_f64m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_f64m1x3_f64m1(dest, 1);
      r_[2].sv64 = __riscv_vget_v_f64m1x3_f64m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif
    simde_float64x1x3_t r = { {
      simde_float64x1_from_private(r_[0]),
      simde_float64x1_from_private(r_[1]),
      simde_float64x1_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld3_f64
  #define vld3_f64(a) simde_vld3_f64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8x3_t
simde_vld3_s8(int8_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3_s8(ptr);
  #else
    simde_int8x8_private r_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint8m1x3_t dest = __riscv_vlseg3e8_v_i8m1x3(&ptr[0], 8);
      r_[0].sv64 = __riscv_vget_v_i8m1x3_i8m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_i8m1x3_i8m1(dest, 1);
      r_[2].sv64 = __riscv_vget_v_i8m1x3_i8m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif
    simde_int8x8x3_t r = { {
      simde_int8x8_from_private(r_[0]),
      simde_int8x8_from_private(r_[1]),
      simde_int8x8_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3_s8
  #define vld3_s8(a) simde_vld3_s8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4x3_t
simde_vld3_s16(int16_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3_s16(ptr);
  #else
    simde_int16x4_private r_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint16m1x3_t dest = __riscv_vlseg3e16_v_i16m1x3(&ptr[0], 4);
      r_[0].sv64 = __riscv_vget_v_i16m1x3_i16m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_i16m1x3_i16m1(dest, 1);
      r_[2].sv64 = __riscv_vget_v_i16m1x3_i16m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif
    simde_int16x4x3_t r = { {
      simde_int16x4_from_private(r_[0]),
      simde_int16x4_from_private(r_[1]),
      simde_int16x4_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3_s16
  #define vld3_s16(a) simde_vld3_s16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2x3_t
simde_vld3_s32(int32_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3_s32(ptr);
  #else
    simde_int32x2_private r_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint32m1x3_t dest = __riscv_vlseg3e32_v_i32m1x3(&ptr[0], 2);
      r_[0].sv64 = __riscv_vget_v_i32m1x3_i32m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_i32m1x3_i32m1(dest, 1);
      r_[2].sv64 = __riscv_vget_v_i32m1x3_i32m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif
    simde_int32x2x3_t r = { {
      simde_int32x2_from_private(r_[0]),
      simde_int32x2_from_private(r_[1]),
      simde_int32x2_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3_s32
  #define vld3_s32(a) simde_vld3_s32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1x3_t
simde_vld3_s64(int64_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3_s64(ptr);
  #else
    simde_int64x1_private r_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vint64m1x3_t dest = __riscv_vlseg3e64_v_i64m1x3(&ptr[0], 1);
      r_[0].sv64 = __riscv_vget_v_i64m1x3_i64m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_i64m1x3_i64m1(dest, 1);
      r_[2].sv64 = __riscv_vget_v_i64m1x3_i64m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif
    simde_int64x1x3_t r = { {
      simde_int64x1_from_private(r_[0]),
      simde_int64x1_from_private(r_[1]),
      simde_int64x1_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3_s64
  #define vld3_s64(a) simde_vld3_s64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8x3_t
simde_vld3_u8(uint8_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3_u8(ptr);
  #else
    simde_uint8x8_private r_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint8m1x3_t dest = __riscv_vlseg3e8_v_u8m1x3(&ptr[0], 8);
      r_[0].sv64 = __riscv_vget_v_u8m1x3_u8m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_u8m1x3_u8m1(dest, 1);
      r_[2].sv64 = __riscv_vget_v_u8m1x3_u8m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif
    simde_uint8x8x3_t r = { {
      simde_uint8x8_from_private(r_[0]),
      simde_uint8x8_from_private(r_[1]),
      simde_uint8x8_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3_u8
  #define vld3_u8(a) simde_vld3_u8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4x3_t
simde_vld3_u16(uint16_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3_u16(ptr);
  #else
    simde_uint16x4_private r_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint16m1x3_t dest = __riscv_vlseg3e16_v_u16m1x3(&ptr[0], 4);
      r_[0].sv64 = __riscv_vget_v_u16m1x3_u16m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_u16m1x3_u16m1(dest, 1);
      r_[2].sv64 = __riscv_vget_v_u16m1x3_u16m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif
    simde_uint16x4x3_t r = { {
      simde_uint16x4_from_private(r_[0]),
      simde_uint16x4_from_private(r_[1]),
      simde_uint16x4_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3_u16
  #define vld3_u16(a) simde_vld3_u16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2x3_t
simde_vld3_u32(uint32_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3_u32(ptr);
  #else
    simde_uint32x2_private r_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint32m1x3_t dest = __riscv_vlseg3e32_v_u32m1x3(&ptr[0], 2);
      r_[0].sv64 = __riscv_vget_v_u32m1x3_u32m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_u32m1x3_u32m1(dest, 1);
      r_[2].sv64 = __riscv_vget_v_u32m1x3_u32m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif
    simde_uint32x2x3_t r = { {
      simde_uint32x2_from_private(r_[0]),
      simde_uint32x2_from_private(r_[1]),
      simde_uint32x2_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3_u32
  #define vld3_u32(a) simde_vld3_u32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1x3_t
simde_vld3_u64(uint64_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3_u64(ptr);
  #else
    simde_uint64x1_private r_[3];
    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint64m1x3_t dest = __riscv_vlseg3e64_v_u64m1x3(&ptr[0], 1);
      r_[0].sv64 = __riscv_vget_v_u64m1x3_u64m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_u64m1x3_u64m1(dest, 1);
      r_[2].sv64 = __riscv_vget_v_u64m1x3_u64m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif
    simde_uint64x1x3_t r = { {
      simde_uint64x1_from_private(r_[0]),
      simde_uint64x1_from_private(r_[1]),
      simde_uint64x1_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3_u64
  #define vld3_u64(a) simde_vld3_u64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8x3_t
simde_vld3q_f16(simde_float16_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vld3q_f16(ptr);
  #else
    simde_float16x8_private r_[3];
    #if defined(SIMDE_RISCV_V_NATIVE) && SIMDE_ARCH_RISCV_ZVFH && (SIMDE_NATURAL_VECTOR_SIZE >= 128)
      vfloat16m1x3_t dest = __riscv_vlseg3e16_v_f16m1x3((_Float16 *)&ptr[0], 8);
      r_[0].sv128 = __riscv_vget_v_f16m1x3_f16m1(dest, 0);
      r_[1].sv128 = __riscv_vget_v_f16m1x3_f16m1(dest, 1);
      r_[2].sv128 = __riscv_vget_v_f16m1x3_f16m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif
    simde_float16x8x3_t r = { {
      simde_float16x8_from_private(r_[0]),
      simde_float16x8_from_private(r_[1]),
      simde_float16x8_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3q_f16
  #define vld3q_f16(a) simde_vld3q_f16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4x3_t
simde_vld3q_f32(simde_float32 const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3q_f32(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_float32x4_private r_[3];
    vfloat32m1x3_t dest = __riscv_vlseg3e32_v_f32m1x3(&ptr[0], 4);
    r_[0].sv128 = __riscv_vget_v_f32m1x3_f32m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_f32m1x3_f32m1(dest, 1);
    r_[2].sv128 = __riscv_vget_v_f32m1x3_f32m1(dest, 2);
    simde_float32x4x3_t r = { {
      simde_float32x4_from_private(r_[0]),
      simde_float32x4_from_private(r_[1]),
      simde_float32x4_from_private(r_[2])
    } };
    return r;
  #else
    simde_float32x4_private r_[3];

    for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_float32x4x3_t r = { {
      simde_float32x4_from_private(r_[0]),
      simde_float32x4_from_private(r_[1]),
      simde_float32x4_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3q_f32
  #define vld3q_f32(a) simde_vld3q_f32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2x3_t
simde_vld3q_f64(simde_float64 const *ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld3q_f64(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_float64x2_private r_[3];
    vfloat64m1x3_t dest = __riscv_vlseg3e64_v_f64m1x3(&ptr[0], 2);
    r_[0].sv128 = __riscv_vget_v_f64m1x3_f64m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_f64m1x3_f64m1(dest, 1);
    r_[2].sv128 = __riscv_vget_v_f64m1x3_f64m1(dest, 2);
    simde_float64x2x3_t r = { {
      simde_float64x2_from_private(r_[0]),
      simde_float64x2_from_private(r_[1]),
      simde_float64x2_from_private(r_[2])
    } };
    return r;
  #else
    simde_float64x2_private r_[3];

    for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_float64x2x3_t r = { {
      simde_float64x2_from_private(r_[0]),
      simde_float64x2_from_private(r_[1]),
      simde_float64x2_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld3q_f64
  #define vld3q_f64(a) simde_vld3q_f64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16x3_t
simde_vld3q_s8(int8_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3q_s8(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int8x16_private r_[3];
    vint8m1x3_t dest = __riscv_vlseg3e8_v_i8m1x3(&ptr[0], 16);
    r_[0].sv128 = __riscv_vget_v_i8m1x3_i8m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_i8m1x3_i8m1(dest, 1);
    r_[2].sv128 = __riscv_vget_v_i8m1x3_i8m1(dest, 2);
    simde_int8x16x3_t r = { {
      simde_int8x16_from_private(r_[0]),
      simde_int8x16_from_private(r_[1]),
      simde_int8x16_from_private(r_[2])
    } };
    return r;
  #else
    simde_int8x16_private r_[3];

    for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_int8x16x3_t r = { {
      simde_int8x16_from_private(r_[0]),
      simde_int8x16_from_private(r_[1]),
      simde_int8x16_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3q_s8
  #define vld3q_s8(a) simde_vld3q_s8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8x3_t
simde_vld3q_s16(int16_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3q_s16(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int16x8_private r_[3];
    vint16m1x3_t dest = __riscv_vlseg3e16_v_i16m1x3(&ptr[0], 8);
    r_[0].sv128 = __riscv_vget_v_i16m1x3_i16m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_i16m1x3_i16m1(dest, 1);
    r_[2].sv128 = __riscv_vget_v_i16m1x3_i16m1(dest, 2);
    simde_int16x8x3_t r = { {
      simde_int16x8_from_private(r_[0]),
      simde_int16x8_from_private(r_[1]),
      simde_int16x8_from_private(r_[2])
    } };
    return r;
  #else
    simde_int16x8_private r_[3];

    for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_int16x8x3_t r = { {
      simde_int16x8_from_private(r_[0]),
      simde_int16x8_from_private(r_[1]),
      simde_int16x8_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3q_s16
  #define vld3q_s16(a) simde_vld3q_s16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4x3_t
simde_vld3q_s32(int32_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3q_s32(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int32x4_private r_[3];
    vint32m1x3_t dest = __riscv_vlseg3e32_v_i32m1x3(&ptr[0], 4);
    r_[0].sv128 = __riscv_vget_v_i32m1x3_i32m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_i32m1x3_i32m1(dest, 1);
    r_[2].sv128 = __riscv_vget_v_i32m1x3_i32m1(dest, 2);
    simde_int32x4x3_t r = { {
      simde_int32x4_from_private(r_[0]),
      simde_int32x4_from_private(r_[1]),
      simde_int32x4_from_private(r_[2])
    } };
    return r;
  #else
    simde_int32x4_private r_[3];

    for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_int32x4x3_t r = { {
      simde_int32x4_from_private(r_[0]),
      simde_int32x4_from_private(r_[1]),
      simde_int32x4_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3q_s32
  #define vld3q_s32(a) simde_vld3q_s32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2x3_t
simde_vld3q_s64(int64_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld3q_s64(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int64x2_private r_[3];
    vint64m1x3_t dest = __riscv_vlseg3e64_v_i64m1x3(&ptr[0], 2);
    r_[0].sv128 = __riscv_vget_v_i64m1x3_i64m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_i64m1x3_i64m1(dest, 1);
    r_[2].sv128 = __riscv_vget_v_i64m1x3_i64m1(dest, 2);
    simde_int64x2x3_t r = { {
      simde_int64x2_from_private(r_[0]),
      simde_int64x2_from_private(r_[1]),
      simde_int64x2_from_private(r_[2])
    } };
    return r;
  #else
    simde_int64x2_private r_[3];

    for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_int64x2x3_t r = { {
      simde_int64x2_from_private(r_[0]),
      simde_int64x2_from_private(r_[1]),
      simde_int64x2_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld3q_s64
  #define vld3q_s64(a) simde_vld3q_s64((a))
#endif


SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16x3_t
simde_vld3q_u8(uint8_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3q_u8(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint8x16_private r_[3];
    vuint8m1x3_t dest = __riscv_vlseg3e8_v_u8m1x3(&ptr[0], 16);
    r_[0].sv128 = __riscv_vget_v_u8m1x3_u8m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_u8m1x3_u8m1(dest, 1);
    r_[2].sv128 = __riscv_vget_v_u8m1x3_u8m1(dest, 2);
    simde_uint8x16x3_t r = { {
      simde_uint8x16_from_private(r_[0]),
      simde_uint8x16_from_private(r_[1]),
      simde_uint8x16_from_private(r_[2])
    } };
    return r;
  #else
    simde_uint8x16_private r_[3];

    for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_uint8x16x3_t r = { {
      simde_uint8x16_from_private(r_[0]),
      simde_uint8x16_from_private(r_[1]),
      simde_uint8x16_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3q_u8
  #define vld3q_u8(a) simde_vld3q_u8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8x3_t
simde_vld3q_u16(uint16_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3q_u16(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint16x8_private r_[3];
    vuint16m1x3_t dest = __riscv_vlseg3e16_v_u16m1x3(&ptr[0], 8);
    r_[0].sv128 = __riscv_vget_v_u16m1x3_u16m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_u16m1x3_u16m1(dest, 1);
    r_[2].sv128 = __riscv_vget_v_u16m1x3_u16m1(dest, 2);
    simde_uint16x8x3_t r = { {
      simde_uint16x8_from_private(r_[0]),
      simde_uint16x8_from_private(r_[1]),
      simde_uint16x8_from_private(r_[2])
    } };
    return r;
  #else
    simde_uint16x8_private r_[3];

    for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_uint16x8x3_t r = { {
      simde_uint16x8_from_private(r_[0]),
      simde_uint16x8_from_private(r_[1]),
      simde_uint16x8_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3q_u16
  #define vld3q_u16(a) simde_vld3q_u16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4x3_t
simde_vld3q_u32(uint32_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3q_u32(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint32x4_private r_[3];
    vuint32m1x3_t dest = __riscv_vlseg3e32_v_u32m1x3(&ptr[0], 4);
    r_[0].sv128 = __riscv_vget_v_u32m1x3_u32m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_u32m1x3_u32m1(dest, 1);
    r_[2].sv128 = __riscv_vget_v_u32m1x3_u32m1(dest, 2);
    simde_uint32x4x3_t r = { {
      simde_uint32x4_from_private(r_[0]),
      simde_uint32x4_from_private(r_[1]),
      simde_uint32x4_from_private(r_[2])
    } };
    return r;
  #else
    simde_uint32x4_private r_[3];

    for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_uint32x4x3_t r = { {
      simde_uint32x4_from_private(r_[0]),
      simde_uint32x4_from_private(r_[1]),
      simde_uint32x4_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3q_u32
  #define vld3q_u32(a) simde_vld3q_u32((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2x3_t
simde_vld3q_u64(uint64_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld3q_u64(ptr);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint64x2_private r_[3];
    vuint64m1x3_t dest = __riscv_vlseg3e64_v_u64m1x3(&ptr[0], 2);
    r_[0].sv128 = __riscv_vget_v_u64m1x3_u64m1(dest, 0);
    r_[1].sv128 = __riscv_vget_v_u64m1x3_u64m1(dest, 1);
    r_[2].sv128 = __riscv_vget_v_u64m1x3_u64m1(dest, 2);
    simde_uint64x2x3_t r = { {
      simde_uint64x2_from_private(r_[0]),
      simde_uint64x2_from_private(r_[1]),
      simde_uint64x2_from_private(r_[2])
    } };
    return r;
  #else
    simde_uint64x2_private r_[3];

    for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_uint64x2x3_t r = { {
      simde_uint64x2_from_private(r_[0]),
      simde_uint64x2_from_private(r_[1]),
      simde_uint64x2_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld3q_u64
  #define vld3q_u64(a) simde_vld3q_u64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8x3_t
simde_vld3_p8(simde_poly8_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3_p8(ptr);
  #else
    simde_poly8x8_private r_[3];

    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint8m1x3_t dest = __riscv_vlseg3e8_v_u8m1x3(&ptr[0], 8);
      r_[0].sv64 = __riscv_vget_v_u8m1x3_u8m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_u8m1x3_u8m1(dest, 1);
      r_[2].sv64 = __riscv_vget_v_u8m1x3_u8m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif

    simde_poly8x8x3_t r = { {
      simde_poly8x8_from_private(r_[0]),
      simde_poly8x8_from_private(r_[1]),
      simde_poly8x8_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3_p8
  #define vld3_p8(a) simde_vld3_p8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4x3_t
simde_vld3_p16(simde_poly16_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3_p16(ptr);
  #else
    simde_poly16x4_private r_[3];

    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint16m1x3_t dest = __riscv_vlseg3e16_v_u16m1x3(&ptr[0], 4);
      r_[0].sv64 = __riscv_vget_v_u16m1x3_u16m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_u16m1x3_u16m1(dest, 1);
      r_[2].sv64 = __riscv_vget_v_u16m1x3_u16m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif

    simde_poly16x4x3_t r = { {
      simde_poly16x4_from_private(r_[0]),
      simde_poly16x4_from_private(r_[1]),
      simde_poly16x4_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3_p16
  #define vld3_p16(a) simde_vld3_p16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1x3_t
simde_vld3_p64(simde_poly64_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vld3_p64(ptr);
  #else
    simde_poly64x1_private r_[3];

    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint64m1x3_t dest = __riscv_vlseg3e64_v_u64m1x3(&ptr[0], 1);
      r_[0].sv64 = __riscv_vget_v_u64m1x3_u64m1(dest, 0);
      r_[1].sv64 = __riscv_vget_v_u64m1x3_u64m1(dest, 1);
      r_[2].sv64 = __riscv_vget_v_u64m1x3_u64m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif

    simde_poly64x1x3_t r = { {
      simde_poly64x1_from_private(r_[0]),
      simde_poly64x1_from_private(r_[1]),
      simde_poly64x1_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld3_p64
  #define vld3_p64(a) simde_vld3_p64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16x3_t
simde_vld3q_p8(simde_poly8_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3q_p8(ptr);
  #else
    simde_poly8x16_private r_[3];

    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint8m1x3_t dest = __riscv_vlseg3e8_v_u8m1x3(&ptr[0], 16);
      r_[0].sv128 = __riscv_vget_v_u8m1x3_u8m1(dest, 0);
      r_[1].sv128 = __riscv_vget_v_u8m1x3_u8m1(dest, 1);
      r_[2].sv128 = __riscv_vget_v_u8m1x3_u8m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif

    simde_poly8x16x3_t r = { {
      simde_poly8x16_from_private(r_[0]),
      simde_poly8x16_from_private(r_[1]),
      simde_poly8x16_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3q_p8
  #define vld3q_p8(a) simde_vld3q_p8((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8x3_t
simde_vld3q_p16(simde_poly16_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vld3q_p16(ptr);
  #else
    simde_poly16x8_private r_[3];

    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint16m1x3_t dest = __riscv_vlseg3e16_v_u16m1x3(&ptr[0], 8);
      r_[0].sv128 = __riscv_vget_v_u16m1x3_u16m1(dest, 0);
      r_[1].sv128 = __riscv_vget_v_u16m1x3_u16m1(dest, 1);
      r_[2].sv128 = __riscv_vget_v_u16m1x3_u16m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif

    simde_poly16x8x3_t r = { {
      simde_poly16x8_from_private(r_[0]),
      simde_poly16x8_from_private(r_[1]),
      simde_poly16x8_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vld3q_p16
  #define vld3q_p16(a) simde_vld3q_p16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2x3_t
simde_vld3q_p64(simde_poly64_t const *ptr) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vld3q_p64(ptr);
  #else
    simde_poly64x2_private r_[3];

    #if defined(SIMDE_RISCV_V_NATIVE)
      vuint64m1x3_t dest = __riscv_vlseg3e64_v_u64m1x3(&ptr[0], 2);
      r_[0].sv128 = __riscv_vget_v_u64m1x3_u64m1(dest, 0);
      r_[1].sv128 = __riscv_vget_v_u64m1x3_u64m1(dest, 1);
      r_[2].sv128 = __riscv_vget_v_u64m1x3_u64m1(dest, 2);
    #else
      for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
        for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
          r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
        }
      }
    #endif

    simde_poly64x2x3_t r = { {
      simde_poly64x2_from_private(r_[0]),
      simde_poly64x2_from_private(r_[1]),
      simde_poly64x2_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vld3q_p64
  #define vld3q_p64(a) simde_vld3q_p64((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4x3_t
simde_vld3_bf16(simde_bfloat16 const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vld3_bf16(ptr);
  #else
    simde_bfloat16x4_private r_[3];

    for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_bfloat16x4x3_t r = { {
      simde_bfloat16x4_from_private(r_[0]),
      simde_bfloat16x4_from_private(r_[1]),
      simde_bfloat16x4_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld3_bf16
  #define vld3_bf16(a) simde_vld3_bf16((a))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8x3_t
simde_vld3q_bf16(simde_bfloat16 const *ptr) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vld3q_bf16(ptr);
  #else
    simde_bfloat16x8_private r_[3];

    for (size_t i = 0; i < (sizeof(r_) / sizeof(r_[0])); i++) {
      for (size_t j = 0 ; j < (sizeof(r_[0].values) / sizeof(r_[0].values[0])) ; j++) {
        r_[i].values[j] = ptr[i + (j * (sizeof(r_) / sizeof(r_[0])))];
      }
    }

    simde_bfloat16x8x3_t r = { {
      simde_bfloat16x8_from_private(r_[0]),
      simde_bfloat16x8_from_private(r_[1]),
      simde_bfloat16x8_from_private(r_[2])
    } };

    return r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vld3q_bf16
  #define vld3q_bf16(a) simde_vld3q_bf16((a))
#endif

#endif /* !defined(SIMDE_BUG_INTEL_857088) */

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_LD3_H) */
