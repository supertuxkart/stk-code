// SPDX-License-Identifier: Apache-2.0
// ----------------------------------------------------------------------------
// Copyright 2020-2026 Arm Limited
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy
// of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
// ----------------------------------------------------------------------------

/**
 * @brief Unit tests for the vectorized SIMD functionality.
 *
 * One of the implementation requirements for astcenc is to provide a platform
 * invariant results, so tests deliberately use EXPECT_EQ() for floating-point
 * functions that might more typically use EXPECT_NEAR(). If we change an
 * implementation of a function so it rounds differently, the tests may start
 * failing and the values in the test may need updating.
 */

#include <limits>

#include "gtest/gtest.h"

#include "../astcenc_internal.h"
#include "../astcenc_vecmathlib.h"

namespace astcenc
{

// Misc utility tests - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#if ASTCENC_SIMD_WIDTH == 8
/**
 * @brief Construct from 8 scalar values.
 *
 * The value of @c a is stored to lane 0 (LSB) in the SIMD register.
 */
static vfloat8 vfloat8_lit(
	float a, float b, float c, float d,
	float e, float f, float g, float h
) {
	alignas(32) float data[8] {
		a, b, c, d, e, f, g, h
	};

	return vfloat8(data);
}

/**
 * @brief Construct from 8 scalar values.
 *
 * The value of @c a is stored to lane 0 (LSB) in the SIMD register.
 */
static vint8 vint8_lit(
	int a, int b, int c, int d,
	int e, int f, int g, int h
) {
	alignas(32) int data[8] {
		a, b, c, d, e, f, g, h
	};

	return vint8(data);
}

#endif

static unsigned int round_down(unsigned int x)
{
	unsigned int remainder = x % ASTCENC_SIMD_WIDTH;
	return x - remainder;
}

static unsigned int round_up(unsigned int x)
{
	unsigned int remainder = x % ASTCENC_SIMD_WIDTH;
	if (!remainder)
	{
		return x;
	}

	return x - remainder + ASTCENC_SIMD_WIDTH;
}

/** @brief Test VLA loop limit round down. */
TEST(SuiteMisc, RoundDownVLA)
{
	// Static ones which are valid for all VLA widths
	EXPECT_EQ(round_down_to_simd_multiple_vla(0),  0u);
	EXPECT_EQ(round_down_to_simd_multiple_vla(8),  8u);
	EXPECT_EQ(round_down_to_simd_multiple_vla(16), 16u);

	// Variable ones which depend on VLA width
	EXPECT_EQ(round_down_to_simd_multiple_vla(3),   round_down(3));
	EXPECT_EQ(round_down_to_simd_multiple_vla(5),   round_down(5));
	EXPECT_EQ(round_down_to_simd_multiple_vla(7),   round_down(7));
	EXPECT_EQ(round_down_to_simd_multiple_vla(231), round_down(231));
}

/** @brief Test VLA loop limit round up. */
TEST(SuiteMisc, RoundUpVLA)
{
	// Static ones which are valid for all VLA widths
	EXPECT_EQ(round_up_to_simd_multiple_vla(0),  0u);
	EXPECT_EQ(round_up_to_simd_multiple_vla(8),  8u);
	EXPECT_EQ(round_up_to_simd_multiple_vla(16), 16u);

	// Variable ones which depend on VLA width
	EXPECT_EQ(round_up_to_simd_multiple_vla(3),   round_up(3));
	EXPECT_EQ(round_up_to_simd_multiple_vla(5),   round_up(5));
	EXPECT_EQ(round_up_to_simd_multiple_vla(7),   round_up(7));
	EXPECT_EQ(round_up_to_simd_multiple_vla(231), round_up(231));
}

#if ASTCENC_SIMD_WIDTH == 1

// VLA (1-wide) tests - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** @brief Test VLA change_sign. */
TEST(SuiteVfloat, ChangeSign)
{
	vfloat a0(-1.0f);
	vfloat b0(-1.0f);
	vfloat r0 = change_sign(a0, b0);
	EXPECT_EQ(r0.lane<0>(), 1.0f);

	vfloat a1( 1.0f);
	vfloat b1(-1.0f);
	vfloat r1 = change_sign(a1, b1);
	EXPECT_EQ(r1.lane<0>(), -1.0f);

	vfloat a2(-3.12f);
	vfloat b2( 3.12f);
	vfloat r2 = change_sign(a2, b2);
	EXPECT_EQ(r2.lane<0>(), -3.12f);

	vfloat a3( 3.12f);
	vfloat b3( 3.12f);
	vfloat r3 = change_sign(a3, b3);
	EXPECT_EQ(r3.lane<0>(), 3.12f);
}

/** @brief Test VLA atan. */
TEST(SuiteVfloat, Atan)
{
	vfloat a0(-0.15f);
	vfloat r0 = atan(a0);
	EXPECT_EQ(r0.lane<0>(), -0.149060920f);

	vfloat a1(0.0f);
	vfloat r1 = atan(a1);
	EXPECT_EQ(r1.lane<0>(),  0.000000000f);

	vfloat a2(0.9f);
	vfloat r2 = atan(a2);
	EXPECT_EQ(r2.lane<0>(),  0.733615935f);

	vfloat a3(2.1f);
	vfloat r3 = atan(a3);
	EXPECT_EQ(r3.lane<0>(),  1.123035190f);
}

/** @brief Test VLA atan2. */
TEST(SuiteVfloat, Atan2)
{
	vfloat a0(-0.15f);
	vfloat b0( 1.15f);
	vfloat r0 = atan2(a0, b0);
	EXPECT_EQ(r0.lane<0>(), -0.129816383f);

	vfloat a1( 0.0f);
	vfloat b1(-3.0f);
	vfloat r1 = atan2(a1, b1);
	EXPECT_EQ(r1.lane<0>(),  3.141592740f);

	vfloat a2( 0.9f);
	vfloat b2(-0.9f);
	vfloat r2 = atan2(a2, b2);
	EXPECT_EQ(r2.lane<0>(),  2.360342740f);

	vfloat a3( 2.1f);
	vfloat b3( 1.1f);
	vfloat r3 = atan2(a3, b3);
	EXPECT_EQ(r3.lane<0>(),  1.084357620f);
}

#elif ASTCENC_SIMD_WIDTH == 4

// VLA (4-wide) tests - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** @brief Test VLA change_sign. */
TEST(SuiteVfloat, ChangeSign)
{
	vfloat4 a(-1.0f,  1.0f, -3.12f, 3.12f);
	vfloat4 b(-1.0f, -1.0f,  3.12f, 3.12f);
	vfloat4 r = change_sign(a, b);

	EXPECT_EQ(r.lane<0>(),  1.0f);
	EXPECT_EQ(r.lane<1>(), -1.0f);
	EXPECT_EQ(r.lane<2>(), -3.12f);
	EXPECT_EQ(r.lane<3>(),  3.12f);
}

/** @brief Test VLA atan. */
TEST(SuiteVfloat, Atan)
{
	vfloat4 a(-0.15f, 0.0f, 0.9f, 2.1f);
	vfloat4 r = atan(a);

	EXPECT_EQ(r.lane<0>(), -0.149060920f);
	EXPECT_EQ(r.lane<1>(),  0.000000000f);
	EXPECT_EQ(r.lane<2>(),  0.733615935f);
	EXPECT_EQ(r.lane<3>(),  1.123035190f);
}

/** @brief Test VLA atan2. */
TEST(SuiteVfloat, Atan2)
{
	vfloat4 a(-0.15f, 0.0f, 0.9f, 2.1f);
	vfloat4 b(1.15f, -3.0f, -0.9f, 1.1f);
	vfloat4 r = atan2(a, b);

	EXPECT_EQ(r.lane<0>(), -0.129816383f);
	EXPECT_EQ(r.lane<1>(),  3.141592740f);
	EXPECT_EQ(r.lane<2>(),  2.360342740f);
	EXPECT_EQ(r.lane<3>(),  1.084357620f);
}

#elif ASTCENC_SIMD_WIDTH == 8

// VLA (8-wide) tests - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** @brief Test VLA change_sign. */
TEST(SuiteVfloat, ChangeSign)
{
	vfloat8 a = vfloat8_lit(-1.0f,  1.0f, -3.12f, 3.12f, -1.0f,  1.0f, -3.12f, 3.12f);
	vfloat8 b = vfloat8_lit(-1.0f, -1.0f,  3.12f, 3.12f, -1.0f, -1.0f,  3.12f, 3.12f);
	vfloat8 r = change_sign(a, b);

	alignas(32) float ra[8];
	storea(r, ra);

	EXPECT_EQ(ra[0],  1.0f);
	EXPECT_EQ(ra[1], -1.0f);
	EXPECT_EQ(ra[2], -3.12f);
	EXPECT_EQ(ra[3],  3.12f);
	EXPECT_EQ(ra[4],  1.0f);
	EXPECT_EQ(ra[5], -1.0f);
	EXPECT_EQ(ra[6], -3.12f);
	EXPECT_EQ(ra[7],  3.12f);
}

/** @brief Test VLA atan. */
TEST(SuiteVfloat, Atan)
{
	vfloat8 a = vfloat8_lit(-0.15f, 0.0f, 0.9f, 2.1f, -0.15f, 0.0f, 0.9f, 2.1f);
	vfloat8 r = atan(a);

	alignas(32) float ra[8];
	storea(r, ra);

	EXPECT_EQ(ra[0], -0.149060920f);
	EXPECT_EQ(ra[1],  0.000000000f);
	EXPECT_EQ(ra[2],  0.733615935f);
	EXPECT_EQ(ra[3],  1.123035190f);
	EXPECT_EQ(ra[4], -0.149060920f);
	EXPECT_EQ(ra[5],  0.000000000f);
	EXPECT_EQ(ra[6],  0.733615935f);
	EXPECT_EQ(ra[7],  1.123035190f);
}

/** @brief Test VLA atan2. */
TEST(SuiteVfloat, Atan2)
{
	vfloat8 a = vfloat8_lit(-0.15f, 0.0f, 0.9f, 2.1f, -0.15f, 0.0f, 0.9f, 2.1f);
	vfloat8 b = vfloat8_lit(1.15f, -3.0f, -0.9f, 1.1f, 1.15f, -3.0f, -0.9f, 1.1f);
	vfloat8 r = atan2(a, b);

	alignas(32) float ra[8];
	storea(r, ra);

	EXPECT_EQ(ra[0], -0.129816383f);
	EXPECT_EQ(ra[1],  3.141592740f);
	EXPECT_EQ(ra[2],  2.360342740f);
	EXPECT_EQ(ra[3],  1.084357620f);
	EXPECT_EQ(ra[4], -0.129816383f);
	EXPECT_EQ(ra[5],  3.141592740f);
	EXPECT_EQ(ra[6],  2.360342740f);
	EXPECT_EQ(ra[7],  1.084357620f);
}

#endif

static const float qnan = std::numeric_limits<float>::quiet_NaN();

alignas(32) static const float f32_data[9] {
	0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f
};

alignas(32) static const int s32_data[9] {
	0, 1, 2, 3, 4, 5 , 6, 7, 8
};

alignas(32) static const uint8_t u8_data[9] {
	0, 1, 2, 3, 4, 5 , 6, 7, 8
};

// VFLOAT4 tests - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** @brief Test unaligned vfloat4 data load. */
TEST(SuiteVfloat4, UnalignedLoad)
{
	vfloat4 a(&(f32_data[1]));
	EXPECT_EQ(a.lane<0>(), 1.0f);
	EXPECT_EQ(a.lane<1>(), 2.0f);
	EXPECT_EQ(a.lane<2>(), 3.0f);
	EXPECT_EQ(a.lane<3>(), 4.0f);
}

/** @brief Test scalar duplicated vfloat4 load. */
TEST(SuiteVfloat4, ScalarDupLoad)
{
	vfloat4 a(1.1f);
	EXPECT_EQ(a.lane<0>(), 1.1f);
	EXPECT_EQ(a.lane<1>(), 1.1f);
	EXPECT_EQ(a.lane<2>(), 1.1f);
	EXPECT_EQ(a.lane<3>(), 1.1f);
}

/** @brief Test scalar vfloat4 load. */
TEST(SuiteVfloat4, ScalarLoad)
{
	vfloat4 a(1.1f, 2.2f, 3.3f, 4.4f);
	EXPECT_EQ(a.lane<0>(), 1.1f);
	EXPECT_EQ(a.lane<1>(), 2.2f);
	EXPECT_EQ(a.lane<2>(), 3.3f);
	EXPECT_EQ(a.lane<3>(), 4.4f);
}

/** @brief Test copy vfloat4 load. */
TEST(SuiteVfloat4, CopyLoad)
{
	vfloat4 s(1.1f, 2.2f, 3.3f, 4.4f);
	vfloat4 a(s.m);
	EXPECT_EQ(a.lane<0>(), 1.1f);
	EXPECT_EQ(a.lane<1>(), 2.2f);
	EXPECT_EQ(a.lane<2>(), 3.3f);
	EXPECT_EQ(a.lane<3>(), 4.4f);
}

/** @brief Test vfloat4 scalar lane set. */
TEST(SuiteVfloat4, SetLane)
{
	vfloat4 a(0.0f);

	a.set_lane<0>(1.0f);
	EXPECT_EQ(a.lane<0>(), 1.0f);
	EXPECT_EQ(a.lane<1>(), 0.0f);
	EXPECT_EQ(a.lane<2>(), 0.0f);
	EXPECT_EQ(a.lane<3>(), 0.0f);

	a.set_lane<1>(2.0f);
	EXPECT_EQ(a.lane<0>(), 1.0f);
	EXPECT_EQ(a.lane<1>(), 2.0f);
	EXPECT_EQ(a.lane<2>(), 0.0f);
	EXPECT_EQ(a.lane<3>(), 0.0f);

	a.set_lane<2>(3.0f);
	EXPECT_EQ(a.lane<0>(), 1.0f);
	EXPECT_EQ(a.lane<1>(), 2.0f);
	EXPECT_EQ(a.lane<2>(), 3.0f);
	EXPECT_EQ(a.lane<3>(), 0.0f);

	a.set_lane<3>(4.0f);
	EXPECT_EQ(a.lane<0>(), 1.0f);
	EXPECT_EQ(a.lane<1>(), 2.0f);
	EXPECT_EQ(a.lane<2>(), 3.0f);
	EXPECT_EQ(a.lane<3>(), 4.0f);
}

/** @brief Test vfloat4 zero. */
TEST(SuiteVfloat4, Zero)
{
	vfloat4 a = vfloat4::zero();
	EXPECT_EQ(a.lane<0>(), 0.0f);
	EXPECT_EQ(a.lane<1>(), 0.0f);
	EXPECT_EQ(a.lane<2>(), 0.0f);
	EXPECT_EQ(a.lane<3>(), 0.0f);
}

/** @brief Test vfloat4 load1. */
TEST(SuiteVfloat4, Load1)
{
	float s = 3.14f;
	vfloat4 a = vfloat4::load1(&s);
	EXPECT_EQ(a.lane<0>(), 3.14f);
	EXPECT_EQ(a.lane<1>(), 3.14f);
	EXPECT_EQ(a.lane<2>(), 3.14f);
	EXPECT_EQ(a.lane<3>(), 3.14f);
}

/** @brief Test vfloat4 loada. */
TEST(SuiteVfloat4, Loada)
{
	vfloat4 a = vfloat4::loada(&(f32_data[0]));
	EXPECT_EQ(a.lane<0>(), 0.0f);
	EXPECT_EQ(a.lane<1>(), 1.0f);
	EXPECT_EQ(a.lane<2>(), 2.0f);
	EXPECT_EQ(a.lane<3>(), 3.0f);
}

/** @brief Test vfloat4 swz to float4. */
TEST(SuiteVfloat4, swz4)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 r = a.swz<0, 3, 2, 1>();
	EXPECT_EQ(r.lane<0>(), 1.0f);
	EXPECT_EQ(r.lane<1>(), 4.0f);
	EXPECT_EQ(r.lane<2>(), 3.0f);
	EXPECT_EQ(r.lane<3>(), 2.0f);

	r = a.swz<3, 1, 1, 0>();
	EXPECT_EQ(r.lane<0>(), 4.0f);
	EXPECT_EQ(r.lane<1>(), 2.0f);
	EXPECT_EQ(r.lane<2>(), 2.0f);
	EXPECT_EQ(r.lane<3>(), 1.0f);
}

/** @brief Test vfloat4 swz to float3. */
TEST(SuiteVfloat4, swz3)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 r = a.swz<0, 3, 2>();
	EXPECT_EQ(r.lane<0>(), 1.0f);
	EXPECT_EQ(r.lane<1>(), 4.0f);
	EXPECT_EQ(r.lane<2>(), 3.0f);
	EXPECT_EQ(r.lane<3>(), 0.0f);

	r = a.swz<3, 1, 1>();
	EXPECT_EQ(r.lane<0>(), 4.0f);
	EXPECT_EQ(r.lane<1>(), 2.0f);
	EXPECT_EQ(r.lane<2>(), 2.0f);
	EXPECT_EQ(r.lane<3>(), 0.0f);
}

/** @brief Test vfloat4 swz to float2. */
TEST(SuiteVfloat4, swz2)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 r = a.swz<0, 3>();
	EXPECT_EQ(r.lane<0>(), 1.0f);
	EXPECT_EQ(r.lane<1>(), 4.0f);

	r = a.swz<2, 1>();
	EXPECT_EQ(r.lane<0>(), 3.0f);
	EXPECT_EQ(r.lane<1>(), 2.0f);
}

/** @brief Test vfloat4 add. */
TEST(SuiteVfloat4, vadd)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b(0.1f, 0.2f, 0.3f, 0.4f);
	a = a + b;
	EXPECT_EQ(a.lane<0>(), 1.0f + 0.1f);
	EXPECT_EQ(a.lane<1>(), 2.0f + 0.2f);
	EXPECT_EQ(a.lane<2>(), 3.0f + 0.3f);
	EXPECT_EQ(a.lane<3>(), 4.0f + 0.4f);
}

