#!/bin/sh
# Tested with NDK 22.1.7171670

export DIRNAME=$(realpath "$(dirname "$0")")

export NDK_PATH_DEFAULT="$DIRNAME/android-ndk"

export ARCH_ARMV7=arm
export HOST_ARMV7=arm-linux-androideabi

export ARCH_AARCH64=arm64
export HOST_AARCH64=aarch64-linux-android

export ARCH_X86=x86
export HOST_X86=i686-linux-android

export ARCH_X86_64=x86_64
export HOST_X86_64=x86_64-linux-android

# A helper function that checks if error ocurred
check_error()
{
    if [ $? -gt 0 ]; then
        echo "Error ocurred."
        exit
    fi
}

# Handle clean command
if [ ! -z "$1" ] && [ "$1" = "clean" ]; then
    rm -rf "$DIRNAME/deps-armeabi-v7a"
    rm -rf "$DIRNAME/deps-arm64-v8a"
    rm -rf "$DIRNAME/deps-x86"
    rm -rf "$DIRNAME/deps-x86_64"
    exit
fi

if [ -z "$NDK_PATH" ]; then
    export NDK_PATH="$NDK_PATH_DEFAULT"
fi

if [ -z "$STK_NDK_VERSION" ]; then
    export STK_NDK_VERSION=23.1.7779620
fi

NDK_PATH="$(realpath "$NDK_PATH")/${STK_NDK_VERSION}"
if [ ! -d "$NDK_PATH" ]; then
    echo "Error: Couldn't find $NDK_PATH directory. Please create a symlink" \
         "to your Android NDK installation in the $NDK_PATH_DEFAULT or set"  \
         "proper path in the NDK_PATH variable"
    exit
fi

export NDK_TOOLCHAIN_PATH="$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64/bin"
export NDK_SYSROOT="$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64/sysroot"
export PATH="$NDK_TOOLCHAIN_PATH:$PATH"

