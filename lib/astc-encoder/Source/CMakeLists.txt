#  SPDX-License-Identifier: Apache-2.0
#  ----------------------------------------------------------------------------
#  Copyright 2020-2025 Arm Limited
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

# Overwrite the LTO flags to force fat LTO; worth 3-4% performance
# See https://gitlab.kitware.com/cmake/cmake/-/issues/16808
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND ${ASTCENC_CLI})
    set(CMAKE_CXX_COMPILE_OPTIONS_IPO "-flto")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND ${ASTCENC_CLI})
    set(CMAKE_CXX_COMPILE_OPTIONS_IPO "-flto=auto")
endif()

if(${ASTCENC_DECOMPRESSOR})
    set(ASTCENC_CODEC dec)
else()
    set(ASTCENC_CODEC enc)
endif()

set(ASTCENC_ARTIFACTS native none sve_256 sve_128 neon avx2 sse4.1 sse2)
set(ASTCENC_CONFIGS ${ASTCENC_ISA_NATIVE} ${ASTCENC_ISA_NONE} ${ASTCENC_ISA_SVE_256} ${ASTCENC_ISA_SVE_128} ${ASTCENC_ISA_NEON} ${ASTCENC_ISA_AVX2} ${ASTCENC_ISA_SSE41} ${ASTCENC_ISA_SSE2})
list(LENGTH ASTCENC_ARTIFACTS ASTCENC_ARTIFACTS_LEN)
math(EXPR ASTCENC_ARTIFACTS_LEN "${ASTCENC_ARTIFACTS_LEN} - 1")

foreach(INDEX RANGE ${ASTCENC_ARTIFACTS_LEN})
    list(GET ASTCENC_ARTIFACTS ${INDEX} ASTCENC_ARTIFACT)
    list(GET ASTCENC_CONFIGS ${INDEX} ASTCENC_CONFIG)
    if(${ASTCENC_CONFIG})
        set(ASTCENC_ISA_SIMD ${ASTCENC_ARTIFACT})

        if(${ASTCENC_ISA_SIMD} MATCHES "sve_256")
           # Not suported on macOS
        elseif(${ASTCENC_ISA_SIMD} MATCHES "sve_128")
           # Not suported on macOS
        elseif(${ASTCENC_ISA_SIMD} MATCHES "neon")
           set(CMAKE_OSX_ARCHITECTURES arm64)
        elseif(${ASTCENC_ISA_SIMD} MATCHES "sse2")
           set(CMAKE_OSX_ARCHITECTURES x86_64)
        elseif(${ASTCENC_ISA_SIMD} MATCHES "sse4.1")
           set(CMAKE_OSX_ARCHITECTURES x86_64)
        elseif(${ASTCENC_ISA_SIMD} MATCHES "avx2")
           set(CMAKE_OSX_ARCHITECTURES x86_64h)
        elseif(${ASTCENC_ISA_SIMD} MATCHES "none")
           # Using "none" uses implicit architecture
        elseif(${ASTCENC_ISA_SIMD} MATCHES "native")
           # Using "native" uses implicit architecture
        else()
            message(FATAL_ERROR "'${ASTCENC_ISA_SIMD}' is unknown ISA")
        endif()

        include(cmake_core.cmake)
    endif()
endforeach()

if(${ASTCENC_CLI} AND ${ASTCENC_UNIVERSAL_BUILD})
    add_custom_target(
        astc${ASTCENC_CODEC}
            ALL
            COMMAND
                lipo -create -output $<TARGET_FILE_DIR:astc${ASTCENC_CODEC}-sse4.1>/astc${ASTCENC_CODEC} -arch x86_64 $<TARGET_FILE:astc${ASTCENC_CODEC}-sse4.1> -arch x86_64h $<TARGET_FILE:astc${ASTCENC_CODEC}-avx2> -arch arm64 $<TARGET_FILE:astc${ASTCENC_CODEC}-neon>
            VERBATIM)

    add_dependencies(
        astc${ASTCENC_CODEC}
            astc${ASTCENC_CODEC}-sse4.1
            astc${ASTCENC_CODEC}-avx2
            astc${ASTCENC_CODEC}-neon)

    install(PROGRAMS $<TARGET_FILE_DIR:astc${ASTCENC_CODEC}-sse4.1>/astc${ASTCENC_CODEC}
            DESTINATION bin)
endif()

if(${ASTCENC_SHAREDLIB} AND ${ASTCENC_UNIVERSAL_BUILD})
    add_custom_target(
        astc${ASTCENC_CODEC}-shared
            ALL
            COMMAND
                lipo -create -output $<TARGET_FILE_DIR:astc${ASTCENC_CODEC}-sse4.1-shared>/libastc${ASTCENC_CODEC}-shared.dylib -arch x86_64 $<TARGET_FILE:astc${ASTCENC_CODEC}-sse4.1-shared> -arch x86_64h $<TARGET_FILE:astc${ASTCENC_CODEC}-avx2-shared> -arch arm64 $<TARGET_FILE:astc${ASTCENC_CODEC}-neon-shared>
            VERBATIM)

    add_dependencies(
        astc${ASTCENC_CODEC}-shared
            astc${ASTCENC_CODEC}-sse4.1-shared
            astc${ASTCENC_CODEC}-avx2-shared
            astc${ASTCENC_CODEC}-neon-shared)

    install(PROGRAMS $<TARGET_FILE_DIR:astc${ASTCENC_CODEC}-sse4.1-shared>/libastc${ASTCENC_CODEC}-shared.dylib
            DESTINATION lib)
endif()

# - - - - - - - - - - - - - - - - - -
# Unit testing
if(${ASTCENC_UNITTEST})
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(CMAKE_OSX_ARCHITECTURES x86_64;arm64)
    add_subdirectory(GoogleTest)

    # Workaround GoogleTest CRT selection issue issue
    # See https://github.com/google/googletest/issues/4067
    set_property(
        TARGET gtest
            PROPERTY
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    set_property(
        TARGET gtest_main
            PROPERTY
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    enable_testing()
    add_subdirectory(UnitTest)
endif()
