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
 *   2020      Sean Maher <seanptmaher@gmail.com> (Copyright owned by Google, LLC)
 *   2023      Yi-Yen Chung <eric681@andestech.com> (Copyright owned by Andes Technology)
 */


#if !defined(SIMDE_ARM_NEON_REINTERPRET_H)
#define SIMDE_ARM_NEON_REINTERPRET_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vreinterpret_s8_s16(simde_int16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s8_s16(a);
  #else
    simde_int8x8_private r_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s8_s16
  #define vreinterpret_s8_s16 simde_vreinterpret_s8_s16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vreinterpret_s8_s32(simde_int32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s8_s32(a);
  #else
    simde_int8x8_private r_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s8_s32
  #define vreinterpret_s8_s32 simde_vreinterpret_s8_s32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vreinterpret_s8_s64(simde_int64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s8_s64(a);
  #else
    simde_int8x8_private r_;
    simde_int64x1_private a_ = simde_int64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s8_s64
  #define vreinterpret_s8_s64 simde_vreinterpret_s8_s64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vreinterpret_s8_u8(simde_uint8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s8_u8(a);
  #else
    simde_int8x8_private r_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s8_u8
  #define vreinterpret_s8_u8 simde_vreinterpret_s8_u8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vreinterpret_s8_u16(simde_uint16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s8_u16(a);
  #else
    simde_int8x8_private r_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s8_u16
  #define vreinterpret_s8_u16 simde_vreinterpret_s8_u16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vreinterpret_s8_u32(simde_uint32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s8_u32(a);
  #else
    simde_int8x8_private r_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s8_u32
  #define vreinterpret_s8_u32 simde_vreinterpret_s8_u32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vreinterpret_s8_u64(simde_uint64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s8_u64(a);
  #else
    simde_int8x8_private r_;
    simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s8_u64
  #define vreinterpret_s8_u64 simde_vreinterpret_s8_u64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vreinterpret_s8_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s8_f32(a);
  #else
    simde_int8x8_private r_;
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s8_f32
  #define vreinterpret_s8_f32 simde_vreinterpret_s8_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vreinterpret_s8_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_s8_f64(a);
  #else
    simde_int8x8_private r_;
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s8_f64
  #define vreinterpret_s8_f64 simde_vreinterpret_s8_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s8_s16(a);
  #else
    simde_int8x16_private r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_s16
  #define vreinterpretq_s8_s16(a) simde_vreinterpretq_s8_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s8_s32(a);
  #else
    simde_int8x16_private r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_s32
  #define vreinterpretq_s8_s32(a) simde_vreinterpretq_s8_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s8_s64(a);
  #else
    simde_int8x16_private r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_s64
  #define vreinterpretq_s8_s64(a) simde_vreinterpretq_s8_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s8_u8(a);
  #else
    simde_int8x16_private r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_u8
  #define vreinterpretq_s8_u8(a) simde_vreinterpretq_s8_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s8_u16(a);
  #else
    simde_int8x16_private r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_u16
  #define vreinterpretq_s8_u16(a) simde_vreinterpretq_s8_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s8_u32(a);
  #else
    simde_int8x16_private r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_u32
  #define vreinterpretq_s8_u32(a) simde_vreinterpretq_s8_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s8_u64(a);
  #else
    simde_int8x16_private r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_u64
  #define vreinterpretq_s8_u64(a) simde_vreinterpretq_s8_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s8_f32(a);
  #else
    simde_int8x16_private r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_f32
  #define vreinterpretq_s8_f32(a) simde_vreinterpretq_s8_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_s8_f64(a);
  #else
    simde_int8x16_private r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_f64
  #define vreinterpretq_s8_f64(a) simde_vreinterpretq_s8_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vreinterpret_s16_s8(simde_int8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s16_s8(a);
  #else
    simde_int16x4_private r_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s16_s8
  #define vreinterpret_s16_s8 simde_vreinterpret_s16_s8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vreinterpret_s16_s32(simde_int32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s16_s32(a);
  #else
    simde_int16x4_private r_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s16_s32
  #define vreinterpret_s16_s32 simde_vreinterpret_s16_s32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vreinterpret_s16_s64(simde_int64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s16_s64(a);
  #else
    simde_int16x4_private r_;
    simde_int64x1_private a_ = simde_int64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s16_s64
  #define vreinterpret_s16_s64 simde_vreinterpret_s16_s64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vreinterpret_s16_u8(simde_uint8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s16_u8(a);
  #else
    simde_int16x4_private r_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s16_u8
  #define vreinterpret_s16_u8 simde_vreinterpret_s16_u8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vreinterpret_s16_u16(simde_uint16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s16_u16(a);
  #else
    simde_int16x4_private r_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s16_u16
  #define vreinterpret_s16_u16 simde_vreinterpret_s16_u16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vreinterpret_s16_u32(simde_uint32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s16_u32(a);
  #else
    simde_int16x4_private r_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s16_u32
  #define vreinterpret_s16_u32 simde_vreinterpret_s16_u32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vreinterpret_s16_u64(simde_uint64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s16_u64(a);
  #else
    simde_int16x4_private r_;
    simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s16_u64
  #define vreinterpret_s16_u64 simde_vreinterpret_s16_u64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vreinterpret_s16_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s16_f32(a);
  #else
    simde_int16x4_private r_;
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s16_f32
  #define vreinterpret_s16_f32 simde_vreinterpret_s16_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vreinterpret_s16_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_s16_f64(a);
  #else
    simde_int16x4_private r_;
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s16_f64
  #define vreinterpret_s16_f64 simde_vreinterpret_s16_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s16_s8(a);
  #else
    simde_int16x8_private r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_s8
  #define vreinterpretq_s16_s8(a) simde_vreinterpretq_s16_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s16_s32(a);
  #else
    simde_int16x8_private r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_s32
  #define vreinterpretq_s16_s32(a) simde_vreinterpretq_s16_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s16_s64(a);
  #else
    simde_int16x8_private r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_s64
  #define vreinterpretq_s16_s64(a) simde_vreinterpretq_s16_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s16_u8(a);
  #else
    simde_int16x8_private r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_u8
  #define vreinterpretq_s16_u8(a) simde_vreinterpretq_s16_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s16_u16(a);
  #else
    simde_int16x8_private r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_u16
  #define vreinterpretq_s16_u16(a) simde_vreinterpretq_s16_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s16_u32(a);
  #else
    simde_int16x8_private r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_u32
  #define vreinterpretq_s16_u32(a) simde_vreinterpretq_s16_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s16_u64(a);
  #else
    simde_int16x8_private r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_u64
  #define vreinterpretq_s16_u64(a) simde_vreinterpretq_s16_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s16_f32(a);
  #else
    simde_int16x8_private r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_f32
  #define vreinterpretq_s16_f32(a) simde_vreinterpretq_s16_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_s16_f64(a);
  #else
    simde_int16x8_private r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_f64
  #define vreinterpretq_s16_f64(a) simde_vreinterpretq_s16_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vreinterpret_s32_s8(simde_int8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s32_s8(a);
  #else
    simde_int32x2_private r_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s32_s8
  #define vreinterpret_s32_s8 simde_vreinterpret_s32_s8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vreinterpret_s32_s16(simde_int16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s32_s16(a);
  #else
    simde_int32x2_private r_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s32_s16
  #define vreinterpret_s32_s16 simde_vreinterpret_s32_s16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vreinterpret_s32_s64(simde_int64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s32_s64(a);
  #else
    simde_int32x2_private r_;
    simde_int64x1_private a_ = simde_int64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s32_s64
  #define vreinterpret_s32_s64 simde_vreinterpret_s32_s64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vreinterpret_s32_u8(simde_uint8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s32_u8(a);
  #else
    simde_int32x2_private r_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s32_u8
  #define vreinterpret_s32_u8 simde_vreinterpret_s32_u8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vreinterpret_s32_u16(simde_uint16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s32_u16(a);
  #else
    simde_int32x2_private r_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s32_u16
  #define vreinterpret_s32_u16 simde_vreinterpret_s32_u16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vreinterpret_s32_u32(simde_uint32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s32_u32(a);
  #else
    simde_int32x2_private r_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s32_u32
  #define vreinterpret_s32_u32 simde_vreinterpret_s32_u32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vreinterpret_s32_u64(simde_uint64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s32_u64(a);
  #else
    simde_int32x2_private r_;
    simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s32_u64
  #define vreinterpret_s32_u64 simde_vreinterpret_s32_u64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vreinterpret_s32_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s32_f32(a);
  #else
    simde_int32x2_private r_;
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s32_f32
  #define vreinterpret_s32_f32 simde_vreinterpret_s32_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vreinterpret_s32_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_s32_f64(a);
  #else
    simde_int32x2_private r_;
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s32_f64
  #define vreinterpret_s32_f64 simde_vreinterpret_s32_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s32_s8(a);
  #else
    simde_int32x4_private r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_s8
  #define vreinterpretq_s32_s8(a) simde_vreinterpretq_s32_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s32_s16(a);
  #else
    simde_int32x4_private r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_s16
  #define vreinterpretq_s32_s16(a) simde_vreinterpretq_s32_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s32_s64(a);
  #else
    simde_int32x4_private r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_s64
  #define vreinterpretq_s32_s64(a) simde_vreinterpretq_s32_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s32_u8(a);
  #else
    simde_int32x4_private r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_u8
  #define vreinterpretq_s32_u8(a) simde_vreinterpretq_s32_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s32_u16(a);
  #else
    simde_int32x4_private r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_u16
  #define vreinterpretq_s32_u16(a) simde_vreinterpretq_s32_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s32_u32(a);
  #else
    simde_int32x4_private r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_u32
  #define vreinterpretq_s32_u32(a) simde_vreinterpretq_s32_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s32_u64(a);
  #else
    simde_int32x4_private r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_u64
  #define vreinterpretq_s32_u64(a) simde_vreinterpretq_s32_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s32_f32(a);
  #else
    simde_int32x4_private r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_f32
  #define vreinterpretq_s32_f32(a) simde_vreinterpretq_s32_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_s32_f64(a);
  #else
    simde_int32x4_private r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_f64
  #define vreinterpretq_s32_f64(a) simde_vreinterpretq_s32_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vreinterpret_s64_s8(simde_int8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s64_s8(a);
  #else
    simde_int64x1_private r_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s64_s8
  #define vreinterpret_s64_s8 simde_vreinterpret_s64_s8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vreinterpret_s64_s16(simde_int16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s64_s16(a);
  #else
    simde_int64x1_private r_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s64_s16
  #define vreinterpret_s64_s16 simde_vreinterpret_s64_s16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vreinterpret_s64_s32(simde_int32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s64_s32(a);
  #else
    simde_int64x1_private r_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s64_s32
  #define vreinterpret_s64_s32 simde_vreinterpret_s64_s32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vreinterpret_s64_u8(simde_uint8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s64_u8(a);
  #else
    simde_int64x1_private r_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s64_u8
  #define vreinterpret_s64_u8 simde_vreinterpret_s64_u8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vreinterpret_s64_u16(simde_uint16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s64_u16(a);
  #else
    simde_int64x1_private r_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s64_u16
  #define vreinterpret_s64_u16 simde_vreinterpret_s64_u16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vreinterpret_s64_u32(simde_uint32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s64_u32(a);
  #else
    simde_int64x1_private r_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s64_u32
  #define vreinterpret_s64_u32 simde_vreinterpret_s64_u32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vreinterpret_s64_u64(simde_uint64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s64_u64(a);
  #else
    simde_int64x1_private r_;
    simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s64_u64
  #define vreinterpret_s64_u64 simde_vreinterpret_s64_u64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vreinterpret_s64_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s64_f32(a);
  #else
    simde_int64x1_private r_;
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s64_f32
  #define vreinterpret_s64_f32 simde_vreinterpret_s64_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vreinterpret_s64_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_s64_f64(a);
  #else
    simde_int64x1_private r_;
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s64_f64
  #define vreinterpret_s64_f64 simde_vreinterpret_s64_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s64_s8(a);
  #else
    simde_int64x2_private r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_s8
  #define vreinterpretq_s64_s8(a) simde_vreinterpretq_s64_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s64_s16(a);
  #else
    simde_int64x2_private r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_s16
  #define vreinterpretq_s64_s16(a) simde_vreinterpretq_s64_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s64_s32(a);
  #else
    simde_int64x2_private r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_s32
  #define vreinterpretq_s64_s32(a) simde_vreinterpretq_s64_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s64_u8(a);
  #else
    simde_int64x2_private r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_u8
  #define vreinterpretq_s64_u8(a) simde_vreinterpretq_s64_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s64_u16(a);
  #else
    simde_int64x2_private r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_u16
  #define vreinterpretq_s64_u16(a) simde_vreinterpretq_s64_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s64_u32(a);
  #else
    simde_int64x2_private r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_u32
  #define vreinterpretq_s64_u32(a) simde_vreinterpretq_s64_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s64_u64(a);
  #else
    simde_int64x2_private r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_u64
  #define vreinterpretq_s64_u64(a) simde_vreinterpretq_s64_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s64_f32(a);
  #else
    simde_int64x2_private r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_f32
  #define vreinterpretq_s64_f32(a) simde_vreinterpretq_s64_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_s64_f64(a);
  #else
    simde_int64x2_private r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_f64
  #define vreinterpretq_s64_f64(a) simde_vreinterpretq_s64_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vreinterpret_u8_s8(simde_int8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u8_s8(a);
  #else
    simde_uint8x8_private r_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u8_s8
  #define vreinterpret_u8_s8 simde_vreinterpret_u8_s8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vreinterpret_u8_s16(simde_int16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u8_s16(a);
  #else
    simde_uint8x8_private r_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u8_s16
  #define vreinterpret_u8_s16 simde_vreinterpret_u8_s16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vreinterpret_u8_s32(simde_int32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u8_s32(a);
  #else
    simde_uint8x8_private r_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u8_s32
  #define vreinterpret_u8_s32 simde_vreinterpret_u8_s32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vreinterpret_u8_s64(simde_int64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u8_s64(a);
  #else
    simde_uint8x8_private r_;
    simde_int64x1_private a_ = simde_int64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u8_s64
  #define vreinterpret_u8_s64 simde_vreinterpret_u8_s64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vreinterpret_u8_u16(simde_uint16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u8_u16(a);
  #else
    simde_uint8x8_private r_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u8_u16
  #define vreinterpret_u8_u16 simde_vreinterpret_u8_u16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vreinterpret_u8_u32(simde_uint32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u8_u32(a);
  #else
    simde_uint8x8_private r_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u8_u32
  #define vreinterpret_u8_u32 simde_vreinterpret_u8_u32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vreinterpret_u8_u64(simde_uint64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u8_u64(a);
  #else
    simde_uint8x8_private r_;
    simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u8_u64
  #define vreinterpret_u8_u64 simde_vreinterpret_u8_u64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vreinterpret_u8_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u8_f32(a);
  #else
    simde_uint8x8_private r_;
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u8_f32
  #define vreinterpret_u8_f32 simde_vreinterpret_u8_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vreinterpret_u8_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_u8_f64(a);
  #else
    simde_uint8x8_private r_;
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u8_f64
  #define vreinterpret_u8_f64 simde_vreinterpret_u8_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u8_s8(a);
  #else
    simde_uint8x16_private r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_s8
  #define vreinterpretq_u8_s8(a) simde_vreinterpretq_u8_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u8_s16(a);
  #else
    simde_uint8x16_private r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_s16
  #define vreinterpretq_u8_s16(a) simde_vreinterpretq_u8_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u8_s32(a);
  #else
    simde_uint8x16_private r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_s32
  #define vreinterpretq_u8_s32(a) simde_vreinterpretq_u8_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u8_s64(a);
  #else
    simde_uint8x16_private r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_s64
  #define vreinterpretq_u8_s64(a) simde_vreinterpretq_u8_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u8_u16(a);
  #else
    simde_uint8x16_private r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_u16
  #define vreinterpretq_u8_u16(a) simde_vreinterpretq_u8_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u8_u32(a);
  #else
    simde_uint8x16_private r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_u32
  #define vreinterpretq_u8_u32(a) simde_vreinterpretq_u8_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u8_u64(a);
  #else
    simde_uint8x16_private r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_u64
  #define vreinterpretq_u8_u64(a) simde_vreinterpretq_u8_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u8_f32(a);
  #else
    simde_uint8x16_private r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_f32
  #define vreinterpretq_u8_f32(a) simde_vreinterpretq_u8_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_u8_f64(a);
  #else
    simde_uint8x16_private r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_f64
  #define vreinterpretq_u8_f64(a) simde_vreinterpretq_u8_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vreinterpret_u16_s8(simde_int8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u16_s8(a);
  #else
    simde_uint16x4_private r_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u16_s8
  #define vreinterpret_u16_s8 simde_vreinterpret_u16_s8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vreinterpret_u16_s16(simde_int16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u16_s16(a);
  #else
    simde_uint16x4_private r_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u16_s16
  #define vreinterpret_u16_s16 simde_vreinterpret_u16_s16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vreinterpret_u16_s32(simde_int32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u16_s32(a);
  #else
    simde_uint16x4_private r_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u16_s32
  #define vreinterpret_u16_s32 simde_vreinterpret_u16_s32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vreinterpret_u16_s64(simde_int64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u16_s64(a);
  #else
    simde_uint16x4_private r_;
    simde_int64x1_private a_ = simde_int64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u16_s64
  #define vreinterpret_u16_s64 simde_vreinterpret_u16_s64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vreinterpret_u16_u8(simde_uint8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u16_u8(a);
  #else
    simde_uint16x4_private r_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u16_u8
  #define vreinterpret_u16_u8 simde_vreinterpret_u16_u8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vreinterpret_u16_u32(simde_uint32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u16_u32(a);
  #else
    simde_uint16x4_private r_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u16_u32
  #define vreinterpret_u16_u32 simde_vreinterpret_u16_u32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vreinterpret_u16_u64(simde_uint64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u16_u64(a);
  #else
    simde_uint16x4_private r_;
    simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u16_u64
  #define vreinterpret_u16_u64 simde_vreinterpret_u16_u64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vreinterpret_u16_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_u16_f16(a);
  #else
    simde_uint16x4_private r_;
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u16_f16
  #define vreinterpret_u16_f16(a) simde_vreinterpret_u16_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vreinterpret_u16_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u16_f32(a);
  #else
    simde_uint16x4_private r_;
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u16_f32
  #define vreinterpret_u16_f32 simde_vreinterpret_u16_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vreinterpret_u16_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_u16_f64(a);
  #else
    simde_uint16x4_private r_;
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u16_f64
  #define vreinterpret_u16_f64 simde_vreinterpret_u16_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u16_s8(a);
  #else
    simde_uint16x8_private r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_s8
  #define vreinterpretq_u16_s8(a) simde_vreinterpretq_u16_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u16_s16(a);
  #else
    simde_uint16x8_private r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_s16
  #define vreinterpretq_u16_s16(a) simde_vreinterpretq_u16_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u16_s32(a);
  #else
    simde_uint16x8_private r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_s32
  #define vreinterpretq_u16_s32(a) simde_vreinterpretq_u16_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u16_s64(a);
  #else
    simde_uint16x8_private r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_s64
  #define vreinterpretq_u16_s64(a) simde_vreinterpretq_u16_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u16_u8(a);
  #else
    simde_uint16x8_private r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_u8
  #define vreinterpretq_u16_u8(a) simde_vreinterpretq_u16_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u16_u32(a);
  #else
    simde_uint16x8_private r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_u32
  #define vreinterpretq_u16_u32(a) simde_vreinterpretq_u16_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u16_u64(a);
  #else
    simde_uint16x8_private r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_u64
  #define vreinterpretq_u16_u64(a) simde_vreinterpretq_u16_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u16_f32(a);
  #else
    simde_uint16x8_private r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_f32
  #define vreinterpretq_u16_f32(a) simde_vreinterpretq_u16_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_u16_f64(a);
  #else
    simde_uint16x8_private r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_f64
  #define vreinterpretq_u16_f64(a) simde_vreinterpretq_u16_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vreinterpret_u32_s8(simde_int8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u32_s8(a);
  #else
    simde_uint32x2_private r_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u32_s8
  #define vreinterpret_u32_s8 simde_vreinterpret_u32_s8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vreinterpret_u32_s16(simde_int16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u32_s16(a);
  #else
    simde_uint32x2_private r_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u32_s16
  #define vreinterpret_u32_s16 simde_vreinterpret_u32_s16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vreinterpret_u32_s32(simde_int32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u32_s32(a);
  #else
    simde_uint32x2_private r_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u32_s32
  #define vreinterpret_u32_s32 simde_vreinterpret_u32_s32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vreinterpret_u32_s64(simde_int64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u32_s64(a);
  #else
    simde_uint32x2_private r_;
    simde_int64x1_private a_ = simde_int64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u32_s64
  #define vreinterpret_u32_s64 simde_vreinterpret_u32_s64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vreinterpret_u32_u8(simde_uint8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u32_u8(a);
  #else
    simde_uint32x2_private r_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u32_u8
  #define vreinterpret_u32_u8 simde_vreinterpret_u32_u8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vreinterpret_u32_u16(simde_uint16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u32_u16(a);
  #else
    simde_uint32x2_private r_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u32_u16
  #define vreinterpret_u32_u16 simde_vreinterpret_u32_u16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vreinterpret_u32_u64(simde_uint64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u32_u64(a);
  #else
    simde_uint32x2_private r_;
    simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u32_u64
  #define vreinterpret_u32_u64 simde_vreinterpret_u32_u64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vreinterpret_u32_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u32_f32(a);
  #else
    simde_uint32x2_private r_;
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u32_f32
  #define vreinterpret_u32_f32 simde_vreinterpret_u32_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vreinterpret_u32_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_u32_f64(a);
  #else
    simde_uint32x2_private r_;
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u32_f64
  #define vreinterpret_u32_f64 simde_vreinterpret_u32_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u32_s8(a);
  #else
    simde_uint32x4_private r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_s8
  #define vreinterpretq_u32_s8(a) simde_vreinterpretq_u32_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u32_s16(a);
  #else
    simde_uint32x4_private r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_s16
  #define vreinterpretq_u32_s16(a) simde_vreinterpretq_u32_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u32_s32(a);
  #else
    simde_uint32x4_private r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_s32
  #define vreinterpretq_u32_s32(a) simde_vreinterpretq_u32_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u32_s64(a);
  #else
    simde_uint32x4_private r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_s64
  #define vreinterpretq_u32_s64(a) simde_vreinterpretq_u32_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u32_u8(a);
  #else
    simde_uint32x4_private r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_u8
  #define vreinterpretq_u32_u8(a) simde_vreinterpretq_u32_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u32_u16(a);
  #else
    simde_uint32x4_private r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_u16
  #define vreinterpretq_u32_u16(a) simde_vreinterpretq_u32_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u32_u64(a);
  #else
    simde_uint32x4_private r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_u64
  #define vreinterpretq_u32_u64(a) simde_vreinterpretq_u32_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_u16_f16(a);
  #else
    simde_uint16x8_private r_;
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_f16
  #define vreinterpretq_u16_f16(a) simde_vreinterpretq_u16_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u32_f32(a);
  #else
    simde_uint32x4_private r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_f32
  #define vreinterpretq_u32_f32(a) simde_vreinterpretq_u32_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_u32_f64(a);
  #else
    simde_uint32x4_private r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_f64
  #define vreinterpretq_u32_f64(a) simde_vreinterpretq_u32_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vreinterpret_u64_s8(simde_int8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u64_s8(a);
  #else
    simde_uint64x1_private r_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u64_s8
  #define vreinterpret_u64_s8 simde_vreinterpret_u64_s8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vreinterpret_u64_s16(simde_int16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u64_s16(a);
  #else
    simde_uint64x1_private r_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u64_s16
  #define vreinterpret_u64_s16 simde_vreinterpret_u64_s16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vreinterpret_u64_s32(simde_int32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u64_s32(a);
  #else
    simde_uint64x1_private r_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u64_s32
  #define vreinterpret_u64_s32 simde_vreinterpret_u64_s32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vreinterpret_u64_s64(simde_int64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u64_s64(a);
  #else
    simde_uint64x1_private r_;
    simde_int64x1_private a_ = simde_int64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u64_s64
  #define vreinterpret_u64_s64 simde_vreinterpret_u64_s64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vreinterpret_u64_u8(simde_uint8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u64_u8(a);
  #else
    simde_uint64x1_private r_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u64_u8
  #define vreinterpret_u64_u8 simde_vreinterpret_u64_u8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vreinterpret_u64_u16(simde_uint16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u64_u16(a);
  #else
    simde_uint64x1_private r_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u64_u16
  #define vreinterpret_u64_u16 simde_vreinterpret_u64_u16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vreinterpret_u64_u32(simde_uint32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u64_u32(a);
  #else
    simde_uint64x1_private r_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u64_u32
  #define vreinterpret_u64_u32 simde_vreinterpret_u64_u32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vreinterpret_u64_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_u64_f16(a);
  #else
    simde_uint64x1_private r_;
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u64_f16
  #define vreinterpret_u64_f16 simde_vreinterpret_u64_f16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vreinterpret_u64_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u64_f32(a);
  #else
    simde_uint64x1_private r_;
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u64_f32
  #define vreinterpret_u64_f32 simde_vreinterpret_u64_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vreinterpret_u64_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_u64_f64(a);
  #else
    simde_uint64x1_private r_;
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u64_f64
  #define vreinterpret_u64_f64 simde_vreinterpret_u64_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u64_s8(a);
  #else
    simde_uint64x2_private r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_s8
  #define vreinterpretq_u64_s8(a) simde_vreinterpretq_u64_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u64_s16(a);
  #else
    simde_uint64x2_private r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_s16
  #define vreinterpretq_u64_s16(a) simde_vreinterpretq_u64_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u64_s32(a);
  #else
    simde_uint64x2_private r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_s32
  #define vreinterpretq_u64_s32(a) simde_vreinterpretq_u64_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u64_s64(a);
  #else
    simde_uint64x2_private r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_s64
  #define vreinterpretq_u64_s64(a) simde_vreinterpretq_u64_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u64_u8(a);
  #else
    simde_uint64x2_private r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_u8
  #define vreinterpretq_u64_u8(a) simde_vreinterpretq_u64_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u64_u16(a);
  #else
    simde_uint64x2_private r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_u16
  #define vreinterpretq_u64_u16(a) simde_vreinterpretq_u64_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u64_u32(a);
  #else
    simde_uint64x2_private r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_u32
  #define vreinterpretq_u64_u32(a) simde_vreinterpretq_u64_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u64_f32(a);
  #else
    simde_uint64x2_private r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_f32
  #define vreinterpretq_u64_f32(a) simde_vreinterpretq_u64_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_u64_f64(a);
  #else
    simde_uint64x2_private r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_f64
  #define vreinterpretq_u64_f64(a) simde_vreinterpretq_u64_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vreinterpret_f32_s8(simde_int8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_f32_s8(a);
  #else
    simde_float32x2_private r_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f32_s8
  #define vreinterpret_f32_s8 simde_vreinterpret_f32_s8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vreinterpret_f32_s16(simde_int16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_f32_s16(a);
  #else
    simde_float32x2_private r_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f32_s16
  #define vreinterpret_f32_s16 simde_vreinterpret_f32_s16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vreinterpret_f32_s32(simde_int32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_f32_s32(a);
  #else
    simde_float32x2_private r_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f32_s32
  #define vreinterpret_f32_s32 simde_vreinterpret_f32_s32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vreinterpret_f32_s64(simde_int64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_f32_s64(a);
  #else
    simde_float32x2_private r_;
    simde_int64x1_private a_ = simde_int64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f32_s64
  #define vreinterpret_f32_s64 simde_vreinterpret_f32_s64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vreinterpret_f32_u8(simde_uint8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_f32_u8(a);
  #else
    simde_float32x2_private r_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f32_u8
  #define vreinterpret_f32_u8 simde_vreinterpret_f32_u8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vreinterpret_f32_u16(simde_uint16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_f32_u16(a);
  #else
    simde_float32x2_private r_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f32_u16
  #define vreinterpret_f32_u16 simde_vreinterpret_f32_u16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vreinterpret_f16_u16(simde_uint16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f16_u16(a);
  #else
    simde_float16x4_private r_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f16_u16
  #define vreinterpret_f16_u16(a) simde_vreinterpret_f16_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vreinterpret_f32_u32(simde_uint32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_f32_u32(a);
  #else
    simde_float32x2_private r_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f32_u32
  #define vreinterpret_f32_u32 simde_vreinterpret_f32_u32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vreinterpret_f32_u64(simde_uint64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_f32_u64(a);
  #else
    simde_float32x2_private r_;
    simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f32_u64
  #define vreinterpret_f32_u64 simde_vreinterpret_f32_u64
