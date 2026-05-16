#  SPDX-License-Identifier: Apache-2.0
#  ----------------------------------------------------------------------------
#  Copyright 2020-2026 Arm Limited
#
#  Licensed under the Apache License, Version 2.0 (the "License"); you may not
#  use this file except in compliance with the License. You may obtain a copy
#  of the License at:
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#  License for the specific language governing permissions and limitations
#  under the License.
#  ----------------------------------------------------------------------------

# CMake configuration
cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0069 NEW)  # LTO support
cmake_policy(SET CMP0091 NEW)  # MSVC runtime support

if(MSVC)
    # Disable structure was padded due to alignment specifier
    add_compile_options("/wd4324")
endif()

project(astcencoder VERSION 5.4.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_XCODE_ATTRIBUTE_VALID_ARCHS "x86_64 x86_64h arm64")

include(CTest)

option(ASTCENC_ISA_AVX2 "Enable astcenc builds for AVX2 SIMD")
option(ASTCENC_ISA_SSE41 "Enable astcenc builds for SSE4.1 SIMD")
option(ASTCENC_ISA_SSE2 "Enable astcenc builds for SSE2 SIMD")
option(ASTCENC_ISA_SVE_256 "Enable astcenc builds for 256-bit SVE SIMD")
option(ASTCENC_ISA_SVE_128 "Enable astcenc builds for 128-bit SVE SIMD")
option(ASTCENC_ISA_NEON "Enable astcenc builds for NEON SIMD")
option(ASTCENC_ISA_NONE "Enable astcenc builds for no SIMD")
option(ASTCENC_ISA_NATIVE "Enable astcenc builds for native SIMD")
option(ASTCENC_DECOMPRESSOR "Enable astcenc builds for decompression only")
option(ASTCENC_SHAREDLIB "Enable astcenc builds with core library shared objects")
option(ASTCENC_BIG_ENDIAN "Enable astcenc big-endian support")
option(ASTCENC_DIAGNOSTICS "Enable astcenc builds with diagnostic trace")
option(ASTCENC_ASAN "Enable astcenc builds with address sanitizer")
option(ASTCENC_UBSAN "Enable astcenc builds with undefined behavior sanitizer")
option(ASTCENC_UNITTEST "Enable astcenc builds with unit tests")
option(ASTCENC_INVARIANCE "Enable astcenc floating point invariance" ON)
option(ASTCENC_CLI "Enable build of astcenc command line tools" ON)
option(ASTCENC_X86_GATHERS "Enable use of native x86 gathers" ON)
option(ASTCENC_WERROR "Force builds to treat warnings as errors" ON)

# Preflight for some macOS-specific build options
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    option(ASTCENC_UNIVERSAL_BUILD "Enable universal multi-arch build" ON)

    if(${ASTCENC_UNIVERSAL_BUILD})
        set(ASTCENC_ISA_SSE41 ON)
        set(ASTCENC_ISA_AVX2 ON)
        set(ASTCENC_ISA_NEON ON)

        if(${ASTCENC_ISA_SSE2})
            message(FATAL_ERROR "ISA_SSE2 cannot be used in a universal build")
        endif()

        if(${ASTCENC_ISA_NONE})
            message(FATAL_ERROR "ISA_NONE cannot be used in a universal build")
        endif()

        if(${ASTCENC_ISA_NATIVE})
            message(FATAL_ERROR "ISA_NATIVE cannot be used in a universal build")
        endif()
    endif()
else()
    set(ASTCENC_UNIVERSAL_BUILD OFF)
endif()

# Count options which MUST be x64
set(ASTCENC_X64_ISA_COUNT 0)
set(ASTCENC_CONFIGS ${ASTCENC_ISA_AVX2} ${ASTCENC_ISA_SSE41} ${ASTCENC_ISA_SSE2})
foreach(ASTCENC_CONFIG ${ASTCENC_CONFIGS})
    if(${ASTCENC_CONFIG})
        math(EXPR ASTCENC_X64_ISA_COUNT "${ASTCENC_X64_ISA_COUNT} + 1")
    endif()
