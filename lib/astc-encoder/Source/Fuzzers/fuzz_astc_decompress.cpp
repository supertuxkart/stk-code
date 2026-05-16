// SPDX-License-Identifier: Apache-2.0
// ----------------------------------------------------------------------------
// Copyright 2026 Arm Limited
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

#include "astcenc.h"
#include <fuzzer/FuzzedDataProvider.h>
#include <vector>
#include <iostream>

extern "C" int LLVMFuzzerTestOneInput(
	const uint8_t *data,
	size_t size
) {
	FuzzedDataProvider fdp(data, size);

	// Randomize block size
	// Legal 2D block sizes: 4x4, 5x4, 5x5, 6x5, 6x6, 8x5, 8x6, 8x8, 10x5, 10x6, 10x8, 10x10, 12x10, 12x12
	static const struct { uint8_t x, y; } block_sizes[] = {
		{4, 4}, {5, 4}, {5, 5}, {6, 5}, {6, 6}, {8, 5}, {8, 6}, {8, 8},
		{10, 5}, {10, 6}, {10, 8}, {10, 10}, {12, 10}, {12, 12}
	};

	int bs_idx = fdp.ConsumeIntegralInRange<int>(0, (sizeof(block_sizes) / sizeof(block_sizes[0])) - 1);
	uint32_t block_x = block_sizes[bs_idx].x;
	uint32_t block_y = block_sizes[bs_idx].y;
	uint32_t block_z = 1;

	// Randomize profile
	astcenc_profile profile = (astcenc_profile)fdp.ConsumeIntegralInRange<int>(0, 3);

	// Randomize quality
	float quality = fdp.ConsumeFloatingPointInRange<float>(ASTCENC_PRE_FASTEST, ASTCENC_PRE_EXHAUSTIVE);

	// Randomize flags, but constrain to be safe with arbitrary input data
	unsigned int flags = fdp.ConsumeIntegralInRange<unsigned int>(0, ASTCENC_ALL_FLAGS);
	flags &= ~ASTCENC_FLG_SELF_DECOMPRESS_ONLY;

	astcenc_config config;
	astcenc_error status = astcenc_config_init(profile, block_x, block_y, block_z, quality, flags, &config);
	if (status != ASTCENC_SUCCESS)
	{
		return 0;
	}

	astcenc_context* context = nullptr;
	status = astcenc_context_alloc(&config, 1, &context, nullptr);
	if (status != ASTCENC_SUCCESS)
	{
		return 0;
	}

	// Prepare for decompression
	// We need some "compressed" data. ASTC blocks are 16 bytes.
	// Let's decide how many blocks to decompress.
	size_t num_blocks_x = fdp.ConsumeIntegralInRange<size_t>(1, 4);
	size_t num_blocks_y = fdp.ConsumeIntegralInRange<size_t>(1, 4);
	size_t compressed_data_len = num_blocks_x * num_blocks_y * 16;

	if (fdp.remaining_bytes() < compressed_data_len)
	{
		astcenc_context_free(context);
		return 0;
	}

	std::vector<uint8_t> compressed_data = fdp.ConsumeBytes<uint8_t>(compressed_data_len);

	// Image dimensions
	uint32_t dim_x = num_blocks_x * block_x;
	uint32_t dim_y = num_blocks_y * block_y;
	uint32_t dim_z = 1;

	// Allocate output image
	astcenc_image image_out;
	image_out.dim_x = dim_x;
	image_out.dim_y = dim_y;
	image_out.dim_z = dim_z;
	image_out.data_type = ASTCENC_TYPE_U8;

	uint8_t* slice_data = new uint8_t[dim_x * dim_y * 4];
	void* slices[] = { slice_data };
	image_out.data = slices;

	astcenc_swizzle swizzle = {
		ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A
	};

	status = astcenc_decompress_image(context, compressed_data.data(), compressed_data.size(), &image_out, &swizzle, 0);

	delete[] slice_data;
	astcenc_context_free(context);

	return 0;
}
