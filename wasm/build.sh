#!/bin/bash

CORE_COUNT="$(nproc --all)"
BASE_DIR="$(realpath "$(dirname "$0")")"
SRC_DIR="$(dirname "$BASE_DIR")"
WEB_DIR="$BASE_DIR/web"
DATA_DIR="$SRC_DIR/game"
BUILD_DIR="$SRC_DIR/cmake_build"
EMSDK_DIR="$BASE_DIR/emsdk"
PACKAGER="$EMSDK_DIR/upstream/emscripten/tools/file_packager.py"
PREFIX="$BASE_DIR/prefix"

mkdir -p $BUILD_DIR
cd $BUILD_DIR

source $EMSDK_DIR/emsdk_env.sh
emcmake cmake .. -DNO_SHADERC=on
make -j4

mkdir -p "$WEB_DIR/game"
cp ./bin/* "$WEB_DIR/game/"
python3 $BASE_DIR/patch_js.py $BASE_DIR/fragments $WEB_DIR/game/supertuxkart.js