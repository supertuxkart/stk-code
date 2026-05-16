// SPDX-License-Identifier: Apache-2.0
// ----------------------------------------------------------------------------
// Copyright 2020-2024 Arm Limited
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
 * @brief Application entry point.
 *
 * This module contains the first command line entry point veneer, used to
 * validate that the host extended ISA availability matches the tool build.
 * It is compiled without any extended ISA support so it's guaranteed to be
 * executable without any invalid instruction errors.
 */

#include <cstdio>

/**
 * @brief The main veneer entry point.
 *
 * @param argc   The number of arguments.
 * @param argv   The vector of arguments.
 *
 * @return 0 on success, non-zero otherwise.
 */
int astcenc_main_veneer(
	int argc,
	char **argv);

// x86-64 builds
#if (ASTCENC_SSE > 20)    || (ASTCENC_AVX > 0) || \
    (ASTCENC_POPCNT > 0) || (ASTCENC_F16C > 0)

static bool g_init { false };

/** Does this CPU support SSE 4.1? Set to -1 if not yet initialized. */
static bool g_cpu_has_sse41 { false };

/** Does this CPU support AVX2? Set to -1 if not yet initialized. */
static bool g_cpu_has_avx2 { false };

/** Does this CPU support POPCNT? Set to -1 if not yet initialized. */
static bool g_cpu_has_popcnt { false };

/** Does this CPU support F16C? Set to -1 if not yet initialized. */
static bool g_cpu_has_f16c { false };

/* ============================================================================
   Platform code for Visual Studio
============================================================================ */
#if !defined(__clang__) && defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>

/**
 * @brief Detect platform CPU ISA support and update global trackers.
 */
static void detect_cpu_isa()
{
	int data[4];

	__cpuid(data, 0);
	int num_id = data[0];

	if (num_id >= 1)
	{
		__cpuidex(data, 1, 0);
		// SSE41 = Bank 1, ECX, bit 19
		g_cpu_has_sse41 = data[2] & (1 << 19) ? true : false;
		// POPCNT = Bank 1, ECX, bit 23
		g_cpu_has_popcnt = data[2] & (1 << 23) ? true : false;
		// F16C = Bank 1, ECX, bit 29
		g_cpu_has_f16c = data[2] & (1 << 29) ? true : false;
	}

	if (num_id >= 7)
	{
		__cpuidex(data, 7, 0);
		// AVX2 = Bank 7, EBX, bit 5
		g_cpu_has_avx2 = data[1] & (1 << 5) ? true : false;
	}

	// Ensure state bits are updated before init flag is updated
	MemoryBarrier();
	g_init = true;
}

/* ============================================================================
   Platform code for GCC and Clang
============================================================================ */
#else
#include <cpuid.h>

/**
 * @brief Detect platform CPU ISA support and update global trackers.
 */
static void detect_cpu_isa()
{
	unsigned int data[4];

	if (__get_cpuid_count(1, 0, &data[0], &data[1], &data[2], &data[3]))
	{
		// SSE41 = Bank 1, ECX, bit 19
		g_cpu_has_sse41 = data[2] & (1 << 19) ? true : false;
		// POPCNT = Bank 1, ECX, bit 23
		g_cpu_has_popcnt = data[2] & (1 << 23) ? true : false;
		// F16C = Bank 1, ECX, bit 29
		g_cpu_has_f16c = data[2] & (1 << 29) ? true : false;
	}

	g_cpu_has_avx2 = 0;
	if (__get_cpuid_count(7, 0, &data[0], &data[1], &data[2], &data[3]))
	{
		// AVX2 = Bank 7, EBX, bit 5
		g_cpu_has_avx2 = data[1] & (1 << 5) ? true : false;
	}

	// Ensure state bits are updated before init flag is updated
	__sync_synchronize();
	g_init = true;
}
#endif

#if ASTCENC_POPCNT > 0
/**
 * @brief Run-time detection if the host CPU supports the POPCNT extension.
 *
 * @return @c true if supported, @c false if not.
 */
static bool cpu_supports_popcnt()
{
	if (!g_init)
	{
		detect_cpu_isa();
	}

	return g_cpu_has_popcnt;
}
#endif

#if ASTCENC_F16C > 0
/**
 * @brief Run-time detection if the host CPU supports F16C extension.
 *
 * @return @c true if supported, @c false if not.
 */
static bool cpu_supports_f16c()
{
	if (!g_init)
	{
		detect_cpu_isa();
	}

	return g_cpu_has_f16c;
}
#endif

#if ASTCENC_SSE >= 41
/**
 * @brief Run-time detection if the host CPU supports SSE 4.1 extension.
 *
 * @return @c true if supported, @c false if not.
 */
static bool cpu_supports_sse41()
{
	if (!g_init)
	{
		detect_cpu_isa();
	}

	return g_cpu_has_sse41;
}
#endif

#if ASTCENC_AVX >= 2
/**
 * @brief Run-time detection if the host CPU supports AVX 2 extension.
 *
 * @return @c true if supported, @c false if not.
 */
static bool cpu_supports_avx2()
{
	if (!g_init)
	{
		detect_cpu_isa();
	}

	return g_cpu_has_avx2;
}
#endif

/**
 * @brief Print a string to stderr.
 */
static inline void print_error(
	const char* format
) {
	fprintf(stderr, "%s", format);
}

/**
 * @brief Validate CPU ISA support meets the requirements of this build of the library.
 *
 * Each library build is statically compiled for a particular set of CPU ISA features, such as the
 * SIMD support or other ISA extensions such as POPCNT. This function checks that the host CPU
 * actually supports everything this build needs.
 *
 * @return Return @c true if validated, @c false otherwise.
 */
static bool validate_cpu_isa()
{
	#if ASTCENC_AVX >= 2
		if (!cpu_supports_avx2())
		{
			print_error("ERROR: Host does not support AVX2 ISA extension\n");
			return false;
		}
	#endif

	#if ASTCENC_F16C >= 1
		if (!cpu_supports_f16c())
		{
			print_error("ERROR: Host does not support F16C ISA extension\n");
			return false;
		}
	#endif

	#if ASTCENC_SSE >= 41
		if (!cpu_supports_sse41())
		{
			print_error("ERROR: Host does not support SSE4.1 ISA extension\n");
			return false;
		}
	#endif

	#if ASTCENC_POPCNT >= 1
		if (!cpu_supports_popcnt())
		{
			print_error("ERROR: Host does not support POPCNT ISA extension\n");
			return false;
		}
	#endif

	return true;
}

// Validate Arm SVE availability
#elif ASTCENC_SVE != 0

#include <sys/auxv.h>
static bool cpu_supports_sve()
{
	long hwcaps = getauxval(AT_HWCAP);
	return (hwcaps & HWCAP_SVE) != 0;
}

/**
 * @brief Print a string to stderr.
 */
static inline void print_error(
	const char* format
) {
	fprintf(stderr, "%s", format);
}

/**
 * @brief Validate that SVE is supported.
 *
 * Note that this function checks that SVE is supported, but because it
 * runs in the veneer which is compiled without SVE support, we cannot
 * check the SVE width is correct. This is checked later.
 */
static bool validate_cpu_isa()
{
	if (!cpu_supports_sve())
	{
		print_error("ERROR: Host does not support SVE ISA extension\n");
		return false;
	}

	return true;
}

#else

// Fallback for cases with no dynamic ISA availability
static bool validate_cpu_isa()
{
	return true;
}

#endif

int main(
	int argc,
	char **argv
) {
	if (!validate_cpu_isa())
	{
		return 1;
	}

	return astcenc_main_veneer(argc, argv);
}
