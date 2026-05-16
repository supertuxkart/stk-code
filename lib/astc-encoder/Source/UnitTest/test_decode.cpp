// SPDX-License-Identifier: Apache-2.0
// ----------------------------------------------------------------------------
// Copyright 2023-2026 Arm Limited
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
 */

#include <limits>

#include "gtest/gtest.h"

#include "../astcenc.h"

namespace astcenc
{

/** @brief Test harness for exploring issue #447. */
TEST(decode, decode12x12)
{
	astcenc_error status;
	astcenc_config config;
	astcenc_context* context;

	static const astcenc_swizzle swizzle {
		ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A
	};

	uint8_t data[16] {
#if 0
		0x84,0x00,0x38,0xC8,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0xB3,0x4D,0x78
#else
		0x29,0x00,0x1A,0x97,0x01,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0xCF,0x97,0x86
#endif
	};

	uint8_t output[12*12*4];
	astcenc_config_init(ASTCENC_PRF_LDR, 12, 12, 1, ASTCENC_PRE_MEDIUM, 0, &config);

	status = astcenc_context_alloc(&config, 1, &context, nullptr);
	EXPECT_EQ(status, ASTCENC_SUCCESS);

	astcenc_image image;
	image.dim_x = 12;
	image.dim_y = 12;
	image.dim_z = 1;
	image.data_type = ASTCENC_TYPE_U8;
	uint8_t* slices = output;
	image.data = reinterpret_cast<void**>(&slices);

	status = astcenc_decompress_image(context, data, 16, &image, &swizzle, 0);
	EXPECT_EQ(status, ASTCENC_SUCCESS);
#if 0
	for (int y = 0; y < 12; y++)
	{
		for (int x = 0; x < 12; x++)
		{
			uint8_t* pixel = output + (12 * 4 * y) + (4 * x);
			printf("[%2dx%2d] = %03d, %03d, %03d, %03d\n", x, y, pixel[0], pixel[1], pixel[2], pixel[3]);
		}
	}
#endif

	astcenc_context_free(context);
}

/** @brief Test harness for context inheritance. */
TEST(decode, context_inherit)
{
	astcenc_error status;
	astcenc_config config;
	astcenc_context* parent_context;
	astcenc_context* context;
	astcenc_context* error_context;

	static const astcenc_swizzle swizzle {
		ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A
	};

	uint8_t data[16] {
#if 0
		0x84,0x00,0x38,0xC8,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0xB3,0x4D,0x78
#else
		0x29,0x00,0x1A,0x97,0x01,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0xCF,0x97,0x86
#endif
	};

	uint8_t output[12*12*4];
	astcenc_config_init(ASTCENC_PRF_LDR, 12, 12, 1, ASTCENC_PRE_MEDIUM, 0, &config);

	status = astcenc_context_alloc(&config, 1, &parent_context, nullptr);
	EXPECT_EQ(status, ASTCENC_SUCCESS);

	status = astcenc_context_alloc(nullptr, 1, &context, parent_context);
	EXPECT_EQ(status, ASTCENC_SUCCESS);

	status = astcenc_context_alloc(&config, 1, &error_context, parent_context);
	EXPECT_EQ(status, ASTCENC_ERR_BAD_PARAM);

	astcenc_image image;
	image.dim_x = 12;
	image.dim_y = 12;
	image.dim_z = 1;
	image.data_type = ASTCENC_TYPE_U8;
	uint8_t* slices = output;
	image.data = reinterpret_cast<void**>(&slices);

	status = astcenc_decompress_image(context, data, 16, &image, &swizzle, 0);
	EXPECT_EQ(status, ASTCENC_SUCCESS);

	astcenc_context_free(context);
	astcenc_context_free(parent_context);
}

}