endforeach()

# Count options which MUST be arm64
set(ASTCENC_ARM64_ISA_COUNT 0)
set(ASTCENC_CONFIGS ${ASTCENC_ISA_NEON} ${ASTCENC_ISA_SVE_128} ${ASTCENC_ISA_SVE_256})
foreach(ASTCENC_CONFIG ${ASTCENC_CONFIGS})
    if(${ASTCENC_CONFIG})
        math(EXPR ASTCENC_ARM64_ISA_COUNT "${ASTCENC_ARM64_ISA_COUNT} + 1")
    endif()
endforeach()

# Non-macOS builds
if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    if(("${ASTCENC_ARM64_ISA_COUNT}" GREATER 0) AND ("${ASTCENC_X64_ISA_COUNT}" GREATER 0))
        message(FATAL_ERROR "Builds can only support a single architecture per configure.")
    endif()
endif()

# If nothing more specific is set then fall back on the compiler's defaults
if(("${ASTCENC_ARM64_ISA_COUNT}" EQUAL 0) AND ("${ASTCENC_X64_ISA_COUNT}" EQUAL 0) AND (NOT "${ASTCENC_ISA_NONE}"))
    set(ASTCENC_ISA_NATIVE ON)
endif()

function(printopt optName optVal)
    if(${optVal})
        message(STATUS "  ${optName}  - ON")
    else()
        message(STATUS "  ${optName}  - OFF")
    endif()
endfunction()

if("${ASTCENC_BLOCK_MAX_TEXELS}")
     message(STATUS "  Max block texels - ${ASTCENC_BLOCK_MAX_TEXELS}")
endif()

message(STATUS "Arm backend options")
printopt("SVE 256b backend   " ${ASTCENC_ISA_SVE_256})
printopt("SVE 128b backend   " ${ASTCENC_ISA_SVE_128})
printopt("NEON backend       " ${ASTCENC_ISA_NEON})
message(STATUS "x86-64 backend options")
printopt("AVX2 backend       " ${ASTCENC_ISA_AVX2})
printopt("SSE4.1 backend     " ${ASTCENC_ISA_SSE41})
printopt("SSE2 backend       " ${ASTCENC_ISA_SSE2})
printopt("Use native gathers " ${ASTCENC_X86_GATHERS})
message(STATUS "Agnostic backend options")
printopt("NONE backend       " ${ASTCENC_ISA_NONE})
printopt("NATIVE backend     " ${ASTCENC_ISA_NATIVE})
message(STATUS "Build options")
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    printopt("Universal bin      " ${ASTCENC_UNIVERSAL_BUILD})
endif()
printopt("Invariance         " ${ASTCENC_INVARIANCE})
if(${ASTCENC_BIG_ENDIAN})
    printopt("Big endian         " ${ASTCENC_BIG_ENDIAN})
endif()
printopt("Shared libs        " ${ASTCENC_SHAREDLIB})
printopt("Decompressor       " ${ASTCENC_DECOMPRESSOR})
message(STATUS "Developer options")
printopt("Diagnostics        " ${ASTCENC_DIAGNOSTICS})
printopt("ASAN               " ${ASTCENC_ASAN})
printopt("UBSAN              " ${ASTCENC_UBSAN})
printopt("Unit tests         " ${ASTCENC_UNITTEST})

# Subcomponents
add_subdirectory(Source)

# Configure package archive
if(ASTCENC_PACKAGE)
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
        string(TOLOWER "macOS" ASTCENC_PKG_OS)
    else()
        string(TOLOWER ${CMAKE_SYSTEM_NAME} ASTCENC_PKG_OS)
    endif()

    set(CPACK_PACKAGE_FILE_NAME "astcenc-${CMAKE_PROJECT_VERSION}-${ASTCENC_PKG_OS}-${ASTCENC_PACKAGE}")
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
    set(CPACK_PACKAGE_CHECKSUM SHA256)
    set(CPACK_GENERATOR ZIP)

    include(CPack) # Must be included after CPack configuration.
endif()