/** @brief Test vfloat4 self-add. */
TEST(SuiteVfloat4, vselfadd1)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b(0.1f, 0.2f, 0.3f, 0.4f);

	// Test increment by another variable
	a += b;
	EXPECT_EQ(a.lane<0>(), 1.0f + 0.1f);
	EXPECT_EQ(a.lane<1>(), 2.0f + 0.2f);
	EXPECT_EQ(a.lane<2>(), 3.0f + 0.3f);
	EXPECT_EQ(a.lane<3>(), 4.0f + 0.4f);

	// Test increment by an expression
	a += b + b;
	EXPECT_EQ(a.lane<0>(), 1.30000007f);
	EXPECT_EQ(a.lane<1>(), 2.60000014f);
	EXPECT_EQ(a.lane<2>(), 3.90000010f);
	EXPECT_EQ(a.lane<3>(), 5.20000029f);
}

/** @brief Test vfloat4 sub. */
TEST(SuiteVfloat4, vsub)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b(0.1f, 0.2f, 0.3f, 0.4f);
	a = a - b;
	EXPECT_EQ(a.lane<0>(), 1.0f - 0.1f);
	EXPECT_EQ(a.lane<1>(), 2.0f - 0.2f);
	EXPECT_EQ(a.lane<2>(), 3.0f - 0.3f);
	EXPECT_EQ(a.lane<3>(), 4.0f - 0.4f);
}

/** @brief Test vfloat4 mul. */
TEST(SuiteVfloat4, vmul)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b(0.1f, 0.2f, 0.3f, 0.4f);
	a = a * b;
	EXPECT_EQ(a.lane<0>(), 1.0f * 0.1f);
	EXPECT_EQ(a.lane<1>(), 2.0f * 0.2f);
	EXPECT_EQ(a.lane<2>(), 3.0f * 0.3f);
	EXPECT_EQ(a.lane<3>(), 4.0f * 0.4f);
}

/** @brief Test vfloat4 mul. */
TEST(SuiteVfloat4, vsmul)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	float b = 3.14f;
	a = a * b;
	EXPECT_EQ(a.lane<0>(), 1.0f * 3.14f);
	EXPECT_EQ(a.lane<1>(), 2.0f * 3.14f);
	EXPECT_EQ(a.lane<2>(), 3.0f * 3.14f);
	EXPECT_EQ(a.lane<3>(), 4.0f * 3.14f);
}

/** @brief Test vfloat4 mul. */
TEST(SuiteVfloat4, svmul)
{
	float a = 3.14f;
	vfloat4 b(1.0f, 2.0f, 3.0f, 4.0f);
	b = a * b;
	EXPECT_EQ(b.lane<0>(), 3.14f * 1.0f);
	EXPECT_EQ(b.lane<1>(), 3.14f * 2.0f);
	EXPECT_EQ(b.lane<2>(), 3.14f * 3.0f);
	EXPECT_EQ(b.lane<3>(), 3.14f * 4.0f);
}

/** @brief Test vfloat4 div. */
TEST(SuiteVfloat4, vdiv)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b(0.1f, 0.2f, 0.3f, 0.4f);
	a = a / b;
	EXPECT_EQ(a.lane<0>(), 1.0f / 0.1f);
	EXPECT_EQ(a.lane<1>(), 2.0f / 0.2f);
	EXPECT_EQ(a.lane<2>(), 3.0f / 0.3f);
	EXPECT_EQ(a.lane<3>(), 4.0f / 0.4f);
}

/** @brief Test vfloat4 div. */
TEST(SuiteVfloat4, vsdiv)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	float b = 0.3f;
	a = a / b;
	EXPECT_EQ(a.lane<0>(), 1.0f / 0.3f);
	EXPECT_EQ(a.lane<1>(), 2.0f / 0.3f);
	EXPECT_EQ(a.lane<2>(), 3.0f / 0.3f);
	EXPECT_EQ(a.lane<3>(), 4.0f / 0.3f);
}

/** @brief Test vfloat4 div. */
TEST(SuiteVfloat4, svdiv)
{
	float a = 3.0f;
	vfloat4 b(0.1f, 0.2f, 0.3f, 0.4f);
	b = a / b;
	EXPECT_EQ(b.lane<0>(), 3.0f / 0.1f);
	EXPECT_EQ(b.lane<1>(), 3.0f / 0.2f);
	EXPECT_EQ(b.lane<2>(), 3.0f / 0.3f);
	EXPECT_EQ(b.lane<3>(), 3.0f / 0.4f);
}

/** @brief Test vfloat4 ceq. */
TEST(SuiteVfloat4, ceq)
{
	vfloat4 a1(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b1(0.1f, 0.2f, 0.3f, 0.4f);
	vmask4 r1 = a1 == b1;
	EXPECT_EQ(0u, mask(r1));
	EXPECT_EQ(false, any(r1));
	EXPECT_EQ(false, all(r1));

	vfloat4 a2(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b2(1.0f, 0.2f, 0.3f, 0.4f);
	vmask4 r2 = a2 == b2;
	EXPECT_EQ(0x1u, mask(r2));
	EXPECT_EQ(true, any(r2));
	EXPECT_EQ(false, all(r2));

	vfloat4 a3(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b3(1.0f, 0.2f, 3.0f, 0.4f);
	vmask4 r3 = a3 == b3;
	EXPECT_EQ(0x5u, mask(r3));
	EXPECT_EQ(true, any(r3));
	EXPECT_EQ(false, all(r3));

	vfloat4 a4(1.0f, 2.0f, 3.0f, 4.0f);
	vmask4 r4 = a4 == a4;
	EXPECT_EQ(0xFu, mask(r4));
	EXPECT_EQ(true, any(r4));
	EXPECT_EQ(true, all(r4));
}

/** @brief Test vfloat4 cne. */
TEST(SuiteVfloat4, cne)
{
	vfloat4 a1(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b1(0.1f, 0.2f, 0.3f, 0.4f);
	vmask4 r1 = a1 != b1;
	EXPECT_EQ(0xFu, mask(r1));
	EXPECT_EQ(true, any(r1));
	EXPECT_EQ(true, all(r1));

	vfloat4 a2(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b2(1.0f, 0.2f, 0.3f, 0.4f);
	vmask4 r2 = a2 != b2;
	EXPECT_EQ(0xEu, mask(r2));
	EXPECT_EQ(true, any(r2));
	EXPECT_EQ(false, all(r2));

	vfloat4 a3(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b3(1.0f, 0.2f, 3.0f, 0.4f);
	vmask4 r3 = a3 != b3;
	EXPECT_EQ(0xAu, mask(r3));
	EXPECT_EQ(true, any(r3));
	EXPECT_EQ(false, all(r3));

	vfloat4 a4(1.0f, 2.0f, 3.0f, 4.0f);
	vmask4 r4 = a4 != a4;
	EXPECT_EQ(0u, mask(r4));
	EXPECT_EQ(false, any(r4));
	EXPECT_EQ(false, all(r4));
}

/** @brief Test vfloat4 clt. */
TEST(SuiteVfloat4, clt)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b(0.9f, 2.1f, 3.0f, 4.1f);
	vmask4 r = a < b;
	EXPECT_EQ(0xAu, mask(r));
}

/** @brief Test vfloat4 cle. */
TEST(SuiteVfloat4, cle)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b(0.9f, 2.1f, 3.0f, 4.1f);
	vmask4 r = a <= b;
	EXPECT_EQ(0xEu, mask(r));
}

/** @brief Test vfloat4 cgt. */
TEST(SuiteVfloat4, cgt)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b(0.9f, 2.1f, 3.0f, 4.1f);
	vmask4 r = a > b;
	EXPECT_EQ(0x1u, mask(r));
}

/** @brief Test vfloat4 cge. */
TEST(SuiteVfloat4, cge)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b(0.9f, 2.1f, 3.0f, 4.1f);
	vmask4 r = a >= b;
	EXPECT_EQ(0x5u, mask(r));
}

/** @brief Test vfloat4 min. */
TEST(SuiteVfloat4, min)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b(0.9f, 2.1f, 3.0f, 4.1f);
	vfloat4 r = min(a, b);
	EXPECT_EQ(r.lane<0>(), 0.9f);
	EXPECT_EQ(r.lane<1>(), 2.0f);
	EXPECT_EQ(r.lane<2>(), 3.0f);
	EXPECT_EQ(r.lane<3>(), 4.0f);

	float c = 0.3f;
	r = min(a, c);
	EXPECT_EQ(r.lane<0>(), 0.3f);
	EXPECT_EQ(r.lane<1>(), 0.3f);
	EXPECT_EQ(r.lane<2>(), 0.3f);
	EXPECT_EQ(r.lane<3>(), 0.3f);

	float d = 1.5f;
	r = min(a, d);
	EXPECT_EQ(r.lane<0>(), 1.0f);
	EXPECT_EQ(r.lane<1>(), 1.5f);
	EXPECT_EQ(r.lane<2>(), 1.5f);
	EXPECT_EQ(r.lane<3>(), 1.5f);
}

/** @brief Test vfloat4 max. */
TEST(SuiteVfloat4, max)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 b(0.9f, 2.1f, 3.0f, 4.1f);
	vfloat4 r = max(a, b);
	EXPECT_EQ(r.lane<0>(), 1.0f);
	EXPECT_EQ(r.lane<1>(), 2.1f);
	EXPECT_EQ(r.lane<2>(), 3.0f);
	EXPECT_EQ(r.lane<3>(), 4.1f);

	float c = 4.3f;
	r = max(a, c);
	EXPECT_EQ(r.lane<0>(), 4.3f);
	EXPECT_EQ(r.lane<1>(), 4.3f);
	EXPECT_EQ(r.lane<2>(), 4.3f);
	EXPECT_EQ(r.lane<3>(), 4.3f);

	float d = 1.5f;
	r = max(a, d);
	EXPECT_EQ(r.lane<0>(), 1.5f);
	EXPECT_EQ(r.lane<1>(), 2.0f);
	EXPECT_EQ(r.lane<2>(), 3.0f);
	EXPECT_EQ(r.lane<3>(), 4.0f);
}

/** @brief Test vfloat4 clamp. */
TEST(SuiteVfloat4, clamp)
{
	vfloat4 a1(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 r1 = clamp(2.1f, 3.0f, a1);
	EXPECT_EQ(r1.lane<0>(), 2.1f);
	EXPECT_EQ(r1.lane<1>(), 2.1f);
	EXPECT_EQ(r1.lane<2>(), 3.0f);
	EXPECT_EQ(r1.lane<3>(), 3.0f);

	vfloat4 a2(1.0f, 2.0f, qnan, 4.0f);
	vfloat4 r2 = clamp(2.1f, 3.0f, a2);
	EXPECT_EQ(r2.lane<0>(), 2.1f);
	EXPECT_EQ(r2.lane<1>(), 2.1f);
	EXPECT_EQ(r2.lane<2>(), 2.1f);
	EXPECT_EQ(r2.lane<3>(), 3.0f);
}

/** @brief Test vfloat4 clampz. */
TEST(SuiteVfloat4, clampzo)
{
	vfloat4 a1(-1.0f, 0.0f, 0.1f, 4.0f);
	vfloat4 r1 = clampzo(a1);
	EXPECT_EQ(r1.lane<0>(), 0.0f);
	EXPECT_EQ(r1.lane<1>(), 0.0f);
	EXPECT_EQ(r1.lane<2>(), 0.1f);
	EXPECT_EQ(r1.lane<3>(), 1.0f);

	vfloat4 a2(-1.0f, 0.0f, qnan, 4.0f);
	vfloat4 r2 = clampzo(a2);
	EXPECT_EQ(r2.lane<0>(), 0.0f);
	EXPECT_EQ(r2.lane<1>(), 0.0f);
	EXPECT_EQ(r2.lane<2>(), 0.0f);
	EXPECT_EQ(r2.lane<3>(), 1.0f);
}

/** @brief Test vfloat4 abs. */
TEST(SuiteVfloat4, abs)
{
	vfloat4 a(-1.0f, 0.0f, 0.1f, 4.0f);
	vfloat4 r = abs(a);
	EXPECT_EQ(r.lane<0>(), 1.0f);
	EXPECT_EQ(r.lane<1>(), 0.0f);
	EXPECT_EQ(r.lane<2>(), 0.1f);
	EXPECT_EQ(r.lane<3>(), 4.0f);
}

/** @brief Test vfloat4 round. */
TEST(SuiteVfloat4, round)
{
	vfloat4 a1(1.1f, 1.5f, 1.6f, 4.0f);
	vfloat4 r1 = round(a1);
	EXPECT_EQ(r1.lane<0>(), 1.0f);
	EXPECT_EQ(r1.lane<1>(), 2.0f);
	EXPECT_EQ(r1.lane<2>(), 2.0f);
	EXPECT_EQ(r1.lane<3>(), 4.0f);

	vfloat4 a2(-2.5f, -2.5f, -3.5f, -3.5f);
	vfloat4 r2 = round(a2);
	EXPECT_EQ(r2.lane<0>(), -2.0f);
	EXPECT_EQ(r2.lane<2>(), -4.0f);
}

/** @brief Test vfloat4 hmin. */
TEST(SuiteVfloat4, hmin)
{
	vfloat4 a1(1.1f, 1.5f, 1.6f, 4.0f);
	vfloat4 r1 = hmin(a1);
	EXPECT_EQ(r1.lane<0>(), 1.1f);
	EXPECT_EQ(r1.lane<1>(), 1.1f);
	EXPECT_EQ(r1.lane<2>(), 1.1f);
	EXPECT_EQ(r1.lane<3>(), 1.1f);

	vfloat4 a2(1.1f, 1.5f, 1.6f, 0.2f);
	vfloat4 r2 = hmin(a2);
	EXPECT_EQ(r2.lane<0>(), 0.2f);
	EXPECT_EQ(r2.lane<1>(), 0.2f);
	EXPECT_EQ(r2.lane<2>(), 0.2f);
	EXPECT_EQ(r2.lane<3>(), 0.2f);
}

/** @brief Test vfloat4 hmin_s. */
TEST(SuiteVfloat4, hmin_s)
{
	vfloat4 a1(1.1f, 1.5f, 1.6f, 4.0f);
	float r1 = hmin_s(a1);
	EXPECT_EQ(r1, 1.1f);

	vfloat4 a2(1.1f, 1.5f, 1.6f, 0.2f);
	float r2 = hmin_s(a2);
	EXPECT_EQ(r2, 0.2f);
}

/** @brief Test vfloat4 hmin_rgb_s. */
TEST(SuiteVfloat4, hmin_rgb_s)
{
	vfloat4 a1(1.1f, 1.5f, 1.6f, 0.2f);
	float r1 = hmin_rgb_s(a1);
	EXPECT_EQ(r1, 1.1f);

	vfloat4 a2(1.5f, 0.9f, 1.6f, 1.2f);
	float r2 = hmin_rgb_s(a2);
	EXPECT_EQ(r2, 0.9f);
}

/** @brief Test vfloat4 hmax. */
TEST(SuiteVfloat4, hmax)
{
	vfloat4 a1(1.1f, 1.5f, 1.6f, 4.0f);
	vfloat4 r1 = hmax(a1);
	EXPECT_EQ(r1.lane<0>(), 4.0f);
	EXPECT_EQ(r1.lane<1>(), 4.0f);
	EXPECT_EQ(r1.lane<2>(), 4.0f);
	EXPECT_EQ(r1.lane<3>(), 4.0f);

	vfloat4 a2(1.1f, 1.5f, 1.6f, 0.2f);
	vfloat4 r2 = hmax(a2);
	EXPECT_EQ(r2.lane<0>(), 1.6f);
	EXPECT_EQ(r2.lane<1>(), 1.6f);
	EXPECT_EQ(r2.lane<2>(), 1.6f);
	EXPECT_EQ(r2.lane<3>(), 1.6f);
}

/** @brief Test vfloat4 hmax_s. */
TEST(SuiteVfloat4, hmax_s)
{
	vfloat4 a1(1.1f, 1.5f, 1.6f, 4.0f);
	float r1 = hmax_s(a1);
	EXPECT_EQ(r1, 4.0f);

	vfloat4 a2(1.1f, 1.5f, 1.6f, 0.2f);
	float r2 = hmax_s(a2);
	EXPECT_EQ(r2, 1.6f);
}

/** @brief Test vfloat4 hadd_s. */
TEST(SuiteVfloat4, hadd_s)
{
	vfloat4 a1(1.1f, 1.5f, 1.6f, 4.0f);
	float sum = (1.1f + 1.6f) + (1.5f + 4.0f);
	float r = hadd_s(a1);
	EXPECT_EQ(r, sum);
}

/** @brief Test vfloat4 hadd_rgb_s. */
TEST(SuiteVfloat4, hadd_rgb_s)
{
	vfloat4 a1(1.1f, 1.5f, 1.6f, 4.0f);
	float sum = 1.1f + 1.5f + 1.6f;
	float r = hadd_rgb_s(a1);
	EXPECT_EQ(r, sum);
}

/** @brief Test vfloat4 sqrt. */
TEST(SuiteVfloat4, sqrt)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 r = sqrt(a);
	EXPECT_EQ(r.lane<0>(), std::sqrt(1.0f));
	EXPECT_EQ(r.lane<1>(), std::sqrt(2.0f));
	EXPECT_EQ(r.lane<2>(), std::sqrt(3.0f));
	EXPECT_EQ(r.lane<3>(), std::sqrt(4.0f));
}

/** @brief Test vfloat4 select. */
TEST(SuiteVfloat4, select)
{
	vfloat4 m1(1.0f, 1.0f, 1.0f, 1.0f);
	vfloat4 m2(1.0f, 2.0f, 1.0f, 2.0f);
	vmask4 cond = m1 == m2;

	vfloat4 a(1.0f, 3.0f, 3.0f, 1.0f);
	vfloat4 b(4.0f, 2.0f, 2.0f, 4.0f);

	// Select in one direction
	vfloat4 r1 = select(a, b, cond);
	EXPECT_EQ(r1.lane<0>(), 4.0f);
	EXPECT_EQ(r1.lane<1>(), 3.0f);
	EXPECT_EQ(r1.lane<2>(), 2.0f);
	EXPECT_EQ(r1.lane<3>(), 1.0f);

	// Select in the other
	vfloat4 r2 = select(b, a, cond);
	EXPECT_EQ(r2.lane<0>(), 1.0f);
	EXPECT_EQ(r2.lane<1>(), 2.0f);
	EXPECT_EQ(r2.lane<2>(), 3.0f);
	EXPECT_EQ(r2.lane<3>(), 4.0f);
}