build_deps()
{
    export ARCH_OPTION=$1
    if [ "$ARCH_OPTION" = "armv7" ]; then
        ARCH_OPTION="armeabi-v7a"
        export ARCH=$ARCH_ARMV7
        # Special case
        export HOST=armv7a-linux-androideabi16
        export HOST_DIR=$HOST_ARMV7
    elif [ "$ARCH_OPTION" = "aarch64" ]; then
        ARCH_OPTION="arm64-v8a"
        export ARCH=$ARCH_AARCH64
        export HOST_DIR=$HOST_AARCH64
        export HOST="${HOST_DIR}21"
    elif [ "$ARCH_OPTION" = "x86" ]; then
        export ARCH=$ARCH_X86
        export HOST_DIR=$HOST_X86
        export HOST="${HOST_DIR}16"
    elif [ "$ARCH_OPTION" = "x86_64" ]; then
        export ARCH=$ARCH_X86_64
        export HOST_DIR=$HOST_X86_64
        export HOST="${HOST_DIR}21"
    else
        echo "Unknown architecture: $1. Possible values are:"\
             "all, armv7, aarch64, x86, x86_64"
        exit
    fi

    # Zlib
    if [ ! -f "$DIRNAME/deps-$ARCH_OPTION/zlib.stamp" ]; then
        echo "Compiling $ARCH_OPTION zlib"
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/zlib"
        cp -a -f "$DIRNAME/../lib/zlib/"* "$DIRNAME/deps-$ARCH_OPTION/zlib"

        cd "$DIRNAME/deps-$ARCH_OPTION/zlib"
        cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
                -DHOST=$HOST -DARCH=$ARCH -DCMAKE_C_FLAGS="-fpic -O3 -g" &&
        make -j $(($(nproc) + 1))
        check_error
        touch "$DIRNAME/deps-$ARCH_OPTION/zlib.stamp"
    fi

    # Libpng
    if [ ! -f "$DIRNAME/deps-$ARCH_OPTION/libpng.stamp" ]; then
        echo "Compiling $ARCH_OPTION libpng"
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/libpng"
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/libpng/lib"
        cp -a -f "$DIRNAME/../lib/libpng/"* "$DIRNAME/deps-$ARCH_OPTION/libpng"

        cd "$DIRNAME/deps-$ARCH_OPTION/libpng"
        MLIBRARY="$NDK_SYSROOT/usr/lib/$HOST_DIR/libm.a"
        cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
                -DHOST=$HOST -DARCH=$ARCH                                     \
                -DZLIB_LIBRARY="$DIRNAME/deps-$ARCH_OPTION/zlib/libz.a"       \
                -DZLIB_INCLUDE_DIR="$DIRNAME/deps-$ARCH_OPTION/zlib/"         \
                -DM_LIBRARY="$MLIBRARY"                                       \
                -DPNG_TESTS=0 -DCMAKE_C_FLAGS="-fpic -O3 -g" &&
        make -j $(($(nproc) + 1))
        check_error
        touch "$DIRNAME/deps-$ARCH_OPTION/libpng.stamp"
    fi

    # Freetype bootstrap
    if [ ! -f "$DIRNAME/deps-$ARCH_OPTION/freetype_bootstrap.stamp" ]; then
        echo "Compiling $ARCH_OPTION freetype bootstrap"
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/freetype/build"
        cp -a -f "$DIRNAME/../lib/freetype/"* "$DIRNAME/deps-$ARCH_OPTION/freetype"
        cd "$DIRNAME/deps-$ARCH_OPTION/freetype/build"
        cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
                 -DHOST=$HOST -DARCH=$ARCH                                     \
                 -DZLIB_LIBRARY="$DIRNAME/deps-$ARCH_OPTION/zlib/libz.a"       \
                 -DZLIB_INCLUDE_DIR="$DIRNAME/deps-$ARCH_OPTION/zlib/"         \
                 -DPNG_LIBRARY="$DIRNAME/deps-$ARCH_OPTION/libpng/libpng.a"    \
                 -DPNG_PNG_INCLUDE_DIR="$DIRNAME/deps-$ARCH_OPTION/libpng/"    \
                 -DFT_WITH_HARFBUZZ=OFF -DFT_WITH_BZIP2=OFF                    \
                 -DFT_WITH_BROTLI=OFF -DFT_WITH_ZLIB=ON -DFT_WITH_PNG=ON       \
                 -DCMAKE_C_FLAGS="-fpic -O3 -g" &&
        make -j $(($(nproc) + 1))
        check_error
        # We need to rebuild freetype after harfbuzz is compiled
        touch "$DIRNAME/deps-$ARCH_OPTION/freetype_bootstrap.stamp"
    fi

    # Harfbuzz
    if [ ! -f "$DIRNAME/deps-$ARCH_OPTION/harfbuzz.stamp" ]; then
        echo "Compiling $ARCH_OPTION harfbuzz"
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/harfbuzz/build"
        cp -a -f "$DIRNAME/../lib/harfbuzz/"* "$DIRNAME/deps-$ARCH_OPTION/harfbuzz"

        cd "$DIRNAME/deps-$ARCH_OPTION/harfbuzz/build"
        cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake    \
                 -DHOST=$HOST -DARCH=$ARCH -DBUILD_SHARED_LIBS=OFF                \
                 -DFREETYPE_LIBRARY="$DIRNAME/deps-$ARCH_OPTION/freetype/build/libfreetype.a $DIRNAME/deps-$ARCH_OPTION/libpng/libpng.a $DIRNAME/deps-$ARCH_OPTION/zlib/libz.a" \
                 -DFREETYPE_INCLUDE_DIRS="$DIRNAME/deps-$ARCH_OPTION/freetype/include/" \
                 -DHB_HAVE_GLIB=OFF -DHB_HAVE_GOBJECT=OFF -DHB_HAVE_ICU=OFF       \
                 -DHB_HAVE_FREETYPE=ON -DHB_BUILD_SUBSET=OFF                      \
                 -DCMAKE_C_FLAGS="-fpic -O3 -g" -DCMAKE_CXX_FLAGS="-std=gnu++0x -fpic -O3 -g" &&
        make -j $(($(nproc) + 1))
        check_error
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/harfbuzz/include/harfbuzz"
        cp $DIRNAME/deps-$ARCH_OPTION/harfbuzz/src/*.h "$DIRNAME/deps-$ARCH_OPTION/harfbuzz/include/harfbuzz"
        touch "$DIRNAME/deps-$ARCH_OPTION/harfbuzz.stamp"
    fi

    # Freetype
    if [ ! -f "$DIRNAME/deps-$ARCH_OPTION/freetype.stamp" ]; then
        echo "Compiling $ARCH_OPTION freetype"
        cd "$DIRNAME/deps-$ARCH_OPTION/freetype/build"
        rm -rf *
        cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
                 -DHOST=$HOST -DARCH=$ARCH                                     \
                 -DZLIB_LIBRARY="$DIRNAME/deps-$ARCH_OPTION/zlib/libz.a"                      \
                 -DZLIB_INCLUDE_DIR="$DIRNAME/deps-$ARCH_OPTION/zlib/"                        \
                 -DPNG_LIBRARY="$DIRNAME/deps-$ARCH_OPTION/libpng/libpng.a"                   \
                 -DPNG_PNG_INCLUDE_DIR="$DIRNAME/deps-$ARCH_OPTION/libpng/"                   \
                 -DHarfBuzz_LIBRARY="$DIRNAME/deps-$ARCH_OPTION/harfbuzz/build/libharfbuzz.a" \
                 -DHarfBuzz_INCLUDE_DIR="$DIRNAME/deps-$ARCH_OPTION/harfbuzz/src/"            \
                 -DFT_WITH_HARFBUZZ=ON -DFT_WITH_BZIP2=OFF                     \
                 -DFT_WITH_BROTLI=OFF -DFT_WITH_ZLIB=ON -DFT_WITH_PNG=ON       \
                 -DCMAKE_C_FLAGS="-fpic -O3 -g" &&
        make -j $(($(nproc) + 1))
        check_error
        touch "$DIRNAME/deps-$ARCH_OPTION/freetype.stamp"
    fi

    # Openal
    if [ ! -f "$DIRNAME/deps-$ARCH_OPTION/openal.stamp" ]; then
        echo "Compiling $ARCH_OPTION openal"
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/openal"
        cp -a -f "$DIRNAME/../lib/openal/"* "$DIRNAME/deps-$ARCH_OPTION/openal"

        cd "$DIRNAME/deps-$ARCH_OPTION/openal"
        cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake   \
                -DHOST=$HOST -DARCH=$ARCH  -DALSOFT_UTILS=0 -DALSOFT_EXAMPLES=0 \
                -DLIBTYPE=STATIC -DOPENSL_LIBRARY="-lOpenSLES"                  \
                -DOPENSL_INCLUDE_DIR="$NDK_SYSROOT/usr/include/SLES/"           \
                -DOPENSL_ANDROID_INCLUDE_DIR="$NDK_SYSROOT/usr/include/SLES/"   \
                -DCMAKE_C_FLAGS="-fpic -O3 -g" -DCMAKE_CXX_FLAGS="-fpic -O3 -g" &&
        make -j $(($(nproc) + 1))
        check_error
        touch "$DIRNAME/deps-$ARCH_OPTION/openal.stamp"
    fi

    # MbedTLS
    if [ ! -f "$DIRNAME/deps-$ARCH_OPTION/mbedtls.stamp" ]; then
        echo "Compiling $ARCH_OPTION mbedtls"
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/mbedtls"
        cp -a -f "$DIRNAME/../lib/mbedtls/"* "$DIRNAME/deps-$ARCH_OPTION/mbedtls"

        cd "$DIRNAME/deps-$ARCH_OPTION/mbedtls"
        cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
                -DHOST=$HOST -DARCH=$ARCH -DBUILD_SHARED_LIBS=OFF             \
                -DENABLE_TESTING=OFF -DENABLE_PROGRAMS=OFF                    \
                -DCMAKE_C_FLAGS="-fpic -O3 -g" &&
        make -j $(($(nproc) + 1))
        check_error
        touch "$DIRNAME/deps-$ARCH_OPTION/mbedtls.stamp"
    fi

    # Curl
    if [ ! -f "$DIRNAME/deps-$ARCH_OPTION/curl.stamp" ]; then
        echo "Compiling $ARCH_OPTION curl"
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/curl"
        cp -a -f "$DIRNAME/../lib/curl/"* "$DIRNAME/deps-$ARCH_OPTION/curl"

        cd "$DIRNAME/deps-$ARCH_OPTION/curl"
        cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
                -DHOST=$HOST -DARCH=$ARCH -DBUILD_SHARED_LIBS=OFF             \
                -DZLIB_LIBRARY="$DIRNAME/deps-$ARCH_OPTION/zlib/libz.a"       \
                -DZLIB_INCLUDE_DIR="$DIRNAME/deps-$ARCH_OPTION/zlib/"         \
                -DMBEDCRYPTO_LIBRARY="$DIRNAME/deps-$ARCH_OPTION/mbedtls/library/libmbedcrypto.a" \
                -DMBEDTLS_LIBRARY="$DIRNAME/deps-$ARCH_OPTION/mbedtls/library/libmbedtls.a"       \
                -DMBEDX509_LIBRARY="$DIRNAME/deps-$ARCH_OPTION/mbedtls/library/libmbedx509.a"     \
                -DMBEDTLS_INCLUDE_DIRS="$DIRNAME/deps-$ARCH_OPTION/mbedtls/include/"              \
                -DBUILD_TESTING=OFF -DBUILD_CURL_EXE=OFF                      \
                -DCURL_USE_MBEDTLS=ON -DUSE_ZLIB=ON -DCURL_USE_OPENSSL=OFF    \
                -DCURL_USE_LIBSSH=OFF -DCURL_USE_LIBSSH2=OFF                  \
                -DCURL_USE_GSSAPI=OFF -DUSE_NGHTTP2=OFF -DUSE_QUICHE=OFF      \
                -DHTTP_ONLY=ON -DCURL_CA_BUNDLE=none -DCURL_CA_PATH=none      \
                -DENABLE_THREADED_RESOLVER=ON -DCMAKE_C_FLAGS="-fpic -O3 -g" &&
        make -j $(($(nproc) + 1))
        check_error
        touch "$DIRNAME/deps-$ARCH_OPTION/curl.stamp"
    fi

    # Libjpeg
    if [ ! -f "$DIRNAME/deps-$ARCH_OPTION/libjpeg.stamp" ]; then
        echo "Compiling $ARCH_OPTION libjpeg"
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/libjpeg"
        cp -a -f "$DIRNAME/../lib/libjpeg/"* "$DIRNAME/deps-$ARCH_OPTION/libjpeg"

        cd "$DIRNAME/deps-$ARCH_OPTION/libjpeg"
        cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
                -DHOST=$HOST -DARCH=$ARCH -DCMAKE_C_FLAGS="-fpic -O3 -g" &&
        make -j $(($(nproc) + 1))
        check_error
        touch "$DIRNAME/deps-$ARCH_OPTION/libjpeg.stamp"
    fi

    # Libogg
    if [ ! -f "$DIRNAME/deps-$ARCH_OPTION/libogg.stamp" ]; then
        echo "Compiling $ARCH_OPTION libogg"
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/libogg"
        cp -a -f "$DIRNAME/../lib/libogg/"* "$DIRNAME/deps-$ARCH_OPTION/libogg"

        cd "$DIRNAME/deps-$ARCH_OPTION/libogg"
        cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
                -DHOST=$HOST -DARCH=$ARCH -DCMAKE_C_FLAGS="-fpic -O3 -g" &&
        make -j $(($(nproc) + 1))
        check_error
        touch "$DIRNAME/deps-$ARCH_OPTION/libogg.stamp"
    fi

    # Libvorbis
    if [ ! -f "$DIRNAME/deps-$ARCH_OPTION/libvorbis.stamp" ]; then
        echo "Compiling $ARCH_OPTION libvorbis"
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/libvorbis"
        cp -a -f "$DIRNAME/../lib/libvorbis/"* "$DIRNAME/deps-$ARCH_OPTION/libvorbis"

        cd "$DIRNAME/deps-$ARCH_OPTION/libvorbis"
        cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
                -DHOST=$HOST -DARCH=$ARCH -DCMAKE_C_FLAGS="-fpic -O3 -g"      \
                -DOGG_LIBRARY="$DIRNAME/deps-$ARCH_OPTION/libogg/libogg.a"    \
                -DOGG_INCLUDE_DIR="$DIRNAME/deps-$ARCH_OPTION/libogg/include" &&
        make -j $(($(nproc) + 1))
        check_error
        touch "$DIRNAME/deps-$ARCH_OPTION/libvorbis.stamp"
    fi

    # Shaderc
    if [ ! -f "$DIRNAME/deps-$ARCH_OPTION/shaderc.stamp" ]; then
        echo "Compiling $ARCH_OPTION shaderc"
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/shaderc"
        cp -a -f "$DIRNAME/../lib/shaderc/"* "$DIRNAME/deps-$ARCH_OPTION/shaderc"

        cd "$DIRNAME/deps-$ARCH_OPTION/shaderc"
        cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake  \
                -DHOST=$HOST -DARCH=$ARCH -DCMAKE_C_FLAGS="-fpic -O3"          \
                -DCMAKE_CXX_FLAGS="-fpic -O3" -DSHADERC_SKIP_INSTALL=1         \
                -DCMAKE_BUILD_TYPE=Release                                     \
                -DSHADERC_SKIP_TESTS=1 -DSHADERC_SKIP_EXAMPLES=1               \
                -DSPIRV_HEADERS_SKIP_INSTALL=1 -DSPIRV_HEADERS_SKIP_EXAMPLES=1 \
                -DSKIP_SPIRV_TOOLS_INSTALL=1 -DSPIRV_SKIP_TESTS=1              \
                -DSPIRV_SKIP_EXECUTABLES=1 -DENABLE_GLSLANG_BINARIES=0         \
                -DENABLE_CTEST=0 &&
        make -j $(($(nproc) + 1))
        # Strip debug symbol to make app bundle smaller
        llvm-strip --strip-debug "$DIRNAME/deps-$ARCH_OPTION/shaderc/libshaderc/libshaderc_combined.a"
        check_error
        touch "$DIRNAME/deps-$ARCH_OPTION/shaderc.stamp"
    fi

    # Libsquish
    if [ ! -f "$DIRNAME/deps-$ARCH_OPTION/libsquish.stamp" ]; then
        echo "Compiling $ARCH_OPTION libsquish"
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/libsquish"
        cp -a -f "$DIRNAME/../lib/libsquish/"* "$DIRNAME/deps-$ARCH_OPTION/libsquish"
    
        cd "$DIRNAME/deps-$ARCH_OPTION/libsquish"
        if [[ "$ARCH_OPTION" = "x86" || "$ARCH_OPTION" = "x86_64" ]]; then
            cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
                    -DHOST=$HOST -DARCH=$ARCH                                     \
                    -DCMAKE_C_FLAGS="-fpic -O3 -g -DSQUISH_USE_SSE=2 -msse2"      \
                    -DCMAKE_CXX_FLAGS="-fpic -O3 -g -DSQUISH_USE_SSE=2 -msse2"
        else
            cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
                    -DHOST=$HOST -DARCH=$ARCH -DCMAKE_C_FLAGS="-fpic -O3 -g"      \
                    -DCMAKE_CXX_FLAGS="-fpic -O3 -g"
        fi
        make -j $(($(nproc) + 1))
        check_error
        touch "$DIRNAME/deps-$ARCH_OPTION/libsquish.stamp"
    fi

    # ASTC-encoder
    if [ ! -f "$DIRNAME/deps-$ARCH_OPTION/astc-encoder.stamp" ]; then
        echo "Compiling $ARCH_OPTION astc-encoder"
        mkdir -p "$DIRNAME/deps-$ARCH_OPTION/astc-encoder"
        cp -a -f "$DIRNAME/../lib/astc-encoder/"* "$DIRNAME/deps-$ARCH_OPTION/astc-encoder"

        cd "$DIRNAME/deps-$ARCH_OPTION/astc-encoder"
        sed -i '/-Werror/d' Source/cmake_core.cmake
        sed -i 's|${ASTCENC_TARGET}-static|astcenc|g' Source/cmake_core.cmake
        if [ "$ARCH_OPTION" = "armeabi-v7a" ]; then
            cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
                    -DHOST=$HOST -DARCH=$ARCH -DSTK_ARM_NEON=ON                   \
                    -DCMAKE_C_FLAGS="-fpic -O3 -g -mfpu=neon"                     \
                    -DCMAKE_CXX_FLAGS="-fpic -O3 -g -mfpu=neon"                   \
                    -DASTCENC_INVARIANCE=OFF -DASTCENC_CLI=OFF
        elif [ "$ARCH_OPTION" = "arm64-v8a" ]; then
            cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
                    -DHOST=$HOST -DARCH=$ARCH -DCMAKE_C_FLAGS="-fpic -O3 -g"      \
                    -DCMAKE_CXX_FLAGS="-fpic -O3 -g"                              \
                    -DASTCENC_ISA_NEON=ON -DASTCENC_INVARIANCE=OFF -DASTCENC_CLI=OFF
        else
            if [ "$ARCH_OPTION" = "x86" ]; then
                sed -i 's/_mm_popcnt_u64/__builtin_popcountll/g' Source/astcenc_vecmathlib_sse_4.h
            fi
            cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
                    -DHOST=$HOST -DARCH=$ARCH -DCMAKE_C_FLAGS="-fpic -O3 -g"      \
                    -DCMAKE_CXX_FLAGS="-fpic -O3 -g"                              \
                    -DASTCENC_ISA_SSE41=ON -DASTCENC_INVARIANCE=OFF -DASTCENC_CLI=OFF
        fi
        make -j $(($(nproc) + 1))
        check_error
        touch "$DIRNAME/deps-$ARCH_OPTION/astc-encoder.stamp"
    fi
}

if [ -z "$COMPILE_ARCH" ] || [ "$COMPILE_ARCH" = "all" ]; then
    build_deps armv7
    build_deps aarch64
    build_deps x86
    build_deps x86_64
else
    build_deps "$COMPILE_ARCH"
fi
