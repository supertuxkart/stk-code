/* ==========================================================================
 * Copyright (c) 2022 SuperTuxKart-Team
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ==========================================================================
 */
#include "simd_wrapper.h"

#include <cstddef>
#include "MaskedOcclusionCulling.h"

#include <SDL_cpuinfo.h>

// This file needs to be put inside sse folder so neon is on for android
MaskedOcclusionCulling::Implementation DetectCPUFeatures(MaskedOcclusionCulling::pfnAlignedAlloc alignedAlloc, MaskedOcclusionCulling::pfnAlignedFree alignedFree)
{
	// When using simde uses the one with the most conversion supported
	// AVX2 at the moment has little neon conversion, so using AVX2 will be
	// slower with simde
	MaskedOcclusionCulling::Implementation fastest = MaskedOcclusionCulling::SSE41;

#if defined(OC_NATIVE_SIMD)
	if (SDL_HasAVX2() == SDL_TRUE)
		return MaskedOcclusionCulling::AVX2;
	if (SDL_HasSSE41() == SDL_TRUE)
		return MaskedOcclusionCulling::SSE41;
	if (SDL_HasSSE2() == SDL_TRUE)
		return MaskedOcclusionCulling::SSE2;
	return MaskedOcclusionCulling::UNSUPPORTED;

#elif defined(SIMDE_ARCH_ARM_NEON) && !defined(SIMDE_NO_NATIVE)
	// We use simde for arm with sse to neon so returns the fastest method
	// if neon is available
	if (SDL_HasNEON() == SDL_TRUE)
		return fastest;
	return MaskedOcclusionCulling::UNSUPPORTED;

#else
	// Assume the rest platforms always have hardware support
	return fastest;
#endif

}
