#!/bin/bash

set -x
set -e

CORE_COUNT="$(nproc --all)"
BASE_DIR="$(realpath "$(dirname "$0")")"
BUILD_DIR="$BASE_DIR/build"
EMSDK_DIR="$BASE_DIR/emsdk"
PREFIX="$BASE_DIR/prefix"
INCLUDE="$PREFIX/include"
LIB="$PREFIX/lib"

source $EMSDK_DIR/emsdk_env.sh
export PKG_CONFIG_PATH="$LIB/pkgconfig"
export CFLAGS="-fwasm-exceptions -sSUPPORT_LONGJMP=wasm -pthread"
export CPPFLAGS="-fwasm-exceptions -sSUPPORT_LONGJMP=wasm -pthread"
export LDFLAGS="-fwasm-exceptions -sSUPPORT_LONGJMP=wasm -pthread"

clone_repo() {
  local url="$1"
  local tag="$2"
  local path="$3"
  if [ ! -d "$path" ]; then
    git clone "$url" -b "$tag" --depth=1 "$path"
  fi
}

build_ogg() {
  local SRC_DIR="$BUILD_DIR/ogg"
  clone_repo "https://github.com/xiph/ogg" v1.3.5 "$SRC_DIR"
  cd "$SRC_DIR"

  ./autogen.sh
  emconfigure ./configure --host=none-linux --prefix="$PREFIX" --disable-shared
  emmake make -j$CORE_COUNT
  make install
}

build_vorbis() {
  local SRC_DIR="$BUILD_DIR/vorbis"
  clone_repo "https://github.com/xiph/vorbis" v1.3.7 "$SRC_DIR"
  cd "$SRC_DIR"

  ./autogen.sh
  emconfigure ./configure --host=none-linux --prefix="$PREFIX" --with-ogg="$PREFIX" --disable-shared 
  emmake make -j$CORE_COUNT
  make install
}

build_openssl() {
  local SRC_DIR="$BUILD_DIR/openssl"
  clone_repo "https://github.com/openssl/openssl" openssl-3.3.0 "$SRC_DIR"
  cd "$SRC_DIR"

  emconfigure ./Configure linux-x32 -no-asm -static -no-afalgeng -no-dso -DOPENSSL_SYS_NETWARE -DSIG_DFL=0 -DSIG_IGN=0 -DHAVE_FORK=0 -DOPENSSL_NO_AFALGENG=1 -DOPENSSL_NO_SPEED=1 -DOPENSSL_NO_DYNAMIC_ENGINE -DDLOPEN_FLAG=0
  sed -i 's|^CROSS_COMPILE.*$|CROSS_COMPILE=|g' Makefile
  emmake make -j$CORE_COUNT build_generated libssl.a libcrypto.a
  cp -r include/openssl $PREFIX/include
  cp libcrypto.a libssl.a $PREFIX/lib
}

build_zlib() {
  local SRC_DIR="$BUILD_DIR/zlib"
  clone_repo "https://github.com/madler/zlib" v1.3.1 "$SRC_DIR"
  cd "$SRC_DIR"

  emconfigure ./configure --prefix="$PREFIX" --static
  emmake make -j$CORE_COUNT
  make install
}

build_curl() {
  local SRC_DIR="$BUILD_DIR/curl"
  clone_repo "https://github.com/curl/curl" curl-8_8_0 "$SRC_DIR"
  cd "$SRC_DIR"

  autoreconf -fi
  emconfigure ./configure --host none-linux --prefix="$PREFIX" \
    --with-ssl="$PREFIX" --with-zlib="$PREFIX" \
    --disable-shared --disable-threaded-resolver \
    --without-libpsl --disable-netrc --disable-ipv6 \
    --disable-tftp --disable-ntlm-wb
  emmake make -j$CORE_COUNT
  make install
}

build_jpeg() {
  local SRC_DIR="$BUILD_DIR/jpeg"
  clone_repo "https://github.com/libjpeg-turbo/libjpeg-turbo" 3.0.3 "$SRC_DIR"
  cd "$SRC_DIR"
  mkdir -p build
  cd build

  emcmake cmake -G"Unix Makefiles" -DCMAKE_INSTALL_PREFIX:PATH="$PREFIX" -DWITH_SIMD=0 -DENABLE_SHARED=0 ..
  emmake make -j$CORE_COUNT
  make install
}

build_png() {
  local SRC_DIR="$BUILD_DIR/png"
  clone_repo "https://github.com/pnggroup/libpng" v1.6.43 "$SRC_DIR"
  cd "$SRC_DIR"

  emconfigure ./configure --host none-linux --prefix="$PREFIX" --disable-shared CPPFLAGS="$CPPFLAGS -I$INCLUDE" LDFLAGS="$LDFLAGS -L$LIB"
  emmake make -j$CORE_COUNT 
  make install
}

build_freetype() {
  local with_harfbuzz="$1"
  local SRC_DIR="$BUILD_DIR/freetype"
  clone_repo "https://github.com/freetype/freetype" VER-2-13-2 "$SRC_DIR"
  cd "$SRC_DIR"
  mkdir -p build
  cd build

  if [ ! "$with_harfbuzz" ]; then
    emcmake cmake .. -DCMAKE_INSTALL_PREFIX:PATH="$PREFIX" \
      -DZLIB_LIBRARY="$LIB/libz.a" -DZLIB_INCLUDE_DIR="$INCLUDE" \
      -DPNG_LIBRARY="$LIB/libpng.a" -DPNG_PNG_INCLUDE_DIR="$INCLUDE" 
  else
    emcmake cmake .. -DCMAKE_INSTALL_PREFIX:PATH="$PREFIX" \
      -DZLIB_LIBRARY="$LIB/libz.a" -DZLIB_INCLUDE_DIR="$INCLUDE" \
      -DPNG_LIBRARY="$LIB/libpng.a" -DPNG_PNG_INCLUDE_DIR="$INCLUDE" \
      -DHarfBuzz_LIBRARY="$LIB/libharfbuzz.a" -DHarfBuzz_INCLUDE_DIR="$INCLUDE/harfbuzz/" \
      -DFT_REQUIRE_HARFBUZZ=TRUE   
  fi
  emmake make -j$CORE_COUNT
  make install
}

build_harfbuzz() {
  local SRC_DIR="$BUILD_DIR/harfbuzz"
  clone_repo "https://github.com/harfbuzz/harfbuzz" 8.5.0 "$SRC_DIR"
  cd "$SRC_DIR"

  emconfigure ./autogen.sh
  emconfigure ./configure --host=none-linux --prefix="$PREFIX" --disable-shared PKG_CONFIG_PATH="$LIB/pkgconfig"
  emmake make -j$CORE_COUNT
  make install
}

if [ ! -d "$INCLUDE/ogg" ]; then
  build_ogg
fi

if [ ! -d "$INCLUDE/vorbis" ]; then
  build_vorbis
fi

if [ ! -d "$INCLUDE/openssl" ]; then
  build_openssl
fi

if [ ! -f "$INCLUDE/zlib.h" ]; then
  build_zlib
fi

if [ ! -d "$INCLUDE/curl" ]; then
  build_curl
fi

if [ ! -f "$INCLUDE/turbojpeg.h" ]; then
  build_jpeg
fi

if [ ! -f "$INCLUDE/png.h" ]; then
  build_png
fi

if [ ! -d "$INCLUDE/freetype2" ]; then
  build_freetype
fi

if [ ! -d "$INCLUDE/harfbuzz" ]; then
  build_harfbuzz

  #rebuild freetype with harfbuzz support
  rm -rf "$INCLUDE/freetype2"
  build_freetype true
fi
