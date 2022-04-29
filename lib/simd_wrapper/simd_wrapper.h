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
#ifndef HEADER_SIMD_WRAPPER_HPP
#define HEADER_SIMD_WRAPPER_HPP

#include <simde/simde-arch.h>
#if defined(SIMDE_ARCH_AMD64) || defined(SIMDE_ARCH_X86)
// Native SSE
#if __MMX__ || CPU_ENABLE_MMX
 #include <mmintrin.h>
 #define CPU_MMX_SUPPORT (1)
#endif
#if __SSE__ || defined(_M_X64) || ( defined(_M_IX86_FP) && ( _M_IX86_FP >= 1 ) ) || CPU_ENABLE_SSE
 #include <xmmintrin.h>
 #define CPU_SSE_SUPPORT (1)
#endif
#if __SSE2__ || defined(_M_X64) || ( defined(_M_IX86_FP) && ( _M_IX86_FP >= 2 ) ) || CPU_ENABLE_SSE2
 #include <emmintrin.h>
 #define CPU_SSE2_SUPPORT (1)
#endif
#if __SSE3__ || __AVX__ || CPU_ENABLE_SSE3
 #include <pmmintrin.h>
 #define CPU_SSE3_SUPPORT (1)
#endif
#if __SSSE3__ || __AVX__  || CPU_ENABLE_SSSE3
 #include <tmmintrin.h>
 #define CPU_SSSE3_SUPPORT (1)
#endif
#if __SSE4_1__ || __AVX__  || CPU_ENABLE_SSE4_1
 #include <smmintrin.h>
 #define CPU_SSE4_1_SUPPORT (1)
#endif
#if __SSE4_2__ || CPU_ENABLE_SSE4_2
 #include <nmmintrin.h>
 #define CPU_SSE4_2_SUPPORT (1)
#endif

#elif defined(SIMDE_ARCH_ARM_NEON)
// We only enable compile time SSE* to Neon for now because it's easy to test
// Enable up to SSE4.2 because after that (starting from AVX) it has few
// native conversion, which will use the slower C99 fallback
#define CPU_MMX_SUPPORT (1)
#define CPU_SSE_SUPPORT (1)
#define CPU_SSE2_SUPPORT (1)
#define CPU_SSE3_SUPPORT (1)
#define CPU_SSSE3_SUPPORT (1)
#define CPU_SSE4_1_SUPPORT (1)
#define CPU_SSE4_2_SUPPORT (1)

#if defined(_MSC_VER) && defined(__cplusplus)
// Fix math related functions missing in msvc
#include <cmath>
#endif

#define SIMDE_ENABLE_NATIVE_ALIASES
#include "simde/x86/sse4.2.h"
#endif

#ifndef _MM_FROUND_TO_NEG_INF
#define _MM_FROUND_TO_NEG_INF SIMDE_MM_FROUND_TO_NEG_INF
#endif

#ifndef _MM_FROUND_NO_EXC
#define _MM_FROUND_NO_EXC SIMDE_MM_FROUND_NO_EXC
#endif

#ifndef _MM_SET_ROUNDING_MODE
#define _MM_SET_ROUNDING_MODE _MM_SET_ROUNDING_MODE
#endif

#ifndef _MM_ROUND_NEAREST
#define _MM_ROUND_NEAREST SIMDE_MM_ROUND_NEAREST
#endif

#ifndef _MM_ROUND_UP
#define _MM_ROUND_UP SIMDE_MM_ROUND_UP
#endif

#ifndef _MM_ROUND_DOWN
#define _MM_ROUND_DOWN SIMDE_MM_ROUND_DOWN
#endif

// Utilities for aligned allocation
inline void* simd_aligned_alloc(size_t alignment, size_t bytes)
{
    // we need to allocate enough storage for the requested bytes, some
    // book-keeping (to store the location returned by malloc) and some extra
    // padding to allow us to find an aligned byte. I'm not entirely sure if
    // 2 * alignment is enough here, its just a guess.
    const size_t total_size = bytes + (2 * alignment) + sizeof(size_t);

    // use malloc to allocate the memory.
    char* data = (char*)malloc(sizeof(char) * total_size);

    if (data)
    {
        // store the original start of the malloc'd data.
        const void* const data_start = data;

        // dedicate enough space to the book-keeping.
        data += sizeof(size_t);

        // find a memory location with correct alignment.  the alignment minus 
        // the remainder of this mod operation is how many bytes forward we need 
        // to move to find an aligned byte.
        const size_t offset = alignment - (((size_t)data) % alignment);

        // set data to the aligned memory.
        data += offset;

        // write the book-keeping.
        size_t* book_keeping = (size_t*)(data - sizeof(size_t));
        *book_keeping = (size_t)data_start;
    }

    return data;
}

inline void simd_aligned_free(void* raw_data)
{
    if (raw_data)
    {
        char* data = (char*)raw_data;

        // we have to assume this memory was allocated with simd_aligned_alloc.  
        // this means the sizeof(size_t) bytes before data are the book-keeping 
        // which points to the location we need to pass to free.
        data -= sizeof(size_t);

        // set data to the location stored in book-keeping.
        data = (char*)(*((size_t*)data));

        // free the memory.
        free(data);
    }
}

#endif