/** @brief Test vfloat4 gatherf. */
TEST(SuiteVfloat4, gatherf)
{
	vint4 indices(0, 4, 3, 2);
	vfloat4 r = gatherf(f32_data, indices);
	EXPECT_EQ(r.lane<0>(), 0.0f);
	EXPECT_EQ(r.lane<1>(), 4.0f);
	EXPECT_EQ(r.lane<2>(), 3.0f);
	EXPECT_EQ(r.lane<3>(), 2.0f);
}

/** @brief Test vfloat4 storea. */
TEST(SuiteVfloat4, storea)
{
	ASTCENC_ALIGNAS float out[4];
	vfloat4 a(f32_data);
	storea(a, out);
	EXPECT_EQ(out[0], 0.0f);
	EXPECT_EQ(out[1], 1.0f);
	EXPECT_EQ(out[2], 2.0f);
	EXPECT_EQ(out[3], 3.0f);
}

/** @brief Test vfloat4 store. */
TEST(SuiteVfloat4, store)
{
	ASTCENC_ALIGNAS float out[5];
	vfloat4 a(f32_data);
	store(a, &(out[1]));
	EXPECT_EQ(out[1], 0.0f);
	EXPECT_EQ(out[2], 1.0f);
	EXPECT_EQ(out[3], 2.0f);
	EXPECT_EQ(out[4], 3.0f);
}

/** @brief Test vfloat4 dot. */
TEST(SuiteVfloat4, dot)
{
	vfloat4 a1(1.0f, 2.0f, 4.0f, 8.0f);
	vfloat4 b1(1.0f, 0.5f, 0.25f, 0.125f);
	vfloat4 r1 = dot(a1, b1);
	EXPECT_EQ(r1.lane<0>(), 4.0f);
	EXPECT_EQ(r1.lane<1>(), 4.0f);
	EXPECT_EQ(r1.lane<2>(), 4.0f);
	EXPECT_EQ(r1.lane<3>(), 4.0f);

	// These values will fail to add to the same value if reassociated
	float l0 =          141.2540435791015625f;
	float l1 =      5345345.5000000000000000f;
	float l2 =       234234.7031250000000000f;
	float l3 = 124353454080.0000000000000000f;

	vfloat4 a2(1.0f, 1.0f, 1.0f, 1.0f);
	vfloat4 b2(l0, l1, l2, l3);
	vfloat4 r2 = dot(a2, b2);

	// Test that reassociation causes a failure with the numbers we chose
	EXPECT_FALSE(any(r2 == vfloat4(l0 + l1 + l2 + l3)));

	// Test that the sum works, for the association pattern we want used
	EXPECT_TRUE(all(r2 == vfloat4((l0 + l2) + (l1 + l3))));
}

/** @brief Test vfloat4 dot_s. */
TEST(SuiteVfloat4, dot_s)
{
	vfloat4 a1(1.0f, 2.0f, 4.0f, 8.0f);
	vfloat4 b1(1.0f, 0.5f, 0.25f, 0.125f);
	float r1 = dot_s(a1, b1);
	EXPECT_EQ(r1, 4.0f);

	// These values will fail to add to the same value if reassociated
	float l0 =          141.2540435791015625f;
	float l1 =      5345345.5000000000000000f;
	float l2 =       234234.7031250000000000f;
	float l3 = 124353454080.0000000000000000f;

	vfloat4 a2(1.0f, 1.0f, 1.0f, 1.0f);
	vfloat4 b2(l0, l1, l2, l3);
	float r2 = dot_s(a2, b2);

	// Test that reassociation causes a failure with the numbers we chose
	EXPECT_NE(r2, l0 + l1 + l2 + l3);

	// Test that the sum works, for the association pattern we want used
	EXPECT_EQ(r2, (l0 + l2) + (l1 + l3));
}

/** @brief Test vfloat4 dot3. */
TEST(SuiteVfloat4, dot3)
{
	vfloat4 a(1.0f, 2.0f, 4.0f, 8.0f);
	vfloat4 b(1.0f, 0.5f, 0.25f, 0.125f);
	vfloat4 r = dot3(a, b);
	EXPECT_EQ(r.lane<0>(), 3.0f);
	EXPECT_EQ(r.lane<1>(), 3.0f);
	EXPECT_EQ(r.lane<2>(), 3.0f);
	EXPECT_EQ(r.lane<3>(), 0.0f);
}

/** @brief Test vfloat4 dot3_s. */
TEST(SuiteVfloat4, dot3_s)
{
	vfloat4 a(1.0f, 2.0f, 4.0f, 8.0f);
	vfloat4 b(1.0f, 0.5f, 0.25f, 0.125f);
	float r = dot3_s(a, b);
	EXPECT_EQ(r, 3.0f);
}