#endif


SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vreinterpret_f32_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_f32_f64(a);
  #else
    simde_float32x2_private r_;
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f32_f64
  #define vreinterpret_f32_f64 simde_vreinterpret_f32_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vreinterpretq_f32_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_f32_s8(a);
  #else
    simde_float32x4_private r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f32_s8
  #define vreinterpretq_f32_s8(a) simde_vreinterpretq_f32_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vreinterpretq_f32_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_f32_s16(a);
  #else
    simde_float32x4_private r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f32_s16
  #define vreinterpretq_f32_s16(a) simde_vreinterpretq_f32_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vreinterpretq_f32_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_f32_s32(a);
  #else
    simde_float32x4_private r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f32_s32
  #define vreinterpretq_f32_s32(a) simde_vreinterpretq_f32_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vreinterpretq_f32_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_f32_s64(a);
  #else
    simde_float32x4_private r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f32_s64
  #define vreinterpretq_f32_s64(a) simde_vreinterpretq_f32_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vreinterpretq_f32_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_f32_u8(a);
  #else
    simde_float32x4_private r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f32_u8
  #define vreinterpretq_f32_u8(a) simde_vreinterpretq_f32_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vreinterpretq_f32_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_f32_u16(a);
  #else
    simde_float32x4_private r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f32_u16
  #define vreinterpretq_f32_u16(a) simde_vreinterpretq_f32_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vreinterpretq_f16_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f16_u16(a);
  #else
    simde_float16x8_private r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f16_u16
  #define vreinterpretq_f16_u16(a) simde_vreinterpretq_f16_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vreinterpretq_f32_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_f32_u32(a);
  #else
    simde_float32x4_private r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f32_u32
  #define vreinterpretq_f32_u32(a) simde_vreinterpretq_f32_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vreinterpretq_f32_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_f32_u64(a);
  #else
    simde_float32x4_private r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f32_u64
  #define vreinterpretq_f32_u64(a) simde_vreinterpretq_f32_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vreinterpretq_f32_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_f32_f64(a);
  #else
    simde_float32x4_private r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f32_f64
  #define vreinterpretq_f32_f64(a) simde_vreinterpretq_f32_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vreinterpret_f64_s8(simde_int8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_f64_s8(a);
  #else
    simde_float64x1_private r_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f64_s8
  #define vreinterpret_f64_s8 simde_vreinterpret_f64_s8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vreinterpret_f64_s16(simde_int16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_f64_s16(a);
  #else
    simde_float64x1_private r_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f64_s16
  #define vreinterpret_f64_s16 simde_vreinterpret_f64_s16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vreinterpret_f64_s32(simde_int32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_f64_s32(a);
  #else
    simde_float64x1_private r_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f64_s32
  #define vreinterpret_f64_s32 simde_vreinterpret_f64_s32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vreinterpret_f64_s64(simde_int64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_f64_s64(a);
  #else
    simde_float64x1_private r_;
    simde_int64x1_private a_ = simde_int64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f64_s64
  #define vreinterpret_f64_s64 simde_vreinterpret_f64_s64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vreinterpret_f64_u8(simde_uint8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_f64_u8(a);
  #else
    simde_float64x1_private r_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f64_u8
  #define vreinterpret_f64_u8 simde_vreinterpret_f64_u8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vreinterpret_f64_u16(simde_uint16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_f64_u16(a);
  #else
    simde_float64x1_private r_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f64_u16
  #define vreinterpret_f64_u16 simde_vreinterpret_f64_u16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vreinterpret_f64_u32(simde_uint32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_f64_u32(a);
  #else
    simde_float64x1_private r_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f64_u32
  #define vreinterpret_f64_u32 simde_vreinterpret_f64_u32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vreinterpret_f64_u64(simde_uint64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_f64_u64(a);
  #else
    simde_float64x1_private r_;
    simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f64_u64
  #define vreinterpret_f64_u64 simde_vreinterpret_f64_u64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vreinterpret_f64_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_f64_f32(a);
  #else
    simde_float64x1_private r_;
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f64_f32
  #define vreinterpret_f64_f32 simde_vreinterpret_f64_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_f64_s8(a);
  #else
    simde_float64x2_private r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_s8
  #define vreinterpretq_f64_s8(a) simde_vreinterpretq_f64_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_f64_s16(a);
  #else
    simde_float64x2_private r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_s16
  #define vreinterpretq_f64_s16(a) simde_vreinterpretq_f64_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_f64_s32(a);
  #else
    simde_float64x2_private r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_s32
  #define vreinterpretq_f64_s32(a) simde_vreinterpretq_f64_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_f64_s64(a);
  #else
    simde_float64x2_private r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_s64
  #define vreinterpretq_f64_s64(a) simde_vreinterpretq_f64_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_f64_u8(a);
  #else
    simde_float64x2_private r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_u8
  #define vreinterpretq_f64_u8(a) simde_vreinterpretq_f64_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_f64_u16(a);
  #else
    simde_float64x2_private r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_u16
  #define vreinterpretq_f64_u16(a) simde_vreinterpretq_f64_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_f64_u32(a);
  #else
    simde_float64x2_private r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_u32
  #define vreinterpretq_f64_u32(a) simde_vreinterpretq_f64_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_f64_u64(a);
  #else
    simde_float64x2_private r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_u64
  #define vreinterpretq_f64_u64(a) simde_vreinterpretq_f64_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_f64_f32(a);
  #else
    simde_float64x2_private r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_f32
  #define vreinterpretq_f64_f32(a) simde_vreinterpretq_f64_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vreinterpret_f16_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f16_f32(a);
  #else
    simde_float16x4_private r_;
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f16_f32
  #define vreinterpret_f16_f32 simde_vreinterpret_f16_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vreinterpret_f16_s16(simde_int16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f16_s16(a);
  #else
    simde_float16x4_private r_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f16_s16
  #define vreinterpret_f16_s16 simde_vreinterpret_f16_s16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vreinterpret_f16_s32(simde_int32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f16_s32(a);
  #else
    simde_float16x4_private r_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f16_s32
  #define vreinterpret_f16_s32 simde_vreinterpret_f16_s32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vreinterpret_f16_s64(simde_int64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f16_s64(a);
  #else
    simde_float16x4_private r_;
    simde_int64x1_private a_ = simde_int64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f16_s64
  #define vreinterpret_f16_s64 simde_vreinterpret_f16_s64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vreinterpret_f16_s8(simde_int8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f16_s8(a);
  #else
    simde_float16x4_private r_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f16_s8
  #define vreinterpret_f16_s8 simde_vreinterpret_f16_s8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vreinterpret_f16_u32(simde_uint32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f16_u32(a);
  #else
    simde_float16x4_private r_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f16_u32
  #define vreinterpret_f16_u32 simde_vreinterpret_f16_u32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vreinterpret_f16_u64(simde_uint64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f16_u64(a);
  #else
    simde_float16x4_private r_;
    simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f16_u64
  #define vreinterpret_f16_u64 simde_vreinterpret_f16_u64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vreinterpret_f16_u8(simde_uint8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f16_u8(a);
  #else
    simde_float16x4_private r_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f16_u8
  #define vreinterpret_f16_u8 simde_vreinterpret_f16_u8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vreinterpretq_f16_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f16_f32(a);
  #else
    simde_float16x8_private r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f16_f32
  #define vreinterpretq_f16_f32(a) simde_vreinterpretq_f16_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vreinterpretq_f16_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f16_s16(a);
  #else
    simde_float16x8_private r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f16_s16
  #define vreinterpretq_f16_s16(a) simde_vreinterpretq_f16_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vreinterpretq_f16_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f16_s32(a);
  #else
    simde_float16x8_private r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f16_s32
  #define vreinterpretq_f16_s32(a) simde_vreinterpretq_f16_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vreinterpretq_f16_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f16_s64(a);
  #else
    simde_float16x8_private r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f16_s64
  #define vreinterpretq_f16_s64(a) simde_vreinterpretq_f16_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vreinterpretq_f16_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f16_s8(a);
  #else
    simde_float16x8_private r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f16_s8
  #define vreinterpretq_f16_s8(a) simde_vreinterpretq_f16_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vreinterpretq_f16_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f16_u32(a);
  #else
    simde_float16x8_private r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f16_u32
  #define vreinterpretq_f16_u32(a) simde_vreinterpretq_f16_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vreinterpretq_f16_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f16_u64(a);
  #else
    simde_float16x8_private r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f16_u64
  #define vreinterpretq_f16_u64(a) simde_vreinterpretq_f16_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vreinterpretq_f16_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f16_u8(a);
  #else
    simde_float16x8_private r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f16_u8
  #define vreinterpretq_f16_u8(a) simde_vreinterpretq_f16_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vreinterpret_f16_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f16_f64(a);
  #else
    simde_float16x4_private r_;
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f16_f64
  #define vreinterpret_f16_f64 simde_vreinterpret_f16_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vreinterpretq_f16_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f16_f64(a);
  #else
    simde_float16x8_private r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f16_f64
  #define vreinterpretq_f16_f64(a) simde_vreinterpretq_f16_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vreinterpret_f32_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f32_f16(a);
  #else
    simde_float32x2_private r_;
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f32_f16
  #define vreinterpret_f32_f16 simde_vreinterpret_f32_f16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vreinterpretq_f32_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f32_f16(a);
  #else
    simde_float32x4_private r_;
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f32_f16
  #define vreinterpretq_f32_f16 simde_vreinterpretq_f32_f16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vreinterpret_f64_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f64_f16(a);
  #else
    simde_float64x1_private r_;
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f64_f16
  #define vreinterpret_f64_f16 simde_vreinterpret_f64_f16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f64_f16(a);
  #else
    simde_float64x2_private r_;
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_f16
  #define vreinterpretq_f64_f16 simde_vreinterpretq_f64_f16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vreinterpret_u8_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_u8_f16(a);
  #else
    simde_uint8x8_private r_;
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u8_f16
  #define vreinterpret_u8_f16(a) simde_vreinterpret_u8_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_u8_f16(a);
  #else
    simde_uint8x16_private r_;
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_f16
  #define vreinterpretq_u8_f16(a) simde_vreinterpretq_u8_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vreinterpret_s8_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_s8_f16(a);
  #else
    simde_int8x8_private r_;
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s8_f16
  #define vreinterpret_s8_f16(a) simde_vreinterpret_s8_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_s8_f16(a);
  #else
    simde_int8x16_private r_;
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_f16
  #define vreinterpretq_s8_f16(a) simde_vreinterpretq_s8_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vreinterpret_s16_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_s16_f16(a);
  #else
    simde_int16x4_private r_;
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s16_f16
  #define vreinterpret_s16_f16(a) simde_vreinterpret_s16_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_s16_f16(a);
  #else
    simde_int16x8_private r_;
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_f16
  #define vreinterpretq_s16_f16(a) simde_vreinterpretq_s16_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vreinterpret_s32_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_s32_f16(a);
  #else
    simde_int32x2_private r_;
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s32_f16
  #define vreinterpret_s32_f16(a) simde_vreinterpret_s32_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_s32_f16(a);
  #else
    simde_int32x4_private r_;
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_f16
  #define vreinterpretq_s32_f16(a) simde_vreinterpretq_s32_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vreinterpret_s64_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_s64_f16(a);
  #else
    simde_int64x1_private r_;
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s64_f16
  #define vreinterpret_s64_f16(a) simde_vreinterpret_s64_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_s64_f16(a);
  #else
    simde_int64x2_private r_;
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_f16
  #define vreinterpretq_s64_f16(a) simde_vreinterpretq_s64_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vreinterpret_u32_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_u32_f16(a);
  #else
    simde_uint32x2_private r_;
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u32_f16
  #define vreinterpret_u32_f16(a) simde_vreinterpret_u32_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_u32_f16(a);
  #else
    simde_uint32x4_private r_;
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_f16
  #define vreinterpretq_u32_f16(a) simde_vreinterpretq_u32_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_u64_f16(a);
  #else
    simde_uint64x2_private r_;
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_f16
  #define vreinterpretq_u64_f16 simde_vreinterpretq_u64_f16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vreinterpret_p8_s8(simde_int8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p8_s8(a);
  #else
    simde_poly8x8_private r_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p8_s8
  #define vreinterpret_p8_s8 simde_vreinterpret_p8_s8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vreinterpret_p8_s16(simde_int16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p8_s16(a);
  #else
    simde_poly8x8_private r_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p8_s16
  #define vreinterpret_p8_s16 simde_vreinterpret_p8_s16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vreinterpret_p8_s32(simde_int32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p8_s32(a);
  #else
    simde_poly8x8_private r_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p8_s32
  #define vreinterpret_p8_s32 simde_vreinterpret_p8_s32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vreinterpret_p8_s64(simde_int64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p8_s64(a);
  #else
    simde_poly8x8_private r_;
    simde_int64x1_private a_ = simde_int64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p8_s64
  #define vreinterpret_p8_s64 simde_vreinterpret_p8_s64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vreinterpret_p8_p16(simde_poly16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p8_p16(a);
  #else
    simde_poly8x8_private r_;
    simde_poly16x4_private a_ = simde_poly16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p8_p16
  #define vreinterpret_p8_p16 simde_vreinterpret_p8_p16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vreinterpret_p8_p64(simde_poly64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_p8_p64(a);
  #else
    simde_poly8x8_private r_;
    simde_poly64x1_private a_ = simde_poly64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p8_p64
  #define vreinterpret_p8_p64 simde_vreinterpret_p8_p64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vreinterpret_p8_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p8_f32(a);
  #else
    simde_poly8x8_private r_;
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p8_f32
  #define vreinterpret_p8_f32 simde_vreinterpret_p8_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vreinterpret_p8_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_p8_f64(a);
  #else
    simde_poly8x8_private r_;
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p8_f64
  #define vreinterpret_p8_f64 simde_vreinterpret_p8_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p8_s8(a);
  #else
    simde_poly8x16_private r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_s8
  #define vreinterpretq_p8_s8(a) simde_vreinterpretq_p8_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p8_s16(a);
  #else
    simde_poly8x16_private r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_s16
  #define vreinterpretq_p8_s16(a) simde_vreinterpretq_p8_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p8_s32(a);
  #else
    simde_poly8x16_private r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_s32
  #define vreinterpretq_p8_s32(a) simde_vreinterpretq_p8_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p8_s64(a);
  #else
    simde_poly8x16_private r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_s64
  #define vreinterpretq_p8_s64(a) simde_vreinterpretq_p8_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p8_p16(a);
  #else
    simde_poly8x16_private r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_p16
  #define vreinterpretq_p8_p16(a) simde_vreinterpretq_p8_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_p64(simde_poly64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_p8_p64(a);
  #else
    simde_poly8x16_private r_;
    simde_poly64x2_private a_ = simde_poly64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_p64
  #define vreinterpretq_p8_p64(a) simde_vreinterpretq_p8_p64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p8_f32(a);
  #else
    simde_poly8x16_private r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_f32
  #define vreinterpretq_p8_f32(a) simde_vreinterpretq_p8_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_p8_f64(a);
  #else
    simde_poly8x16_private r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_f64
  #define vreinterpretq_p8_f64(a) simde_vreinterpretq_p8_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vreinterpret_p16_s8(simde_int8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p16_s8(a);
  #else
    simde_poly16x4_private r_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p16_s8
  #define vreinterpret_p16_s8 simde_vreinterpret_p16_s8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vreinterpret_p16_s16(simde_int16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p16_s16(a);
  #else
    simde_poly16x4_private r_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p16_s16
  #define vreinterpret_p16_s16 simde_vreinterpret_p16_s16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vreinterpret_p16_s32(simde_int32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p16_s32(a);
  #else
    simde_poly16x4_private r_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p16_s32
  #define vreinterpret_p16_s32 simde_vreinterpret_p16_s32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vreinterpret_p16_s64(simde_int64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p16_s64(a);
  #else
    simde_poly16x4_private r_;
    simde_int64x1_private a_ = simde_int64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p16_s64
  #define vreinterpret_p16_s64 simde_vreinterpret_p16_s64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vreinterpret_p16_p8(simde_poly8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p16_p8(a);
  #else
    simde_poly16x4_private r_;
    simde_poly8x8_private a_ = simde_poly8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p16_p8
  #define vreinterpret_p16_p8 simde_vreinterpret_p16_p8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vreinterpret_p16_p64(simde_poly64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_p16_p64(a);
  #else
    simde_poly16x4_private r_;
    simde_poly64x1_private a_ = simde_poly64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p16_p64
  #define vreinterpret_p16_p64 simde_vreinterpret_p16_p64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vreinterpret_p16_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_p16_f16(a);
  #else
    simde_poly16x4_private r_;
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p16_f16
  #define vreinterpret_p16_f16(a) simde_vreinterpret_p16_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vreinterpret_p16_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p16_f32(a);
  #else
    simde_poly16x4_private r_;
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p16_f32
  #define vreinterpret_p16_f32 simde_vreinterpret_p16_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vreinterpret_p16_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_p16_f64(a);
  #else
    simde_poly16x4_private r_;
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p16_f64
  #define vreinterpret_p16_f64 simde_vreinterpret_p16_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p16_s8(a);
  #else
    simde_poly16x8_private r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_s8
  #define vreinterpretq_p16_s8(a) simde_vreinterpretq_p16_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p16_s16(a);
  #else
    simde_poly16x8_private r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_s16
  #define vreinterpretq_p16_s16(a) simde_vreinterpretq_p16_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p16_s32(a);
  #else
    simde_poly16x8_private r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_s32
  #define vreinterpretq_p16_s32(a) simde_vreinterpretq_p16_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p16_s64(a);
  #else
    simde_poly16x8_private r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_s64
  #define vreinterpretq_p16_s64(a) simde_vreinterpretq_p16_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p16_p8(a);
  #else
    simde_poly16x8_private r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_p8
  #define vreinterpretq_p16_p8(a) simde_vreinterpretq_p16_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_p64(simde_poly64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_p16_p64(a);
  #else
    simde_poly16x8_private r_;
    simde_poly64x2_private a_ = simde_poly64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_p64
  #define vreinterpretq_p16_p64(a) simde_vreinterpretq_p16_p64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p16_f32(a);
  #else
    simde_poly16x8_private r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_f32
  #define vreinterpretq_p16_f32(a) simde_vreinterpretq_p16_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_p16_f64(a);
  #else
    simde_poly16x8_private r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_f64
  #define vreinterpretq_p16_f64(a) simde_vreinterpretq_p16_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_p16_f16(a);
  #else
    simde_poly16x8_private r_;
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_f16
  #define vreinterpretq_p16_f16(a) simde_vreinterpretq_p16_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vreinterpret_p64_s8(simde_int8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_p64_s8(a);
  #else
    simde_poly64x1_private r_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p64_s8
  #define vreinterpret_p64_s8 simde_vreinterpret_p64_s8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vreinterpret_p64_s16(simde_int16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_p64_s16(a);
  #else
    simde_poly64x1_private r_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p64_s16
  #define vreinterpret_p64_s16 simde_vreinterpret_p64_s16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vreinterpret_p64_s32(simde_int32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_p64_s32(a);
  #else
    simde_poly64x1_private r_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p64_s32
  #define vreinterpret_p64_s32 simde_vreinterpret_p64_s32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vreinterpret_p64_p8(simde_poly8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_p64_p8(a);
  #else
    simde_poly64x1_private r_;
    simde_poly8x8_private a_ = simde_poly8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p64_p8
  #define vreinterpret_p64_p8 simde_vreinterpret_p64_p8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vreinterpret_p64_p16(simde_poly16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_p64_p16(a);
  #else
    simde_poly64x1_private r_;
    simde_poly16x4_private a_ = simde_poly16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p64_p16
  #define vreinterpret_p64_p16 simde_vreinterpret_p64_p16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vreinterpret_p64_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_p64_f16(a);
  #else
    simde_poly64x1_private r_;
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p64_f16
  #define vreinterpret_p64_f16 simde_vreinterpret_p64_f16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vreinterpret_p64_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_p64_f32(a);
  #else
    simde_poly64x1_private r_;
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p64_f32
  #define vreinterpret_p64_f32 simde_vreinterpret_p64_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vreinterpret_p64_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_p64_f64(a);
  #else
    simde_poly64x1_private r_;
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p64_f64
  #define vreinterpret_p64_f64 simde_vreinterpret_p64_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vreinterpretq_p64_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_p64_s8(a);
  #else
    simde_poly64x2_private r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p64_s8
  #define vreinterpretq_p64_s8(a) simde_vreinterpretq_p64_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vreinterpretq_p64_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_p64_s16(a);
  #else
    simde_poly64x2_private r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p64_s16
  #define vreinterpretq_p64_s16(a) simde_vreinterpretq_p64_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vreinterpretq_p64_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_p64_s32(a);
  #else
    simde_poly64x2_private r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p64_s32
  #define vreinterpretq_p64_s32(a) simde_vreinterpretq_p64_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vreinterpretq_p64_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_p64_s64(a);
  #else
    simde_poly64x2_private r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p64_s64
  #define vreinterpretq_p64_s64(a) simde_vreinterpretq_p64_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vreinterpretq_p64_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_p64_p8(a);
  #else
    simde_poly64x2_private r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p64_p8
  #define vreinterpretq_p64_p8(a) simde_vreinterpretq_p64_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vreinterpretq_p64_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_p64_p16(a);
  #else
    simde_poly64x2_private r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p64_p16
  #define vreinterpretq_p64_p16(a) simde_vreinterpretq_p64_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vreinterpretq_p64_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_p64_f32(a);
  #else
    simde_poly64x2_private r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p64_f32
  #define vreinterpretq_p64_f32(a) simde_vreinterpretq_p64_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vreinterpretq_p64_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_p64_f64(a);
  #else
    simde_poly64x2_private r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p64_f64
  #define vreinterpretq_p64_f64(a) simde_vreinterpretq_p64_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vreinterpret_p8_f16(simde_float16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_p8_f16(a);
  #else
    simde_poly8x8_private r_;
    simde_float16x4_private a_ = simde_float16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p8_f16
  #define vreinterpret_p8_f16(a) simde_vreinterpret_p8_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_p8_f16(a);
  #else
    simde_poly8x16_private r_;
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_f16
  #define vreinterpretq_p8_f16(a) simde_vreinterpretq_p8_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vreinterpretq_p64_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_p64_f16(a);
  #else
    simde_poly64x2_private r_;
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p64_f16
  #define vreinterpretq_p64_f16 simde_vreinterpretq_p64_f16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vreinterpret_s8_p8(simde_poly8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s8_p8(a);
  #else
    simde_int8x8_private r_;
    simde_poly8x8_private a_ = simde_poly8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s8_p8
  #define vreinterpret_s8_p8 simde_vreinterpret_s8_p8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vreinterpret_s8_p16(simde_poly16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s8_p16(a);
  #else
    simde_int8x8_private r_;
    simde_poly16x4_private a_ = simde_poly16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s8_p16
  #define vreinterpret_s8_p16 simde_vreinterpret_s8_p16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vreinterpret_s8_p64(simde_poly64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_s8_p64(a);
  #else
    simde_int8x8_private r_;
    simde_poly64x1_private a_ = simde_poly64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s8_p64
  #define vreinterpret_s8_p64 simde_vreinterpret_s8_p64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s8_p8(a);
  #else
    simde_int8x16_private r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_p8
  #define vreinterpretq_s8_p8(a) simde_vreinterpretq_s8_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s8_p16(a);
  #else
    simde_int8x16_private r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_p16
  #define vreinterpretq_s8_p16(a) simde_vreinterpretq_s8_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_p64(simde_poly64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_s8_p64(a);
  #else
    simde_int8x16_private r_;
    simde_poly64x2_private a_ = simde_poly64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_p64
  #define vreinterpretq_s8_p64(a) simde_vreinterpretq_s8_p64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vreinterpret_s16_p8(simde_poly8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s16_p8(a);
  #else
    simde_int16x4_private r_;
    simde_poly8x8_private a_ = simde_poly8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s16_p8
  #define vreinterpret_s16_p8 simde_vreinterpret_s16_p8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vreinterpret_s16_p16(simde_poly16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s16_p16(a);
  #else
    simde_int16x4_private r_;
    simde_poly16x4_private a_ = simde_poly16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s16_p16
  #define vreinterpret_s16_p16 simde_vreinterpret_s16_p16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vreinterpret_s16_p64(simde_poly64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_s16_p64(a);
  #else
    simde_int16x4_private r_;
    simde_poly64x1_private a_ = simde_poly64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s16_p64
  #define vreinterpret_s16_p64 simde_vreinterpret_s16_p64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s16_p8(a);
  #else
    simde_int16x8_private r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_p8
  #define vreinterpretq_s16_p8(a) simde_vreinterpretq_s16_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s16_p16(a);
  #else
    simde_int16x8_private r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_p16
  #define vreinterpretq_s16_p16(a) simde_vreinterpretq_s16_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_p64(simde_poly64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_s16_p64(a);
  #else
    simde_int16x8_private r_;
    simde_poly64x2_private a_ = simde_poly64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_p64
  #define vreinterpretq_s16_p64(a) simde_vreinterpretq_s16_p64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vreinterpret_s32_p8(simde_poly8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s32_p8(a);
  #else
    simde_int32x2_private r_;
    simde_poly8x8_private a_ = simde_poly8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s32_p8
  #define vreinterpret_s32_p8 simde_vreinterpret_s32_p8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vreinterpret_s32_p16(simde_poly16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s32_p16(a);
  #else
    simde_int32x2_private r_;
    simde_poly16x4_private a_ = simde_poly16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s32_p16
  #define vreinterpret_s32_p16 simde_vreinterpret_s32_p16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vreinterpret_s32_p64(simde_poly64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_s32_p64(a);
  #else
    simde_int32x2_private r_;
    simde_poly64x1_private a_ = simde_poly64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s32_p64
  #define vreinterpret_s32_p64 simde_vreinterpret_s32_p64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s32_p8(a);
  #else
    simde_int32x4_private r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_p8
  #define vreinterpretq_s32_p8(a) simde_vreinterpretq_s32_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s32_p16(a);
  #else
    simde_int32x4_private r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_p16
  #define vreinterpretq_s32_p16(a) simde_vreinterpretq_s32_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_p64(simde_poly64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_s32_p64(a);
  #else
    simde_int32x4_private r_;
    simde_poly64x2_private a_ = simde_poly64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_p64
  #define vreinterpretq_s32_p64(a) simde_vreinterpretq_s32_p64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vreinterpret_s64_p8(simde_poly8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s64_p8(a);
  #else
    simde_int64x1_private r_;
    simde_poly8x8_private a_ = simde_poly8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s64_p8
  #define vreinterpret_s64_p8 simde_vreinterpret_s64_p8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vreinterpret_s64_p16(simde_poly16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_s64_p16(a);
  #else
    simde_int64x1_private r_;
    simde_poly16x4_private a_ = simde_poly16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s64_p16
  #define vreinterpret_s64_p16 simde_vreinterpret_s64_p16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vreinterpret_s64_p64(simde_poly64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_s64_p64(a);
  #else
    simde_int64x1_private r_;
    simde_poly64x1_private a_ = simde_poly64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s64_p64
  #define vreinterpret_s64_p64 simde_vreinterpret_s64_p64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s64_p8(a);
  #else
    simde_int64x2_private r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_p8
  #define vreinterpretq_s64_p8(a) simde_vreinterpretq_s64_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_s64_p16(a);
  #else
    simde_int64x2_private r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_p16
  #define vreinterpretq_s64_p16(a) simde_vreinterpretq_s64_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_p64(simde_poly64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_s64_p64(a);
  #else
    simde_int64x2_private r_;
    simde_poly64x2_private a_ = simde_poly64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_p64
  #define vreinterpretq_s64_p64(a) simde_vreinterpretq_s64_p64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vreinterpret_f32_p8(simde_poly8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_f32_p8(a);
  #else
    simde_float32x2_private r_;
    simde_poly8x8_private a_ = simde_poly8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f32_p8
  #define vreinterpret_f32_p8 simde_vreinterpret_f32_p8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vreinterpret_f32_p16(simde_poly16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_f32_p16(a);
  #else
    simde_float32x2_private r_;
    simde_poly16x4_private a_ = simde_poly16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f32_p16
  #define vreinterpret_f32_p16 simde_vreinterpret_f32_p16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vreinterpret_f16_p16(simde_poly16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f16_p16(a);
  #else
    simde_float16x4_private r_;
    simde_poly16x4_private a_ = simde_poly16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f16_p16
  #define vreinterpret_f16_p16(a) simde_vreinterpret_f16_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vreinterpretq_f32_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_f32_p8(a);
  #else
    simde_float32x4_private r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f32_p8
  #define vreinterpretq_f32_p8(a) simde_vreinterpretq_f32_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vreinterpretq_f32_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_f32_p16(a);
  #else
    simde_float32x4_private r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f32_p16
  #define vreinterpretq_f32_p16(a) simde_vreinterpretq_f32_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vreinterpretq_f16_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f16_p16(a);
  #else
    simde_float16x8_private r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f16_p16
  #define vreinterpretq_f16_p16(a) simde_vreinterpretq_f16_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vreinterpret_f64_p8(simde_poly8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_f64_p8(a);
  #else
    simde_float64x1_private r_;
    simde_poly8x8_private a_ = simde_poly8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f64_p8
  #define vreinterpret_f64_p8 simde_vreinterpret_f64_p8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vreinterpret_f64_p16(simde_poly16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_f64_p16(a);
  #else
    simde_float64x1_private r_;
    simde_poly16x4_private a_ = simde_poly16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f64_p16
  #define vreinterpret_f64_p16 simde_vreinterpret_f64_p16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vreinterpret_f64_p64(simde_poly64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpret_f64_p64(a);
  #else
    simde_float64x1_private r_;
    simde_poly64x1_private a_ = simde_poly64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f64_p64
  #define vreinterpret_f64_p64 simde_vreinterpret_f64_p64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_f64_p8(a);
  #else
    simde_float64x2_private r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_p8
  #define vreinterpretq_f64_p8(a) simde_vreinterpretq_f64_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_f64_p16(a);
  #else
    simde_float64x2_private r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_p16
  #define vreinterpretq_f64_p16(a) simde_vreinterpretq_f64_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_p64(simde_poly64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vreinterpretq_f64_p64(a);
  #else
    simde_float64x2_private r_;
    simde_poly64x2_private a_ = simde_poly64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_p64
  #define vreinterpretq_f64_p64(a) simde_vreinterpretq_f64_p64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vreinterpret_f16_p64(simde_poly64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f16_p64(a);
  #else
    simde_float16x4_private r_;
    simde_poly64x1_private a_ = simde_poly64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f16_p64
  #define vreinterpret_f16_p64 simde_vreinterpret_f16_p64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x4_t
