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
	#error The MaskedOcclusionCulling.cpp should be compiled with lowest supported target platform, e.g. /arch:SSE2
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility functions (not directly related to the algorithm/rasterizer)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MaskedOcclusionCulling::TransformVertices(const float *mtx, const float *inVtx, float *xfVtx, unsigned int nVtx, const VertexLayout &vtxLayout)
{
	// This function pretty slow, about 10-20% slower than if the vertices are stored in aligned SOA form.
	if (nVtx == 0)
		return;

	// Load matrix and swizzle out the z component. For post-multiplication (OGL), the matrix is assumed to be column 
	// major, with one column per SSE register. For pre-multiplication (DX), the matrix is assumed to be row major.
	__m128 mtxCol0 = _mm_loadu_ps(mtx);
	__m128 mtxCol1 = _mm_loadu_ps(mtx + 4);
	__m128 mtxCol2 = _mm_loadu_ps(mtx + 8);
	__m128 mtxCol3 = _mm_loadu_ps(mtx + 12);

	int stride = vtxLayout.mStride;
	const char *vPtr = (const char *)inVtx;
	float *outPtr = xfVtx;

	// Iterate through all vertices and transform
	for (unsigned int vtx = 0; vtx < nVtx; ++vtx)
	{
		__m128 xVal = _mm_load1_ps((float*)(vPtr));
		__m128 yVal = _mm_load1_ps((float*)(vPtr + vtxLayout.mOffsetY));
		__m128 zVal = _mm_load1_ps((float*)(vPtr + vtxLayout.mOffsetZ));

		__m128 xform = _mm_add_ps(_mm_mul_ps(mtxCol0, xVal), _mm_add_ps(_mm_mul_ps(mtxCol1, yVal), _mm_add_ps(_mm_mul_ps(mtxCol2, zVal), mtxCol3)));
		_mm_storeu_ps(outPtr, xform);
		vPtr += stride;
		outPtr += 4;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SSE2 version
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef MaskedOcclusionCulling::pfnAlignedAlloc pfnAlignedAlloc;
typedef MaskedOcclusionCulling::pfnAlignedFree  pfnAlignedFree;
typedef MaskedOcclusionCulling::VertexLayout    VertexLayout;

namespace MaskedOcclusionCullingSSE2
{
	#include "MaskedOcclusionCullingCommonSSE.inl"

	FORCE_INLINE __m128i _mmw_mullo_epi32(const __m128i &a, const __m128i &b)
	{ 
		// Do products for even / odd lanes & merge the result
		__m128i even = _mm_and_si128(_mm_mul_epu32(a, b), _mm_setr_epi32(~0, 0, ~0, 0));
		__m128i odd = _mm_slli_epi64(_mm_mul_epu32(_mm_srli_epi64(a, 32), _mm_srli_epi64(b, 32)), 32);
		return _mm_or_si128(even, odd);
	}
	FORCE_INLINE __m128i _mmw_min_epi32(const __m128i &a, const __m128i &b)
	{ 
		__m128i cond = _mm_cmpgt_epi32(a, b);
		return _mm_or_si128(_mm_andnot_si128(cond, a), _mm_and_si128(cond, b));
	}
	FORCE_INLINE __m128i _mmw_max_epi32(const __m128i &a, const __m128i &b)
	{ 
		__m128i cond = _mm_cmpgt_epi32(b, a);
		return _mm_or_si128(_mm_andnot_si128(cond, a), _mm_and_si128(cond, b));
	}
	FORCE_INLINE __m128i _mmw_abs_epi32(const __m128i &a)
	{
		__m128i mask = _mm_cmplt_epi32(a, _mm_setzero_si128());
		return _mm_add_epi32(_mm_xor_si128(a, mask), _mm_srli_epi32(mask, 31));
	}
	FORCE_INLINE int _mmw_testz_epi32(const __m128i &a, const __m128i &b)
	{ 
		return _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_and_si128(a, b), _mm_setzero_si128())) == 0xFFFF;
	}
	FORCE_INLINE __m128 _mmw_blendv_ps(const __m128 &a, const __m128 &b, const __m128 &c)
	{	
		__m128 cond = _mm_castsi128_ps(_mm_srai_epi32(_mm_castps_si128(c), 31));
		return _mm_or_ps(_mm_andnot_ps(cond, a), _mm_and_ps(cond, b));
	}
	FORCE_INLINE __m128 _mmx_dp4_ps(const __m128 &a, const __m128 &b)
	{ 
		// Product and two shuffle/adds pairs (similar to hadd_ps)
		__m128 prod = _mm_mul_ps(a, b);
		__m128 dp = _mm_add_ps(prod, _mm_shuffle_ps(prod, prod, _MM_SHUFFLE(2, 3, 0, 1)));
		dp = _mm_add_ps(dp, _mm_shuffle_ps(dp, dp, _MM_SHUFFLE(0, 1, 2, 3)));
		return dp;
	}
	FORCE_INLINE __m128 _mmw_floor_ps(const __m128 &a)
	{ 
		int originalMode = _MM_GET_ROUNDING_MODE();
		_MM_SET_ROUNDING_MODE(_MM_ROUND_DOWN);
		__m128 rounded = _mm_cvtepi32_ps(_mm_cvtps_epi32(a));
		_MM_SET_ROUNDING_MODE(originalMode);
		return rounded;
	}
	FORCE_INLINE __m128 _mmw_ceil_ps(const __m128 &a)
	{ 
		int originalMode = _MM_GET_ROUNDING_MODE();
		_MM_SET_ROUNDING_MODE(_MM_ROUND_UP);
		__m128 rounded = _mm_cvtepi32_ps(_mm_cvtps_epi32(a));
		_MM_SET_ROUNDING_MODE(originalMode);
		return rounded;
	}
	FORCE_INLINE __m128i _mmw_transpose_epi8(const __m128i &a)
	{
		// Perform transpose through two 16->8 bit pack and byte shifts
		__m128i res = a;
		const __m128i mask = _mm_setr_epi8(~0, 0, ~0, 0, ~0, 0, ~0, 0, ~0, 0, ~0, 0, ~0, 0, ~0, 0);
		res = _mm_packus_epi16(_mm_and_si128(res, mask), _mm_srli_epi16(res, 8));
		res = _mm_packus_epi16(_mm_and_si128(res, mask), _mm_srli_epi16(res, 8));
		return res;
	}
	FORCE_INLINE __m128i _mmw_sllv_ones(const __m128i &ishift)
	{
		__m128i shift = _mmw_min_epi32(ishift, _mm_set1_epi32(32));
		
		// Uses scalar approach to perform _mm_sllv_epi32(~0, shift)
		static const unsigned int maskLUT[33] = {
			~0U << 0, ~0U << 1, ~0U << 2 ,  ~0U << 3, ~0U << 4, ~0U << 5, ~0U << 6 , ~0U << 7, ~0U << 8, ~0U << 9, ~0U << 10 , ~0U << 11, ~0U << 12, ~0U << 13, ~0U << 14 , ~0U << 15,
			~0U << 16, ~0U << 17, ~0U << 18 , ~0U << 19, ~0U << 20, ~0U << 21, ~0U << 22 , ~0U << 23, ~0U << 24, ~0U << 25, ~0U << 26 , ~0U << 27, ~0U << 28, ~0U << 29, ~0U << 30 , ~0U << 31,
			0U };

		__m128i retMask;
		simd_i32(retMask)[0] = (int)maskLUT[simd_i32(shift)[0]];
		simd_i32(retMask)[1] = (int)maskLUT[simd_i32(shift)[1]];
		simd_i32(retMask)[2] = (int)maskLUT[simd_i32(shift)[2]];
		simd_i32(retMask)[3] = (int)maskLUT[simd_i32(shift)[3]];
		return retMask;
	}

	static MaskedOcclusionCulling::Implementation gInstructionSet = MaskedOcclusionCulling::SSE2;

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Object construction and allocation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace MaskedOcclusionCullingSSE41
{
	extern MaskedOcclusionCulling *CreateMaskedOcclusionCulling(pfnAlignedAlloc alignedAlloc, pfnAlignedFree alignedFree);
};

namespace MaskedOcclusionCullingAVX512
{
	extern MaskedOcclusionCulling *CreateMaskedOcclusionCulling(pfnAlignedAlloc alignedAlloc, pfnAlignedFree alignedFree);
}

namespace MaskedOcclusionCullingAVX2
{
	extern MaskedOcclusionCulling *CreateMaskedOcclusionCulling(pfnAlignedAlloc alignedAlloc, pfnAlignedFree alignedFree);
}

extern MaskedOcclusionCulling::Implementation DetectCPUFeatures(MaskedOcclusionCulling::pfnAlignedAlloc alignedAlloc, MaskedOcclusionCulling::pfnAlignedFree alignedFree);

MaskedOcclusionCulling *MaskedOcclusionCulling::Create(Implementation RequestedSIMD)
{
	return Create(RequestedSIMD, moc_aligned_alloc, moc_aligned_free);
}

MaskedOcclusionCulling *MaskedOcclusionCulling::Create(Implementation RequestedSIMD, pfnAlignedAlloc alignedAlloc, pfnAlignedFree alignedFree)
{
	MaskedOcclusionCulling *object = nullptr;

	MaskedOcclusionCulling::Implementation impl = DetectCPUFeatures(alignedAlloc, alignedFree);
	if (impl == UNSUPPORTED)
		return nullptr;

	if (RequestedSIMD < impl)
		impl = RequestedSIMD;

	// Return best supported version
	if (object == nullptr && impl >= AVX2)
		object = MaskedOcclusionCullingAVX2::CreateMaskedOcclusionCulling(alignedAlloc, alignedFree); // Use AVX2 version
	if (object == nullptr && impl >= SSE41)
		object = MaskedOcclusionCullingSSE41::CreateMaskedOcclusionCulling(alignedAlloc, alignedFree); // Use SSE4.1 version
	if (object == nullptr)
		object = MaskedOcclusionCullingSSE2::CreateMaskedOcclusionCulling(alignedAlloc, alignedFree); // Use SSE2 (slow) version

	return object;
}

void MaskedOcclusionCulling::Destroy(MaskedOcclusionCulling *moc)
{
	pfnAlignedFree alignedFreeCallback = moc->mAlignedFreeCallback;
	moc->~MaskedOcclusionCulling();
	alignedFreeCallback(moc);
}