/** @brief Test vfloat4 normalize. */
TEST(SuiteVfloat4, normalize)
{
	vfloat4 a(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 r = normalize(a);
	EXPECT_EQ(r.lane<0>(), 1.0f / astc::sqrt(30.0f));
	EXPECT_EQ(r.lane<1>(), 2.0f / astc::sqrt(30.0f));
	EXPECT_EQ(r.lane<2>(), 3.0f / astc::sqrt(30.0f));
	EXPECT_EQ(r.lane<3>(), 4.0f / astc::sqrt(30.0f));
}

/** @brief Test vfloat4 normalize_safe. */
TEST(SuiteVfloat4, normalize_safe)
{
	vfloat4 s(-1.0f, -1.0f, -1.0f, -1.0f);

	vfloat4 a1(1.0f, 2.0f, 3.0f, 4.0f);
	vfloat4 r1 = normalize_safe(a1, s);
	EXPECT_EQ(r1.lane<0>(), 1.0f / astc::sqrt(30.0f));
	EXPECT_EQ(r1.lane<1>(), 2.0f / astc::sqrt(30.0f));
	EXPECT_EQ(r1.lane<2>(), 3.0f / astc::sqrt(30.0f));
	EXPECT_EQ(r1.lane<3>(), 4.0f / astc::sqrt(30.0f));

	vfloat4 a2(0.0f, 0.0f, 0.0f, 0.0f);
	vfloat4 r2 = normalize_safe(a2, s);
	EXPECT_EQ(r2.lane<0>(), -1.0f);
	EXPECT_EQ(r2.lane<1>(), -1.0f);
	EXPECT_EQ(r2.lane<2>(), -1.0f);
	EXPECT_EQ(r2.lane<3>(), -1.0f);
}

/** @brief Test vfloat4 float_to_int. */
TEST(SuiteVfloat4, float_to_int)
{
	vfloat4 a(1.1f, 1.5f, -1.6f, 4.0f);
	vint4 r = float_to_int(a);
	EXPECT_EQ(r.lane<0>(), 1);
	EXPECT_EQ(r.lane<1>(), 1);
	EXPECT_EQ(r.lane<2>(), -1);
	EXPECT_EQ(r.lane<3>(), 4);
}

/** @brief Test vfloat4 round. */
TEST(SuiteVfloat4, float_to_int_rtn)
{
	vfloat4 a(1.1f, 1.5f, 1.6f, 4.0f);
	vint4 r = float_to_int_rtn(a);
	EXPECT_EQ(r.lane<0>(), 1);
	EXPECT_EQ(r.lane<1>(), 2);
	EXPECT_EQ(r.lane<2>(), 2);
	EXPECT_EQ(r.lane<3>(), 4);
}

/** @brief Test vfloat4 round. */
TEST(SuiteVfloat4, int_to_float)
{
	vint4 a(1, 2, 3, 4);
	vfloat4 r = int_to_float(a);
	EXPECT_EQ(r.lane<0>(), 1.0f);
	EXPECT_EQ(r.lane<1>(), 2.0f);
	EXPECT_EQ(r.lane<2>(), 3.0f);
	EXPECT_EQ(r.lane<3>(), 4.0f);
}

/** @brief Test vfloat4 float to fp16 conversion. */
TEST(SuiteVfloat4, float_to_float16)
{
	vfloat4 a(1.5, 234.5, 345345.0, qnan);
	vint4 r = float_to_float16(a);

	// Normal numbers
	EXPECT_EQ(r.lane<0>(), 0x3E00);
	EXPECT_EQ(r.lane<1>(), 0x5B54);

	// Large numbers convert to infinity
	EXPECT_EQ(r.lane<2>(), 0x7C00);

	// NaN must convert to any valid NaN encoding
	EXPECT_EQ((r.lane<3>() >> 10) & 0x1F, 0x1F); // Exponent must be all 1s
	EXPECT_NE(r.lane<3>() & (0x3FF), 0);         // Mantissa must be non-zero
}

/** @brief Test float to fp16 conversion. */
TEST(sfloat, float_to_float16)
{
	int r = float_to_float16(234.5);
	EXPECT_EQ(r, 0x5B54);
}

/** @brief Test vfloat4 fp16 to float conversion. */
TEST(SuiteVfloat4, float16_to_float)
{	vint4 a(0x3E00, 0x5B54, 0x7C00, 0xFFFF);
	vfloat4 r = float16_to_float(a);

	// Normal numbers
	EXPECT_EQ(r.lane<0>(), 1.5);
	EXPECT_EQ(r.lane<1>(), 234.5);

	// Infinities must be preserved
	EXPECT_NE(std::isinf(r.lane<2>()), 0);

	// NaNs must be preserved
	EXPECT_NE(std::isnan(r.lane<3>()), 0);
}

/** @brief Test fp16 to float conversion. */
TEST(sfloat, float16_to_float)
{
	float r = float16_to_float(0x5B54);
	EXPECT_EQ(r, 234.5);
}

// VINT4 tests - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** @brief Test unaligned vint4 data load. */
TEST(SuiteVint4, UnalignedLoad)
{
	vint4 a(&(s32_data[1]));
	EXPECT_EQ(a.lane<0>(), 1);
	EXPECT_EQ(a.lane<1>(), 2);
	EXPECT_EQ(a.lane<2>(), 3);
	EXPECT_EQ(a.lane<3>(), 4);
}

/** @brief Test unaligned vint4 data load. */
TEST(SuiteVint4, UnalignedLoad8)
{
	vint4 a(&(u8_data[1]));
	EXPECT_EQ(a.lane<0>(), 1);
	EXPECT_EQ(a.lane<1>(), 2);
	EXPECT_EQ(a.lane<2>(), 3);
	EXPECT_EQ(a.lane<3>(), 4);
}

/** @brief Test scalar duplicated vint4 load. */
TEST(SuiteVint4, ScalarDupLoad)
{
	vint4 a(42);
	EXPECT_EQ(a.lane<0>(), 42);
	EXPECT_EQ(a.lane<1>(), 42);
	EXPECT_EQ(a.lane<2>(), 42);
	EXPECT_EQ(a.lane<3>(), 42);
}

/** @brief Test scalar vint4 load. */
TEST(SuiteVint4, ScalarLoad)
{
	vint4 a(11, 22, 33, 44);
	EXPECT_EQ(a.lane<0>(), 11);
	EXPECT_EQ(a.lane<1>(), 22);
	EXPECT_EQ(a.lane<2>(), 33);
	EXPECT_EQ(a.lane<3>(), 44);
}

/** @brief Test copy vint4 load. */
TEST(SuiteVint4, CopyLoad)
{
	vint4 s(11, 22, 33, 44);
	vint4 a(s.m);
	EXPECT_EQ(a.lane<0>(), 11);
	EXPECT_EQ(a.lane<1>(), 22);
	EXPECT_EQ(a.lane<2>(), 33);
	EXPECT_EQ(a.lane<3>(), 44);
}

/** @brief Test vint4 scalar lane set. */
TEST(int4, SetLane)
{
	vint4 a(0);

	a.set_lane<0>(1);
	EXPECT_EQ(a.lane<0>(), 1);
	EXPECT_EQ(a.lane<1>(), 0);
	EXPECT_EQ(a.lane<2>(), 0);
	EXPECT_EQ(a.lane<3>(), 0);

	a.set_lane<1>(2);
	EXPECT_EQ(a.lane<0>(), 1);
	EXPECT_EQ(a.lane<1>(), 2);
	EXPECT_EQ(a.lane<2>(), 0);
	EXPECT_EQ(a.lane<3>(), 0);

	a.set_lane<2>(3);
	EXPECT_EQ(a.lane<0>(), 1);
	EXPECT_EQ(a.lane<1>(), 2);
	EXPECT_EQ(a.lane<2>(), 3);
	EXPECT_EQ(a.lane<3>(), 0);

	a.set_lane<3>(4);
	EXPECT_EQ(a.lane<0>(), 1);
	EXPECT_EQ(a.lane<1>(), 2);
	EXPECT_EQ(a.lane<2>(), 3);
	EXPECT_EQ(a.lane<3>(), 4);
}

/** @brief Test vint4 zero. */
TEST(SuiteVint4, Zero)
{
	vint4 a = vint4::zero();
	EXPECT_EQ(a.lane<0>(), 0);
	EXPECT_EQ(a.lane<1>(), 0);
	EXPECT_EQ(a.lane<2>(), 0);
	EXPECT_EQ(a.lane<3>(), 0);
}

/** @brief Test vint4 load1. */
TEST(SuiteVint4, Load1)
{
	int s = 42;
	vint4 a = vint4::load1(&s);
	EXPECT_EQ(a.lane<0>(), 42);
	EXPECT_EQ(a.lane<1>(), 42);
	EXPECT_EQ(a.lane<2>(), 42);
	EXPECT_EQ(a.lane<3>(), 42);
}

/** @brief Test vint4 loada. */
TEST(SuiteVint4, Loada)
{
	vint4 a = vint4::loada(&(s32_data[0]));
	EXPECT_EQ(a.lane<0>(), 0);
	EXPECT_EQ(a.lane<1>(), 1);
	EXPECT_EQ(a.lane<2>(), 2);
	EXPECT_EQ(a.lane<3>(), 3);
}

/** @brief Test vint4 lane_id. */
TEST(SuiteVint4, LaneID)
{
	vint4 a = vint4::lane_id();
	EXPECT_EQ(a.lane<0>(), 0);
	EXPECT_EQ(a.lane<1>(), 1);
	EXPECT_EQ(a.lane<2>(), 2);
	EXPECT_EQ(a.lane<3>(), 3);
}

/** @brief Test vint4 add. */
TEST(SuiteVint4, vadd)
{
	vint4 a(1, 2, 3, 4);
	vint4 b(2, 3, 4, 5);
	a = a + b;
	EXPECT_EQ(a.lane<0>(), 1 + 2);
	EXPECT_EQ(a.lane<1>(), 2 + 3);
	EXPECT_EQ(a.lane<2>(), 3 + 4);
	EXPECT_EQ(a.lane<3>(), 4 + 5);
}

/** @brief Test vint4 self-add. */
TEST(SuiteVint4, vselfadd)
{
	vint4 a(1, 2, 3, 4);
	vint4 b(2, 3, 4, 5);
	a += b;

	EXPECT_EQ(a.lane<0>(), 1 + 2);
	EXPECT_EQ(a.lane<1>(), 2 + 3);
	EXPECT_EQ(a.lane<2>(), 3 + 4);
	EXPECT_EQ(a.lane<3>(), 4 + 5);
}

/** @brief Test vint4 add. */
TEST(SuiteVint4, vsadd)
{
	vint4 a(1, 2, 3, 4);
	int b = 5;
	a = a + b;
	EXPECT_EQ(a.lane<0>(), 1 + 5);
	EXPECT_EQ(a.lane<1>(), 2 + 5);
	EXPECT_EQ(a.lane<2>(), 3 + 5);
	EXPECT_EQ(a.lane<3>(), 4 + 5);
}

/** @brief Test vint4 sub. */
TEST(SuiteVint4, vsub)
{
	vint4 a(1, 2, 4, 4);
	vint4 b(2, 3, 3, 5);
	a = a - b;
	EXPECT_EQ(a.lane<0>(), 1 - 2);
	EXPECT_EQ(a.lane<1>(), 2 - 3);
	EXPECT_EQ(a.lane<2>(), 4 - 3);
	EXPECT_EQ(a.lane<3>(), 4 - 5);
}

/** @brief Test vint4 sub. */
TEST(SuiteVint4, vssub)
{
	vint4 a(1, 2, 4, 4);
	int b = 5;
	a = a - b;
	EXPECT_EQ(a.lane<0>(), 1 - 5);
	EXPECT_EQ(a.lane<1>(), 2 - 5);
	EXPECT_EQ(a.lane<2>(), 4 - 5);
	EXPECT_EQ(a.lane<3>(), 4 - 5);
}

/** @brief Test vint4 mul. */
TEST(SuiteVint4, vmul)
{
	vint4 a(1, 2, 4, 4);
	vint4 b(2, 3, 3, 5);
	a = a * b;
	EXPECT_EQ(a.lane<0>(), 1 * 2);
	EXPECT_EQ(a.lane<1>(), 2 * 3);
	EXPECT_EQ(a.lane<2>(), 4 * 3);
	EXPECT_EQ(a.lane<3>(), 4 * 5);
}

/** @brief Test vint4 mul. */
TEST(SuiteVint4, vsmul)
{
	vint4 a(1, 2, 4, 4);
	a = a * 3;
	EXPECT_EQ(a.lane<0>(), 1 * 3);
	EXPECT_EQ(a.lane<1>(), 2 * 3);
	EXPECT_EQ(a.lane<2>(), 4 * 3);
	EXPECT_EQ(a.lane<3>(), 4 * 3);

	vint4 b(1, 2, -4, 4);
	b = b * -3;
	EXPECT_EQ(b.lane<0>(), 1 * -3);
	EXPECT_EQ(b.lane<1>(), 2 * -3);
	EXPECT_EQ(b.lane<2>(), -4 * -3);
	EXPECT_EQ(b.lane<3>(), 4 * -3);
}

/** @brief Test vint4 bitwise invert. */
TEST(SuiteVint4, bit_invert)
{
	vint4 a(-1, 0, 1, 2);
	a = ~a;
	EXPECT_EQ(a.lane<0>(), ~-1);
	EXPECT_EQ(a.lane<1>(), ~0);
	EXPECT_EQ(a.lane<2>(), ~1);
	EXPECT_EQ(a.lane<3>(), ~2);
}

/** @brief Test vint4 bitwise or. */
TEST(SuiteVint4, bit_vor)
{
	vint4 a(1, 2, 3, 4);
	vint4 b(2, 3, 4, 5);
	a = a | b;
	EXPECT_EQ(a.lane<0>(), 3);
	EXPECT_EQ(a.lane<1>(), 3);
	EXPECT_EQ(a.lane<2>(), 7);
	EXPECT_EQ(a.lane<3>(), 5);
}

TEST(SuiteVint4, bit_vsor)
{
	vint4 a(1, 2, 3, 4);
	int b = 2;
	a = a | b;
	EXPECT_EQ(a.lane<0>(), 3);
	EXPECT_EQ(a.lane<1>(), 2);
	EXPECT_EQ(a.lane<2>(), 3);
	EXPECT_EQ(a.lane<3>(), 6);
}

/** @brief Test vint4 bitwise and. */
TEST(SuiteVint4, bit_vand)
{
	vint4 a(1, 2, 3, 4);
	vint4 b(2, 3, 4, 5);
	a = a & b;
	EXPECT_EQ(a.lane<0>(), 0);
	EXPECT_EQ(a.lane<1>(), 2);
	EXPECT_EQ(a.lane<2>(), 0);
	EXPECT_EQ(a.lane<3>(), 4);
}

/** @brief Test vint4 bitwise and. */
TEST(SuiteVint4, bit_vsand)
{
	vint4 a(1, 2, 3, 4);
	int b = 2;
	a = a & b;
	EXPECT_EQ(a.lane<0>(), 0);
	EXPECT_EQ(a.lane<1>(), 2);
	EXPECT_EQ(a.lane<2>(), 2);
	EXPECT_EQ(a.lane<3>(), 0);
}

/** @brief Test vint4 bitwise xor. */
TEST(SuiteVint4, bit_vxor)
{
	vint4 a(1, 2, 3, 4);
	vint4 b(2, 3, 4, 5);
	a = a ^ b;
	EXPECT_EQ(a.lane<0>(), 3);
	EXPECT_EQ(a.lane<1>(), 1);
	EXPECT_EQ(a.lane<2>(), 7);
	EXPECT_EQ(a.lane<3>(), 1);
}

/** @brief Test vint4 bitwise xor. */
TEST(SuiteVint4, bit_vsxor)
{
	vint4 a(1, 2, 3, 4);
	int b = 2;
	a = a ^ b;
	EXPECT_EQ(a.lane<0>(), 3);
	EXPECT_EQ(a.lane<1>(), 0);
	EXPECT_EQ(a.lane<2>(), 1);
	EXPECT_EQ(a.lane<3>(), 6);
}

/** @brief Test vint4 ceq. */
TEST(SuiteVint4, ceq)
{
	vint4 a1(1, 2, 3, 4);
	vint4 b1(0, 1, 2, 3);
	vmask4 r1 = a1 == b1;
	EXPECT_EQ(0u, mask(r1));
	EXPECT_EQ(false, any(r1));
	EXPECT_EQ(false, all(r1));

	vint4 a2(1, 2, 3, 4);
	vint4 b2(1, 0, 0, 0);
	vmask4 r2 = a2 == b2;
	EXPECT_EQ(0x1u, mask(r2));
	EXPECT_EQ(true, any(r2));
	EXPECT_EQ(false, all(r2));

	vint4 a3(1, 2, 3, 4);
	vint4 b3(1, 0, 3, 0);
	vmask4 r3 = a3 == b3;
	EXPECT_EQ(0x5u, mask(r3));
	EXPECT_EQ(true, any(r3));
	EXPECT_EQ(false, all(r3));

	vint4 a4(1, 2, 3, 4);
	vmask4 r4 = a4 == a4;
	EXPECT_EQ(0xFu, mask(r4));
	EXPECT_EQ(true, any(r4));
	EXPECT_EQ(true, all(r4));
}

/** @brief Test vint4 cne. */
TEST(SuiteVint4, cne)
{
	vint4 a1(1, 2, 3, 4);
	vint4 b1(0, 1, 2, 3);
	vmask4 r1 = a1 != b1;
	EXPECT_EQ(0xFu, mask(r1));
	EXPECT_EQ(true, any(r1));
	EXPECT_EQ(true, all(r1));

	vint4 a2(1, 2, 3, 4);
	vint4 b2(1, 0, 0, 0);
	vmask4 r2 = a2 != b2;
	EXPECT_EQ(0xEu, mask(r2));
	EXPECT_EQ(true, any(r2));
	EXPECT_EQ(false, all(r2));

	vint4 a3(1, 2, 3, 4);
	vint4 b3(1, 0, 3, 0);
	vmask4 r3 = a3 != b3;
	EXPECT_EQ(0xAu, mask(r3));
	EXPECT_EQ(true, any(r3));
	EXPECT_EQ(false, all(r3));

	vint4 a4(1, 2, 3, 4);
	vmask4 r4 = a4 != a4;
	EXPECT_EQ(0u, mask(r4));
	EXPECT_EQ(false, any(r4));
	EXPECT_EQ(false, all(r4));
}

/** @brief Test vint4 clt. */
TEST(SuiteVint4, clt)
{
	vint4 a(1, 2, 3, 4);
	vint4 b(0, 3, 3, 5);
	vmask4 r = a < b;
	EXPECT_EQ(0xAu, mask(r));
}

/** @brief Test vint4 cgt. */
TEST(SuiteVint4, cle)
{
	vint4 a(1, 2, 3, 4);
	vint4 b(0, 3, 3, 5);
	vmask4 r = a > b;
	EXPECT_EQ(0x1u, mask(r));
}

/** @brief Test vint4 lsl. */
TEST(SuiteVint4, lsl)
{
	vint4 a(1, 2, 4, 4);
	a = lsl<0>(a);
	EXPECT_EQ(a.lane<0>(), 1);
	EXPECT_EQ(a.lane<1>(), 2);
	EXPECT_EQ(a.lane<2>(), 4);
	EXPECT_EQ(a.lane<3>(), 4);

	a = lsl<1>(a);
	EXPECT_EQ(a.lane<0>(), 2);
	EXPECT_EQ(a.lane<1>(), 4);
	EXPECT_EQ(a.lane<2>(), 8);
	EXPECT_EQ(a.lane<3>(), 8);

	a = lsl<2>(a);
	EXPECT_EQ(a.lane<0>(), 8);
	EXPECT_EQ(a.lane<1>(), 16);
	EXPECT_EQ(a.lane<2>(), 32);
	EXPECT_EQ(a.lane<3>(), 32);
}

/** @brief Test vint4 lsr. */
TEST(SuiteVint4, lsr)
{
	vint4 a(1, 2, 4, -4);
	a = lsr<0>(a);
	EXPECT_EQ(a.lane<0>(),  1);
	EXPECT_EQ(a.lane<1>(),  2);
	EXPECT_EQ(a.lane<2>(),  4);
	EXPECT_EQ(a.lane<3>(),  static_cast<int>(0xFFFFFFFC));

	a = lsr<1>(a);
	EXPECT_EQ(a.lane<0>(),  0);
	EXPECT_EQ(a.lane<1>(),  1);
	EXPECT_EQ(a.lane<2>(),  2);
	EXPECT_EQ(a.lane<3>(),  0x7FFFFFFE);

	a = lsr<2>(a);
	EXPECT_EQ(a.lane<0>(),  0);
	EXPECT_EQ(a.lane<1>(),  0);
	EXPECT_EQ(a.lane<2>(),  0);
	EXPECT_EQ(a.lane<3>(),  0x1FFFFFFF);
}

/** @brief Test vint4 asr. */
TEST(SuiteVint4, asr)
{
	vint4 a(1, 2, 4, -4);
	a = asr<0>(a);
	EXPECT_EQ(a.lane<0>(),  1);
	EXPECT_EQ(a.lane<1>(),  2);
	EXPECT_EQ(a.lane<2>(),  4);
	EXPECT_EQ(a.lane<3>(), -4);

	a = asr<1>(a);
	EXPECT_EQ(a.lane<0>(),  0);
	EXPECT_EQ(a.lane<1>(),  1);
	EXPECT_EQ(a.lane<2>(),  2);
	EXPECT_EQ(a.lane<3>(), -2);

	// Note - quirk of asr is that you will get "stuck" at -1
	a = asr<2>(a);
	EXPECT_EQ(a.lane<0>(),  0);
	EXPECT_EQ(a.lane<1>(),  0);
	EXPECT_EQ(a.lane<2>(),  0);
	EXPECT_EQ(a.lane<3>(), -1);
}

/** @brief Test vint4 min. */
TEST(SuiteVint4, min)
{
	vint4 a(1, 2, 3, 4);
	vint4 b(0, 3, 3, 5);
	vint4 r = min(a, b);
	EXPECT_EQ(r.lane<0>(), 0);
	EXPECT_EQ(r.lane<1>(), 2);
	EXPECT_EQ(r.lane<2>(), 3);
	EXPECT_EQ(r.lane<3>(), 4);
}

/** @brief Test vint4 max. */
TEST(SuiteVint4, max)
{
	vint4 a(1, 2, 3, 4);
	vint4 b(0, 3, 3, 5);
	vint4 r = max(a, b);
	EXPECT_EQ(r.lane<0>(), 1);
	EXPECT_EQ(r.lane<1>(), 3);
	EXPECT_EQ(r.lane<2>(), 3);
	EXPECT_EQ(r.lane<3>(), 5);
}

/** @brief Test vint4 clamp. */
TEST(SuiteVint4, clamp)
{
	vint4 a(1, 2, 3, 4);
	vint4 r = clamp(2, 3, a);
	EXPECT_EQ(r.lane<0>(), 2);
	EXPECT_EQ(r.lane<1>(), 2);
	EXPECT_EQ(r.lane<2>(), 3);
	EXPECT_EQ(r.lane<3>(), 3);
}

/** @brief Test vint4 hmin. */
TEST(SuiteVint4, hmin)
{
	vint4 a1(1, 2, 1, 2);
	vint4 r1 = hmin(a1);
	EXPECT_EQ(r1.lane<0>(), 1);
	EXPECT_EQ(r1.lane<1>(), 1);
	EXPECT_EQ(r1.lane<2>(), 1);
	EXPECT_EQ(r1.lane<3>(), 1);

	vint4 a2(1, 2, -1, 5);
	vint4 r2 = hmin(a2);
	EXPECT_EQ(r2.lane<0>(), -1);
	EXPECT_EQ(r2.lane<1>(), -1);
	EXPECT_EQ(r2.lane<2>(), -1);
	EXPECT_EQ(r2.lane<3>(), -1);
}

/** @brief Test vint4 hmax. */
TEST(SuiteVint4, hmax)
{
	vint4 a1(1, 3, 1, 2);
	vint4 r1 = hmax(a1);
	EXPECT_EQ(r1.lane<0>(), 3);
	EXPECT_EQ(r1.lane<1>(), 3);
	EXPECT_EQ(r1.lane<2>(), 3);
	EXPECT_EQ(r1.lane<3>(), 3);

	vint4 a2(1, 2, -1, 5);
	vint4 r2 = hmax(a2);
	EXPECT_EQ(r2.lane<0>(), 5);
	EXPECT_EQ(r2.lane<1>(), 5);
	EXPECT_EQ(r2.lane<2>(), 5);
	EXPECT_EQ(r2.lane<3>(), 5);
}

/** @brief Test vint4 hadd_rgb_s. */
TEST(SuiteVint4, hadd_rgb_s)
{
	vint4 a1(1, 3, 5, 7);
	int r1 = hadd_rgb_s(a1);
	EXPECT_EQ(r1, 9);

	vint4 a2(1, 2, -1, 5);
	int r2 = hadd_rgb_s(a2);
	EXPECT_EQ(r2, 2);
}

/** @brief Test vint4 clz. */
TEST(SuiteVint4, clz)
{
	int msb_set = static_cast<int>(0x80000000);
	vint4 a1(msb_set, 0x40000000, 0x20000000, 0x10000000);
	vint4 r1 = clz(a1);
	EXPECT_EQ(r1.lane<0>(), 0);
	EXPECT_EQ(r1.lane<1>(), 1);
	EXPECT_EQ(r1.lane<2>(), 2);
	EXPECT_EQ(r1.lane<3>(), 3);

	vint4 a2(0x0, 0x1, 0x2, 0x4);
	vint4 r2 = clz(a2);
	EXPECT_EQ(r2.lane<0>(), 32);
	EXPECT_EQ(r2.lane<1>(), 31);
	EXPECT_EQ(r2.lane<2>(), 30);
	EXPECT_EQ(r2.lane<3>(), 29);
}

/** @brief Test vint4 two_to_the_n. */
TEST(SuiteVint4, two_to_the_n)
{
	vint4 a1(0, 1, 2, 3);
	vint4 r1 = two_to_the_n(a1);
	EXPECT_EQ(r1.lane<0>(), 1 << 0);
	EXPECT_EQ(r1.lane<1>(), 1 << 1);
	EXPECT_EQ(r1.lane<2>(), 1 << 2);
	EXPECT_EQ(r1.lane<3>(), 1 << 3);

	vint4 a2(27, 28, 29, 30);
	vint4 r2 = two_to_the_n(a2);
	EXPECT_EQ(r2.lane<0>(), 1 << 27);
	EXPECT_EQ(r2.lane<1>(), 1 << 28);
	EXPECT_EQ(r2.lane<2>(), 1 << 29);
	EXPECT_EQ(r2.lane<3>(), 1 << 30);

	// Shifts higher than 30 are not allowed as it overflows the int type;
	// and results in implementation-defined behavior because of how we
	// generate the shifted result in two_to_the_n().
	// -  Shift by 31 shifts into sign bit
	// -  Shift by 32 shifts off the end
}

/** @brief Test vint4 storea. */
TEST(SuiteVint4, storea)
{
	ASTCENC_ALIGNAS int out[4];
	vint4 a(s32_data);
	storea(a, out);
	EXPECT_EQ(out[0], 0);
	EXPECT_EQ(out[1], 1);
	EXPECT_EQ(out[2], 2);
	EXPECT_EQ(out[3], 3);
}

/** @brief Test vint4 store. */
TEST(SuiteVint4, store)
{
	ASTCENC_ALIGNAS int out[5];
	vint4 a(s32_data);
	store(a, &(out[1]));
	EXPECT_EQ(out[1], 0);
	EXPECT_EQ(out[2], 1);
	EXPECT_EQ(out[3], 2);
	EXPECT_EQ(out[4], 3);
}

/** @brief Test vint4 store_nbytes. */
TEST(SuiteVint4, store_nbytes)
{
	ASTCENC_ALIGNAS int out;
	vint4 a(42, 314, 75, 90);
	store_nbytes(a, reinterpret_cast<uint8_t*>(&out));
	EXPECT_EQ(out, 42);
}

/** @brief Test vint4 store_lanes_masked. */
TEST(SuiteVint4, store_lanes_masked)
{
	uint8_t resulta[16] { 0 };

	// Store nothing
	vmask4 mask1 = vint4(0) == vint4(1);
	vint4 data1 = vint4(1);

	store_lanes_masked(resulta, data1, mask1);
	vint4 result1v = vint4::load(resulta);
	vint4 expect1v = vint4::zero();
	EXPECT_TRUE(all(result1v == expect1v));

	// Store half
	vmask4 mask2 = vint4(1, 1, 0, 0) == vint4(1);
	vint4 data2 = vint4(2);

	store_lanes_masked(resulta, data2, mask2);
	vint4 result2v = vint4::load(resulta);
	vint4 expect2v = vint4(2, 2, 0, 0);
	EXPECT_TRUE(all(result2v == expect2v));

	// Store all
	vmask4 mask3 = vint4(1) == vint4(1);
	vint4 data3 = vint4(3);

	store_lanes_masked(resulta, data3, mask3);
	vint4 result3v = vint4::load(resulta);
	vint4 expect3v = vint4(3);
	EXPECT_TRUE(all(result3v == expect3v));
}

/** @brief Test vint4 store_lanes_masked to unaligned address. */
TEST(SuiteVint4, store_lanes_masked_unaligned)
{
	uint8_t resulta[17] { 0 };

	// Store nothing
	vmask4 mask1 = vint4(0) == vint4(1);
	vint4 data1 = vint4(1);

	store_lanes_masked(resulta + 1, data1, mask1);
	vint4 result1v = vint4::load(resulta + 1);
	vint4 expect1v = vint4::zero();
	EXPECT_TRUE(all(result1v == expect1v));

	// Store half
	vmask4 mask2 = vint4(1, 1, 0, 0) == vint4(1);
	vint4 data2 = vint4(2);

	store_lanes_masked(resulta + 1, data2, mask2);
	vint4 result2v = vint4::load(resulta + 1);
	vint4 expect2v = vint4(2, 2, 0, 0);
	EXPECT_TRUE(all(result2v == expect2v));

	// Store all
	vmask4 mask3 = vint4(1) == vint4(1);
	vint4 data3 = vint4(3);

	store_lanes_masked(resulta + 1, data3, mask3);
	vint4 result3v = vint4::load(resulta + 1);
	vint4 expect3v = vint4(3);
	EXPECT_TRUE(all(result3v == expect3v));
}

/** @brief Test vint4 pack_and_store_low_bytes. */
TEST(SuiteVint4, pack_and_store_low_bytes)
{
	vint4 a(1, 2, 3, 4);
	uint8_t bytes[4] { 0 };
	pack_and_store_low_bytes(a, bytes);

	EXPECT_EQ(bytes[0], 1);
	EXPECT_EQ(bytes[1], 2);
	EXPECT_EQ(bytes[2], 3);
	EXPECT_EQ(bytes[3], 4);
}

/** @brief Test vint4 select. */
TEST(SuiteVint4, select)
{
	vint4 m1(1, 1, 1, 1);
	vint4 m2(1, 2, 1, 2);
	vmask4 cond = m1 == m2;

	vint4 a(1, 3, 3, 1);
	vint4 b(4, 2, 2, 4);

	vint4 r1 = select(a, b, cond);
	EXPECT_EQ(r1.lane<0>(), 4);
	EXPECT_EQ(r1.lane<1>(), 3);
	EXPECT_EQ(r1.lane<2>(), 2);
	EXPECT_EQ(r1.lane<3>(), 1);

	vint4 r2 = select(b, a, cond);
	EXPECT_EQ(r2.lane<0>(), 1);
	EXPECT_EQ(r2.lane<1>(), 2);
	EXPECT_EQ(r2.lane<2>(), 3);
	EXPECT_EQ(r2.lane<3>(), 4);
}

// VMASK4 tests - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/** @brief Test vmask4 scalar literal constructor. */
TEST(SuiteVmask4, scalar_literal_construct)
{
	vfloat4 m1a(0.0f, 0.0f, 0.0f, 0.0f);
	vfloat4 m1b(1.0f, 1.0f, 1.0f, 1.0f);
	vmask4 m1(true);

	vfloat4 r = select(m1a, m1b, m1);

	EXPECT_EQ(r.lane<0>(), 1.0f);
	EXPECT_EQ(r.lane<1>(), 1.0f);
	EXPECT_EQ(r.lane<2>(), 1.0f);
	EXPECT_EQ(r.lane<3>(), 1.0f);

	r = select(m1b, m1a, m1);

	EXPECT_EQ(r.lane<0>(), 0.0f);
	EXPECT_EQ(r.lane<1>(), 0.0f);
	EXPECT_EQ(r.lane<2>(), 0.0f);
	EXPECT_EQ(r.lane<3>(), 0.0f);
}

/** @brief Test vmask4 literal constructor. */
TEST(SuiteVmask4, literal_construct)
{
	vfloat4 m1a(0.0f, 0.0f, 0.0f, 0.0f);
	vfloat4 m1b(1.0f, 1.0f, 1.0f, 1.0f);
	vmask4 m1(true, false, true, false);

	vfloat4 r = select(m1a, m1b, m1);

	EXPECT_EQ(r.lane<0>(), 1.0f);
	EXPECT_EQ(r.lane<1>(), 0.0f);
	EXPECT_EQ(r.lane<2>(), 1.0f);
	EXPECT_EQ(r.lane<3>(), 0.0f);
}

/** @brief Test vmask4 or. */
TEST(SuiteVmask4, TestOr)
{
	vfloat4 m1a(0, 1, 0, 1);
	vfloat4 m1b(1, 1, 1, 1);
	vmask4 m1 = m1a == m1b;

	vfloat4 m2a(1, 1, 0, 0);
	vfloat4 m2b(1, 1, 1, 1);
	vmask4 m2 = m2a == m2b;

	vmask4 r = m1 | m2;
	EXPECT_EQ(mask(r), 0xBu);
}

/** @brief Test vmask4 and. */
TEST(SuiteVmask4, TestAnd)
{
	vfloat4 m1a(0, 1, 0, 1);
	vfloat4 m1b(1, 1, 1, 1);
	vmask4 m1 = m1a == m1b;

	vfloat4 m2a(1, 1, 0, 0);
	vfloat4 m2b(1, 1, 1, 1);
	vmask4 m2 = m2a == m2b;

	vmask4 r = m1 & m2;
	EXPECT_EQ(mask(r), 0x2u);
}

/** @brief Test vmask4 xor. */
TEST(SuiteVmask4, TestXor)
{
	vfloat4 m1a(0, 1, 0, 1);
	vfloat4 m1b(1, 1, 1, 1);
	vmask4 m1 = m1a == m1b;

	vfloat4 m2a(1, 1, 0, 0);
	vfloat4 m2b(1, 1, 1, 1);
	vmask4 m2 = m2a == m2b;

	vmask4 r = m1 ^ m2;
	EXPECT_EQ(mask(r), 0x9u);
}

/** @brief Test vmask4 not. */
TEST(SuiteVmask4, TestNot)
{
	vfloat4 m1a(0, 1, 0, 1);
	vfloat4 m1b(1, 1, 1, 1);
	vmask4 m1 = m1a == m1b;
	vmask4 r = ~m1;
	EXPECT_EQ(mask(r), 0x5u);
}

/** @brief Test vint4 table permute. */
TEST(SuiteVint4, vtable4_16x8)
{
	uint8_t data[16] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	};

	vtable4_16x8 table;
	vtable_prepare(table, data);

	vint4 index(0, 7, 4, 15);

	vint4 result = vtable_lookup_32bit(table, index);

	EXPECT_EQ(result.lane<0>(),  0);
	EXPECT_EQ(result.lane<1>(),  7);
	EXPECT_EQ(result.lane<2>(),  4);
	EXPECT_EQ(result.lane<3>(), 15);
}

