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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Common SSE2/SSE4.1 defines
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SIMD_LANES             4
#define TILE_HEIGHT_SHIFT      2

#define SIMD_LANE_IDX _mm_setr_epi32(0, 1, 2, 3)

#define SIMD_SUB_TILE_COL_OFFSET _mm_setr_epi32(0, SUB_TILE_WIDTH, SUB_TILE_WIDTH * 2, SUB_TILE_WIDTH * 3)
#define SIMD_SUB_TILE_ROW_OFFSET _mm_setzero_si128()
#define SIMD_SUB_TILE_COL_OFFSET_F _mm_setr_ps(0, SUB_TILE_WIDTH, SUB_TILE_WIDTH * 2, SUB_TILE_WIDTH * 3)
#define SIMD_SUB_TILE_ROW_OFFSET_F _mm_setzero_ps()

#define SIMD_LANE_YCOORD_I _mm_setr_epi32(128, 384, 640, 896)
#define SIMD_LANE_YCOORD_F _mm_setr_ps(128.0f, 384.0f, 640.0f, 896.0f)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Common SSE2/SSE4.1 functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef __m128 __mw;
typedef __m128i __mwi;

#define _mmw_set1_ps                _mm_set1_ps
#define _mmw_setzero_ps             _mm_setzero_ps
#define _mmw_and_ps                 _mm_and_ps
#define _mmw_or_ps                  _mm_or_ps
#define _mmw_xor_ps                 _mm_xor_ps
#define _mmw_not_ps(a)              _mm_xor_ps((a), _mm_castsi128_ps(_mm_set1_epi32(~0)))
#define _mmw_andnot_ps              _mm_andnot_ps
#define _mmw_neg_ps(a)              _mm_xor_ps((a), _mm_set1_ps(-0.0f))
#define _mmw_abs_ps(a)              _mm_and_ps((a), _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF)))
#define _mmw_add_ps                 _mm_add_ps
#define _mmw_sub_ps                 _mm_sub_ps
#define _mmw_mul_ps                 _mm_mul_ps
#define _mmw_div_ps                 _mm_div_ps
#define _mmw_min_ps                 _mm_min_ps
#define _mmw_max_ps                 _mm_max_ps
#define _mmw_movemask_ps            _mm_movemask_ps
#define _mmw_cmpge_ps(a,b)          _mm_cmpge_ps(a, b)
#define _mmw_cmpgt_ps(a,b)          _mm_cmpgt_ps(a, b)
#define _mmw_cmpeq_ps(a,b)          _mm_cmpeq_ps(a, b)
#define _mmw_fmadd_ps(a,b,c)        _mm_add_ps(_mm_mul_ps(a,b), c)
#define _mmw_fmsub_ps(a,b,c)        _mm_sub_ps(_mm_mul_ps(a,b), c)
#define _mmw_shuffle_ps             _mm_shuffle_ps
#define _mmw_insertf32x4_ps(a,b,c)  (b)
#define _mmw_cvtepi32_ps            _mm_cvtepi32_ps
#define _mmw_blendv_epi32(a,b,c)    simd_cast<__mwi>(_mmw_blendv_ps(simd_cast<__mw>(a), simd_cast<__mw>(b), simd_cast<__mw>(c)))

#define _mmw_set1_epi32             _mm_set1_epi32
#define _mmw_setzero_epi32          _mm_setzero_si128
#define _mmw_and_epi32              _mm_and_si128
#define _mmw_or_epi32               _mm_or_si128
#define _mmw_xor_epi32              _mm_xor_si128
#define _mmw_not_epi32(a)           _mm_xor_si128((a), _mm_set1_epi32(~0))
#define _mmw_andnot_epi32           _mm_andnot_si128
#define _mmw_neg_epi32(a)           _mm_sub_epi32(_mm_set1_epi32(0), (a))
#define _mmw_add_epi32              _mm_add_epi32
#define _mmw_sub_epi32              _mm_sub_epi32
#define _mmw_subs_epu16             _mm_subs_epu16
#define _mmw_cmpeq_epi32            _mm_cmpeq_epi32
#define _mmw_cmpgt_epi32            _mm_cmpgt_epi32
#define _mmw_srai_epi32             _mm_srai_epi32
#define _mmw_srli_epi32             _mm_srli_epi32
#define _mmw_slli_epi32             _mm_slli_epi32
#define _mmw_cvtps_epi32            _mm_cvtps_epi32
#define _mmw_cvttps_epi32           _mm_cvttps_epi32

#define _mmx_fmadd_ps               _mmw_fmadd_ps
#define _mmx_max_epi32              _mmw_max_epi32
#define _mmx_min_epi32              _mmw_min_epi32

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SIMD casting functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, typename Y> FORCE_INLINE T simd_cast(Y A);
template<> FORCE_INLINE __m128  simd_cast<__m128>(float A) { return _mm_set1_ps(A); }
template<> FORCE_INLINE __m128  simd_cast<__m128>(__m128i A) { return _mm_castsi128_ps(A); }
template<> FORCE_INLINE __m128  simd_cast<__m128>(__m128 A) { return A; }
template<> FORCE_INLINE __m128i simd_cast<__m128i>(int A) { return _mm_set1_epi32(A); }
template<> FORCE_INLINE __m128i simd_cast<__m128i>(__m128 A) { return _mm_castps_si128(A); }
template<> FORCE_INLINE __m128i simd_cast<__m128i>(__m128i A) { return A; }

#define MAKE_ACCESSOR(name, simd_type, base_type, is_const, elements) \
	FORCE_INLINE is_const base_type * name(is_const simd_type &a) { \
		union accessor { simd_type m_native; base_type m_array[elements]; }; \
		is_const accessor *acs = reinterpret_cast<is_const accessor*>(&a); \
		return acs->m_array; \
	}

MAKE_ACCESSOR(simd_f32, __m128, float, , 4)
MAKE_ACCESSOR(simd_f32, __m128, float, const, 4)
MAKE_ACCESSOR(simd_i32, __m128i, int, , 4)
MAKE_ACCESSOR(simd_i32, __m128i, int, const, 4)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Specialized SSE input assembly function for general vertex gather
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FORCE_INLINE void GatherVertices(__m128 *vtxX, __m128 *vtxY, __m128 *vtxW, const float *inVtx, const unsigned int *inTrisPtr, int numLanes, const VertexLayout &vtxLayout)
{
	for (int lane = 0; lane < numLanes; lane++)
	{
		for (int i = 0; i < 3; i++)
		{
			char *vPtrX = (char *)inVtx + inTrisPtr[lane * 3 + i] * vtxLayout.mStride;
			char *vPtrY = vPtrX + vtxLayout.mOffsetY;
			char *vPtrW = vPtrX + vtxLayout.mOffsetW;

			simd_f32(vtxX[i])[lane] = *((float*)vPtrX);
			simd_f32(vtxY[i])[lane] = *((float*)vPtrY);
			simd_f32(vtxW[i])[lane] = *((float*)vPtrW);
		}
	}
}
