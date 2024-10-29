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
 *   2020      Christopher Moore <moore@free.fr>
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 */

#if !defined(SIMDE_ARM_NEON_QTBX_H)
#define SIMDE_ARM_NEON_QTBX_H

#include "reinterpret.h"
#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vqtbx1_u8(simde_uint8x8_t a, simde_uint8x16_t t, simde_uint8x8_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx1_u8(a, t, idx);
  #elif defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    uint8x8x2_t split;
    simde_memcpy(&split, &t, sizeof(split));
    return vtbx2_u8(a, split, idx);
  #else
    simde_uint8x16_private t_ = simde_uint8x16_to_private(t);
    simde_uint8x8_private
      r_,
      a_ = simde_uint8x8_to_private(a),
      idx_ = simde_uint8x8_to_private(idx);

    #if defined(SIMDE_X86_SSE4_1_NATIVE) && defined(SIMDE_X86_MMX_NATIVE)
      __m128i idx128 = _mm_set1_epi64(idx_.m64);
      idx128 = _mm_or_si128(idx128, _mm_cmpgt_epi8(idx128, _mm_set1_epi8(15)));
      __m128i r128 = _mm_shuffle_epi8(t_.m128i, idx128);
      r128 =  _mm_blendv_epi8(r128, _mm_set1_epi64(a_.m64), idx128);
      r_.m64 = _mm_movepi64_pi64(r128);
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = (idx_.values[i] < 16) ? t_.values[idx_.values[i]] : a_.values[i];
      }
    #endif

    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx1_u8
  #define vqtbx1_u8(a, t, idx) simde_vqtbx1_u8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vqtbx1_s8(simde_int8x8_t a, simde_int8x16_t t, simde_uint8x8_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx1_s8(a, t, idx);
  #else
    return simde_vreinterpret_s8_u8(simde_vqtbx1_u8(simde_vreinterpret_u8_s8(a), simde_vreinterpretq_u8_s8(t), idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx1_s8
  #define vqtbx1_s8(a, t, idx) simde_vqtbx1_s8((a), (t), (idx))
#endif

#if !defined(SIMDE_BUG_INTEL_857088)

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vqtbx2_u8(simde_uint8x8_t a, simde_uint8x16x2_t t, simde_uint8x8_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx2_u8(a, t, idx);
  #elif defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    uint8x8x4_t split;
    simde_memcpy(&split, &t, sizeof(split));
    return vtbx4_u8(a, split, idx);
  #else
    simde_uint8x16_private t_[2] = { simde_uint8x16_to_private(t.val[0]), simde_uint8x16_to_private(t.val[1]) };
    simde_uint8x8_private
      r_,
      a_ = simde_uint8x8_to_private(a),
      idx_ = simde_uint8x8_to_private(idx);

    #if defined(SIMDE_X86_SSE4_1_NATIVE) && defined(SIMDE_X86_MMX_NATIVE)
      __m128i idx128 = _mm_set1_epi64(idx_.m64);
      idx128 = _mm_or_si128(idx128, _mm_cmpgt_epi8(idx128, _mm_set1_epi8(31)));
      __m128i r128_0 = _mm_shuffle_epi8(t_[0].m128i, idx128);
      __m128i r128_1 = _mm_shuffle_epi8(t_[1].m128i, idx128);
      __m128i r128 = _mm_blendv_epi8(r128_0, r128_1, _mm_slli_epi32(idx128, 3));
      r128 =  _mm_blendv_epi8(r128, _mm_set1_epi64(a_.m64), idx128);
      r_.m64 = _mm_movepi64_pi64(r128);
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = (idx_.values[i] < 32) ? t_[idx_.values[i] / 16].values[idx_.values[i] & 15] : a_.values[i];
      }
    #endif

    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx2_u8
  #define vqtbx2_u8(a, t, idx) simde_vqtbx2_u8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vqtbx2_s8(simde_int8x8_t a, simde_int8x16x2_t t, simde_uint8x8_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx2_s8(a, t, idx);
  #else
    simde_uint8x16x2_t t_;
    simde_memcpy(&t_, &t, sizeof(t_));
    return simde_vreinterpret_s8_u8(simde_vqtbx2_u8(simde_vreinterpret_u8_s8(a), t_, idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx2_s8
  #define vqtbx2_s8(a, t, idx) simde_vqtbx2_s8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vqtbx3_u8(simde_uint8x8_t a, simde_uint8x16x3_t t, simde_uint8x8_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx3_u8(a, t, idx);
  #elif defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    uint8x8_t idx_hi = vsub_u8(idx, vdup_n_u8(32));
    uint8x8x4_t split_lo;
    uint8x8x2_t split_hi;
    simde_memcpy(&split_lo, &t.val[0], sizeof(split_lo));
    simde_memcpy(&split_hi, &t.val[2], sizeof(split_hi));
    uint8x8_t hi = vtbx2_u8(a, split_hi, idx_hi);
    return vtbx4_u8(hi, split_lo, idx);
  #else
    simde_uint8x16_private t_[3] = { simde_uint8x16_to_private(t.val[0]), simde_uint8x16_to_private(t.val[1]), simde_uint8x16_to_private(t.val[2]) };
    simde_uint8x8_private
      r_,
      a_ = simde_uint8x8_to_private(a),
      idx_ = simde_uint8x8_to_private(idx);

    #if defined(SIMDE_X86_SSE4_1_NATIVE) && defined(SIMDE_X86_MMX_NATIVE)
      __m128i idx128 = _mm_set1_epi64(idx_.m64);
      idx128 = _mm_or_si128(idx128, _mm_cmpgt_epi8(idx128, _mm_set1_epi8(47)));
      __m128i r128_0 = _mm_shuffle_epi8(t_[0].m128i, idx128);
      __m128i r128_1 = _mm_shuffle_epi8(t_[1].m128i, idx128);
      __m128i r128_01 = _mm_blendv_epi8(r128_0, r128_1, _mm_slli_epi32(idx128, 3));
      __m128i r128_2 = _mm_shuffle_epi8(t_[2].m128i, idx128);
      __m128i r128 = _mm_blendv_epi8(r128_01, r128_2, _mm_slli_epi32(idx128, 2));
      r128 =  _mm_blendv_epi8(r128, _mm_set1_epi64(a_.m64), idx128);
      r_.m64 = _mm_movepi64_pi64(r128);
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = (idx_.values[i] < 48) ? t_[idx_.values[i] / 16].values[idx_.values[i] & 15] : a_.values[i];
      }
    #endif

    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx3_u8
  #define vqtbx3_u8(a, t, idx) simde_vqtbx3_u8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vqtbx3_s8(simde_int8x8_t a, simde_int8x16x3_t t, simde_uint8x8_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx3_s8(a, t, idx);
  #else
    simde_uint8x16x3_t t_;
    simde_memcpy(&t_, &t, sizeof(t_));
    return simde_vreinterpret_s8_u8(simde_vqtbx3_u8(simde_vreinterpret_u8_s8(a), t_, idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx3_s8
  #define vqtbx3_s8(a, t, idx) simde_vqtbx3_s8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vqtbx4_u8(simde_uint8x8_t a, simde_uint8x16x4_t t, simde_uint8x8_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx4_u8(a, t, idx);
  #elif defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    uint8x8_t idx_hi = vsub_u8(idx, vdup_n_u8(32));
    uint8x8x4_t split_lo;
    uint8x8x4_t split_hi;
    simde_memcpy(&split_lo, &t.val[0], sizeof(split_lo));
    simde_memcpy(&split_hi, &t.val[2], sizeof(split_hi));
    uint8x8_t lo = vtbx4_u8(a, split_lo, idx);
    return vtbx4_u8(lo, split_hi, idx_hi);
  #else
    simde_uint8x16_private t_[4] = { simde_uint8x16_to_private(t.val[0]), simde_uint8x16_to_private(t.val[1]), simde_uint8x16_to_private(t.val[2]), simde_uint8x16_to_private(t.val[3]) };
    simde_uint8x8_private
      r_,
      a_ = simde_uint8x8_to_private(a),
      idx_ = simde_uint8x8_to_private(idx);

    #if defined(SIMDE_X86_SSE4_1_NATIVE) && defined(SIMDE_X86_MMX_NATIVE)
      __m128i idx128 = _mm_set1_epi64(idx_.m64);
      idx128 = _mm_or_si128(idx128, _mm_cmpgt_epi8(idx128, _mm_set1_epi8(63)));
      __m128i idx128_shl3 = _mm_slli_epi32(idx128, 3);
      __m128i r128_0 = _mm_shuffle_epi8(t_[0].m128i, idx128);
      __m128i r128_1 = _mm_shuffle_epi8(t_[1].m128i, idx128);
      __m128i r128_01 = _mm_blendv_epi8(r128_0, r128_1, idx128_shl3);
      __m128i r128_2 = _mm_shuffle_epi8(t_[2].m128i, idx128);
      __m128i r128_3 = _mm_shuffle_epi8(t_[3].m128i, idx128);
      __m128i r128_23 = _mm_blendv_epi8(r128_2, r128_3, idx128_shl3);
      __m128i r128 = _mm_blendv_epi8(r128_01, r128_23, _mm_slli_epi32(idx128, 2));
      r128 =  _mm_blendv_epi8(r128, _mm_set1_epi64(a_.m64), idx128);
      r_.m64 = _mm_movepi64_pi64(r128);
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = (idx_.values[i] < 64) ? t_[idx_.values[i] / 16].values[idx_.values[i] & 15] : a_.values[i];
      }
    #endif

    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx4_u8
  #define vqtbx4_u8(a, t, idx) simde_vqtbx4_u8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vqtbx4_s8(simde_int8x8_t a, simde_int8x16x4_t t, simde_uint8x8_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx4_s8(a, t, idx);
  #else
    simde_uint8x16x4_t t_;
    simde_memcpy(&t_, &t, sizeof(t_));
    return simde_vreinterpret_s8_u8(simde_vqtbx4_u8(simde_vreinterpret_u8_s8(a), t_, idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx4_s8
  #define vqtbx4_s8(a, t, idx) simde_vqtbx4_s8((a), (t), (idx))
#endif

#endif /* !defined(SIMDE_BUG_INTEL_857088) */

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vqtbx1q_u8(simde_uint8x16_t a, simde_uint8x16_t t, simde_uint8x16_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx1q_u8(a, t, idx);
  #elif defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    uint8x8x2_t split;
    simde_memcpy(&split, &t, sizeof(split));
    uint8x8_t lo = vtbx2_u8(vget_low_u8(a), split, vget_low_u8(idx));
    uint8x8_t hi = vtbx2_u8(vget_high_u8(a), split, vget_high_u8(idx));
    return vcombine_u8(lo, hi);
  #elif defined(SIMDE_POWER_ALTIVEC_P6_NATIVE)
    return vec_sel(a,
                   vec_perm(t, t, idx),
                   vec_cmplt(idx, vec_splats(HEDLEY_STATIC_CAST(unsigned char, 16))));
  #else
    simde_uint8x16_private
      r_,
      a_ = simde_uint8x16_to_private(a),
      t_ = simde_uint8x16_to_private(t),
      idx_ = simde_uint8x16_to_private(idx);

    #if defined(SIMDE_X86_SSE4_1_NATIVE) && defined(SIMDE_X86_MMX_NATIVE)
      idx_.m128i = _mm_or_si128(idx_.m128i, _mm_cmpgt_epi8(idx_.m128i, _mm_set1_epi8(15)));
      r_.m128i =  _mm_blendv_epi8(_mm_shuffle_epi8(t_.m128i, idx_.m128i), a_.m128i, idx_.m128i);
    #elif defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_or(wasm_i8x16_swizzle(t_.v128, idx_.v128),
                             wasm_v128_and(a_.v128, wasm_u8x16_gt(idx_.v128, wasm_i8x16_splat(15))));
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = (idx_.values[i] < 16) ? t_.values[idx_.values[i]] : a_.values[i];
      }
    #endif

    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx1q_u8
  #define vqtbx1q_u8(a, t, idx) simde_vqtbx1q_u8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vqtbx1q_s8(simde_int8x16_t a, simde_int8x16_t t, simde_uint8x16_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx1q_s8(a, t, idx);
  #else
    return simde_vreinterpretq_s8_u8(simde_vqtbx1q_u8(simde_vreinterpretq_u8_s8(a), simde_vreinterpretq_u8_s8(t), idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx1q_s8
  #define vqtbx1q_s8(a, t, idx) simde_vqtbx1q_s8((a), (t), (idx))
#endif

#if !defined(SIMDE_BUG_INTEL_857088)

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vqtbx2q_u8(simde_uint8x16_t a, simde_uint8x16x2_t t, simde_uint8x16_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx2q_u8(a, t, idx);
  #elif defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    uint8x8x4_t split;
    simde_memcpy(&split, &t, sizeof(split));
    uint8x8_t lo = vtbx4_u8(vget_low_u8(a), split, vget_low_u8(idx));
    uint8x8_t hi = vtbx4_u8(vget_high_u8(a), split, vget_high_u8(idx));
    return vcombine_u8(lo, hi);
  #elif defined(SIMDE_POWER_ALTIVEC_P6_NATIVE)
    return vec_sel(a, vec_perm(t.val[0], t.val[1], idx),
                   vec_cmplt(idx, vec_splats(HEDLEY_STATIC_CAST(unsigned char, 32))));
  #else
    simde_uint8x16_private
      r_,
      a_ = simde_uint8x16_to_private(a),
      t_[2] = { simde_uint8x16_to_private(t.val[0]), simde_uint8x16_to_private(t.val[1]) },
      idx_ = simde_uint8x16_to_private(idx);

    #if defined(SIMDE_X86_SSE4_1_NATIVE) && defined(SIMDE_X86_MMX_NATIVE)
      idx_.m128i = _mm_or_si128(idx_.m128i, _mm_cmpgt_epi8(idx_.m128i, _mm_set1_epi8(31)));
      __m128i r_0 = _mm_shuffle_epi8(t_[0].m128i, idx_.m128i);
      __m128i r_1 = _mm_shuffle_epi8(t_[1].m128i, idx_.m128i);
      __m128i r =  _mm_blendv_epi8(r_0, r_1, _mm_slli_epi32(idx_.m128i, 3));
      r_.m128i = _mm_blendv_epi8(r, a_.m128i, idx_.m128i);
    #elif defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_or(wasm_v128_or(wasm_i8x16_swizzle(t_[0].v128, idx_.v128),
                                          wasm_i8x16_swizzle(t_[1].v128, wasm_i8x16_sub(idx_.v128, wasm_i8x16_splat(16)))),
                              wasm_v128_and(a_.v128, wasm_u8x16_gt(idx_.v128, wasm_i8x16_splat(31))));
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = (idx_.values[i] < 32) ? t_[idx_.values[i] / 16].values[idx_.values[i] & 15] : a_.values[i];
      }
    #endif

    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx2q_u8
  #define vqtbx2q_u8(a, t, idx) simde_vqtbx2q_u8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vqtbx2q_s8(simde_int8x16_t a, simde_int8x16x2_t t, simde_uint8x16_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx2q_s8(a, t, idx);
  #else
    simde_uint8x16x2_t t_;
    simde_memcpy(&t_, &t, sizeof(t_));
    return simde_vreinterpretq_s8_u8(simde_vqtbx2q_u8(simde_vreinterpretq_u8_s8(a), t_, idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx2q_s8
  #define vqtbx2q_s8(a, t, idx) simde_vqtbx2q_s8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vqtbx3q_u8(simde_uint8x16_t a, simde_uint8x16x3_t t, simde_uint8x16_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx3q_u8(a, t, idx);
  #elif defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    uint8x16_t idx_hi = vsubq_u8(idx, vdupq_n_u8(32));
    uint8x8x4_t split_lo;
    uint8x8x2_t split_hi;
    simde_memcpy(&split_lo, &t.val[0], sizeof(split_lo));
    simde_memcpy(&split_hi, &t.val[2], sizeof(split_hi));
    uint8x8_t hi_lo = vtbx2_u8(vget_low_u8(a), split_hi, vget_low_u8(idx_hi));
    uint8x8_t hi_hi = vtbx2_u8(vget_high_u8(a), split_hi, vget_high_u8(idx_hi));
    uint8x8_t lo_lo = vtbx4_u8(hi_lo, split_lo, vget_low_u8(idx));
    uint8x8_t lo_hi = vtbx4_u8(hi_hi, split_lo, vget_high_u8(idx));
    return vcombine_u8(lo_lo, lo_hi);
  #elif defined(SIMDE_POWER_ALTIVEC_P6_NATIVE)
    SIMDE_POWER_ALTIVEC_VECTOR(unsigned char) r_01 = vec_perm(t.val[0], t.val[1], idx);
    SIMDE_POWER_ALTIVEC_VECTOR(unsigned char) r_2  = vec_perm(t.val[2], t.val[2], idx);
    return vec_sel(a,
                   vec_sel(r_01, r_2, vec_cmpgt(idx, vec_splats(HEDLEY_STATIC_CAST(unsigned char, 31)))),
                   vec_cmplt(idx, vec_splats(HEDLEY_STATIC_CAST(unsigned char, 48))));
  #else
    simde_uint8x16_private
      r_,
      a_ = simde_uint8x16_to_private(a),
      t_[3] = { simde_uint8x16_to_private(t.val[0]), simde_uint8x16_to_private(t.val[1]), simde_uint8x16_to_private(t.val[2]) },
      idx_ = simde_uint8x16_to_private(idx);

    #if defined(SIMDE_X86_SSE4_1_NATIVE) && defined(SIMDE_X86_MMX_NATIVE)
      idx_.m128i = _mm_or_si128(idx_.m128i, _mm_cmpgt_epi8(idx_.m128i, _mm_set1_epi8(47)));
      __m128i r_0 = _mm_shuffle_epi8(t_[0].m128i, idx_.m128i);
      __m128i r_1 = _mm_shuffle_epi8(t_[1].m128i, idx_.m128i);
      __m128i r_01 = _mm_blendv_epi8(r_0, r_1, _mm_slli_epi32(idx_.m128i, 3));
      __m128i r_2 = _mm_shuffle_epi8(t_[2].m128i, idx_.m128i);
      __m128i r = _mm_blendv_epi8(r_01, r_2, _mm_slli_epi32(idx_.m128i, 2));
      r_.m128i = _mm_blendv_epi8(r, a_.m128i, idx_.m128i);
    #elif defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_or(wasm_v128_or(wasm_i8x16_swizzle(t_[0].v128, idx_.v128),
                                          wasm_i8x16_swizzle(t_[1].v128, wasm_i8x16_sub(idx_.v128, wasm_i8x16_splat(16)))),
                             wasm_v128_or(wasm_i8x16_swizzle(t_[2].v128, wasm_i8x16_sub(idx_.v128, wasm_i8x16_splat(32))) ,
                                          wasm_v128_and(a_.v128, wasm_u8x16_gt(idx_.v128, wasm_i8x16_splat(47)))));
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = (idx_.values[i] < 48) ? t_[idx_.values[i] / 16].values[idx_.values[i] & 15] : a_.values[i];
      }
    #endif

    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx3q_u8
  #define vqtbx3q_u8(a, t, idx) simde_vqtbx3q_u8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vqtbx3q_s8(simde_int8x16_t a, simde_int8x16x3_t t, simde_uint8x16_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx3q_s8(a, t, idx);
  #else
    simde_uint8x16x3_t t_;
    simde_memcpy(&t_, &t, sizeof(t_));
    return simde_vreinterpretq_s8_u8(simde_vqtbx3q_u8(simde_vreinterpretq_u8_s8(a), t_, idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx3q_s8
  #define vqtbx3q_s8(a, t, idx) simde_vqtbx3q_s8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vqtbx4q_u8(simde_uint8x16_t a, simde_uint8x16x4_t t, simde_uint8x16_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx4q_u8(a, t, idx);
  #elif defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    uint8x16_t idx_hi = vsubq_u8(idx, vdupq_n_u8(32));
    uint8x8x4_t split_lo;
    uint8x8x4_t split_hi;
    simde_memcpy(&split_lo, &t.val[0], sizeof(split_lo));
    simde_memcpy(&split_hi, &t.val[2], sizeof(split_hi));
    uint8x8_t lo_lo = vtbx4_u8(vget_low_u8(a), split_lo, vget_low_u8(idx));
    uint8x8_t lo_hi = vtbx4_u8(vget_high_u8(a), split_lo, vget_high_u8(idx));
    uint8x8_t lo = vtbx4_u8(lo_lo, split_hi, vget_low_u8(idx_hi));
    uint8x8_t hi = vtbx4_u8(lo_hi, split_hi, vget_high_u8(idx_hi));
    return vcombine_u8(lo, hi);
  #elif defined(SIMDE_POWER_ALTIVEC_P6_NATIVE)
    SIMDE_POWER_ALTIVEC_VECTOR(unsigned char) r_01 = vec_perm(t.val[0], t.val[1], idx);
    SIMDE_POWER_ALTIVEC_VECTOR(unsigned char) r_23 = vec_perm(t.val[2], t.val[3], idx);
    return vec_sel(a,
                   vec_sel(r_01, r_23, vec_cmpgt(idx, vec_splats(HEDLEY_STATIC_CAST(unsigned char, 31)))),
                   vec_cmplt(idx, vec_splats(HEDLEY_STATIC_CAST(unsigned char, 64))));
  #else
    simde_uint8x16_private
      r_,
      a_ = simde_uint8x16_to_private(a),
      t_[4] = { simde_uint8x16_to_private(t.val[0]), simde_uint8x16_to_private(t.val[1]), simde_uint8x16_to_private(t.val[2]), simde_uint8x16_to_private(t.val[3]) },
      idx_ = simde_uint8x16_to_private(idx);

    #if defined(SIMDE_X86_SSE4_1_NATIVE) && defined(SIMDE_X86_MMX_NATIVE)
      idx_.m128i = _mm_or_si128(idx_.m128i, _mm_cmpgt_epi8(idx_.m128i, _mm_set1_epi8(63)));
      __m128i idx_shl3 = _mm_slli_epi32(idx_.m128i, 3);
      __m128i r_0 = _mm_shuffle_epi8(t_[0].m128i, idx_.m128i);
      __m128i r_1 = _mm_shuffle_epi8(t_[1].m128i, idx_.m128i);
      __m128i r_01 = _mm_blendv_epi8(r_0, r_1, idx_shl3);
      __m128i r_2 = _mm_shuffle_epi8(t_[2].m128i, idx_.m128i);
      __m128i r_3 = _mm_shuffle_epi8(t_[3].m128i, idx_.m128i);
      __m128i r_23 = _mm_blendv_epi8(r_2, r_3, idx_shl3);
      __m128i r = _mm_blendv_epi8(r_01, r_23, _mm_slli_epi32(idx_.m128i, 2));
      r_.m128i = _mm_blendv_epi8(r, a_.m128i, idx_.m128i);
    #elif defined(SIMDE_WASM_SIMD128_NATIVE)
      r_.v128 = wasm_v128_or(wasm_v128_or(wasm_v128_or(wasm_i8x16_swizzle(t_[0].v128, idx_.v128),
                                                       wasm_i8x16_swizzle(t_[1].v128, wasm_i8x16_sub(idx_.v128, wasm_i8x16_splat(16)))),
                                          wasm_v128_or(wasm_i8x16_swizzle(t_[2].v128, wasm_i8x16_sub(idx_.v128, wasm_i8x16_splat(32))),
                                                       wasm_i8x16_swizzle(t_[3].v128, wasm_i8x16_sub(idx_.v128, wasm_i8x16_splat(48))))),
                             wasm_v128_and(a_.v128, wasm_u8x16_gt(idx_.v128, wasm_i8x16_splat(63))));
    #else
      SIMDE_VECTORIZE
      for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
        r_.values[i] = (idx_.values[i] < 64) ? t_[idx_.values[i] / 16].values[idx_.values[i] & 15] : a_.values[i];
      }
    #endif

    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx4q_u8
  #define vqtbx4q_u8(a, t, idx) simde_vqtbx4q_u8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vqtbx4q_s8(simde_int8x16_t a, simde_int8x16x4_t t, simde_uint8x16_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx4q_s8(a, t, idx);
  #else
    simde_uint8x16x4_t t_;
    simde_memcpy(&t_, &t, sizeof(t_));
    return simde_vreinterpretq_s8_u8(simde_vqtbx4q_u8(simde_vreinterpretq_u8_s8(a), t_, idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx4q_s8
  #define vqtbx4q_s8(a, t, idx) simde_vqtbx4q_s8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vqtbx1_p8(simde_poly8x8_t a, simde_poly8x16_t t, simde_uint8x8_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx1_p8(a, t, idx);
  #else
    return simde_vreinterpret_p8_u8(simde_vqtbx1_u8(simde_vreinterpret_u8_p8(a), simde_vreinterpretq_u8_p8(t), idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx1_p8
  #define vqtbx1_p8(a, t, idx) simde_vqtbx1_p8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vqtbx1q_p8(simde_poly8x16_t a, simde_poly8x16_t t, simde_uint8x16_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx1q_p8(a, t, idx);
  #else
    return simde_vreinterpretq_p8_u8(simde_vqtbx1q_u8(simde_vreinterpretq_u8_p8(a), simde_vreinterpretq_u8_p8(t), idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx1q_p8
  #define vqtbx1q_p8(a, t, idx) simde_vqtbx1q_p8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vqtbx2_p8(simde_poly8x8_t a, simde_poly8x16x2_t t, simde_uint8x8_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx2_p8(a, t, idx);
  #else
    simde_uint8x16x2_t t_;
    simde_memcpy(&t_, &t, sizeof(t_));
    return simde_vreinterpret_p8_u8(simde_vqtbx2_u8(simde_vreinterpret_u8_p8(a), t_, idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx2_p8
  #define vqtbx2_p8(a, t, idx) simde_vqtbx2_p8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vqtbx2q_p8(simde_poly8x16_t a, simde_poly8x16x2_t t, simde_uint8x16_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx2q_p8(a, t, idx);
  #else
    simde_uint8x16x2_t t_;
    simde_memcpy(&t_, &t, sizeof(t_));
    return simde_vreinterpretq_p8_u8(simde_vqtbx2q_u8(simde_vreinterpretq_u8_p8(a), t_, idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx2q_p8
  #define vqtbx2q_p8(a, t, idx) simde_vqtbx2q_p8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vqtbx3_p8(simde_poly8x8_t a, simde_poly8x16x3_t t, simde_uint8x8_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx3_p8(a, t, idx);
  #else
    simde_uint8x16x3_t t_;
    simde_memcpy(&t_, &t, sizeof(t_));
    return simde_vreinterpret_p8_u8(simde_vqtbx3_u8(simde_vreinterpret_u8_p8(a), t_, idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx3_p8
  #define vqtbx3_p8(a, t, idx) simde_vqtbx3_p8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vqtbx3q_p8(simde_poly8x16_t a, simde_poly8x16x3_t t, simde_uint8x16_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx3q_p8(a, t, idx);
  #else
    simde_uint8x16x3_t t_;
    simde_memcpy(&t_, &t, sizeof(t_));
    return simde_vreinterpretq_p8_u8(simde_vqtbx3q_u8(simde_vreinterpretq_u8_p8(a), t_, idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx3q_p8
  #define vqtbx3q_p8(a, t, idx) simde_vqtbx3q_p8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vqtbx4_p8(simde_poly8x8_t a, simde_poly8x16x4_t t, simde_uint8x8_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx4_p8(a, t, idx);
  #else
    simde_uint8x16x4_t t_;
    simde_memcpy(&t_, &t, sizeof(t_));
    return simde_vreinterpret_p8_u8(simde_vqtbx4_u8(simde_vreinterpret_u8_p8(a), t_, idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx4_p8
  #define vqtbx4_p8(a, t, idx) simde_vqtbx4_p8((a), (t), (idx))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vqtbx4q_p8(simde_poly8x16_t a, simde_poly8x16x4_t t, simde_uint8x16_t idx) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vqtbx4q_p8(a, t, idx);
  #else
    simde_uint8x16x4_t t_;
    simde_memcpy(&t_, &t, sizeof(t_));
    return simde_vreinterpretq_p8_u8(simde_vqtbx4q_u8(simde_vreinterpretq_u8_p8(a), t_, idx));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vqtbx4q_p8
  #define vqtbx4q_p8(a, t, idx) simde_vqtbx4q_p8((a), (t), (idx))
#endif

#endif /* !defined(SIMDE_BUG_INTEL_857088) */

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_QTBX_H) */
