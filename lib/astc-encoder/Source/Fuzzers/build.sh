#!/bin/bash -eu

#  SPDX-License-Identifier: Apache-2.0
#  ----------------------------------------------------------------------------
#  Copyright 2020-2026 Arm Limited
#  Copyright 2020 Google Inc.
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

# This script is invoked by oss-fuzz from <root>/Source/

# Generate a dummy version header (normally built by CMake)
echo "#pragma once" > astcenccli_version.h
echo "#define VERSION_STRING \"0.0.0\"" >> astcenccli_version.h
echo "#define YEAR_STRING \"2021\"" >> astcenccli_version.h

# Build codec library
for source in ./*.cpp; do
  BASE="${source##*/}"
  BASE="${BASE%.cpp}"
  echo ${BASE}

  $CXX $CXXFLAGS \
      -c \
      -DASTCENC_SSE=0 \
      -DASTCENC_AVX=0 \
      -DASTCENC_POPCNT=0 \
      -I. -std=c++17 -mfpmath=sse -msse2 -fno-strict-aliasing -g \
      $source \
      -o ${BASE}.o
done

ar -qc libastcenc.a *.o

# Build fuzzers
for fuzzer in ./Fuzzers/fuzz_*.cpp; do
  $CXX $CXXFLAGS \
      -DASTCENC_SSE=0 \
      -DASTCENC_AVX=0 \
      -DASTCENC_POPCNT=0 \
      -I. -std=c++17 $fuzzer $LIB_FUZZING_ENGINE ./libastcenc.a \
      -o $OUT/$(basename -s .cpp $fuzzer)
done

# Cleanup temporary build files
rm *.o
rm *.a