/** @brief Test vint4 table permute. */
TEST(SuiteVint4, vtable4_32x8)
{
	uint8_t data[32] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
	};

	vtable4_32x8 table;
	vtable_prepare(table, data);

	vint4 index(0, 7, 4, 31);

	vint4 result = vtable_lookup_32bit(table, index);

	EXPECT_EQ(result.lane<0>(),  0);
	EXPECT_EQ(result.lane<1>(),  7);
	EXPECT_EQ(result.lane<2>(),  4);
	EXPECT_EQ(result.lane<3>(), 31);
}

/** @brief Test vint4 table permute. */
TEST(SuiteVint4, vtable4_64x8)
{
	uint8_t data[64] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
		0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
		0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
	};

	vtable4_64x8 table;
	vtable_prepare(table, data);

	vint4 index(0, 7, 38, 63);

	vint4 result = vtable_lookup_32bit(table, index);

	EXPECT_EQ(result.lane<0>(),  0);
	EXPECT_EQ(result.lane<1>(),  7);
	EXPECT_EQ(result.lane<2>(), 38);
	EXPECT_EQ(result.lane<3>(), 63);
}

/** @brief Test vint4 rgba byte interleave. */
TEST(SuiteVint4, interleave_rgba8)
{
	vint4 r(0x01, 0x11, 0x21, 0x31);
	vint4 g(0x02, 0x12, 0x22, 0x32);
	vint4 b(0x03, 0x13, 0x23, 0x33);
	vint4 a(0x04, 0x14, 0x24, 0x34);

	vint4 result = interleave_rgba8(r, g, b, a);

#if !defined(ASTCENC_BIG_ENDIAN)
	EXPECT_EQ(result.lane<0>(), 0x04030201);
	EXPECT_EQ(result.lane<1>(), 0x14131211);
	EXPECT_EQ(result.lane<2>(), 0x24232221);
	EXPECT_EQ(result.lane<3>(), 0x34333231);
#else
	EXPECT_EQ(result.lane<0>(), 0x01020304);
	EXPECT_EQ(result.lane<1>(), 0x11121314);
	EXPECT_EQ(result.lane<2>(), 0x21222324);
	EXPECT_EQ(result.lane<3>(), 0x31323334);
#endif
}

# if ASTCENC_SIMD_WIDTH == 8

// VFLOAT8 tests - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** @brief Test unaligned vfloat8 data load. */
TEST(SuiteVfloat8, UnalignedLoad)
{
	vfloat8 a(&(f32_data[1]));

	alignas(32) float ra[8];
	storea(a, ra);

	EXPECT_EQ(ra[0], 1.0f);
	EXPECT_EQ(ra[1], 2.0f);
	EXPECT_EQ(ra[2], 3.0f);
	EXPECT_EQ(ra[3], 4.0f);
	EXPECT_EQ(ra[4], 5.0f);
	EXPECT_EQ(ra[5], 6.0f);
	EXPECT_EQ(ra[6], 7.0f);
	EXPECT_EQ(ra[7], 8.0f);
}

/** @brief Test scalar duplicated vfloat8 load. */
TEST(SuiteVfloat8, ScalarDupLoad)
{
	vfloat8 a(1.1f);

	alignas(32) float ra[8];
	storea(a, ra);

	EXPECT_EQ(ra[0], 1.1f);
	EXPECT_EQ(ra[1], 1.1f);
	EXPECT_EQ(ra[2], 1.1f);
	EXPECT_EQ(ra[3], 1.1f);
	EXPECT_EQ(ra[4], 1.1f);
	EXPECT_EQ(ra[5], 1.1f);
	EXPECT_EQ(ra[6], 1.1f);
	EXPECT_EQ(ra[7], 1.1f);
}

/** @brief Test scalar vfloat8 load. */
TEST(SuiteVfloat8, ScalarLoad)
{
	vfloat8 a = vfloat8_lit(1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f);

	alignas(32) float ra[8];
	storea(a, ra);

	EXPECT_EQ(ra[0], 1.1f);
	EXPECT_EQ(ra[1], 2.2f);
	EXPECT_EQ(ra[2], 3.3f);
	EXPECT_EQ(ra[3], 4.4f);
	EXPECT_EQ(ra[4], 5.5f);
	EXPECT_EQ(ra[5], 6.6f);
	EXPECT_EQ(ra[6], 7.7f);
	EXPECT_EQ(ra[7], 8.8f);
}

/** @brief Test copy vfloat8 load. */
TEST(SuiteVfloat8, CopyLoad)
{
	vfloat8 s = vfloat8_lit(1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f);
	vfloat8 a(s.m);

	alignas(32) float ra[8];
	storea(a, ra);

	EXPECT_EQ(ra[0], 1.1f);
	EXPECT_EQ(ra[1], 2.2f);
	EXPECT_EQ(ra[2], 3.3f);
	EXPECT_EQ(ra[3], 4.4f);
	EXPECT_EQ(ra[4], 5.5f);
	EXPECT_EQ(ra[5], 6.6f);
	EXPECT_EQ(ra[6], 7.7f);
	EXPECT_EQ(ra[7], 8.8f);
}

/** @brief Test vfloat8 zero. */
TEST(SuiteVfloat8, Zero)
{
	vfloat8 a = vfloat8::zero();

	alignas(32) float ra[8];
	storea(a, ra);

	EXPECT_EQ(ra[0], 0.0f);
	EXPECT_EQ(ra[1], 0.0f);
	EXPECT_EQ(ra[2], 0.0f);
	EXPECT_EQ(ra[3], 0.0f);
	EXPECT_EQ(ra[4], 0.0f);
	EXPECT_EQ(ra[5], 0.0f);
	EXPECT_EQ(ra[6], 0.0f);
	EXPECT_EQ(ra[7], 0.0f);
}

/** @brief Test vfloat8 load1. */
TEST(SuiteVfloat8, Load1)
{
	float s = 3.14f;
	vfloat8 a = vfloat8::load1(&s);

	alignas(32) float ra[8];
	storea(a, ra);

	EXPECT_EQ(ra[0], 3.14f);
	EXPECT_EQ(ra[1], 3.14f);
	EXPECT_EQ(ra[2], 3.14f);
	EXPECT_EQ(ra[3], 3.14f);
	EXPECT_EQ(ra[4], 3.14f);
	EXPECT_EQ(ra[5], 3.14f);
	EXPECT_EQ(ra[6], 3.14f);
	EXPECT_EQ(ra[7], 3.14f);
}

/** @brief Test vfloat8 loada. */
TEST(SuiteVfloat8, Loada)
{
	vfloat8 a = vfloat8::loada(&(f32_data[0]));

	alignas(32) float ra[8];
	storea(a, ra);

	EXPECT_EQ(ra[0], 0.0f);
	EXPECT_EQ(ra[1], 1.0f);
	EXPECT_EQ(ra[2], 2.0f);
	EXPECT_EQ(ra[3], 3.0f);
	EXPECT_EQ(ra[4], 4.0f);
	EXPECT_EQ(ra[5], 5.0f);
	EXPECT_EQ(ra[6], 6.0f);
	EXPECT_EQ(ra[7], 7.0f);
}

/** @brief Test vfloat8 add. */
TEST(SuiteVfloat8, vadd)
{
	vfloat8 a = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	vfloat8 b = vfloat8_lit(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f);
	a = a + b;

	alignas(32) float ra[8];
	storea(a, ra);

	EXPECT_EQ(ra[0], 1.0f + 0.1f);
	EXPECT_EQ(ra[1], 2.0f + 0.2f);
	EXPECT_EQ(ra[2], 3.0f + 0.3f);
	EXPECT_EQ(ra[3], 4.0f + 0.4f);
	EXPECT_EQ(ra[4], 5.0f + 0.5f);
	EXPECT_EQ(ra[5], 6.0f + 0.6f);
	EXPECT_EQ(ra[6], 7.0f + 0.7f);
	EXPECT_EQ(ra[7], 8.0f + 0.8f);
}

/** @brief Test vfloat8 sub. */
TEST(SuiteVfloat8, vsub)
{
	vfloat8 a = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	vfloat8 b = vfloat8_lit(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f);
	a = a - b;

	alignas(32) float ra[8];
	storea(a, ra);

	EXPECT_EQ(ra[0], 1.0f - 0.1f);
	EXPECT_EQ(ra[1], 2.0f - 0.2f);
	EXPECT_EQ(ra[2], 3.0f - 0.3f);
	EXPECT_EQ(ra[3], 4.0f - 0.4f);
	EXPECT_EQ(ra[4], 5.0f - 0.5f);
	EXPECT_EQ(ra[5], 6.0f - 0.6f);
	EXPECT_EQ(ra[6], 7.0f - 0.7f);
	EXPECT_EQ(ra[7], 8.0f - 0.8f);
}

/** @brief Test vfloat8 mul. */
TEST(SuiteVfloat8, vmul)
{
	vfloat8 a = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	vfloat8 b = vfloat8_lit(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f);
	a = a * b;

	alignas(32) float ra[8];
	storea(a, ra);

	EXPECT_EQ(ra[0], 1.0f * 0.1f);
	EXPECT_EQ(ra[1], 2.0f * 0.2f);
	EXPECT_EQ(ra[2], 3.0f * 0.3f);
	EXPECT_EQ(ra[3], 4.0f * 0.4f);
	EXPECT_EQ(ra[4], 5.0f * 0.5f);
	EXPECT_EQ(ra[5], 6.0f * 0.6f);
	EXPECT_EQ(ra[6], 7.0f * 0.7f);
	EXPECT_EQ(ra[7], 8.0f * 0.8f);
}

/** @brief Test vfloat8 mul. */
TEST(SuiteVfloat8, vsmul)
{
	vfloat8 a = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	float b = 3.14f;
	a = a * b;

	alignas(32) float ra[8];
	storea(a, ra);

	EXPECT_EQ(ra[0], 1.0f * 3.14f);
	EXPECT_EQ(ra[1], 2.0f * 3.14f);
	EXPECT_EQ(ra[2], 3.0f * 3.14f);
	EXPECT_EQ(ra[3], 4.0f * 3.14f);
	EXPECT_EQ(ra[4], 5.0f * 3.14f);
	EXPECT_EQ(ra[5], 6.0f * 3.14f);
	EXPECT_EQ(ra[6], 7.0f * 3.14f);
	EXPECT_EQ(ra[7], 8.0f * 3.14f);
}

/** @brief Test vfloat8 mul. */
TEST(SuiteVfloat8, svmul)
{
	float a = 3.14f;
	vfloat8 b = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	b = a * b;

	alignas(32) float ra[8];
	storea(b, ra);

	EXPECT_EQ(ra[0], 3.14f * 1.0f);
	EXPECT_EQ(ra[1], 3.14f * 2.0f);
	EXPECT_EQ(ra[2], 3.14f * 3.0f);
	EXPECT_EQ(ra[3], 3.14f * 4.0f);
	EXPECT_EQ(ra[4], 3.14f * 5.0f);
	EXPECT_EQ(ra[5], 3.14f * 6.0f);
	EXPECT_EQ(ra[6], 3.14f * 7.0f);
	EXPECT_EQ(ra[7], 3.14f * 8.0f);
}

/** @brief Test vfloat8 div. */
TEST(SuiteVfloat8, vdiv)
{
	vfloat8 a = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	vfloat8 b = vfloat8_lit(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f);
	a = a / b;

	alignas(32) float ra[8];
	storea(a, ra);

	EXPECT_EQ(ra[0], 1.0f / 0.1f);
	EXPECT_EQ(ra[1], 2.0f / 0.2f);
	EXPECT_EQ(ra[2], 3.0f / 0.3f);
	EXPECT_EQ(ra[3], 4.0f / 0.4f);
	EXPECT_EQ(ra[4], 5.0f / 0.5f);
	EXPECT_EQ(ra[5], 6.0f / 0.6f);
	EXPECT_EQ(ra[6], 7.0f / 0.7f);
	EXPECT_EQ(ra[7], 8.0f / 0.8f);
}

/** @brief Test vfloat8 div. */
TEST(SuiteVfloat8, vsdiv)
{
	vfloat8 a = vfloat8_lit(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f);
	float b = 3.14f;
	vfloat8 r = a / b;

	alignas(32) float ra[8];
	storea(r, ra);

	EXPECT_EQ(ra[0], 0.1f / 3.14f);
	EXPECT_EQ(ra[1], 0.2f / 3.14f);
	EXPECT_EQ(ra[2], 0.3f / 3.14f);
	EXPECT_EQ(ra[3], 0.4f / 3.14f);
	EXPECT_EQ(ra[4], 0.5f / 3.14f);
	EXPECT_EQ(ra[5], 0.6f / 3.14f);
	EXPECT_EQ(ra[6], 0.7f / 3.14f);
	EXPECT_EQ(ra[7], 0.8f / 3.14f);
}

