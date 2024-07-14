////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.
////////////////////////////////////////////////////////////////////////////////
#include <algorithm>
#include <vector>
#include <string.h>
#include <assert.h>
#include <float.h>
#include "MaskedOcclusionCulling.h"
#include "CompilerSpecific.inl"

#if MOC_RECORDER_ENABLE
#include "FrameRecorder.h"
#endif

#if defined(__AVX__) || defined(__AVX2__)
	// For performance reasons, the MaskedOcclusionCullingAVX2/512.cpp files should be compiled with VEX encoding for SSE instructions (to avoid
	// AVX-SSE transition penalties, see https://software.intel.com/en-us/articles/avoiding-avx-sse-transition-penalties). However, this file
	// _must_ be compiled without VEX encoding to allow backwards compatibility. Best practice is to use lowest supported target platform
	// (/arch:SSE2) as project default, and elevate only the MaskedOcclusionCullingAVX2/512.cpp files.
	#error The MaskedOcclusionCullingSSE41.cpp should be compiled with lowest supported target platform, e.g. /arch:SSE2
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef MaskedOcclusionCulling::pfnAlignedAlloc pfnAlignedAlloc;
typedef MaskedOcclusionCulling::pfnAlignedFree  pfnAlignedFree;
typedef MaskedOcclusionCulling::VertexLayout    VertexLayout;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SSE4.1 version
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace MaskedOcclusionCullingSSE41
{
	#include "MaskedOcclusionCullingCommonSSE.inl"

	FORCE_INLINE __m128i _mmw_mullo_epi32(const __m128i &a, const __m128i &b) { return _mm_mullo_epi32(a, b); }
	FORCE_INLINE __m128i _mmw_min_epi32(const __m128i &a, const __m128i &b) { return _mm_min_epi32(a, b); }
	FORCE_INLINE __m128i _mmw_max_epi32(const __m128i &a, const __m128i &b) { return _mm_max_epi32(a, b); }
	FORCE_INLINE __m128i _mmw_abs_epi32(const __m128i &a) { return _mm_abs_epi32(a); }
	FORCE_INLINE __m128 _mmw_blendv_ps(const __m128 &a, const __m128 &b, const __m128 &c) { return _mm_blendv_ps(a, b, c); }
	FORCE_INLINE int _mmw_testz_epi32(const __m128i &a, const __m128i &b) { return _mm_testz_si128(a, b); }
	FORCE_INLINE __m128 _mmx_dp4_ps(const __m128 &a, const __m128 &b) { return _mm_dp_ps(a, b, 0xFF); }
	FORCE_INLINE __m128 _mmw_floor_ps(const __m128 &a) { return _mm_round_ps(a, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC); }
	FORCE_INLINE __m128 _mmw_ceil_ps(const __m128 &a) { return _mm_round_ps(a, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC);	}
	FORCE_INLINE __m128i _mmw_transpose_epi8(const __m128i &a)
	{
		const __m128i shuff = _mm_setr_epi8(0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF);
		return _mm_shuffle_epi8(a, shuff);
	}
	FORCE_INLINE __m128i _mmw_sllv_ones(const __m128i &ishift)
	{
		__m128i shift = _mm_min_epi32(ishift, _mm_set1_epi32(32));

		// Uses lookup tables and _mm_shuffle_epi8 to perform _mm_sllv_epi32(~0, shift)
		const __m128i byteShiftLUT = _mm_setr_epi8((char)0xFF, (char)0xFE, (char)0xFC, (char)0xF8, (char)0xF0, (char)0xE0, (char)0xC0, (char)0x80, 0, 0, 0, 0, 0, 0, 0, 0);
		const __m128i byteShiftOffset = _mm_setr_epi8(0, 8, 16, 24, 0, 8, 16, 24, 0, 8, 16, 24, 0, 8, 16, 24);
		const __m128i byteShiftShuffle = _mm_setr_epi8(0x0, 0x0, 0x0, 0x0, 0x4, 0x4, 0x4, 0x4, 0x8, 0x8, 0x8, 0x8, 0xC, 0xC, 0xC, 0xC);

		__m128i byteShift = _mm_shuffle_epi8(shift, byteShiftShuffle);
		byteShift = _mm_min_epi8(_mm_subs_epu8(byteShift, byteShiftOffset), _mm_set1_epi8(8));
		__m128i retMask = _mm_shuffle_epi8(byteShiftLUT, byteShift);

		return retMask;
	}

	static MaskedOcclusionCulling::Implementation gInstructionSet = MaskedOcclusionCulling::SSE41;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Include common algorithm implementation (general, SIMD independent code)
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	#include "MaskedOcclusionCullingCommon.inl"

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Utility function to create a new object using the allocator callbacks
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	MaskedOcclusionCulling *CreateMaskedOcclusionCulling(pfnAlignedAlloc alignedAlloc, pfnAlignedFree alignedFree)
	{
		MaskedOcclusionCullingPrivate *object = (MaskedOcclusionCullingPrivate *)alignedAlloc(64, sizeof(MaskedOcclusionCullingPrivate));
		new (object) MaskedOcclusionCullingPrivate(alignedAlloc, alignedFree);
		return object;
	}
};
