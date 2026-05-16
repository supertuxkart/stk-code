// SPDX-License-Identifier: Apache-2.0
// ----------------------------------------------------------------------------
// Copyright 2021 Arm Limited
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
 * @brief Unit tests for the software half-float library.
 */

#include "gtest/gtest.h"

#include "../astcenc_internal.h"

namespace astcenc
{

#if (ASTCENC_F16C == 0) && (ASTCENC_NEON == 0)

/** @brief Test normal numbers. */
TEST(softfloat, FP16NormalNumbers)
{
	float result = sf16_to_float((15 << 10) + 1);
	EXPECT_NEAR(result,  1.00098f, 0.00005f);
}

/** @brief Test denormal numbers. */
TEST(softfloat, FP16DenormalNumbers)
{
	float result = sf16_to_float((0 << 10) + 1);
	EXPECT_NEAR(result, 5.96046e-08f, 0.00005f);
}

/** @brief Test zero. */
TEST(softfloat, FP16Zero)
{
	float result = sf16_to_float(0x0000);
	EXPECT_EQ(result, 0.0f);
}

/** @brief Test infinity. */
TEST(softfloat, FP16Infinity)
{
	float result = sf16_to_float((31 << 10) + 0);
	EXPECT_TRUE(std::isinf(result));
}

/** @brief Test NaN. */
TEST(softfloat, FP16NaN)
{
	float result = sf16_to_float(0xFFFF);
	EXPECT_TRUE(std::isnan(result));
}

#endif

}