/** @brief Test vfloat8 div. */
TEST(SuiteVfloat8, svdiv)
{
	float a = 3.14f;
	vfloat8 b = vfloat8_lit(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f);
	vfloat8 r = a / b;

	alignas(32) float ra[8];
	storea(r, ra);

	EXPECT_EQ(ra[0], 3.14f / 0.1f);
	EXPECT_EQ(ra[1], 3.14f / 0.2f);
	EXPECT_EQ(ra[2], 3.14f / 0.3f);
	EXPECT_EQ(ra[3], 3.14f / 0.4f);
	EXPECT_EQ(ra[4], 3.14f / 0.5f);
	EXPECT_EQ(ra[5], 3.14f / 0.6f);
	EXPECT_EQ(ra[6], 3.14f / 0.7f);
	EXPECT_EQ(ra[7], 3.14f / 0.8f);
}

/** @brief Test vfloat8 ceq. */
TEST(SuiteVfloat8, ceq)
{
	vfloat8 a1 = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	vfloat8 b1 = vfloat8_lit(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f);
	vmask8 r1 = a1 == b1;
	EXPECT_EQ(0u, mask(r1));
	EXPECT_EQ(false, any(r1));
	EXPECT_EQ(false, all(r1));

	vfloat8 a2 = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	vfloat8 b2 = vfloat8_lit(1.0f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f);
	vmask8 r2 = a2 == b2;
	EXPECT_EQ(0x1u, mask(r2));
	EXPECT_EQ(true, any(r2));
	EXPECT_EQ(false, all(r2));

	vfloat8 a3 = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	vfloat8 b3 = vfloat8_lit(1.0f, 0.2f, 3.0f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f);
	vmask8 r3 = a3 == b3;
	EXPECT_EQ(0x5u, mask(r3));
	EXPECT_EQ(true, any(r3));
	EXPECT_EQ(false, all(r3));

	vfloat8 a4 = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	vmask8 r4 = a4 == a4;
	EXPECT_EQ(0xFFu, mask(r4));
	EXPECT_EQ(true, any(r4));
	EXPECT_EQ(true, all(r4));
}

/** @brief Test vfloat8 cne. */
TEST(SuiteVfloat8, cne)
{
	vfloat8 a1 = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	vfloat8 b1 = vfloat8_lit(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f);
	vmask8 r1 = a1 != b1;
	EXPECT_EQ(0xFFu, mask(r1));
	EXPECT_EQ(true, any(r1));
	EXPECT_EQ(true, all(r1));

	vfloat8 a2 = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	vfloat8 b2 = vfloat8_lit(1.0f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f);
	vmask8 r2 = a2 != b2;
	EXPECT_EQ(0xFEu, mask(r2));
	EXPECT_EQ(true, any(r2));
	EXPECT_EQ(false, all(r2));

	vfloat8 a3 = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	vfloat8 b3 = vfloat8_lit(1.0f, 0.2f, 3.0f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f);
	vmask8 r3 = a3 != b3;
	EXPECT_EQ(0xFAu, mask(r3));
	EXPECT_EQ(true, any(r3));
	EXPECT_EQ(false, all(r3));

	vfloat8 a4 = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	vmask8 r4 = a4 != a4;
	EXPECT_EQ(0u, mask(r4));
	EXPECT_EQ(false, any(r4));
	EXPECT_EQ(false, all(r4));
}

/** @brief Test vfloat8 clt. */
TEST(SuiteVfloat8, clt)
{
	vfloat8 a = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f);
	vfloat8 b = vfloat8_lit(0.9f, 2.1f, 3.0f, 4.1f, 0.9f, 2.1f, 3.0f, 4.1f);
	vmask8 r = a < b;
	EXPECT_EQ(0xAAu, mask(r));
}

/** @brief Test vfloat8 cle. */
TEST(SuiteVfloat8, cle)
{
	vfloat8 a = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f);
	vfloat8 b = vfloat8_lit(0.9f, 2.1f, 3.0f, 4.1f, 0.9f, 2.1f, 3.0f, 4.1f);
	vmask8 r = a <= b;
	EXPECT_EQ(0xEEu, mask(r));
}

/** @brief Test vfloat8 cgt. */
TEST(SuiteVfloat8, cgt)
{
	vfloat8 a = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f);
	vfloat8 b = vfloat8_lit(0.9f, 2.1f, 3.0f, 4.1f, 0.9f, 2.1f, 3.0f, 4.1f);
	vmask8 r = a > b;
	EXPECT_EQ(0x11u, mask(r));
}

/** @brief Test vfloat8 cge. */
TEST(SuiteVfloat8, cge)
{
	vfloat8 a = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f);
	vfloat8 b = vfloat8_lit(0.9f, 2.1f, 3.0f, 4.1f, 0.9f, 2.1f, 3.0f, 4.1f);
	vmask8 r = a >= b;
	EXPECT_EQ(0x55u, mask(r));
}

/** @brief Test vfloat8 min. */
TEST(SuiteVfloat8, min)
{
	vfloat8 a = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f);
	vfloat8 b = vfloat8_lit(0.9f, 2.1f, 3.0f, 4.1f, 0.9f, 2.1f, 3.0f, 4.1f);
	vfloat8 r = min(a, b);

	alignas(32) float ra[8];
	storea(r, ra);

	EXPECT_EQ(ra[0], 0.9f);
	EXPECT_EQ(ra[1], 2.0f);
	EXPECT_EQ(ra[2], 3.0f);
	EXPECT_EQ(ra[3], 4.0f);
	EXPECT_EQ(ra[4], 0.9f);
	EXPECT_EQ(ra[5], 2.0f);
	EXPECT_EQ(ra[6], 3.0f);
	EXPECT_EQ(ra[7], 4.0f);
}

/** @brief Test vfloat8 max. */
TEST(SuiteVfloat8, max)
{
	vfloat8 a = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f);
	vfloat8 b = vfloat8_lit(0.9f, 2.1f, 3.0f, 4.1f, 0.9f, 2.1f, 3.0f, 4.1f);
	vfloat8 r = max(a, b);

	alignas(32) float ra[8];
	storea(r, ra);

	EXPECT_EQ(ra[0], 1.0f);
	EXPECT_EQ(ra[1], 2.1f);
	EXPECT_EQ(ra[2], 3.0f);
	EXPECT_EQ(ra[3], 4.1f);
	EXPECT_EQ(ra[4], 1.0f);
	EXPECT_EQ(ra[5], 2.1f);
	EXPECT_EQ(ra[6], 3.0f);
	EXPECT_EQ(ra[7], 4.1f);
}

/** @brief Test vfloat8 clamp. */
TEST(SuiteVfloat8, clamp)
{
	vfloat8 a1 = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f);
	vfloat8 r1 = clamp(2.1f, 3.0f, a1);

	alignas(32) float ra[8];
	storea(r1, ra);

	EXPECT_EQ(ra[0], 2.1f);
	EXPECT_EQ(ra[1], 2.1f);
	EXPECT_EQ(ra[2], 3.0f);
	EXPECT_EQ(ra[3], 3.0f);
	EXPECT_EQ(ra[4], 2.1f);
	EXPECT_EQ(ra[5], 2.1f);
	EXPECT_EQ(ra[6], 3.0f);
	EXPECT_EQ(ra[7], 3.0f);

	vfloat8 a2 = vfloat8_lit(1.0f, 2.0f, qnan, 4.0f, 1.0f, 2.0f, qnan, 4.0f);
	vfloat8 r2 = clamp(2.1f, 3.0f, a2);

	storea(r2, ra);

	EXPECT_EQ(ra[0], 2.1f);
	EXPECT_EQ(ra[1], 2.1f);
	EXPECT_EQ(ra[2], 2.1f);
	EXPECT_EQ(ra[3], 3.0f);
	EXPECT_EQ(ra[4], 2.1f);
	EXPECT_EQ(ra[5], 2.1f);
	EXPECT_EQ(ra[6], 2.1f);
	EXPECT_EQ(ra[7], 3.0f);
}

/** @brief Test vfloat8 clampz. */
TEST(SuiteVfloat8, clampzo)
{
	vfloat8 a1 = vfloat8_lit(-1.0f, 0.0f, 0.1f, 4.0f, -1.0f, 0.0f, 0.1f, 4.0f);
	vfloat8 r1 = clampzo(a1);

	alignas(32) float ra[8];
	storea(r1, ra);

	EXPECT_EQ(ra[0], 0.0f);
	EXPECT_EQ(ra[1], 0.0f);
	EXPECT_EQ(ra[2], 0.1f);
	EXPECT_EQ(ra[3], 1.0f);
	EXPECT_EQ(ra[4], 0.0f);
	EXPECT_EQ(ra[5], 0.0f);
	EXPECT_EQ(ra[6], 0.1f);
	EXPECT_EQ(ra[7], 1.0f);

	vfloat8 a2 = vfloat8_lit(-1.0f, 0.0f, qnan, 4.0f, -1.0f, 0.0f, qnan, 4.0f);
	vfloat8 r2 = clampzo(a2);

	storea(r2, ra);

	EXPECT_EQ(ra[0], 0.0f);
	EXPECT_EQ(ra[1], 0.0f);
	EXPECT_EQ(ra[2], 0.0f);
	EXPECT_EQ(ra[3], 1.0f);
	EXPECT_EQ(ra[4], 0.0f);
	EXPECT_EQ(ra[5], 0.0f);
	EXPECT_EQ(ra[6], 0.0f);
	EXPECT_EQ(ra[7], 1.0f);
}

/** @brief Test vfloat8 abs. */
TEST(SuiteVfloat8, abs)
{
	vfloat8 a = vfloat8_lit(-1.0f, 0.0f, 0.1f, 4.0f, -1.0f, 0.0f, 0.1f, 4.0f);
	vfloat8 r = abs(a);

	alignas(32) float ra[8];
	storea(r, ra);

	EXPECT_EQ(ra[0], 1.0f);
	EXPECT_EQ(ra[1], 0.0f);
	EXPECT_EQ(ra[2], 0.1f);
	EXPECT_EQ(ra[3], 4.0f);
	EXPECT_EQ(ra[4], 1.0f);
	EXPECT_EQ(ra[5], 0.0f);
	EXPECT_EQ(ra[6], 0.1f);
	EXPECT_EQ(ra[7], 4.0f);
}

/** @brief Test vfloat8 round. */
TEST(SuiteVfloat8, round)
{
	vfloat8 a = vfloat8_lit(1.1f, 1.5f, 1.6f, 4.0f, 1.1f, 1.5f, 1.6f, 4.0f);
	vfloat8 r = round(a);

	alignas(32) float ra[8];
	storea(r, ra);

	EXPECT_EQ(ra[0], 1.0f);
	EXPECT_EQ(ra[1], 2.0f);
	EXPECT_EQ(ra[2], 2.0f);
	EXPECT_EQ(ra[3], 4.0f);
	EXPECT_EQ(ra[4], 1.0f);
	EXPECT_EQ(ra[5], 2.0f);
	EXPECT_EQ(ra[6], 2.0f);
	EXPECT_EQ(ra[7], 4.0f);
}

/** @brief Test vfloat8 hmin. */
TEST(SuiteVfloat8, hmin)
{
	vfloat8 a1 = vfloat8_lit(1.1f, 1.5f, 1.6f, 4.0f, 1.1f, 1.5f, 1.6f, 4.0f);
	vfloat8 r1 = hmin(a1);

	alignas(32) float ra[8];
	storea(r1, ra);

	EXPECT_EQ(ra[0], 1.1f);
	EXPECT_EQ(ra[1], 1.1f);
	EXPECT_EQ(ra[2], 1.1f);
	EXPECT_EQ(ra[3], 1.1f);
	EXPECT_EQ(ra[4], 1.1f);
	EXPECT_EQ(ra[5], 1.1f);
	EXPECT_EQ(ra[6], 1.1f);
	EXPECT_EQ(ra[7], 1.1f);

	vfloat8 a2 = vfloat8_lit(1.1f, 1.5f, 1.6f, 0.2f, 1.1f, 1.5f, 1.6f, 0.2f);
	vfloat8 r2 = hmin(a2);

	storea(r2, ra);

	EXPECT_EQ(ra[0], 0.2f);
	EXPECT_EQ(ra[1], 0.2f);
	EXPECT_EQ(ra[2], 0.2f);
	EXPECT_EQ(ra[3], 0.2f);
	EXPECT_EQ(ra[4], 0.2f);
	EXPECT_EQ(ra[5], 0.2f);
	EXPECT_EQ(ra[6], 0.2f);
	EXPECT_EQ(ra[7], 0.2f);
}

/** @brief Test vfloat8 hmin_s. */
TEST(SuiteVfloat8, hmin_s)
{
	vfloat8 a1 = vfloat8_lit(1.1f, 1.5f, 1.6f, 4.0f, 1.1f, 1.5f, 1.6f, 4.0f);
	float r1 = hmin_s(a1);
	EXPECT_EQ(r1, 1.1f);

	vfloat8 a2 = vfloat8_lit(1.1f, 1.5f, 1.6f, 0.2f, 1.1f, 1.5f, 1.6f, 0.2f);
	float r2 = hmin_s(a2);
	EXPECT_EQ(r2, 0.2f);
}

/** @brief Test vfloat8 hmax. */
TEST(SuiteVfloat8, hmax)
{
	vfloat8 a1 = vfloat8_lit(1.1f, 1.5f, 1.6f, 4.0f, 1.1f, 1.5f, 1.6f, 4.0f);
	vfloat8 r1 = hmax(a1);

	alignas(32) float ra[8];
	storea(r1, ra);

	EXPECT_EQ(ra[0], 4.0f);
	EXPECT_EQ(ra[1], 4.0f);
	EXPECT_EQ(ra[2], 4.0f);
	EXPECT_EQ(ra[3], 4.0f);
	EXPECT_EQ(ra[4], 4.0f);
	EXPECT_EQ(ra[5], 4.0f);
	EXPECT_EQ(ra[6], 4.0f);
	EXPECT_EQ(ra[7], 4.0f);

	vfloat8 a2 = vfloat8_lit(1.1f, 1.5f, 1.6f, 0.2f, 1.1f, 1.5f, 1.6f, 0.2f);
	vfloat8 r2 = hmax(a2);

	storea(r2, ra);

	EXPECT_EQ(ra[0], 1.6f);
	EXPECT_EQ(ra[1], 1.6f);
	EXPECT_EQ(ra[2], 1.6f);
	EXPECT_EQ(ra[3], 1.6f);
	EXPECT_EQ(ra[4], 1.6f);
	EXPECT_EQ(ra[5], 1.6f);
	EXPECT_EQ(ra[6], 1.6f);
	EXPECT_EQ(ra[7], 1.6f);
}

/** @brief Test vfloat8 hmax_s. */
TEST(SuiteVfloat8, hmax_s)
{
	vfloat8 a1 = vfloat8_lit(1.1f, 1.5f, 1.6f, 4.0f, 1.1f, 1.5f, 1.6f, 4.0f);
	float r1 = hmax_s(a1);
	EXPECT_EQ(r1, 4.0f);

	vfloat8 a2 = vfloat8_lit(1.1f, 1.5f, 1.6f, 0.2f, 1.1f, 1.5f, 1.6f, 0.2f);
	float r2 = hmax_s(a2);
	EXPECT_EQ(r2, 1.6f);
}

/** @brief Test vfloat8 hadd_s. */
TEST(SuiteVfloat8, hadd_s)
{
	vfloat8 a1 = vfloat8_lit(1.1f, 1.5f, 1.6f, 4.0f, 1.1f, 1.5f, 1.6f, 4.0f);
	float sum = ((1.1f + 1.6f) + (1.5f + 4.0f)) +
	            ((1.1f + 1.6f) + (1.5f + 4.0f));
	float r = hadd_s(a1);
	EXPECT_EQ(r, sum);
}

/** @brief Test vfloat8 sqrt. */
TEST(SuiteVfloat8, sqrt)
{
	vfloat8 a = vfloat8_lit(1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f);
	vfloat8 r = sqrt(a);

	alignas(32) float ra[8];
	storea(r, ra);

	EXPECT_EQ(ra[0], std::sqrt(1.0f));
	EXPECT_EQ(ra[1], std::sqrt(2.0f));
	EXPECT_EQ(ra[2], std::sqrt(3.0f));
	EXPECT_EQ(ra[3], std::sqrt(4.0f));
	EXPECT_EQ(ra[4], std::sqrt(1.0f));
	EXPECT_EQ(ra[5], std::sqrt(2.0f));
	EXPECT_EQ(ra[6], std::sqrt(3.0f));
	EXPECT_EQ(ra[7], std::sqrt(4.0f));
}

