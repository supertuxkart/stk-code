cmake_minimum_required(VERSION 3.6.0)

include_directories("${PROJECT_SOURCE_DIR}/lib/simd_wrapper")

# Occlusion culling

find_path(SDL2_INCLUDEDIR NAMES SDL.h PATH_SUFFIXES SDL2 include/SDL2 include PATHS)
if (NOT SDL2_INCLUDEDIR)
    message(FATAL_ERROR "SDL2 not found.")
else()
    include_directories("${SDL2_INCLUDEDIR}")
endif()
include_directories("${PROJECT_SOURCE_DIR}/lib/graphics_utils/occlusion")

set(MOC_SSE2_FILES
    occlusion/sse2/MaskedOcclusionCulling.cpp
    occlusion/sse2/MaskedOcclusionCullingDetectCPU.cpp
)

set(MOC_SSE41_FILES
    occlusion/sse41/MaskedOcclusionCullingSSE41.cpp
)

set(MOC_AVX2_FILES
    occlusion/avx2/MaskedOcclusionCullingAVX2.cpp
)

set(OCCLUSION_CULLING_SOURCES
    ${MOC_SSE2_FILES}
    ${MOC_SSE41_FILES}
    ${MOC_AVX2_FILES}
    occlusion/CullingThreadpool.cpp
)

if(MSVC)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" arch_lower)
    if(arch_lower MATCHES "arm64" OR arch_lower MATCHES "aarch64" OR ${CMAKE_GENERATOR} MATCHES "(ARM64)" OR ${BUILD_ARCH} MATCHES "(ARM64)")
        message(STATUS "Occlusion culling: not x86 (64), use simde")
    else()
        if(NOT "${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
            # SSE2 is always enabled on 64-bit architectures, specifying redundant flag produces a compiler warning
            if(MSVC_VERSION LESS 1900)
                SET_SOURCE_FILES_PROPERTIES(${MOC_SSE2_FILES} PROPERTIES COMPILE_FLAGS "/arch:SSE2")
                SET_SOURCE_FILES_PROPERTIES(${MOC_SSE41_FILES} PROPERTIES COMPILE_FLAGS "/arch:SSE2")
            endif()
        endif()
        SET_SOURCE_FILES_PROPERTIES(${MOC_AVX2_FILES} PROPERTIES COMPILE_FLAGS "/arch:AVX2")
        message(STATUS "Occlusion culling: native simd supported")
    endif()
else()
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles(
    "#include <immintrin.h>
    int main() { return 0; }"
    OC_NATIVE_SIMD_EXISTS)
    if (NOT OC_NATIVE_SIMD_EXISTS)
        message(STATUS "Occlusion culling: not x86 (64), use simde")
    else()
        # Setup compiler flags for SSE4.1 / SSE2 files
        SET_SOURCE_FILES_PROPERTIES(${MOC_SSE2_FILES} PROPERTIES COMPILE_FLAGS "-msse2")
        SET_SOURCE_FILES_PROPERTIES(${MOC_SSE41_FILES} PROPERTIES COMPILE_FLAGS "-msse4.1")
        SET_SOURCE_FILES_PROPERTIES(${MOC_AVX2_FILES} PROPERTIES COMPILE_FLAGS "-mavx2 -mfma -msse4.1")
        message(STATUS "Occlusion culling: native simd supported")
    endif()
endif()

# All

if(UNIX OR MINGW)
    add_definitions(-O3)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
endif()

add_library(graphics_utils STATIC
    mipmap/cpusimd.c
    mipmap/imgresize.c
    ${OCCLUSION_CULLING_SOURCES}
)