simde_vreinterpret_f16_p8(simde_poly8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpret_f16_p8(a);
  #else
    simde_float16x4_private r_;
    simde_poly8x8_private a_ = simde_poly8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f16_p8
  #define vreinterpret_f16_p8 simde_vreinterpret_f16_p8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vreinterpretq_f16_p64(simde_poly64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f16_p64(a);
  #else
    simde_float16x8_private r_;
    simde_poly64x2_private a_ = simde_poly64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f16_p64
  #define vreinterpretq_f16_p64(a) simde_vreinterpretq_f16_p64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vreinterpretq_f16_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_FP16)
    return vreinterpretq_f16_p8(a);
  #else
    simde_float16x8_private r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f16_p8
  #define vreinterpretq_f16_p8(a) simde_vreinterpretq_f16_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vreinterpret_u8_p16(simde_poly16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u8_p16(a);
  #else
    simde_uint8x8_private r_;
    simde_poly16x4_private a_ = simde_poly16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u8_p16
  #define vreinterpret_u8_p16 simde_vreinterpret_u8_p16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vreinterpret_u8_p64(simde_poly64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_u8_p64(a);
  #else
    simde_uint8x8_private r_;
    simde_poly64x1_private a_ = simde_poly64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u8_p64
  #define vreinterpret_u8_p64 simde_vreinterpret_u8_p64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u8_p16(a);
  #else
    simde_uint8x16_private r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_p16
  #define vreinterpretq_u8_p16(a) simde_vreinterpretq_u8_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_p64(simde_poly64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_u8_p64(a);
  #else
    simde_uint8x16_private r_;
    simde_poly64x2_private a_ = simde_poly64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_p64
  #define vreinterpretq_u8_p64(a) simde_vreinterpretq_u8_p64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vreinterpret_u16_p8(simde_poly8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u16_p8(a);
  #else
    simde_uint16x4_private r_;
    simde_poly8x8_private a_ = simde_poly8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u16_p8
  #define vreinterpret_u16_p8 simde_vreinterpret_u16_p8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vreinterpret_u16_p64(simde_poly64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_u16_p64(a);
  #else
    simde_uint16x4_private r_;
    simde_poly64x1_private a_ = simde_poly64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u16_p64
  #define vreinterpret_u16_p64 simde_vreinterpret_u16_p64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u16_p8(a);
  #else
    simde_uint16x8_private r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_p8
  #define vreinterpretq_u16_p8(a) simde_vreinterpretq_u16_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_p64(simde_poly64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_u16_p64(a);
  #else
    simde_uint16x8_private r_;
    simde_poly64x2_private a_ = simde_poly64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_p64
  #define vreinterpretq_u16_p64(a) simde_vreinterpretq_u16_p64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vreinterpret_u32_p8(simde_poly8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u32_p8(a);
  #else
    simde_uint32x2_private r_;
    simde_poly8x8_private a_ = simde_poly8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u32_p8
  #define vreinterpret_u32_p8 simde_vreinterpret_u32_p8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u32_p8(a);
  #else
    simde_uint32x4_private r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_p8
  #define vreinterpretq_u32_p8(a) simde_vreinterpretq_u32_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vreinterpret_u32_p16(simde_poly16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u32_p16(a);
  #else
    simde_uint32x2_private r_;
    simde_poly16x4_private a_ = simde_poly16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u32_p16
  #define vreinterpret_u32_p16 simde_vreinterpret_u32_p16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u32_p16(a);
  #else
    simde_uint32x4_private r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_p16
  #define vreinterpretq_u32_p16(a) simde_vreinterpretq_u32_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vreinterpret_u32_p64(simde_poly64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_u32_p64(a);
  #else
    simde_uint32x2_private r_;
    simde_poly64x1_private a_ = simde_poly64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u32_p64
  #define vreinterpret_u32_p64 simde_vreinterpret_u32_p64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_p64(simde_poly64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_u32_p64(a);
  #else
    simde_uint32x4_private r_;
    simde_poly64x2_private a_ = simde_poly64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_p64
  #define vreinterpretq_u32_p64(a) simde_vreinterpretq_u32_p64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vreinterpret_u64_p8(simde_poly8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u64_p8(a);
  #else
    simde_uint64x1_private r_;
    simde_poly8x8_private a_ = simde_poly8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u64_p8
  #define vreinterpret_u64_p8 simde_vreinterpret_u64_p8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u64_p8(a);
  #else
    simde_uint64x2_private r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_p8
  #define vreinterpretq_u64_p8(a) simde_vreinterpretq_u64_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vreinterpret_u64_p16(simde_poly16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u64_p16(a);
  #else
    simde_uint64x1_private r_;
    simde_poly16x4_private a_ = simde_poly16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u64_p16
  #define vreinterpret_u64_p16 simde_vreinterpret_u64_p16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u64_p16(a);
  #else
    simde_uint64x2_private r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_p16
  #define vreinterpretq_u64_p16(a) simde_vreinterpretq_u64_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vreinterpret_p8_u16(simde_uint16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p8_u16(a);
  #else
    simde_poly8x8_private r_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p8_u16
  #define vreinterpret_p8_u16 simde_vreinterpret_p8_u16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vreinterpret_p8_u64(simde_uint64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p8_u64(a);
  #else
    simde_poly8x8_private r_;
    simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p8_u64
  #define vreinterpret_p8_u64 simde_vreinterpret_p8_u64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p8_u16(a);
  #else
    simde_poly8x16_private r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_u16
  #define vreinterpretq_p8_u16(a) simde_vreinterpretq_p8_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p8_u64(a);
  #else
    simde_poly8x16_private r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_u64
  #define vreinterpretq_p8_u64(a) simde_vreinterpretq_p8_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vreinterpret_p8_u32(simde_uint32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p8_u32(a);
  #else
    simde_poly8x8_private r_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p8_u32
  #define vreinterpret_p8_u32 simde_vreinterpret_p8_u32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p8_u32(a);
  #else
    simde_poly8x16_private r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_u32
  #define vreinterpretq_p8_u32(a) simde_vreinterpretq_p8_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vreinterpret_p16_u8(simde_uint8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p16_u8(a);
  #else
    simde_poly16x4_private r_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p16_u8
  #define vreinterpret_p16_u8 simde_vreinterpret_p16_u8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vreinterpret_p16_u32(simde_uint32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p16_u32(a);
  #else
    simde_poly16x4_private r_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p16_u32
  #define vreinterpret_p16_u32 simde_vreinterpret_p16_u32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vreinterpret_p16_u64(simde_uint64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p16_u64(a);
  #else
    simde_poly16x4_private r_;
    simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p16_u64
  #define vreinterpret_p16_u64 simde_vreinterpret_p16_u64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p16_u8(a);
  #else
    simde_poly16x8_private r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_u8
  #define vreinterpretq_p16_u8(a) simde_vreinterpretq_p16_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p16_u32(a);
  #else
    simde_poly16x8_private r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_u32
  #define vreinterpretq_p16_u32(a) simde_vreinterpretq_p16_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p16_u64(a);
  #else
    simde_poly16x8_private r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_u64
  #define vreinterpretq_p16_u64(a) simde_vreinterpretq_p16_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vreinterpret_p64_u8(simde_uint8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_p64_u8(a);
  #else
    simde_poly64x1_private r_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p64_u8
  #define vreinterpret_p64_u8 simde_vreinterpret_p64_u8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vreinterpret_p64_u16(simde_uint16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_p64_u16(a);
  #else
    simde_poly64x1_private r_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p64_u16
  #define vreinterpret_p64_u16 simde_vreinterpret_p64_u16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vreinterpret_p64_u32(simde_uint32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_p64_u32(a);
  #else
    simde_poly64x1_private r_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p64_u32
  #define vreinterpret_p64_u32 simde_vreinterpret_p64_u32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vreinterpretq_p64_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_p64_u8(a);
  #else
    simde_poly64x2_private r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p64_u8
  #define vreinterpretq_p64_u8(a) simde_vreinterpretq_p64_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vreinterpretq_p64_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_p64_u16(a);
  #else
    simde_poly64x2_private r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p64_u16
  #define vreinterpretq_p64_u16(a) simde_vreinterpretq_p64_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vreinterpretq_p64_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_p64_u32(a);
  #else
    simde_poly64x2_private r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p64_u32
  #define vreinterpretq_p64_u32(a) simde_vreinterpretq_p64_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vreinterpret_u8_p8(simde_poly8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u8_p8(a);
  #else
    simde_uint8x8_private r_;
    simde_poly8x8_private a_ = simde_poly8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u8_p8
  #define vreinterpret_u8_p8 simde_vreinterpret_u8_p8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u8_p8(a);
  #else
    simde_uint8x16_private r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_p8
  #define vreinterpretq_u8_p8(a) simde_vreinterpretq_u8_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vreinterpret_u16_p16(simde_poly16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_u16_p16(a);
  #else
    simde_uint16x4_private r_;
    simde_poly16x4_private a_ = simde_poly16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u16_p16
  #define vreinterpret_u16_p16 simde_vreinterpret_u16_p16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_u16_p16(a);
  #else
    simde_uint16x8_private r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_p16
  #define vreinterpretq_u16_p16(a) simde_vreinterpretq_u16_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vreinterpret_u64_p64(simde_poly64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_u64_p64(a);
  #else
    simde_uint64x1_private r_;
    simde_poly64x1_private a_ = simde_poly64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u64_p64
  #define vreinterpret_u64_p64 simde_vreinterpret_u64_p64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_p64(simde_poly64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_u64_p64(a);
  #else
    simde_uint64x2_private r_;
    simde_poly64x2_private a_ = simde_poly64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_p64
  #define vreinterpretq_u64_p64(a) simde_vreinterpretq_u64_p64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vreinterpret_p8_u8(simde_uint8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p8_u8(a);
  #else
    simde_poly8x8_private r_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p8_u8
  #define vreinterpret_p8_u8 simde_vreinterpret_p8_u8
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p8_u8(a);
  #else
    simde_poly8x16_private r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_u8
  #define vreinterpretq_p8_u8(a) simde_vreinterpretq_p8_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vreinterpret_p16_u16(simde_uint16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpret_p16_u16(a);
  #else
    simde_poly16x4_private r_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p16_u16
  #define vreinterpret_p16_u16 simde_vreinterpret_p16_u16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vreinterpretq_p16_u16(a);
  #else
    simde_poly16x8_private r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_u16
  #define vreinterpretq_p16_u16(a) simde_vreinterpretq_p16_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vreinterpret_p64_u64(simde_uint64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpret_p64_u64(a);
  #else
    simde_poly64x1_private r_;
    simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p64_u64
  #define vreinterpret_p64_u64 simde_vreinterpret_p64_u64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vreinterpretq_p64_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE)
    return vreinterpretq_p64_u64(a);
  #else
    simde_poly64x2_private r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p64_u64
  #define vreinterpretq_p64_u64(a) simde_vreinterpretq_p64_u64(a)
#endif

#if !defined(SIMDE_TARGET_NOT_SUPPORT_INT128_TYPE)
SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vreinterpretq_p128_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p128_s8(a);
  #else
    simde_poly128_t r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p128_s8
  #define vreinterpretq_p128_s8(a) simde_vreinterpretq_p128_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vreinterpretq_p128_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p128_s16(a);
  #else
    simde_poly128_t r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p128_s16
  #define vreinterpretq_p128_s16(a) simde_vreinterpretq_p128_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vreinterpretq_p128_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p128_s32(a);
  #else
    simde_poly128_t r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p128_s32
  #define vreinterpretq_p128_s32(a) simde_vreinterpretq_p128_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vreinterpretq_p128_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p128_s64(a);
  #else
    simde_poly128_t r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p128_s64
  #define vreinterpretq_p128_s64(a) simde_vreinterpretq_p128_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vreinterpretq_p128_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p128_u8(a);
  #else
    simde_poly128_t r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p128_u8
  #define vreinterpretq_p128_u8(a) simde_vreinterpretq_p128_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vreinterpretq_p128_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p128_u16(a);
  #else
    simde_poly128_t r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p128_u16
  #define vreinterpretq_p128_u16(a) simde_vreinterpretq_p128_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vreinterpretq_p128_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p128_u32(a);
  #else
    simde_poly128_t r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p128_u32
  #define vreinterpretq_p128_u32(a) simde_vreinterpretq_p128_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vreinterpretq_p128_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p128_u64(a);
  #else
    simde_poly128_t r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p128_u64
  #define vreinterpretq_p128_u64(a) simde_vreinterpretq_p128_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vreinterpretq_p128_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p128_p8(a);
  #else
    simde_poly128_t r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p128_p8
  #define vreinterpretq_p128_p8(a) simde_vreinterpretq_p128_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vreinterpretq_p128_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p128_p16(a);
  #else
    simde_poly128_t r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p128_p16
  #define vreinterpretq_p128_p16(a) simde_vreinterpretq_p128_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vreinterpretq_p128_f16(simde_float16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p128_f16(a);
  #else
    simde_poly128_t r_;
    simde_float16x8_private a_ = simde_float16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p128_f16
  #define vreinterpretq_p128_f16(a) simde_vreinterpretq_p128_f16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vreinterpretq_p128_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p128_f32(a);
  #else
    simde_poly128_t r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p128_f32
  #define vreinterpretq_p128_f32(a) simde_vreinterpretq_p128_f32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vreinterpretq_p128_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p128_f64(a);
  #else
    simde_poly128_t r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p128_f64
  #define vreinterpretq_p128_f64(a) simde_vreinterpretq_p128_f64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_p128(simde_poly128_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_s8_p128(a);
  #else
    simde_int8x16_private r_;
    simde_poly128_t a_ = a;
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_p128
  #define vreinterpretq_s8_p128(a) simde_vreinterpretq_s8_p128(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_p128(simde_poly128_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_s16_p128(a);
  #else
    simde_int16x8_private r_;
    simde_poly128_t a_ = a;
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_p128
  #define vreinterpretq_s16_p128(a) simde_vreinterpretq_s16_p128(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_p128(simde_poly128_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_s32_p128(a);
  #else
    simde_int32x4_private r_;
    simde_poly128_t a_ = a;
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_p128
  #define vreinterpretq_s32_p128(a) simde_vreinterpretq_s32_p128(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_p128(simde_poly128_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_s64_p128(a);
  #else
    simde_int64x2_private r_;
    simde_poly128_t a_ = a;
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_p128
  #define vreinterpretq_s64_p128(a) simde_vreinterpretq_s64_p128(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_p128(simde_poly128_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_u8_p128(a);
  #else
    simde_uint8x16_private r_;
    simde_poly128_t a_ = a;
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_p128
  #define vreinterpretq_u8_p128(a) simde_vreinterpretq_u8_p128(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_p128(simde_poly128_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_u16_p128(a);
  #else
    simde_uint16x8_private r_;
    simde_poly128_t a_ = a;
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_p128
  #define vreinterpretq_u16_p128(a) simde_vreinterpretq_u16_p128(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_p128(simde_poly128_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_u32_p128(a);
  #else
    simde_uint32x4_private r_;
    simde_poly128_t a_ = a;
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_p128
  #define vreinterpretq_u32_p128(a) simde_vreinterpretq_u32_p128(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_p128(simde_poly128_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_u64_p128(a);
  #else
    simde_uint64x2_private r_;
    simde_poly128_t a_ = a;
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_p128
  #define vreinterpretq_u64_p128(a) simde_vreinterpretq_u64_p128(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_p128(simde_poly128_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p8_p128(a);
  #else
    simde_poly8x16_private r_;
    simde_poly128_t a_ = a;
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_p128
  #define vreinterpretq_p8_p128(a) simde_vreinterpretq_p8_p128(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_p128(simde_poly128_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_p16_p128(a);
  #else
    simde_poly16x8_private r_;
    simde_poly128_t a_ = a;
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_p128
  #define vreinterpretq_p16_p128(a) simde_vreinterpretq_p16_p128(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float16x8_t
simde_vreinterpretq_f16_p128(simde_poly128_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_FP16) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_f16_p128(a);
  #else
    simde_float16x8_private r_;
    simde_poly128_t a_ = a;
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f16_p128
  #define vreinterpretq_f16_p128(a) simde_vreinterpretq_f16_p128(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_p128(simde_poly128_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARCH_ARM_CRYPTO)
    return vreinterpretq_f64_p128(a);
  #else
    simde_float64x2_private r_;
    simde_poly128_t a_ = a;
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_p128
  #define vreinterpretq_f64_p128(a) simde_vreinterpretq_f64_p128(a)
#endif

#endif /* !defined(SIMDE_TARGET_NOT_SUPPORT_INT128_TYPE) */

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vreinterpret_bf16_s8(simde_int8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_bf16_s8(a);
  #else
    simde_bfloat16x4_private r_;
    simde_int8x8_private a_ = simde_int8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_bf16_s8
  #define vreinterpret_bf16_s8(a) simde_vreinterpret_bf16_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vreinterpret_bf16_s16(simde_int16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_bf16_s16(a);
  #else
    simde_bfloat16x4_private r_;
    simde_int16x4_private a_ = simde_int16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_bf16_s16
  #define vreinterpret_bf16_s16(a) simde_vreinterpret_bf16_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vreinterpret_bf16_s32(simde_int32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_bf16_s32(a);
  #else
    simde_bfloat16x4_private r_;
    simde_int32x2_private a_ = simde_int32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_bf16_s32
  #define vreinterpret_bf16_s32(a) simde_vreinterpret_bf16_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vreinterpret_bf16_s64(simde_int64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_bf16_s64(a);
  #else
    simde_bfloat16x4_private r_;
    simde_int64x1_private a_ = simde_int64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_bf16_s64
  #define vreinterpret_bf16_s64(a) simde_vreinterpret_bf16_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vreinterpret_bf16_u8(simde_uint8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_bf16_u8(a);
  #else
    simde_bfloat16x4_private r_;
    simde_uint8x8_private a_ = simde_uint8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_bf16_u8
  #define vreinterpret_bf16_u8(a) simde_vreinterpret_bf16_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vreinterpret_bf16_u16(simde_uint16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_bf16_u16(a);
  #else
    simde_bfloat16x4_private r_;
    simde_uint16x4_private a_ = simde_uint16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_bf16_u16
  #define vreinterpret_bf16_u16(a) simde_vreinterpret_bf16_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vreinterpret_bf16_u32(simde_uint32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_bf16_u32(a);
  #else
    simde_bfloat16x4_private r_;
    simde_uint32x2_private a_ = simde_uint32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_bf16_u32
  #define vreinterpret_bf16_u32(a) simde_vreinterpret_bf16_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vreinterpret_bf16_u64(simde_uint64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_bf16_u64(a);
  #else
    simde_bfloat16x4_private r_;
    simde_uint64x1_private a_ = simde_uint64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_bf16_u64
  #define vreinterpret_bf16_u64(a) simde_vreinterpret_bf16_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vreinterpret_bf16_f32(simde_float32x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_bf16_f32(a);
  #else
    simde_bfloat16x4_private r_;
    simde_float32x2_private a_ = simde_float32x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_bf16_f32
  #define vreinterpret_bf16_f32 simde_vreinterpret_bf16_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vreinterpret_bf16_f64(simde_float64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_bf16_f64(a);
  #else
    simde_bfloat16x4_private r_;
    simde_float64x1_private a_ = simde_float64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_bf16_f64
  #define vreinterpret_bf16_f64 simde_vreinterpret_bf16_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vreinterpretq_bf16_s8(simde_int8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_bf16_s8(a);
  #else
    simde_bfloat16x8_private r_;
    simde_int8x16_private a_ = simde_int8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_bf16_s8
  #define vreinterpretq_bf16_s8(a) simde_vreinterpretq_bf16_s8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vreinterpretq_bf16_s16(simde_int16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_bf16_s16(a);
  #else
    simde_bfloat16x8_private r_;
    simde_int16x8_private a_ = simde_int16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_bf16_s16
  #define vreinterpretq_bf16_s16(a) simde_vreinterpretq_bf16_s16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vreinterpretq_bf16_s32(simde_int32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_bf16_s32(a);
  #else
    simde_bfloat16x8_private r_;
    simde_int32x4_private a_ = simde_int32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_bf16_s32
  #define vreinterpretq_bf16_s32(a) simde_vreinterpretq_bf16_s32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vreinterpretq_bf16_s64(simde_int64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_bf16_s64(a);
  #else
    simde_bfloat16x8_private r_;
    simde_int64x2_private a_ = simde_int64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_bf16_s64
  #define vreinterpretq_bf16_s64(a) simde_vreinterpretq_bf16_s64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vreinterpretq_bf16_u8(simde_uint8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_bf16_u8(a);
  #else
    simde_bfloat16x8_private r_;
    simde_uint8x16_private a_ = simde_uint8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_bf16_u8
  #define vreinterpretq_bf16_u8(a) simde_vreinterpretq_bf16_u8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vreinterpretq_bf16_u16(simde_uint16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_bf16_u16(a);
  #else
    simde_bfloat16x8_private r_;
    simde_uint16x8_private a_ = simde_uint16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_bf16_u16
  #define vreinterpretq_bf16_u16(a) simde_vreinterpretq_bf16_u16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vreinterpretq_bf16_u32(simde_uint32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_bf16_u32(a);
  #else
    simde_bfloat16x8_private r_;
    simde_uint32x4_private a_ = simde_uint32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_bf16_u32
  #define vreinterpretq_bf16_u32(a) simde_vreinterpretq_bf16_u32(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vreinterpretq_bf16_u64(simde_uint64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_bf16_u64(a);
  #else
    simde_bfloat16x8_private r_;
    simde_uint64x2_private a_ = simde_uint64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_bf16_u64
  #define vreinterpretq_bf16_u64(a) simde_vreinterpretq_bf16_u64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vreinterpretq_bf16_f32(simde_float32x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_bf16_f32(a);
  #else
    simde_bfloat16x8_private r_;
    simde_float32x4_private a_ = simde_float32x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_bf16_f32
  #define vreinterpretq_bf16_f32 simde_vreinterpretq_bf16_f32
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vreinterpretq_bf16_f64(simde_float64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_bf16_f64(a);
  #else
    simde_bfloat16x8_private r_;
    simde_float64x2_private a_ = simde_float64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_bf16_f64
  #define vreinterpretq_bf16_f64 simde_vreinterpretq_bf16_f64
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x8_t
simde_vreinterpret_s8_bf16(simde_bfloat16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_s8_bf16(a);
  #else
    simde_int8x8_private r_;
    simde_bfloat16x4_private a_ = simde_bfloat16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s8_bf16
  #define vreinterpret_s8_bf16(a) simde_vreinterpret_s8_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x4_t
simde_vreinterpret_s16_bf16(simde_bfloat16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_s16_bf16(a);
  #else
    simde_int16x4_private r_;
    simde_bfloat16x4_private a_ = simde_bfloat16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s16_bf16
  #define vreinterpret_s16_bf16(a) simde_vreinterpret_s16_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x2_t
simde_vreinterpret_s32_bf16(simde_bfloat16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_s32_bf16(a);
  #else
    simde_int32x2_private r_;
    simde_bfloat16x4_private a_ = simde_bfloat16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s32_bf16
  #define vreinterpret_s32_bf16(a) simde_vreinterpret_s32_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x1_t
simde_vreinterpret_s64_bf16(simde_bfloat16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_s64_bf16(a);
  #else
    simde_int64x1_private r_;
    simde_bfloat16x4_private a_ = simde_bfloat16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_s64_bf16
  #define vreinterpret_s64_bf16(a) simde_vreinterpret_s64_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x8_t
simde_vreinterpret_u8_bf16(simde_bfloat16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_u8_bf16(a);
  #else
    simde_uint8x8_private r_;
    simde_bfloat16x4_private a_ = simde_bfloat16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u8_bf16
  #define vreinterpret_u8_bf16(a) simde_vreinterpret_u8_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x4_t
simde_vreinterpret_u16_bf16(simde_bfloat16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_u16_bf16(a);
  #else
    simde_uint16x4_private r_;
    simde_bfloat16x4_private a_ = simde_bfloat16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u16_bf16
  #define vreinterpret_u16_bf16(a) simde_vreinterpret_u16_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vreinterpret_u32_bf16(simde_bfloat16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_u32_bf16(a);
  #else
    simde_uint32x2_private r_;
    simde_bfloat16x4_private a_ = simde_bfloat16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u32_bf16
  #define vreinterpret_u32_bf16(a) simde_vreinterpret_u32_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vreinterpret_u64_bf16(simde_bfloat16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_u64_bf16(a);
  #else
    simde_uint64x1_private r_;
    simde_bfloat16x4_private a_ = simde_bfloat16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_u64_bf16
  #define vreinterpret_u64_bf16(a) simde_vreinterpret_u64_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x2_t
simde_vreinterpret_f32_bf16(simde_bfloat16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_f32_bf16(a);
  #else
    simde_float32x2_private r_;
    simde_bfloat16x4_private a_ = simde_bfloat16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f32_bf16
  #define vreinterpret_f32_bf16 simde_vreinterpret_f32_bf16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x1_t
simde_vreinterpret_f64_bf16(simde_bfloat16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_f64_bf16(a);
  #else
    simde_float64x1_private r_;
    simde_bfloat16x4_private a_ = simde_bfloat16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_f64_bf16
  #define vreinterpret_f64_bf16 simde_vreinterpret_f64_bf16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int8x16_t
simde_vreinterpretq_s8_bf16(simde_bfloat16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_s8_bf16(a);
  #else
    simde_int8x16_private r_;
    simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s8_bf16
  #define vreinterpretq_s8_bf16(a) simde_vreinterpretq_s8_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int16x8_t
simde_vreinterpretq_s16_bf16(simde_bfloat16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_s16_bf16(a);
  #else
    simde_int16x8_private r_;
    simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s16_bf16
  #define vreinterpretq_s16_bf16(a) simde_vreinterpretq_s16_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int32x4_t
simde_vreinterpretq_s32_bf16(simde_bfloat16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_s32_bf16(a);
  #else
    simde_int32x4_private r_;
    simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s32_bf16
  #define vreinterpretq_s32_bf16(a) simde_vreinterpretq_s32_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_int64x2_t
simde_vreinterpretq_s64_bf16(simde_bfloat16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_s64_bf16(a);
  #else
    simde_int64x2_private r_;
    simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_int64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_s64_bf16
  #define vreinterpretq_s64_bf16(a) simde_vreinterpretq_s64_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint8x16_t
simde_vreinterpretq_u8_bf16(simde_bfloat16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_u8_bf16(a);
  #else
    simde_uint8x16_private r_;
    simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u8_bf16
  #define vreinterpretq_u8_bf16(a) simde_vreinterpretq_u8_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint16x8_t
simde_vreinterpretq_u16_bf16(simde_bfloat16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_u16_bf16(a);
  #else
    simde_uint16x8_private r_;
    simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u16_bf16
  #define vreinterpretq_u16_bf16(a) simde_vreinterpretq_u16_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vreinterpretq_u32_bf16(simde_bfloat16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_u32_bf16(a);
  #else
    simde_uint32x4_private r_;
    simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u32_bf16
  #define vreinterpretq_u32_bf16(a) simde_vreinterpretq_u32_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vreinterpretq_u64_bf16(simde_bfloat16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_u64_bf16(a);
  #else
    simde_uint64x2_private r_;
    simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_u64_bf16
  #define vreinterpretq_u64_bf16(a) simde_vreinterpretq_u64_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float32x4_t
simde_vreinterpretq_f32_bf16(simde_bfloat16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_f32_bf16(a);
  #else
    simde_float32x4_private r_;
    simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f32_bf16
  #define vreinterpretq_f32_bf16 simde_vreinterpretq_f32_bf16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_float64x2_t
simde_vreinterpretq_f64_bf16(simde_bfloat16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_f64_bf16(a);
  #else
    simde_float64x2_private r_;
    simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_float64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_f64_bf16
  #define vreinterpretq_f64_bf16 simde_vreinterpretq_f64_bf16
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vreinterpret_bf16_p8(simde_poly8x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_bf16_p8(a);
  #else
    simde_bfloat16x4_private r_;
    simde_poly8x8_private a_ = simde_poly8x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_bf16_p8
  #define vreinterpret_bf16_p8(a) simde_vreinterpret_bf16_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vreinterpret_bf16_p16(simde_poly16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_bf16_p16(a);
  #else
    simde_bfloat16x4_private r_;
    simde_poly16x4_private a_ = simde_poly16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_bf16_p16
  #define vreinterpret_bf16_p16(a) simde_vreinterpret_bf16_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x4_t
simde_vreinterpret_bf16_p64(simde_poly64x1_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_bf16_p64(a);
  #else
    simde_bfloat16x4_private r_;
    simde_poly64x1_private a_ = simde_poly64x1_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_bf16_p64
  #define vreinterpret_bf16_p64(a) simde_vreinterpret_bf16_p64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vreinterpretq_bf16_p8(simde_poly8x16_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_bf16_p8(a);
  #else
    simde_bfloat16x8_private r_;
    simde_poly8x16_private a_ = simde_poly8x16_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_bf16_p8
  #define vreinterpretq_bf16_p8(a) simde_vreinterpretq_bf16_p8(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vreinterpretq_bf16_p16(simde_poly16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_bf16_p16(a);
  #else
    simde_bfloat16x8_private r_;
    simde_poly16x8_private a_ = simde_poly16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_bf16_p16
  #define vreinterpretq_bf16_p16(a) simde_vreinterpretq_bf16_p16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vreinterpretq_bf16_p64(simde_poly64x2_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_bf16_p64(a);
  #else
    simde_bfloat16x8_private r_;
    simde_poly64x2_private a_ = simde_poly64x2_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_bf16_p64
  #define vreinterpretq_bf16_p64(a) simde_vreinterpretq_bf16_p64(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x8_t
simde_vreinterpret_p8_bf16(simde_bfloat16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_p8_bf16(a);
  #else
    simde_poly8x8_private r_;
    simde_bfloat16x4_private a_ = simde_bfloat16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p8_bf16
  #define vreinterpret_p8_bf16(a) simde_vreinterpret_p8_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x4_t
simde_vreinterpret_p16_bf16(simde_bfloat16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_p16_bf16(a);
  #else
    simde_poly16x4_private r_;
    simde_bfloat16x4_private a_ = simde_bfloat16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p16_bf16
  #define vreinterpret_p16_bf16(a) simde_vreinterpret_p16_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x1_t
simde_vreinterpret_p64_bf16(simde_bfloat16x4_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpret_p64_bf16(a);
  #else
    simde_poly64x1_private r_;
    simde_bfloat16x4_private a_ = simde_bfloat16x4_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpret_p64_bf16
  #define vreinterpret_p64_bf16(a) simde_vreinterpret_p64_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly8x16_t
simde_vreinterpretq_p8_bf16(simde_bfloat16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_p8_bf16(a);
  #else
    simde_poly8x16_private r_;
    simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly8x16_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p8_bf16
  #define vreinterpretq_p8_bf16(a) simde_vreinterpretq_p8_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly16x8_t
simde_vreinterpretq_p16_bf16(simde_bfloat16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_p16_bf16(a);
  #else
    simde_poly16x8_private r_;
    simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p16_bf16
  #define vreinterpretq_p16_bf16(a) simde_vreinterpretq_p16_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_poly64x2_t
simde_vreinterpretq_p64_bf16(simde_bfloat16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_p64_bf16(a);
  #else
    simde_poly64x2_private r_;
    simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_poly64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p64_bf16
  #define vreinterpretq_p64_bf16(a) simde_vreinterpretq_p64_bf16(a)
#endif

#if !defined(SIMDE_TARGET_NOT_SUPPORT_INT128_TYPE)

SIMDE_FUNCTION_ATTRIBUTES
simde_poly128_t
simde_vreinterpretq_p128_bf16(simde_bfloat16x8_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_p128_bf16(a);
  #else
    simde_poly128_t r_;
    simde_bfloat16x8_private a_ = simde_bfloat16x8_to_private(a);
    simde_memcpy(&r_, &a_, sizeof(r_));
    return r_;
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_p128_bf16
  #define vreinterpretq_p128_bf16(a) simde_vreinterpretq_p128_bf16(a)
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_bfloat16x8_t
simde_vreinterpretq_bf16_p128(simde_poly128_t a) {
  #if defined(SIMDE_ARM_NEON_A32V8_NATIVE) && defined(SIMDE_ARM_NEON_BF16)
    return vreinterpretq_bf16_p128(a);
  #else
    simde_bfloat16x8_t r_;
    simde_poly128_t a_ = a;
    simde_memcpy(&r_, &a_, sizeof(r_));
    return simde_bfloat16x8_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V8_ENABLE_NATIVE_ALIASES)
  #undef vreinterpretq_bf16_p128
  #define vreinterpretq_bf16_p128(a) simde_vreinterpretq_bf16_p128(a)
#endif

#endif /* !defined(SIMDE_TARGET_NOT_SUPPORT_INT128_TYPE) */

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif
