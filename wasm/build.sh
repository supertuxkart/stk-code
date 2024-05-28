#!/bin/bash

CORE_COUNT="$(nproc --all)"
BASE_DIR="$(realpath "$(dirname "$0")")"
SRC_DIR="$(dirname "$BASE_DIR")"
BUILD_DIR="$SRC_DIR/cmake_build"
PREFIX="$BASE_DIR/prefix"

mkdir -p $BUILD_DIR
cd $BUILD_DIR

emcmake cmake .. -DNO_SHADERC=on
make -j$CORE_COUNT