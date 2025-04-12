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
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 */

#if !defined(SIMDE_ARM_NEON_ABD_H)
#define SIMDE_ARM_NEON_ABD_H

#include "abs.h"
#include "subl.h"
#include "movn.h"
#include "movl.h"
#include "reinterpret.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_float16_t
simde_vabdh_f16(simde_float16_t a, simde_float16_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vabdh_f16(a, b);
  #else
    simde_float32_t a_ = simde_float16_to_float32(a);
    simde_float32_t b_ = simde_float16_to_float32(b);
    simde_float32_t r_ = a_ - b_;
    return r_ < 0 ? simde_float16_from_float32(-r_) : simde_float16_from_float32(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vabdh_f16
  #define vabdh_f16(a, b) simde_vabdh_f16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32_t
simde_vabds_f32(simde_float32_t a, simde_float32_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vabds_f32(a, b);
  #else
    simde_float32_t r = a - b;
    return r < 0 ? -r : r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vabds_f32
  #define vabds_f32(a, b) simde_vabds_f32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64_t
simde_vabdd_f64(simde_float64_t a, simde_float64_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vabdd_f64(a, b);
  #else
    simde_float64_t r = a - b;
    return r < 0 ? -r : r;
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vabdd_f64
  #define vabdd_f64(a, b) simde_vabdd_f64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vabd_f16(simde_float16x4_t a, simde_float16x4_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vabd_f16(a, b);
  #else
    return simde_vabs_f16(simde_vsub_f16(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vabd_f16
  #define vabd_f16(a, b) simde_vabd_f16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vabd_f32(simde_float32x2_t a, simde_float32x2_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vabd_f32(a, b);
  #else
    return simde_vabs_f32(simde_vsub_f32(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vabd_f32
  #define vabd_f32(a, b) simde_vabd_f32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vabd_f64(simde_float64x1_t a, simde_float64x1_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vabd_f64(a, b);
  #else
    return simde_vabs_f64(simde_vsub_f64(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vabd_f64
  #define vabd_f64(a, b) simde_vabd_f64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vabd_s8(simde_int8x8_t a, simde_int8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vabd_s8(a, b);
  #elif defined(SIMDE_X86_MMX_NATIVE)
    simde_int8x8_private
      r_,
      a_ = simde_int8x8_to_private(a),
      b_ = simde_int8x8_to_private(b);

    const __m64 m = _mm_cmpgt_pi8(b_.m64, a_.m64);
    r_.m64 =
      _mm_xor_si64(
        _mm_add_pi8(
          _mm_sub_pi8(a_.m64, b_.m64),
          m
        ),
        m
      );

    return simde_int8x8_from_private(r_);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int8x8_private r_, max_, min_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_int8x8_private b_ = simde_int8x8_to_private(b);

    max_.sv64 = __riscv_vmax_vv_i8m1(a_.sv64, b_.sv64, 8);
    min_.sv64 = __riscv_vmin_vv_i8m1(a_.sv64, b_.sv64, 8);
    r_.sv64 = __riscv_vsub_vv_i8m1(max_.sv64, min_.sv64, 8);
    return simde_int8x8_from_private(r_);
  #else
    return simde_vmovn_s16(simde_vabsq_s16(simde_vsubl_s8(a, b)));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vabd_s8
  #define vabd_s8(a, b) simde_vabd_s8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vabd_s16(simde_int16x4_t a, simde_int16x4_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vabd_s16(a, b);
  #elif defined(SIMDE_X86_MMX_NATIVE) && defined(SIMDE_X86_SSE_NATIVE)
    simde_int16x4_private
      r_,
      a_ = simde_int16x4_to_private(a),
      b_ = simde_int16x4_to_private(b);

    r_.m64 = _mm_sub_pi16(_mm_max_pi16(a_.m64, b_.m64), _mm_min_pi16(a_.m64, b_.m64));

    return simde_int16x4_from_private(r_);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int16x4_private r_, max_, min_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_int16x4_private b_ = simde_int16x4_to_private(b);

    max_.sv64 = __riscv_vmax_vv_i16m1(a_.sv64, b_.sv64, 4);
    min_.sv64 = __riscv_vmin_vv_i16m1(a_.sv64, b_.sv64, 4);
    r_.sv64 = __riscv_vsub_vv_i16m1(max_.sv64, min_.sv64, 4);
    return simde_int16x4_from_private(r_);
  #else
    return simde_vmovn_s32(simde_vabsq_s32(simde_vsubl_s16(a, b)));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vabd_s16
  #define vabd_s16(a, b) simde_vabd_s16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vabd_s32(simde_int32x2_t a, simde_int32x2_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vabd_s32(a, b);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_int32x2_private r_, max_, min_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_int32x2_private b_ = simde_int32x2_to_private(b);

    max_.sv64 = __riscv_vmax_vv_i32m1(a_.sv64, b_.sv64, 2);
    min_.sv64 = __riscv_vmin_vv_i32m1(a_.sv64, b_.sv64, 2);
    r_.sv64 = __riscv_vsub_vv_i32m1(max_.sv64, min_.sv64, 2);
    return simde_int32x2_from_private(r_);
  #else
    return simde_vmovn_s64(simde_vabsq_s64(simde_vsubl_s32(a, b)));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vabd_s32
  #define vabd_s32(a, b) simde_vabd_s32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vabd_u8(simde_uint8x8_t a, simde_uint8x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vabd_u8(a, b);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint8x8_private r_, max_, min_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_uint8x8_private b_ = simde_uint8x8_to_private(b);

    max_.sv64 = __riscv_vmaxu_vv_u8m1(a_.sv64, b_.sv64, 8);
    min_.sv64 = __riscv_vminu_vv_u8m1(a_.sv64, b_.sv64, 8);
    r_.sv64 = __riscv_vsub_vv_u8m1(max_.sv64, min_.sv64, 8);
    return simde_uint8x8_from_private(r_);
  #else
    return simde_vmovn_u16(
      simde_vreinterpretq_u16_s16(
        simde_vabsq_s16(
          simde_vsubq_s16(
            simde_vreinterpretq_s16_u16(simde_vmovl_u8(a)),
            simde_vreinterpretq_s16_u16(simde_vmovl_u8(b))))));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vabd_u8
  #define vabd_u8(a, b) simde_vabd_u8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vabd_u16(simde_uint16x4_t a, simde_uint16x4_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vabd_u16(a, b);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint16x4_private r_, max_, min_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_uint16x4_private b_ = simde_uint16x4_to_private(b);

    max_.sv64 = __riscv_vmaxu_vv_u16m1(a_.sv64, b_.sv64, 4);
    min_.sv64 = __riscv_vminu_vv_u16m1(a_.sv64, b_.sv64, 4);
    r_.sv64 = __riscv_vsub_vv_u16m1(max_.sv64, min_.sv64, 4);
    return simde_uint16x4_from_private(r_);
  #else
    return simde_vmovn_u32(
      simde_vreinterpretq_u32_s32(
        simde_vabsq_s32(
          simde_vsubq_s32(
            simde_vreinterpretq_s32_u32(simde_vmovl_u16(a)),
            simde_vreinterpretq_s32_u32(simde_vmovl_u16(b))))));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vabd_u16
  #define vabd_u16(a, b) simde_vabd_u16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vabd_u32(simde_uint32x2_t a, simde_uint32x2_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vabd_u32(a, b);
  #elif defined(SIMDE_RISCV_V_NATIVE)
    simde_uint32x2_private r_, max_, min_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_uint32x2_private b_ = simde_uint32x2_to_private(b);

    max_.sv64 = __riscv_vmaxu_vv_u32m1(a_.sv64, b_.sv64, 2);
    min_.sv64 = __riscv_vminu_vv_u32m1(a_.sv64, b_.sv64, 2);
    r_.sv64 = __riscv_vsub_vv_u32m1(max_.sv64, min_.sv64, 2);
    return simde_uint32x2_from_private(r_);
  #else
    return simde_vmovn_u64(
      simde_vreinterpretq_u64_s64(
        simde_vabsq_s64(
          simde_vsubq_s64(
            simde_vreinterpretq_s64_u64(simde_vmovl_u32(a)),
            simde_vreinterpretq_s64_u64(simde_vmovl_u32(b))))));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vabd_u32
  #define vabd_u32(a, b) simde_vabd_u32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vabdq_f16(simde_float16x8_t a, simde_float16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vabdq_f16(a, b);
  #else
    return simde_vabsq_f16(simde_vsubq_f16(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vabdq_f16
  #define vabdq_f16(a, b) simde_vabdq_f16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vabdq_f32(simde_float32x4_t a, simde_float32x4_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vabdq_f32(a, b);
  #else
    return simde_vabsq_f32(simde_vsubq_f32(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vabdq_f32
  #define vabdq_f32(a, b) simde_vabdq_f32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vabdq_f64(simde_float64x2_t a, simde_float64x2_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vabdq_f64(a, b);
  #else
    return simde_vabsq_f64(simde_vsubq_f64(a, b));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vabdq_f64
  #define vabdq_f64(a, b) simde_vabdq_f64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vabdq_s8(simde_int8x16_t a, simde_int8x16_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vabdq_s8(a, b);
  #elif defined(SIMDE_POWER_ALTIVEC_P6_NATIVE)
    return vec_sub(vec_max(a, b), vec_min(a, b));
  #elif defined(SIMDE_ZARCH_ZVECTOR_13_NATIVE)
    return vec_max(a, b) - vec_min(a, b);
  #else
    simde_int8x16_private
      r_,
      a_ = simde_int8x16_to_private(a),
      b_ = simde_int8x16_to_private(b);

    #if defined(SIMDE_X86_SSE4_1_NATIVE)
      r_.m128i = _mm_sub_epi8(_mm_max_epi8(a_.m128i, b_.m128i), _mm_min_epi8(a_.m128i, b_.m128i));
    #elif defined(SIMDE_X86_SSE2_NATIVE)
      const __m128i m = _mm_cmpgt_epi8(b_.m128i, a_.m128i);
      r_.m128i =
        _mm_xor_si128(
          _mm_add_epi8(
            _mm_sub_epi8(a_.m128i, b_.m128i),
            m
          ),
          m
        );
    #elif defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_i8x16_sub(wasm_i8x16_max(a_.v128, b_.v128), wasm_i8x16_min(a_.v128, b_.v128));
    #elif defined(SIMDE_RISCV_V_NATIVE)
      simde_int8x16_private max_, min_;

      max_.sv128 = __riscv_vmax_vv_i8m1(a_.sv128, b_.sv128, 16);
      min_.sv128 = __riscv_vmin_vv_i8m1(a_.sv128, b_.sv128, 16);
      r_.sv128 = __riscv_vsub_vv_i8m1(max_.sv128, min_.sv128, 16);
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        int16_t tmp = HEDLEY_STATIC_CAST(int16_t, a_.values[i]) - HEDLEY_STATIC_CAST(int16_t, b_.values[i]);
        r_.values[i] = HEDLEY_STATIC_CAST(int8_t, tmp < 0 ? -tmp : tmp);
      }
    #endif

    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vabdq_s8
  #define vabdq_s8(a, b) simde_vabdq_s8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vabdq_s16(simde_int16x8_t a, simde_int16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vabdq_s16(a, b);
  #elif defined(SIMDE_POWER_ALTIVEC_P6_NATIVE)
    return vec_sub(vec_max(a, b), vec_min(a, b));
  #elif defined(SIMDE_ZARCH_ZVECTOR_13_NATIVE)
    return vec_max(a, b) - vec_min(a, b);
  #else
    simde_int16x8_private
      r_,
      a_ = simde_int16x8_to_private(a),
      b_ = simde_int16x8_to_private(b);

    #if defined(SIMDE_X86_SSE2_NATIVE)
      /* https://github.com/simd-everywhere/simde/issues/855#issuecomment-881658604 */
      r_.m128i = _mm_sub_epi16(_mm_max_epi16(a_.m128i, b_.m128i), _mm_min_epi16(a_.m128i, b_.m128i));
    #elif defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_i16x8_sub(wasm_i16x8_max(a_.v128, b_.v128), wasm_i16x8_min(a_.v128, b_.v128));
    #elif defined(SIMDE_RISCV_V_NATIVE)
      simde_int16x8_private max_, min_;

      max_.sv128 = __riscv_vmax_vv_i16m1(a_.sv128, b_.sv128, 8);
      min_.sv128 = __riscv_vmin_vv_i16m1(a_.sv128, b_.sv128, 8);
      r_.sv128 = __riscv_vsub_vv_i16m1(max_.sv128, min_.sv128, 8);
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] =
          (a_.values[i] < b_.values[i]) ?
            (b_.values[i] - a_.values[i]) :
            (a_.values[i] - b_.values[i]);
      }

    #endif
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vabdq_s16
  #define vabdq_s16(a, b) simde_vabdq_s16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vabdq_s32(simde_int32x4_t a, simde_int32x4_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vabdq_s32(a, b);
  #elif defined(SIMDE_POWER_ALTIVEC_P6_NATIVE)
    return vec_sub(vec_max(a, b), vec_min(a, b));
  #elif defined(SIMDE_ZARCH_ZVECTOR_13_NATIVE)
    return vec_max(a, b) - vec_min(a, b);
  #else
    simde_int32x4_private
      r_,
      a_ = simde_int32x4_to_private(a),
      b_ = simde_int32x4_to_private(b);

    #if defined(SIMDE_X86_SSE4_1_NATIVE)
      r_.m128i = _mm_sub_epi32(_mm_max_epi32(a_.m128i, b_.m128i), _mm_min_epi32(a_.m128i, b_.m128i));
    #elif defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_i32x4_sub(wasm_i32x4_max(a_.v128, b_.v128), wasm_i32x4_min(a_.v128, b_.v128));
    #elif defined(SIMDE_X86_SSE2_NATIVE)
      const __m128i m = _mm_cmpgt_epi32(b_.m128i, a_.m128i);
      r_.m128i =
        _mm_xor_si128(
          _mm_add_epi32(
            _mm_sub_epi32(a_.m128i, b_.m128i),
            m
          ),
          m
        );
    #elif defined(SIMDE_RISCV_V_NATIVE)
      simde_int32x4_private max_, min_;

      max_.sv128 = __riscv_vmax_vv_i32m1(a_.sv128, b_.sv128, 4);
      min_.sv128 = __riscv_vmin_vv_i32m1(a_.sv128, b_.sv128, 4);
      r_.sv128 = __riscv_vsub_vv_i32m1(max_.sv128, min_.sv128, 4);
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        int64_t tmp = HEDLEY_STATIC_CAST(int64_t, a_.values[i]) - HEDLEY_STATIC_CAST(int64_t, b_.values[i]);
        r_.values[i] = HEDLEY_STATIC_CAST(int32_t, tmp < 0 ? -tmp : tmp);
      }
    #endif

    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vabdq_s32
  #define vabdq_s32(a, b) simde_vabdq_s32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vabdq_u8(simde_uint8x16_t a, simde_uint8x16_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vabdq_u8(a, b);
  #elif defined(SIMDE_POWER_ALTIVEC_P9_NATIVE)
    return vec_absd(a, b);
  #elif defined(SIMDE_POWER_ALTIVEC_P6_NATIVE)
    return vec_sub(vec_max(a, b), vec_min(a, b));
  #elif defined(SIMDE_ZARCH_ZVECTOR_13_NATIVE)
    return vec_max(a, b) - vec_min(a, b);
  #else
    simde_uint8x16_private
      r_,
      a_ = simde_uint8x16_to_private(a),
      b_ = simde_uint8x16_to_private(b);

    #if defined(SIMDE_X86_SSE2_NATIVE)
      r_.m128i = _mm_sub_epi8(_mm_max_epu8(a_.m128i, b_.m128i), _mm_min_epu8(a_.m128i, b_.m128i));
    #elif defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_i8x16_sub(wasm_u8x16_max(a_.v128, b_.v128), wasm_u8x16_min(a_.v128, b_.v128));
    #elif defined(SIMDE_RISCV_V_NATIVE)
      simde_uint8x16_private max_, min_;

      max_.sv128 = __riscv_vmaxu_vv_u8m1(a_.sv128, b_.sv128, 16);
      min_.sv128 = __riscv_vminu_vv_u8m1(a_.sv128, b_.sv128, 16);
      r_.sv128 = __riscv_vsub_vv_u8m1(max_.sv128, min_.sv128, 16);
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        int16_t tmp = HEDLEY_STATIC_CAST(int16_t, a_.values[i]) - HEDLEY_STATIC_CAST(int16_t, b_.values[i]);
        r_.values[i] = HEDLEY_STATIC_CAST(uint8_t, tmp < 0 ? -tmp : tmp);
      }
    #endif

    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vabdq_u8
  #define vabdq_u8(a, b) simde_vabdq_u8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vabdq_u16(simde_uint16x8_t a, simde_uint16x8_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vabdq_u16(a, b);
  #elif defined(SIMDE_POWER_ALTIVEC_P9_NATIVE)
    return vec_absd(a, b);
  #elif defined(SIMDE_POWER_ALTIVEC_P6_NATIVE)
    return vec_sub(vec_max(a, b), vec_min(a, b));
  #elif defined(SIMDE_ZARCH_ZVECTOR_13_NATIVE)
    return vec_max(a, b) - vec_min(a, b);
  #else
    simde_uint16x8_private
      r_,
      a_ = simde_uint16x8_to_private(a),
      b_ = simde_uint16x8_to_private(b);

    #if defined(SIMDE_X86_SSE4_2_NATIVE)
      r_.m128i = _mm_sub_epi16(_mm_max_epu16(a_.m128i, b_.m128i), _mm_min_epu16(a_.m128i, b_.m128i));
    #elif defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_i16x8_sub(wasm_u16x8_max(a_.v128, b_.v128), wasm_u16x8_min(a_.v128, b_.v128));
    #elif defined(SIMDE_RISCV_V_NATIVE)
      simde_uint16x8_private max_, min_;

      max_.sv128 = __riscv_vmaxu_vv_u16m1(a_.sv128, b_.sv128, 8);
      min_.sv128 = __riscv_vminu_vv_u16m1(a_.sv128, b_.sv128, 8);
      r_.sv128 = __riscv_vsub_vv_u16m1(max_.sv128, min_.sv128, 8);
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        int32_t tmp = HEDLEY_STATIC_CAST(int32_t, a_.values[i]) - HEDLEY_STATIC_CAST(int32_t, b_.values[i]);
        r_.values[i] = HEDLEY_STATIC_CAST(uint16_t, tmp < 0 ? -tmp : tmp);
      }
    #endif

    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vabdq_u16
  #define vabdq_u16(a, b) simde_vabdq_u16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vabdq_u32(simde_uint32x4_t a, simde_uint32x4_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vabdq_u32(a, b);
  #elif defined(SIMDE_POWER_ALTIVEC_P9_NATIVE)
    return vec_absd(a, b);
  #elif defined(SIMDE_POWER_ALTIVEC_P6_NATIVE)
    return vec_sub(vec_max(a, b), vec_min(a, b));
  #elif defined(SIMDE_ZARCH_ZVECTOR_13_NATIVE)
    return vec_max(a, b) - vec_min(a, b);
  #else
    simde_uint32x4_private
      r_,
      a_ = simde_uint32x4_to_private(a),
      b_ = simde_uint32x4_to_private(b);

    #if defined(SIMDE_X86_SSE4_2_NATIVE)
      r_.m128i = _mm_sub_epi32(_mm_max_epu32(a_.m128i, b_.m128i), _mm_min_epu32(a_.m128i, b_.m128i));
    #elif defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_i32x4_sub(wasm_u32x4_max(a_.v128, b_.v128), wasm_u32x4_min(a_.v128, b_.v128));
    #elif defined(SIMDE_RISCV_V_NATIVE)
      simde_uint32x4_private max_, min_;

      max_.sv128 = __riscv_vmaxu_vv_u32m1(a_.sv128, b_.sv128, 4);
      min_.sv128 = __riscv_vminu_vv_u32m1(a_.sv128, b_.sv128, 4);
      r_.sv128 = __riscv_vsub_vv_u32m1(max_.sv128, min_.sv128, 4);
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        int64_t tmp = HEDLEY_STATIC_CAST(int64_t, a_.values[i]) - HEDLEY_STATIC_CAST(int64_t, b_.values[i]);
        r_.values[i] = HEDLEY_STATIC_CAST(uint32_t, tmp < 0 ? -tmp : tmp);
      }
    #endif

    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vabdq_u32
  #define vabdq_u32(a, b) simde_vabdq_u32((a), (b))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_ABD_H) */
