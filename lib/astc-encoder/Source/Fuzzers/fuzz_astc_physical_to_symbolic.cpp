// SPDX-License-Identifier: Apache-2.0
// ----------------------------------------------------------------------------
// Copyright 2020-2021 Arm Limited
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
 * @brief Fuzz target for physical_to_symbolic().
 *
 * This function is the first entrypoint for decompressing a 16 byte block of
 * input ASTC data from disk. The 16 bytes can contain arbitrary data; they
 * are read from an external source, but the block size used must be a valid
 * ASTC block footprint.
 */

#include "astcenc_internal.h"

#include <fuzzer/FuzzedDataProvider.h>
#include <array>
#include <vector>

struct BlockSizes
{
	int x;
	int y;
	int z;
};

std::array<BlockSizes, 3> testSz {{
	{ 4,  4, 1}, // Highest bitrate
	{12, 12, 1}, // Largest 2D block
	{6,  6,  6}  // Largest 3D block
}};

std::array<block_size_descriptor, 3> testBSD;

/**
 * @brief Utility function to create all of the block size descriptors needed.
 *
 * This is triggered once via a static initializer.
 *
 * Triggering once is important so that we only create a single BSD per block
 * size we need, rather than one per fuzzer iteration (it's expensive). This
 * improves fuzzer throughput by ~ 1000x!
 *
 * Triggering via a static initializer, rather than a lazy init in the fuzzer
 * function, is important because is means that the BSD is allocated before
 * fuzzing starts. This means that leaksanitizer will ignore the fact that we
 * "leak" the dynamic allocations inside the BSD (we never call term()).
 */
bool bsd_initializer()
{
	for (int i = 0; i < testSz.size(); i++)
	{
		init_block_size_descriptor(
		    testSz[i].x,
		    testSz[i].y,
		    testSz[i].z,
		    false,
		    4,
		    1.0f,
		    testBSD[i]);
	}

	return true;
}

extern "C" int LLVMFuzzerTestOneInput(
	const uint8_t *data,
	size_t size
) {
	// Preinitialize the block size descriptors we need
	static bool init = bsd_initializer();

	// Must have 4 (select block size) and 16 (payload) bytes
	if (size < 4 + 16)
	{
		return 0;
	}

	FuzzedDataProvider stream(data, size);

	// Select a block size to test
	int i = stream.ConsumeIntegralInRange<int>(0, testSz.size() - 1);

	// Populate the physical block
	uint8_t pcb[16];
	std::vector<uint8_t> buffer = stream.ConsumeBytes<uint8_t>(16);
	std::memcpy(pcb, buffer.data(), 16);

	// Call the function under test
	symbolic_compressed_block scb;
	physical_to_symbolic(testBSD[i], pcb, scb);

	return 0;
}