/** @brief Test vfloat8 select. */
TEST(SuiteVfloat8, select)
{
	vfloat8 m1 = vfloat8_lit(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	vfloat8 m2 = vfloat8_lit(1.0f, 2.0f, 1.0f, 2.0f, 1.0f, 2.0f, 1.0f, 2.0f);
	vmask8 cond = m1 == m2;

	vfloat8 a = vfloat8_lit(1.0f, 3.0f, 3.0f, 1.0f, 1.0f, 3.0f, 3.0f, 1.0);
	vfloat8 b = vfloat8_lit(4.0f, 2.0f, 2.0f, 4.0f, 4.0f, 2.0f, 2.0f, 4.0);

	// Select in one direction
	vfloat8 r1 = select(a, b, cond);

	alignas(32) float ra[8];
	storea(r1, ra);

	EXPECT_EQ(ra[0], 4.0f);
	EXPECT_EQ(ra[1], 3.0f);
	EXPECT_EQ(ra[2], 2.0f);
	EXPECT_EQ(ra[3], 1.0f);
	EXPECT_EQ(ra[4], 4.0f);
	EXPECT_EQ(ra[5], 3.0f);
	EXPECT_EQ(ra[6], 2.0f);
	EXPECT_EQ(ra[7], 1.0f);

	// Select in the other
	vfloat8 r2 = select(b, a, cond);

	storea(r2, ra);

	EXPECT_EQ(ra[0], 1.0f);
	EXPECT_EQ(ra[1], 2.0f);
	EXPECT_EQ(ra[2], 3.0f);
	EXPECT_EQ(ra[3], 4.0f);
	EXPECT_EQ(ra[4], 1.0f);
	EXPECT_EQ(ra[5], 2.0f);
	EXPECT_EQ(ra[6], 3.0f);
	EXPECT_EQ(ra[7], 4.0f);
}

/** @brief Test vfloat8 gatherf. */
TEST(SuiteVfloat8, gatherf)
{
	vint8 indices = vint8_lit(0, 4, 3, 2, 7, 4, 3, 2);
	vfloat8 r = gatherf(f32_data, indices);

	alignas(32) float ra[8];
	storea(r, ra);

	EXPECT_EQ(ra[0], 0.0f);
	EXPECT_EQ(ra[1], 4.0f);
	EXPECT_EQ(ra[2], 3.0f);
	EXPECT_EQ(ra[3], 2.0f);
	EXPECT_EQ(ra[4], 7.0f);
	EXPECT_EQ(ra[5], 4.0f);
	EXPECT_EQ(ra[6], 3.0f);
	EXPECT_EQ(ra[7], 2.0f);
}

/** @brief Test vfloat8 store. */
TEST(SuiteVfloat8, store)
{
	vfloat8 a(f32_data);

	alignas(32) float ra[9];
	store(a, ra + 1);

	EXPECT_EQ(ra[1], 0.0f);
	EXPECT_EQ(ra[2], 1.0f);
	EXPECT_EQ(ra[3], 2.0f);
	EXPECT_EQ(ra[4], 3.0f);
	EXPECT_EQ(ra[5], 4.0f);
	EXPECT_EQ(ra[6], 5.0f);
	EXPECT_EQ(ra[7], 6.0f);
	EXPECT_EQ(ra[8], 7.0f);
}

/** @brief Test vfloat8 storea. */
TEST(SuiteVfloat8, storea)
{
	vfloat8 a(f32_data);

	alignas(32) float ra[8];
	storea(a, ra);

	EXPECT_EQ(ra[0], 0.0f);
	EXPECT_EQ(ra[1], 1.0f);
	EXPECT_EQ(ra[2], 2.0f);
	EXPECT_EQ(ra[3], 3.0f);
	EXPECT_EQ(ra[4], 4.0f);
	EXPECT_EQ(ra[5], 5.0f);
	EXPECT_EQ(ra[6], 6.0f);
	EXPECT_EQ(ra[7], 7.0f);
}

/** @brief Test vfloat8 float_to_int. */
TEST(SuiteVfloat8, float_to_int)
{
	vfloat8 a = vfloat8_lit(1.1f, 1.5f, 1.6f, 4.0f, 1.1f, 1.5f, 1.6f, 4.0f);
	vint8 r = float_to_int(a);

	alignas(32) int ra[8];
	store(r, ra);

	EXPECT_EQ(ra[0], 1);
	EXPECT_EQ(ra[1], 1);
	EXPECT_EQ(ra[2], 1);
	EXPECT_EQ(ra[3], 4);
	EXPECT_EQ(ra[4], 1);
	EXPECT_EQ(ra[5], 1);
	EXPECT_EQ(ra[6], 1);
	EXPECT_EQ(ra[7], 4);
}

// vint8 tests - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** @brief Test unaligned vint8 data load. */
TEST(SuiteVint8, UnalignedLoad)
{
	vint8 a(&(s32_data[1]));

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 1);
	EXPECT_EQ(ra[1], 2);
	EXPECT_EQ(ra[2], 3);
	EXPECT_EQ(ra[3], 4);
	EXPECT_EQ(ra[4], 5);
	EXPECT_EQ(ra[5], 6);
	EXPECT_EQ(ra[6], 7);
	EXPECT_EQ(ra[7], 8);
}

/** @brief Test unaligned vint8 data load. */
TEST(SuiteVint8, UnalignedLoad8)
{
	vint8 a(&(u8_data[1]));

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 1);
	EXPECT_EQ(ra[1], 2);
	EXPECT_EQ(ra[2], 3);
	EXPECT_EQ(ra[3], 4);
	EXPECT_EQ(ra[4], 5);
	EXPECT_EQ(ra[5], 6);
	EXPECT_EQ(ra[6], 7);
	EXPECT_EQ(ra[7], 8);
}

/** @brief Test scalar duplicated vint8 load. */
TEST(SuiteVint8, ScalarDupLoad)
{
	vint8 a(42);

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 42);
	EXPECT_EQ(ra[1], 42);
	EXPECT_EQ(ra[2], 42);
	EXPECT_EQ(ra[3], 42);
	EXPECT_EQ(ra[4], 42);
	EXPECT_EQ(ra[5], 42);
	EXPECT_EQ(ra[6], 42);
	EXPECT_EQ(ra[7], 42);
}

/** @brief Test scalar vint8 load. */
TEST(SuiteVint8, ScalarLoad)
{
	vint8 a = vint8_lit(11, 22, 33, 44, 55, 66, 77, 88);

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 11);
	EXPECT_EQ(ra[1], 22);
	EXPECT_EQ(ra[2], 33);
	EXPECT_EQ(ra[3], 44);
	EXPECT_EQ(ra[4], 55);
	EXPECT_EQ(ra[5], 66);
	EXPECT_EQ(ra[6], 77);
	EXPECT_EQ(ra[7], 88);
}

/** @brief Test copy vint8 load. */
TEST(SuiteVint8, CopyLoad)
{
	vint8 s = vint8_lit(11, 22, 33, 44, 55, 66, 77, 88);
	vint8 a(s.m);

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 11);
	EXPECT_EQ(ra[1], 22);
	EXPECT_EQ(ra[2], 33);
	EXPECT_EQ(ra[3], 44);
	EXPECT_EQ(ra[4], 55);
	EXPECT_EQ(ra[5], 66);
	EXPECT_EQ(ra[6], 77);
	EXPECT_EQ(ra[7], 88);
}

/** @brief Test vint8 zero. */
TEST(SuiteVint8, Zero)
{
	vint8 a = vint8::zero();

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 0);
	EXPECT_EQ(ra[1], 0);
	EXPECT_EQ(ra[2], 0);
	EXPECT_EQ(ra[3], 0);
	EXPECT_EQ(ra[4], 0);
	EXPECT_EQ(ra[5], 0);
	EXPECT_EQ(ra[6], 0);
	EXPECT_EQ(ra[7], 0);
}

/** @brief Test vint8 load1. */
TEST(SuiteVint8, Load1)
{
	int s = 42;
	vint8 a = vint8::load1(&s);

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 42);
	EXPECT_EQ(ra[1], 42);
	EXPECT_EQ(ra[2], 42);
	EXPECT_EQ(ra[3], 42);
	EXPECT_EQ(ra[4], 42);
	EXPECT_EQ(ra[5], 42);
	EXPECT_EQ(ra[6], 42);
	EXPECT_EQ(ra[7], 42);
}

/** @brief Test vint8 loada. */
TEST(SuiteVint8, Loada)
{
	vint8 a = vint8::loada(&(s32_data[0]));

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 0);
	EXPECT_EQ(ra[1], 1);
	EXPECT_EQ(ra[2], 2);
	EXPECT_EQ(ra[3], 3);
	EXPECT_EQ(ra[4], 4);
	EXPECT_EQ(ra[5], 5);
	EXPECT_EQ(ra[6], 6);
	EXPECT_EQ(ra[7], 7);
}

/** @brief Test vint8 lane_id. */
TEST(SuiteVint8, LaneID)
{
	vint8 a = vint8::lane_id();

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 0);
	EXPECT_EQ(ra[1], 1);
	EXPECT_EQ(ra[2], 2);
	EXPECT_EQ(ra[3], 3);
	EXPECT_EQ(ra[4], 4);
	EXPECT_EQ(ra[5], 5);
	EXPECT_EQ(ra[6], 6);
	EXPECT_EQ(ra[7], 7);
}

/** @brief Test vint8 add. */
TEST(SuiteVint8, vadd)
{
	vint8 a = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b = vint8_lit(2, 3, 4, 5, 2, 3, 4, 5);
	a = a + b;

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 1 + 2);
	EXPECT_EQ(ra[1], 2 + 3);
	EXPECT_EQ(ra[2], 3 + 4);
	EXPECT_EQ(ra[3], 4 + 5);
	EXPECT_EQ(ra[4], 1 + 2);
	EXPECT_EQ(ra[5], 2 + 3);
	EXPECT_EQ(ra[6], 3 + 4);
	EXPECT_EQ(ra[7], 4 + 5);
}


/** @brief Test vint8 self-add. */
TEST(SuiteVint8, vselfadd1)
{
	vint8 a = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b = vint8_lit(2, 3, 4, 5, 2, 3, 4, 5);
	a += b;

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 1 + 2);
	EXPECT_EQ(ra[1], 2 + 3);
	EXPECT_EQ(ra[2], 3 + 4);
	EXPECT_EQ(ra[3], 4 + 5);
	EXPECT_EQ(ra[4], 1 + 2);
	EXPECT_EQ(ra[5], 2 + 3);
	EXPECT_EQ(ra[6], 3 + 4);
	EXPECT_EQ(ra[7], 4 + 5);
}

/** @brief Test vint8 sub. */
TEST(SuiteVint8, vsub)
{
	vint8 a = vint8_lit(1, 2, 4, 4, 1, 2, 4, 4);
	vint8 b = vint8_lit(2, 3, 3, 5, 2, 3, 3, 5);
	a = a - b;

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 1 - 2);
	EXPECT_EQ(ra[1], 2 - 3);
	EXPECT_EQ(ra[2], 4 - 3);
	EXPECT_EQ(ra[3], 4 - 5);
	EXPECT_EQ(ra[4], 1 - 2);
	EXPECT_EQ(ra[5], 2 - 3);
	EXPECT_EQ(ra[6], 4 - 3);
	EXPECT_EQ(ra[7], 4 - 5);
}

/** @brief Test vint8 mul. */
TEST(SuiteVint8, vmul)
{
	vint8 a = vint8_lit(1, 2, 4, 4, 1, 2, 4, 4);
	vint8 b = vint8_lit(2, 3, 3, 5, 2, 3, 3, 5);
	a = a * b;

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 1 * 2);
	EXPECT_EQ(ra[1], 2 * 3);
	EXPECT_EQ(ra[2], 4 * 3);
	EXPECT_EQ(ra[3], 4 * 5);
	EXPECT_EQ(ra[4], 1 * 2);
	EXPECT_EQ(ra[5], 2 * 3);
	EXPECT_EQ(ra[6], 4 * 3);
	EXPECT_EQ(ra[7], 4 * 5);
}

/** @brief Test vint8 bitwise invert. */
TEST(SuiteVint8, bit_invert)
{
	vint8 a = vint8_lit(-1, 0, 1, 2, -1, 0, 1, 2);
	a = ~a;

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], ~-1);
	EXPECT_EQ(ra[1], ~0);
	EXPECT_EQ(ra[2], ~1);
	EXPECT_EQ(ra[3], ~2);
	EXPECT_EQ(ra[4], ~-1);
	EXPECT_EQ(ra[5], ~0);
	EXPECT_EQ(ra[6], ~1);
	EXPECT_EQ(ra[7], ~2);
}

/** @brief Test vint8 bitwise or. */
TEST(SuiteVint8, bit_vor)
{
	vint8 a = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b = vint8_lit(2, 3, 4, 5, 2, 3, 4, 5);
	a = a | b;

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 3);
	EXPECT_EQ(ra[1], 3);
	EXPECT_EQ(ra[2], 7);
	EXPECT_EQ(ra[3], 5);
	EXPECT_EQ(ra[4], 3);
	EXPECT_EQ(ra[5], 3);
	EXPECT_EQ(ra[6], 7);
	EXPECT_EQ(ra[7], 5);
}

/** @brief Test vint8 bitwise and. */
TEST(SuiteVint8, bit_vand)
{
	vint8 a = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b = vint8_lit(2, 3, 4, 5, 2, 3, 4, 5);
	a = a & b;

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 0);
	EXPECT_EQ(ra[1], 2);
	EXPECT_EQ(ra[2], 0);
	EXPECT_EQ(ra[3], 4);
	EXPECT_EQ(ra[4], 0);
	EXPECT_EQ(ra[5], 2);
	EXPECT_EQ(ra[6], 0);
	EXPECT_EQ(ra[7], 4);
}

/** @brief Test vint8 bitwise xor. */
TEST(SuiteVint8, bit_vxor)
{
	vint8 a = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b = vint8_lit(2, 3, 4, 5, 2, 3, 4, 5);
	a = a ^ b;

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 3);
	EXPECT_EQ(ra[1], 1);
	EXPECT_EQ(ra[2], 7);
	EXPECT_EQ(ra[3], 1);
	EXPECT_EQ(ra[4], 3);
	EXPECT_EQ(ra[5], 1);
	EXPECT_EQ(ra[6], 7);
	EXPECT_EQ(ra[7], 1);
}

/** @brief Test vint8 ceq. */
TEST(SuiteVint8, ceq)
{
	vint8 a1 = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b1 = vint8_lit(0, 1, 2, 3, 0, 1, 2, 3);
	vmask8 r1 = a1 == b1;
	EXPECT_EQ(0u, mask(r1));
	EXPECT_EQ(false, any(r1));
	EXPECT_EQ(false, all(r1));

	vint8 a2 = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b2 = vint8_lit(1, 0, 0, 0, 1, 0, 0, 0);
	vmask8 r2 = a2 == b2;
	EXPECT_EQ(0x11u, mask(r2));
	EXPECT_EQ(true, any(r2));
	EXPECT_EQ(false, all(r2));

	vint8 a3 = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b3 = vint8_lit(1, 0, 3, 0, 1, 0, 3, 0);
	vmask8 r3 = a3 == b3;
	EXPECT_EQ(0x55u, mask(r3));
	EXPECT_EQ(true, any(r3));
	EXPECT_EQ(false, all(r3));

	vint8 a4 = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vmask8 r4 = a4 == a4;
	EXPECT_EQ(0xFFu, mask(r4));
	EXPECT_EQ(true, any(r4));
	EXPECT_EQ(true, all(r4));
}

/** @brief Test vint8 cne. */
TEST(SuiteVint8, cne)
{
	vint8 a1 = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b1 = vint8_lit(0, 1, 2, 3, 0, 1, 2, 3);
	vmask8 r1 = a1 != b1;
	EXPECT_EQ(0xFFu, mask(r1));
	EXPECT_EQ(true, any(r1));
	EXPECT_EQ(true, all(r1));

	vint8 a2 = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b2 = vint8_lit(1, 0, 0, 0, 1, 0, 0, 0);
	vmask8 r2 = a2 != b2;
	EXPECT_EQ(0xEEu, mask(r2));
	EXPECT_EQ(true, any(r2));
	EXPECT_EQ(false, all(r2));

	vint8 a3 = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b3 = vint8_lit(1, 0, 3, 0, 1, 0, 3, 0);
	vmask8 r3 = a3 != b3;
	EXPECT_EQ(0xAAu, mask(r3));
	EXPECT_EQ(true, any(r3));
	EXPECT_EQ(false, all(r3));

	vint8 a4 = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vmask8 r4 = a4 != a4;
	EXPECT_EQ(0u, mask(r4));
	EXPECT_EQ(false, any(r4));
	EXPECT_EQ(false, all(r4));
}

/** @brief Test vint8 clt. */
TEST(SuiteVint8, clt)
{
	vint8 a = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b = vint8_lit(0, 3, 3, 5, 0, 3, 3, 5);
	vmask8 r = a < b;
	EXPECT_EQ(0xAAu, mask(r));
}

/** @brief Test vint8 cgt. */
TEST(SuiteVint8, cgt)
{
	vint8 a = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b = vint8_lit(0, 3, 3, 5, 0, 3, 3, 5);
	vmask8 r = a > b;
	EXPECT_EQ(0x11u, mask(r));
}

/** @brief Test vint8 min. */
TEST(SuiteVint8, min)
{
	vint8 a = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b = vint8_lit(0, 3, 3, 5, 0, 3, 3, 5);
	vint8 r = min(a, b);

	alignas(32) int ra[8];
	store(r, ra);

	EXPECT_EQ(ra[0], 0);
	EXPECT_EQ(ra[1], 2);
	EXPECT_EQ(ra[2], 3);
	EXPECT_EQ(ra[3], 4);
	EXPECT_EQ(ra[4], 0);
	EXPECT_EQ(ra[5], 2);
	EXPECT_EQ(ra[6], 3);
	EXPECT_EQ(ra[7], 4);
}

/** @brief Test vint8 max. */
TEST(SuiteVint8, max)
{
	vint8 a = vint8_lit(1, 2, 3, 4, 1, 2, 3, 4);
	vint8 b = vint8_lit(0, 3, 3, 5, 0, 3, 3, 5);
	vint8 r = max(a, b);

	alignas(32) int ra[8];
	store(r, ra);

	EXPECT_EQ(ra[0], 1);
	EXPECT_EQ(ra[1], 3);
	EXPECT_EQ(ra[2], 3);
	EXPECT_EQ(ra[3], 5);
	EXPECT_EQ(ra[4], 1);
	EXPECT_EQ(ra[5], 3);
	EXPECT_EQ(ra[6], 3);
	EXPECT_EQ(ra[7], 5);
}

/** @brief Test vint8 lsl. */
TEST(SuiteVint8, lsl)
{
	vint8 a = vint8_lit(1, 2, 4, -4, 1, 2, 4, -4);
	a = lsl<0>(a);

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 1);
	EXPECT_EQ(ra[1], 2);
	EXPECT_EQ(ra[2], 4);
	EXPECT_EQ(ra[3], static_cast<int>(0xFFFFFFFC));
	EXPECT_EQ(ra[4], 1);
	EXPECT_EQ(ra[5], 2);
	EXPECT_EQ(ra[6], 4);
	EXPECT_EQ(ra[7], static_cast<int>(0xFFFFFFFC));

	a = lsl<1>(a);

	store(a, ra);

	EXPECT_EQ(ra[0], 2);
	EXPECT_EQ(ra[1], 4);
	EXPECT_EQ(ra[2], 8);
	EXPECT_EQ(ra[3], static_cast<int>(0xFFFFFFF8));
	EXPECT_EQ(ra[4], 2);
	EXPECT_EQ(ra[5], 4);
	EXPECT_EQ(ra[6], 8);
	EXPECT_EQ(ra[7], static_cast<int>(0xFFFFFFF8));

	a = lsl<2>(a);

	store(a, ra);

	EXPECT_EQ(ra[0], 8);
	EXPECT_EQ(ra[1], 16);
	EXPECT_EQ(ra[2], 32);
	EXPECT_EQ(ra[3], static_cast<int>(0xFFFFFFE0));
	EXPECT_EQ(ra[4], 8);
	EXPECT_EQ(ra[5], 16);
	EXPECT_EQ(ra[6], 32);
	EXPECT_EQ(ra[7], static_cast<int>(0xFFFFFFE0));
}

