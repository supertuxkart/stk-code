// SPDX-License-Identifier: Apache-2.0
// ----------------------------------------------------------------------------
// Copyright 2024 Arm Limited
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
 * @brief Application entry point second veneer.
 *
 * This module contains the second command line entry point veneer, used to
 * validate that Arm SVE vector width matches the tool build. When used, it is
 * compiled with SVE ISA support but without any vector legnth override, so it
 * will see the native SVE vector length exposed to the application.
 */

#include <cstdio>

#if ASTCENC_SVE != 0
	#include <arm_sve.h>
#endif

/**
 * @brief The main entry point.
 *
 * @param argc   The number of arguments.
 * @param argv   The vector of arguments.
 *
 * @return 0 on success, non-zero otherwise.
 */
int astcenc_main(
	int argc,
	char **argv);

/**
 * @brief Print a formatted string to stderr.
 */
template<typename ... _Args>
static inline void print_error(
	const char* format,
	_Args...args
) {
	fprintf(stderr, format, args...);
}

int astcenc_main_veneer(
	int argc,
	char **argv
) {
	// We don't need this check for 128-bit SVE, because that is compiled as
	// VLA code, using predicate masks in the augmented NEON.
#if ASTCENC_SVE > 4
	// svcntw() returns compile-time length if used with -msve-vector-bits
	if (svcntw() != ASTCENC_SVE)
	{
		int bits = ASTCENC_SVE * 32;
		print_error("ERROR: Host SVE support is not a %u-bit implementation\n", bits);
		return 1;
	}
#endif

	return astcenc_main(argc, argv);
}