/** @brief Test vint8 lsr. */
TEST(SuiteVint8, lsr)
{
	vint8 a = vint8_lit(1, 2, 4, -4, 1, 2, 4, -4);
	a = lsr<0>(a);

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0], 1);
	EXPECT_EQ(ra[1], 2);
	EXPECT_EQ(ra[2], 4);
	EXPECT_EQ(ra[3], static_cast<int>(0xFFFFFFFC));
	EXPECT_EQ(ra[4], 1);
	EXPECT_EQ(ra[5], 2);
	EXPECT_EQ(ra[6], 4);
	EXPECT_EQ(ra[7], static_cast<int>(0xFFFFFFFC));

	a = lsr<1>(a);

	store(a, ra);

	EXPECT_EQ(ra[0], 0);
	EXPECT_EQ(ra[1], 1);
	EXPECT_EQ(ra[2], 2);
	EXPECT_EQ(ra[3], 0x7FFFFFFE);
	EXPECT_EQ(ra[4], 0);
	EXPECT_EQ(ra[5], 1);
	EXPECT_EQ(ra[6], 2);
	EXPECT_EQ(ra[7], 0x7FFFFFFE);

	a = lsr<2>(a);

	store(a, ra);

	EXPECT_EQ(ra[0], 0);
	EXPECT_EQ(ra[1], 0);
	EXPECT_EQ(ra[2], 0);
	EXPECT_EQ(ra[3], 0x1FFFFFFF);
	EXPECT_EQ(ra[4], 0);
	EXPECT_EQ(ra[5], 0);
	EXPECT_EQ(ra[6], 0);
	EXPECT_EQ(ra[7], 0x1FFFFFFF);
}

/** @brief Test vint8 asr. */
TEST(SuiteVint8, asr)
{
	vint8 a = vint8_lit(1, 2, 4, -4, 1, 2, 4, -4);
	a = asr<0>(a);

	alignas(32) int ra[8];
	store(a, ra);

	EXPECT_EQ(ra[0],  1);
	EXPECT_EQ(ra[1],  2);
	EXPECT_EQ(ra[2],  4);
	EXPECT_EQ(ra[3], -4);
	EXPECT_EQ(ra[4],  1);
	EXPECT_EQ(ra[5],  2);
	EXPECT_EQ(ra[6],  4);
	EXPECT_EQ(ra[7], -4);

	a = asr<1>(a);

	store(a, ra);

	EXPECT_EQ(ra[0],  0);
	EXPECT_EQ(ra[1],  1);
	EXPECT_EQ(ra[2],  2);
	EXPECT_EQ(ra[3], -2);
	EXPECT_EQ(ra[4],  0);
	EXPECT_EQ(ra[5],  1);
	EXPECT_EQ(ra[6],  2);
	EXPECT_EQ(ra[7], -2);

	// Note - quirk of asr is that you will get "stuck" at -1
	a = asr<2>(a);

	store(a, ra);

	EXPECT_EQ(ra[0],  0);
	EXPECT_EQ(ra[1],  0);
	EXPECT_EQ(ra[2],  0);
	EXPECT_EQ(ra[3], -1);
	EXPECT_EQ(ra[4],  0);
	EXPECT_EQ(ra[5],  0);
	EXPECT_EQ(ra[6],  0);
	EXPECT_EQ(ra[7], -1);
}

/** @brief Test vint8 hmin. */
TEST(SuiteVint8, hmin)
{
	vint8 a1 = vint8_lit(1, 2, 1, 2, 1, 2, 1, 2);
	vint8 r1 = hmin(a1);

	alignas(32) int ra[8];
	store(r1, ra);

	EXPECT_EQ(ra[0], 1);
	EXPECT_EQ(ra[1], 1);
	EXPECT_EQ(ra[2], 1);
	EXPECT_EQ(ra[3], 1);
	EXPECT_EQ(ra[4], 1);
	EXPECT_EQ(ra[5], 1);
	EXPECT_EQ(ra[6], 1);
	EXPECT_EQ(ra[7], 1);

	vint8 a2 = vint8_lit(1, 2, -1, 5, 1, 2, -1, 5);
	vint8 r2 = hmin(a2);

	store(r2, ra);

	EXPECT_EQ(ra[0], -1);
	EXPECT_EQ(ra[1], -1);
	EXPECT_EQ(ra[2], -1);
	EXPECT_EQ(ra[3], -1);
	EXPECT_EQ(ra[4], -1);
	EXPECT_EQ(ra[5], -1);
	EXPECT_EQ(ra[6], -1);
	EXPECT_EQ(ra[7], -1);
}

/** @brief Test vint8 hmax. */
TEST(SuiteVint8, hmax)
{
	vint8 a1 = vint8_lit(1, 2, 1, 2, 1, 3, 1, 2);
	vint8 r1 = hmax(a1);

	alignas(32) int ra[8];
	store(r1, ra);

	EXPECT_EQ(ra[0], 3);
	EXPECT_EQ(ra[1], 3);
	EXPECT_EQ(ra[2], 3);
	EXPECT_EQ(ra[3], 3);
	EXPECT_EQ(ra[4], 3);
	EXPECT_EQ(ra[5], 3);
	EXPECT_EQ(ra[6], 3);
	EXPECT_EQ(ra[7], 3);

	vint8 a2 = vint8_lit(1, 2, -1, 5, 1, 2, -1, 5);
	vint8 r2 = hmax(a2);

	store(r2, ra);

	EXPECT_EQ(ra[0], 5);
	EXPECT_EQ(ra[1], 5);
	EXPECT_EQ(ra[2], 5);
	EXPECT_EQ(ra[3], 5);
	EXPECT_EQ(ra[4], 5);
	EXPECT_EQ(ra[5], 5);
	EXPECT_EQ(ra[6], 5);
	EXPECT_EQ(ra[7], 5);
}

/** @brief Test vint8 storea. */
TEST(SuiteVint8, storea)
{
	alignas(32) int out[8];
	vint8 a(s32_data);
	storea(a, out);
	EXPECT_EQ(out[0], 0);
	EXPECT_EQ(out[1], 1);
	EXPECT_EQ(out[2], 2);
	EXPECT_EQ(out[3], 3);
	EXPECT_EQ(out[4], 4);
	EXPECT_EQ(out[5], 5);
	EXPECT_EQ(out[6], 6);
	EXPECT_EQ(out[7], 7);
}

/** @brief Test vint8 store. */
TEST(SuiteVint8, store)
{
	vint8 a(s32_data);

	alignas(32) int ra[9];
	store(a, ra + 1);

	EXPECT_EQ(ra[1], 0);
	EXPECT_EQ(ra[2], 1);
	EXPECT_EQ(ra[3], 2);
	EXPECT_EQ(ra[4], 3);
	EXPECT_EQ(ra[5], 4);
	EXPECT_EQ(ra[6], 5);
	EXPECT_EQ(ra[7], 6);
	EXPECT_EQ(ra[8], 7);
}

/** @brief Test vint8 store_nbytes. */
TEST(SuiteVint8, store_nbytes)
{
	alignas(32) int out[2];
	vint8 a = vint8_lit(42, 314, 75, 90, 42, 314, 75, 90);
	store_nbytes(a, reinterpret_cast<uint8_t*>(&out));
	EXPECT_EQ(out[0], 42);
	EXPECT_EQ(out[1], 314);
}

/** @brief Test vint8 store_lanes_masked. */
TEST(SuiteVint8, store_lanes_masked)
{
	uint8_t resulta[32] { 0 };

	// Store nothing
	vmask8 mask1 = vint8(0) == vint8(1);
	vint8 data1 = vint8(1);

	store_lanes_masked(resulta, data1, mask1);
	vint8 result1v = vint8::load(resulta);
	vint8 expect1v = vint8::zero();
	EXPECT_TRUE(all(result1v == expect1v));

	// Store half
	vmask8 mask2 = vint8_lit(1, 1, 1, 1, 0, 0, 0, 0) == vint8(1);
	vint8 data2 = vint8(2);

	store_lanes_masked(resulta, data2, mask2);
	vint8 result2v = vint8::load(resulta);
	vint8 expect2v = vint8_lit(2, 2, 2, 2, 0, 0, 0, 0);
	EXPECT_TRUE(all(result2v == expect2v));

	// Store all
	vmask8 mask3 = vint8(1) == vint8(1);
	vint8 data3 = vint8(3);

	store_lanes_masked(resulta, data3, mask3);
	vint8 result3v = vint8::load(resulta);
	vint8 expect3v = vint8(3);
	EXPECT_TRUE(all(result3v == expect3v));
}

/** @brief Test vint8 store_lanes_masked to unaligned address. */
TEST(SuiteVint8, store_lanes_masked_unaligned)
{
	uint8_t resulta[33] { 0 };

	// Store nothing
	vmask8 mask1 = vint8(0) == vint8(1);
	vint8 data1 = vint8(1);

	store_lanes_masked(resulta + 1, data1, mask1);
	vint8 result1v = vint8::load(resulta + 1);
	vint8 expect1v = vint8::zero();
	EXPECT_TRUE(all(result1v == expect1v));

	// Store half
	vmask8 mask2 = vint8_lit(1, 1, 1, 1, 0, 0, 0, 0) == vint8(1);
	vint8 data2 = vint8(2);

	store_lanes_masked(resulta + 1, data2, mask2);
	vint8 result2v = vint8::load(resulta + 1);
	vint8 expect2v = vint8_lit(2, 2, 2, 2, 0, 0, 0, 0);
	EXPECT_TRUE(all(result2v == expect2v));

	// Store all
	vmask8 mask3 = vint8(1) == vint8(1);
	vint8 data3 = vint8(3);

	store_lanes_masked(resulta + 1, data3, mask3);
	vint8 result3v = vint8::load(resulta + 1);
	vint8 expect3v = vint8(3);
	EXPECT_TRUE(all(result3v == expect3v));
}

/** @brief Test vint8 pack_and_store_low_bytes. */
TEST(SuiteVint8, pack_and_store_low_bytes)
{
	vint8 a = vint8_lit(1, 2, 3, 4, 2, 3, 4, 5);
	uint8_t bytes[8] { 0 };

	pack_and_store_low_bytes(a, bytes);

	EXPECT_EQ(bytes[0], 1);
	EXPECT_EQ(bytes[1], 2);
	EXPECT_EQ(bytes[2], 3);
	EXPECT_EQ(bytes[3], 4);
	EXPECT_EQ(bytes[4], 2);
	EXPECT_EQ(bytes[5], 3);
	EXPECT_EQ(bytes[6], 4);
	EXPECT_EQ(bytes[7], 5);
}

/** @brief Test vint8 select. */
TEST(SuiteVint8, select)
{
	vint8 m1 = vint8_lit(1, 1, 1, 1, 1, 1, 1, 1);
	vint8 m2 = vint8_lit(1, 2, 1, 2, 1, 2, 1, 2);
	vmask8 cond = m1 == m2;

	vint8 a = vint8_lit(1, 3, 3, 1, 1, 3, 3, 1);
	vint8 b = vint8_lit(4, 2, 2, 4, 4, 2, 2, 4);

	vint8 r1 = select(a, b, cond);

	alignas(32) int ra[8];
	store(r1, ra);

	EXPECT_EQ(ra[0], 4);
	EXPECT_EQ(ra[1], 3);
	EXPECT_EQ(ra[2], 2);
	EXPECT_EQ(ra[3], 1);
	EXPECT_EQ(ra[4], 4);
	EXPECT_EQ(ra[5], 3);
	EXPECT_EQ(ra[6], 2);
	EXPECT_EQ(ra[7], 1);

	vint8 r2 = select(b, a, cond);

	store(r2, ra);

	EXPECT_EQ(ra[0], 1);
	EXPECT_EQ(ra[1], 2);
	EXPECT_EQ(ra[2], 3);
	EXPECT_EQ(ra[3], 4);
	EXPECT_EQ(ra[4], 1);
	EXPECT_EQ(ra[5], 2);
	EXPECT_EQ(ra[6], 3);
	EXPECT_EQ(ra[7], 4);
}

// vmask8 tests - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** @brief Test vmask8 scalar literal constructor. */
TEST(SuiteVmask8, scalar_literal_construct)
{
	vfloat8 ma(0.0f);
	vfloat8 mb(1.0f);

	vmask8 m1(true);
	vfloat8 r1 = select(ma, mb, m1);
	vmask8 rm1 = r1 == mb;
	EXPECT_EQ(all(rm1), true);

	vmask8 m2(false);
	vfloat8 r2 = select(ma, mb, m2);
	vmask8 rm2 = r2 == mb;
	EXPECT_EQ(any(rm2), false);
}

/** @brief Test vmask8 or. */
TEST(SuiteVmask8, TestOr)
{
	vfloat8 m1a = vfloat8_lit(0, 1, 0, 1, 0, 1, 0, 1);
	vfloat8 m1b = vfloat8_lit(1, 1, 1, 1, 1, 1, 1, 1);
	vmask8 m1 = m1a == m1b;

	vfloat8 m2a = vfloat8_lit(1, 1, 0, 0, 1, 1, 0, 0);
	vfloat8 m2b = vfloat8_lit(1, 1, 1, 1, 1, 1, 1, 1);
	vmask8 m2 = m2a == m2b;

	vmask8 r = m1 | m2;
	EXPECT_EQ(mask(r), 0xBBu);
}

/** @brief Test vmask8 and. */
TEST(SuiteVmask8, TestAnd)
{
	vfloat8 m1a = vfloat8_lit(0, 1, 0, 1, 0, 1, 0, 1);
	vfloat8 m1b = vfloat8_lit(1, 1, 1, 1, 1, 1, 1, 1);
	vmask8 m1 = m1a == m1b;

	vfloat8 m2a = vfloat8_lit(1, 1, 0, 0, 1, 1, 0, 0);
	vfloat8 m2b = vfloat8_lit(1, 1, 1, 1, 1, 1, 1, 1);
	vmask8 m2 = m2a == m2b;

	vmask8 r = m1 & m2;
	EXPECT_EQ(mask(r), 0x22u);
}

/** @brief Test vmask8 xor. */
TEST(SuiteVmask8, TestXor)
{
	vfloat8 m1a = vfloat8_lit(0, 1, 0, 1, 0, 1, 0, 1);
	vfloat8 m1b = vfloat8_lit(1, 1, 1, 1, 1, 1, 1, 1);
	vmask8 m1 = m1a == m1b;

	vfloat8 m2a = vfloat8_lit(1, 1, 0, 0, 1, 1, 0, 0);
	vfloat8 m2b = vfloat8_lit(1, 1, 1, 1, 1, 1, 1, 1);
	vmask8 m2 = m2a == m2b;

	vmask8 r = m1 ^ m2;
	EXPECT_EQ(mask(r), 0x99u);
}

/** @brief Test vmask8 not. */
TEST(SuiteVmask8, TestNot)
{
	vfloat8 m1a = vfloat8_lit(0, 1, 0, 1, 0, 1, 0, 1);
	vfloat8 m1b = vfloat8_lit(1, 1, 1, 1, 1, 1, 1, 1);
	vmask8 m1 = m1a == m1b;
	vmask8 r = ~m1;
	EXPECT_EQ(mask(r), 0x55u);
}

/** @brief Test vint8 table permute. */
TEST(SuiteVint8, vtable8_16x8)
{
	uint8_t data[16] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	};

	vtable8_16x8 table;
	vtable_prepare(table, data);

	vint8 index = vint8_lit(0, 7, 4, 15, 1, 2, 14, 4);

	vint8 result = vtable_lookup_32bit(table, index);

	alignas(32) int ra[8];
	store(result, ra);

	EXPECT_EQ(ra[0],  0);
	EXPECT_EQ(ra[1],  7);
	EXPECT_EQ(ra[2],  4);
	EXPECT_EQ(ra[3], 15);
	EXPECT_EQ(ra[4],  1);
	EXPECT_EQ(ra[5],  2);
	EXPECT_EQ(ra[6], 14);
	EXPECT_EQ(ra[7],  4);
}

/** @brief Test vint8 table permute. */
TEST(SuiteVint8, vtable8_32x8)
{
	uint8_t data[32] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
	};

	vtable8_32x8 table;
	vtable_prepare(table, data);

	vint8 index = vint8_lit(0, 7, 4, 15, 16, 20, 23, 31);

	vint8 result = vtable_lookup_32bit(table, index);

	alignas(32) int ra[8];
	store(result, ra);

	EXPECT_EQ(ra[0],  0);
	EXPECT_EQ(ra[1],  7);
	EXPECT_EQ(ra[2],  4);
	EXPECT_EQ(ra[3], 15);
	EXPECT_EQ(ra[4], 16);
	EXPECT_EQ(ra[5], 20);
	EXPECT_EQ(ra[6], 23);
	EXPECT_EQ(ra[7], 31);
}

/** @brief Test vint8 table permute. */
TEST(SuiteVint8, vtable8_64x8)
{
	uint8_t data[64] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
		0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
		0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
	};

	vtable8_64x8 table;
	vtable_prepare(table, data);

	vint8 index = vint8_lit(0, 7, 4, 15, 16, 20, 38, 63);

	vint8 result = vtable_lookup_32bit(table, index);

	alignas(32) int ra[8];
	store(result, ra);

	EXPECT_EQ(ra[0],  0);
	EXPECT_EQ(ra[1],  7);
	EXPECT_EQ(ra[2],  4);
	EXPECT_EQ(ra[3], 15);
	EXPECT_EQ(ra[4], 16);
	EXPECT_EQ(ra[5], 20);
	EXPECT_EQ(ra[6], 38);
	EXPECT_EQ(ra[7], 63);
}

#endif

}
