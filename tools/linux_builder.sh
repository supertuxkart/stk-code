#!/bin/sh

#
# (C) 2020 Dawid Gan, under the Holy Hedgehog License (do whatever you want)
#

# This is a build script that creates STK linux package.
#
# To run the script you need debootstrap and schroot packages, and working
# chroot environment.
#
# The build environment can be created using debootstrap:
#
#   debootstrap --arch i386 --components=main \
#               jessie ./chroot-jessie32 http://ftp.debian.org/debian
#
#   debootstrap --arch amd64 --components=main \
#               jessie ./chroot-jessie64 http://ftp.debian.org/debian
#
#
# Here is example configuration for schroot:
# /etc/schroot/chroot.d/chroot-jessie32.conf
#
#   [chroot-jessie32]
#   description=Debian Jessie
#   personality=linux32
#   directory=/path/to/chroot-jessie32
#   root-users=deve
#   type=directory
#   users=deve
#
#
# /etc/schroot/chroot.d/chroot-jessie64.conf
#
#   [chroot-jessie64]
#   description=Debian Jessie 64-bit
#   #personality=linux32
#   directory=/path/to/chroot-jessie64
#   root-users=deve
#   type=directory
#   users=deve
#
#
# Packages that are needed to compile all STK dependencies have to be installed
# manually inside both chroot directories.
      

export DIRNAME="$(dirname "$(readlink -f "$0")")"

######################## CONFIG ########################

export STK_VERSION="git`date +%Y%m%d`"
export THREADS_NUMBER=`nproc`
export SCHROOT_32BIT_NAME="chroot-stretch32"
export SCHROOT_64BIT_NAME="chroot-stretch64"
export SCHROOT_ARMV7_NAME="chroot-stretch-armhf"
export SCHROOT_ARM64_NAME="chroot-stretch-arm64"

export STKCODE_DIR="$DIRNAME/.."
export STKASSETS_DIR="$STKCODE_DIR/../supertuxkart-assets"
export OPENGLRECORDER_DIR="$STKCODE_DIR/../libopenglrecorder"
export STKEDITOR_DIR="$STKCODE_DIR/../supertuxkart-editor"

export BLACKLIST_LIBS="ld-linux libbsd.so libc.so libdl.so libdrm libexpat \
                       libGL libgl libm.so libmvec.so libpthread libresolv \
                       librt.so libX libxcb libxshm \
                       libEGL libgbm libwayland libffi bcm_host libvc"

export BUILD_DIR="build-linux"
export DEPENDENCIES_DIR="$STKCODE_DIR/dependencies-linux"
export STK_INSTALL_DIR="$STKCODE_DIR/build-linux-install"

export STATIC_GCC=1

# Use it if you build STK with Debian Jessie
export ENABLE_JESSIE_HACKS=1

########################################################


# A helper function that checks if error ocurred
check_error()
{
    if [ $? -gt 0 ]; then
        echo "Error ocurred."
        exit
    fi
}

write_run_game_sh()
{
    if [ -z "$1" ]; then
        return
    fi
    
    export INSTALL_DIR=$1
    export FILE="$INSTALL_DIR/run_game.sh"
    
    echo '#!/bin/sh'                                                      > "$FILE"
    echo ''                                                              >> "$FILE"
    echo 'export DIRNAME="$(dirname "$(readlink -f "$0")")"'             >> "$FILE"
    echo 'export SYSTEM_LD_LIBRARY_PATH="$LD_LIBRARY_PATH"'              >> "$FILE"
    echo ''                                                              >> "$FILE"
    echo 'export SUPERTUXKART_DATADIR="$DIRNAME"'                        >> "$FILE"
    echo 'export SUPERTUXKART_ASSETS_DIR="$DIRNAME/data/"'               >> "$FILE"
    echo ''                                                              >> "$FILE"
    echo 'cd "$DIRNAME"'                                                 >> "$FILE"
    echo ''                                                              >> "$FILE"
    echo 'export LD_LIBRARY_PATH="$DIRNAME/lib:$LD_LIBRARY_PATH"'        >> "$FILE"
    echo '"$DIRNAME/bin/supertuxkart" "$@"'                              >> "$FILE"
    echo ''                                                              >> "$FILE"
}

build_stk()
{
    if [ -z "$1" ] || [ -z "$2" ]; then
        return
    fi
    
    export ARCH_OPTION="$1"
    export STK_CMAKE_FLAGS="$2"
    export DEPENDENCIES_DIR="$DEPENDENCIES_DIR-$ARCH_OPTION"
    export BUILD_DIR="$BUILD_DIR-$ARCH_OPTION"
    export INSTALL_DIR="$DEPENDENCIES_DIR/dependencies"
    export INSTALL_LIB_DIR="$INSTALL_DIR/lib"
    export INSTALL_INCLUDE_DIR="$INSTALL_DIR/include"

    export PKG_CONFIG_PATH="$INSTALL_LIB_DIR/pkgconfig"
    export CFLAGS="-I$INSTALL_INCLUDE_DIR"
    export CPPFLAGS="-I$INSTALL_INCLUDE_DIR"
    export LDFLAGS="-Wl,-rpath,$INSTALL_LIB_DIR -L$INSTALL_LIB_DIR"
    
    export PATH="$INSTALL_DIR/bin:$PATH"
    
    if [ "$STATIC_GCC" -gt 0 ]; then
        LDFLAGS="$LDFLAGS -static-libgcc -static-libstdc++"
    fi
    
    cd "$STKCODE_DIR"
    mkdir -p "$DEPENDENCIES_DIR"
    
    # CMake
    if [ ! -f "$DEPENDENCIES_DIR/cmake.stamp" ]; then
        echo "Compiling CMake"
        git clone --depth 1 -b v3.24.1 https://github.com/Kitware/CMake.git "$DEPENDENCIES_DIR/cmake"
    
        cd "$DEPENDENCIES_DIR/cmake"
        ./bootstrap --prefix="$INSTALL_DIR" \
                    --parallel=$THREADS_NUMBER \
                    -- -DCMAKE_USE_OPENSSL=0 &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/cmake.stamp"
    fi
    
    # ISPC
    if [ ! -f "$DEPENDENCIES_DIR/ispc.stamp" ]; then
        if [ "$ARCH_OPTION" = "x86_64" ]; then
            echo "Downloading ISPC"
            
            mkdir -p "$DEPENDENCIES_DIR/ispc"
            cd "$DEPENDENCIES_DIR/ispc"
            ISPC_VERSION="v1.18.0"
            wget https://github.com/ispc/ispc/releases/download/$ISPC_VERSION/ispc-$ISPC_VERSION-linux.tar.gz -O ispc.tar.gz
            check_error
            tar -xzf "ispc.tar.gz"
            check_error
            cp "$DEPENDENCIES_DIR/ispc/ispc-$ISPC_VERSION-linux/bin/ispc" "$INSTALL_DIR/bin/"
        fi
        
        touch "$DEPENDENCIES_DIR/ispc.stamp"
    fi
    
    # Zlib
    if [ ! -f "$DEPENDENCIES_DIR/zlib.stamp" ]; then
        echo "Compiling zlib"
        mkdir -p "$DEPENDENCIES_DIR/zlib"
        cp -a -f "$DEPENDENCIES_DIR/../lib/zlib/"* "$DEPENDENCIES_DIR/zlib"
    
        cd "$DEPENDENCIES_DIR/zlib"
        cmake . -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
                -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                -DINSTALL_PKGCONFIG_DIR="$PKG_CONFIG_PATH" &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/zlib.stamp"
    fi
    
    # Libpng
    if [ ! -f "$DEPENDENCIES_DIR/libpng.stamp" ]; then
        echo "Compiling libpng"
        mkdir -p "$DEPENDENCIES_DIR/libpng"
        mkdir -p "$DEPENDENCIES_DIR/libpng/lib"
        cp -a -f "$DEPENDENCIES_DIR/../lib/libpng/"* "$DEPENDENCIES_DIR/libpng"
    
        cd "$DEPENDENCIES_DIR/libpng"
        ./configure --prefix="$INSTALL_DIR" &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/libpng.stamp"
    fi
    
    # Freetype bootstrap
    if [ ! -f "$DEPENDENCIES_DIR/freetype_bootstrap.stamp" ]; then
        echo "Compiling freetype bootstrap"
        mkdir -p "$DEPENDENCIES_DIR/freetype/build"
        cp -a -f "$DEPENDENCIES_DIR/../lib/freetype/"* "$DEPENDENCIES_DIR/freetype"
    
        cd "$DEPENDENCIES_DIR/freetype/build"
        cmake .. -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
                 -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                 -DBUILD_SHARED_LIBS=1 \
                 -DFT_DISABLE_HARFBUZZ=1 \
                 -DFT_DISABLE_BZIP2=1 \
                 -DFT_DISABLE_BROTLI=1 \
                 -DFT_REQUIRE_ZLIB=1 \
                 -DFT_REQUIRE_PNG=1 &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/freetype_bootstrap.stamp"
    fi

    # Harfbuzz
    if [ ! -f "$DEPENDENCIES_DIR/harfbuzz.stamp" ]; then
        echo "Compiling harfbuzz"
        mkdir -p "$DEPENDENCIES_DIR/harfbuzz"
        cp -a -f "$DEPENDENCIES_DIR/../lib/harfbuzz/"* "$DEPENDENCIES_DIR/harfbuzz"
    
        cd "$DEPENDENCIES_DIR/harfbuzz"
        ./autogen.sh
        ./configure --prefix="$INSTALL_DIR" \
                    --with-freetype=yes \
                    --with-glib=no \
                    --with-gobject=no \
                    --with-cairo=no \
                    --with-fontconfig=no \
                    --with-icu=no \
                    --with-graphite2=no &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/harfbuzz.stamp"
    fi

    # Freetype
    if [ ! -f "$DEPENDENCIES_DIR/freetype.stamp" ]; then
        echo "Compiling freetype"
        mkdir -p "$DEPENDENCIES_DIR/freetype/build"
        cp -a -f "$DEPENDENCIES_DIR/../lib/freetype/"* "$DEPENDENCIES_DIR/freetype"

        cd "$DEPENDENCIES_DIR/freetype/build"
        rm -rf ./*
        cmake .. -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
                 -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                 -DBUILD_SHARED_LIBS=1 \
                 -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=1 \
                 -DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec=1 \
                 -DFT_REQUIRE_HARFBUZZ=1 \
                 -DFT_DISABLE_BZIP2=1 \
                 -DFT_DISABLE_BROTLI=1 \
                 -DFT_REQUIRE_ZLIB=1 \
                 -DFT_REQUIRE_PNG=1 &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/freetype.stamp"
    fi

    # Openal
    if [ ! -f "$DEPENDENCIES_DIR/openal.stamp" ]; then
        echo "Compiling openal"
        mkdir -p "$DEPENDENCIES_DIR/openal"
        cp -a -f "$DEPENDENCIES_DIR/../lib/openal/"* "$DEPENDENCIES_DIR/openal"
        
        if [ "$ENABLE_JESSIE_HACKS" -gt 0 ]; then
            JESSIE_HACK="-DHAVE_LIBATOMIC=0"
        fi
    
        cd "$DEPENDENCIES_DIR/openal"
        cmake . -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
                -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                -DALSOFT_UTILS=0         \
                -DALSOFT_EXAMPLES=0      \
                -DALSOFT_TESTS=0         \
                -DALSOFT_BACKEND_SNDIO=0 \
                $JESSIE_HACK &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/openal.stamp"
    fi
    
    # MbedTLS
    if [ ! -f "$DEPENDENCIES_DIR/mbedtls.stamp" ]; then
        echo "Compiling mbedtls"
        mkdir -p "$DEPENDENCIES_DIR/mbedtls"
        cp -a -f "$DEPENDENCIES_DIR/../lib/mbedtls/"* "$DEPENDENCIES_DIR/mbedtls"
    
        cd "$DEPENDENCIES_DIR/mbedtls"
        cmake . -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
                -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                -DUSE_SHARED_MBEDTLS_LIBRARY=1 \
                -DENABLE_TESTING=0 \
                -DENABLE_PROGRAMS=0 &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/mbedtls.stamp"
    fi
    
    # Curl
    if [ ! -f "$DEPENDENCIES_DIR/curl.stamp" ]; then
        echo "Compiling curl"
        mkdir -p "$DEPENDENCIES_DIR/curl"
        cp -a -f "$DEPENDENCIES_DIR/../lib/curl/"* "$DEPENDENCIES_DIR/curl"
    
        cd "$DEPENDENCIES_DIR/curl"
        cmake . -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
                -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                -DBUILD_TESTING=0 \
                -DBUILD_CURL_EXE=0 \
                -DCURL_USE_MBEDTLS=1 \
                -DUSE_ZLIB=1 \
                -DCURL_USE_OPENSSL=0 \
                -DCURL_USE_LIBSSH=0 \
                -DCURL_USE_LIBSSH2=0 \
                -DCURL_USE_GSSAPI=0 \
                -DUSE_NGHTTP2=0 \
                -DUSE_QUICHE=0 \
                -DHTTP_ONLY=1 \
                -DCURL_CA_BUNDLE=none \
                -DCURL_CA_PATH=none \
                -DENABLE_THREADED_RESOLVER=1 &&
        make -j$THREADS_NUMBER && 
        make install
        check_error
        rm -rf "$INSTALL_DIR/lib/cmake/CURL"
        touch "$DEPENDENCIES_DIR/curl.stamp"
    fi
    
    # Libjpeg
    if [ ! -f "$DEPENDENCIES_DIR/libjpeg.stamp" ]; then
        echo "Compiling libjpeg"
        mkdir -p "$DEPENDENCIES_DIR/libjpeg"
        cp -a -f "$DEPENDENCIES_DIR/../lib/libjpeg/"* "$DEPENDENCIES_DIR/libjpeg"
    
        cd "$DEPENDENCIES_DIR/libjpeg"
        chmod a+x ./configure
        ASM_NASM=yasm \
        cmake . -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
                -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/libjpeg.stamp"
    fi
    
    # Libogg
    if [ ! -f "$DEPENDENCIES_DIR/libogg.stamp" ]; then
        echo "Compiling libogg"
        mkdir -p "$DEPENDENCIES_DIR/libogg"
        cp -a -f "$DEPENDENCIES_DIR/../lib/libogg/"* "$DEPENDENCIES_DIR/libogg"
    
        cd "$DEPENDENCIES_DIR/libogg"
        ./autogen.sh
        ./configure --prefix="$INSTALL_DIR" &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/libogg.stamp"
    fi
    
    # Libvorbis
    if [ ! -f "$DEPENDENCIES_DIR/libvorbis.stamp" ]; then
        echo "Compiling libvorbis"
        mkdir -p "$DEPENDENCIES_DIR/libvorbis"
        cp -a -f "$DEPENDENCIES_DIR/../lib/libvorbis/"* "$DEPENDENCIES_DIR/libvorbis"
        
        cd "$DEPENDENCIES_DIR/libvorbis"
        cmake . -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
                -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                -DBUILD_SHARED_LIBS=1 &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/libvorbis.stamp"
    fi
    
    # Shaderc
    if [ ! -f "$DEPENDENCIES_DIR/shaderc.stamp" ]; then
        echo "Compiling shaderc"
        
        "$DEPENDENCIES_DIR/../lib/shaderc/utils/git-sync-deps"
        
        mkdir -p "$DEPENDENCIES_DIR/shaderc"
        cp -a -f "$DEPENDENCIES_DIR/../lib/shaderc/"* "$DEPENDENCIES_DIR/shaderc"

        cd "$DEPENDENCIES_DIR/shaderc"
        cmake . -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
                -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                -DCMAKE_C_FLAGS="-fpic -O3"           \
                -DCMAKE_CXX_FLAGS="-fpic -O3"         \
                -DSHADERC_SKIP_INSTALL=1              \
                -DCMAKE_BUILD_TYPE=Release            \
                -DSHADERC_SKIP_TESTS=1                \
                -DSHADERC_SKIP_EXAMPLES=1             \
                -DSPIRV_HEADERS_SKIP_INSTALL=1        \
                -DSPIRV_HEADERS_SKIP_EXAMPLES=1       \
                -DSKIP_SPIRV_TOOLS_INSTALL=1          \
                -DSPIRV_SKIP_TESTS=1                  \
                -DSPIRV_SKIP_EXECUTABLES=1            \
                -DENABLE_GLSLANG_BINARIES=0           \
                -DENABLE_CTEST=0 &&
        make -j$THREADS_NUMBER &&
        cp "$DEPENDENCIES_DIR/shaderc/libshaderc/libshaderc"* "$INSTALL_DIR/lib/" &&
        cp -a -f "$DEPENDENCIES_DIR/shaderc/libshaderc/include/"* "$INSTALL_DIR/include/"
        check_error
        touch "$DEPENDENCIES_DIR/shaderc.stamp"
    fi
    
    # ASTC-encoder
    if [ ! -f "$DEPENDENCIES_DIR/astc-encoder.stamp" ]; then
        echo "Compiling astc-encoder"
        mkdir -p "$DEPENDENCIES_DIR/astc-encoder"
        cp -a -f "$DEPENDENCIES_DIR/../lib/astc-encoder/"* "$DEPENDENCIES_DIR/astc-encoder"

        cd "$DEPENDENCIES_DIR/astc-encoder"
        sed -i '/-Werror/d' Source/cmake_core.cmake
        sed -i 's|${ASTC_TARGET}-static|astcenc|g' Source/cmake_core.cmake
        if [ "$ARCH_OPTION" = "armv7" ]; then
            ASTC_CMAKE_FLAGS=""
            ASTC_CFLAGS="-mfpu=neon"
        elif [ "$ARCH_OPTION" = "arm64" ]; then
            ASTC_CMAKE_FLAGS="-DISA_NEON=ON -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang"
        elif [ "$ARCH_OPTION" = "x86" ]; then
            #ASTC_CMAKE_FLAGS="-DISA_SSE2=ON"
            ASTC_CMAKE_FLAGS=""
        elif [ "$ARCH_OPTION" = "x86_64" ]; then
            ASTC_CMAKE_FLAGS="-DISA_SSE41=ON"
        fi
    
        cmake . -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
                -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                $ASTC_CMAKE_FLAGS \
                -DCMAKE_C_FLAGS="-fpic -O3 -g $ASTC_CFLAGS" \
                -DCMAKE_CXX_FLAGS="-fpic -O3 -g $ASTC_CFLAGS" \
                -DNO_INVARIANCE=ON -DCLI=OFF &&
        make -j$THREADS_NUMBER &&
        cp "$DEPENDENCIES_DIR/astc-encoder/Source/libastcenc.a" "$INSTALL_DIR/lib/" &&
        cp "$DEPENDENCIES_DIR/astc-encoder/Source/astcenc.h" "$INSTALL_DIR/include/"
        check_error
        touch "$DEPENDENCIES_DIR/astc-encoder.stamp"
    fi

    # Wayland
    if [ ! -f "$DEPENDENCIES_DIR/wayland.stamp" ]; then
        echo "Compiling wayland"
        mkdir -p "$DEPENDENCIES_DIR/wayland"
        cp -a -f "$DEPENDENCIES_DIR/../lib/wayland/"* "$DEPENDENCIES_DIR/wayland"
    
        cd "$DEPENDENCIES_DIR/wayland"
        ./autogen.sh
        ./configure --prefix="$INSTALL_DIR" --disable-documentation &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/wayland.stamp"
    fi
        
    # SDL2
    if [ ! -f "$DEPENDENCIES_DIR/sdl2.stamp" ]; then
        echo "Compiling SDL2"
        mkdir -p "$DEPENDENCIES_DIR/sdl2"
        cp -a -f "$DEPENDENCIES_DIR/../lib/sdl2/"* "$DEPENDENCIES_DIR/sdl2"
    
        cd "$DEPENDENCIES_DIR/sdl2"
        ./configure --prefix="$INSTALL_DIR" &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/sdl2.stamp"
    fi
    
    # Libvpx
    if [ ! -f "$DEPENDENCIES_DIR/libvpx.stamp" ]; then
        echo "Compiling libvpx"
        mkdir -p "$DEPENDENCIES_DIR/libvpx"
        cp -a -f "$DEPENDENCIES_DIR/../lib/libvpx/"* "$DEPENDENCIES_DIR/libvpx"
    
        cd "$DEPENDENCIES_DIR/libvpx"
        ./configure --prefix="$INSTALL_DIR" \
                    --enable-shared &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/libvpx.stamp"
    fi
    
    # Libbluetooth
    if [ ! -f "$DEPENDENCIES_DIR/bluez.stamp" ]; then
        echo "Compiling libbluetooth"
        mkdir -p "$DEPENDENCIES_DIR/bluez"
        cp -a -f "$DEPENDENCIES_DIR/../lib/bluez/"* "$DEPENDENCIES_DIR/bluez"
    
        cd "$DEPENDENCIES_DIR/bluez"
        ./bootstrap
        ./configure --prefix="$INSTALL_DIR" \
                    --enable-library \
                    --disable-debug \
                    --disable-systemd \
                    --disable-tools \
                    --disable-obex \
                    --disable-cups \
                    --disable-client \
                    --disable-datafiles \
                    --disable-monitor \
                    --disable-udev &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/bluez.stamp"
    fi
    
    # Sqlite
    if [ ! -f "$DEPENDENCIES_DIR/sqlite.stamp" ]; then
        echo "Compiling sqlite"
        mkdir -p "$DEPENDENCIES_DIR/sqlite"
        cp -a -f "$DEPENDENCIES_DIR/../lib/sqlite/"* "$DEPENDENCIES_DIR/sqlite"
        sed -i s/' STATIC '/' SHARED '/g "$DEPENDENCIES_DIR/sqlite/CMakeLists.txt"
    
        cd "$DEPENDENCIES_DIR/sqlite"
        cmake . -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
                -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                -DINSTALL_PKGCONFIG_DIR="$PKG_CONFIG_PATH" &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/sqlite.stamp"
    fi
    
    # Openglrecorder
    if [ ! -f "$DEPENDENCIES_DIR/openglrecorder.stamp" ]; then
        echo "Compiling openglrecorder"
        mkdir -p "$DEPENDENCIES_DIR/openglrecorder"
        cp -a -f "$OPENGLRECORDER_DIR/"* "$DEPENDENCIES_DIR/openglrecorder"
    
        cd "$DEPENDENCIES_DIR/openglrecorder"
        cmake . -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
                -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                -DBUILD_PULSE_WO_DL=0 &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/openglrecorder.stamp"
    fi

    # Supertuxkart
    mkdir -p "$STKCODE_DIR/$BUILD_DIR"
    cd "$STKCODE_DIR/$BUILD_DIR"
    
    if [ -f "$INSTALL_DIR/bin/ispc" ]; then
        HAS_ISPC=1
    else
        HAS_ISPC=0
    fi
    
    cmake .. -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
             -DUSE_SYSTEM_ANGELSCRIPT=0 \
             -DUSE_SYSTEM_ENET=0 \
             -DUSE_SYSTEM_WIIUSE=0 \
             -DUSE_CRYPTO_OPENSSL=0 \
             -DENABLE_WAYLAND_DEVICE=0 \
             -DBC7_ISPC=$HAS_ISPC \
             -DCMAKE_DISABLE_FIND_PACKAGE_Fontconfig=1 \
             $STK_CMAKE_FLAGS &&
    make -j$THREADS_NUMBER
    check_error
    
    # Stk editor
    mkdir -p "$STKEDITOR_DIR/$BUILD_DIR"
    cd "$STKEDITOR_DIR/$BUILD_DIR"
    cmake .. -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
             -DSTATIC_ZLIB=1 \
             -DSTATIC_PHYSFS=1 \
             -DCMAKE_DISABLE_FIND_PACKAGE_Fontconfig=1 \
             $STK_CMAKE_FLAGS &&
    make -j$THREADS_NUMBER
    check_error
}

copy_libraries()
{
    if [ -z "$1" ] || [ -z "$2" ]; then
        return
    fi
    
    export ARCH_OPTION="$1"
    export LIB_INSTALL_DIR="$2"
    export DEPENDENCIES_DIR="$DEPENDENCIES_DIR-$ARCH_OPTION"
    export BUILD_DIR="$BUILD_DIR-$ARCH_OPTION"
    
    if [ -z "$DEPENDENCIES_DIR" ] || [ -z "$BUILD_DIR" ] || [ -z "$LIB_INSTALL_DIR" ]; then
        return
    fi
    
    LIBRARIES_LIST=`LD_LIBRARY_PATH="$DEPENDENCIES_DIR/dependencies/lib" \
                    ldd "$STKCODE_DIR/$BUILD_DIR/bin/supertuxkart" | \
                    cut -d">" -f2 | cut -d"(" -f1 | grep "\.so"`

    for FILE in $LIBRARIES_LIST; do 
        BLACKLISTED=0
    
        for BLACKLIST_LIB in $BLACKLIST_LIBS; do
            if [ `echo "$FILE" | grep -c "$BLACKLIST_LIB"` -gt 0 ]; then
                BLACKLISTED=1
                break
            fi
        done

        if [ $BLACKLISTED -eq 1 ]; then
            continue
        fi 
    
        if [ -f "$FILE" ]; then
            echo " Copying $FILE"
            cp -L "$FILE" "$LIB_INSTALL_DIR"
        fi
    done
}

test_package()
{
    if [ -z "$1" ]; then
        return
    fi
    
    PACKAGE_DIR="$1"
    BINARY_ARCH="$2"
    
    if [ `objdump -a "$PACKAGE_DIR/bin/supertuxkart" | grep -c "$BINARY_ARCH"` -eq 0 ]; then
        echo "Error: bin/supertuxkart is not $BINARY_ARCH"
        exit 1
    fi
    
    if [ `objdump -a "$PACKAGE_DIR/bin/supertuxkart-editor" | grep -c "$BINARY_ARCH"` -eq 0 ]; then
        echo "Error: bin/supertuxkart-editor is not $BINARY_ARCH"
        exit 1
    fi

    if [ `LD_LIBRARY_PATH="$PACKAGE_DIR/lib" ldd "$PACKAGE_DIR/bin/supertuxkart" | grep -c "not found"` -gt 0 ]; then
        echo "Error: bin/supertuxkart has some missing libraries"
        exit 1
    fi
    
    if [ `ldd "$PACKAGE_DIR/bin/supertuxkart-editor" | grep -c "not found"` -gt 0 ]; then
        echo "Error: bin/supertuxkart-editor has some missing libraries"
        exit 1
    fi

    LD_LIBRARY_PATH="$PACKAGE_DIR/lib" "$PACKAGE_DIR/bin/supertuxkart" --version
    
    if [ $? -ne 0 ]; then
        echo "Error: Couldn't start bin/supertuxkart"
        exit 1
    fi
}

create_package()
{
    SCHROOT_NAME="$1"
    ARCH="$2"
    BINARY_ARCH="$3"
    
    echo "Building $ARCH version..."
    
    schroot -c $SCHROOT_NAME -- "$0" build_stk "$ARCH" "-DDEBUG_SYMBOLS=1"
    
    if [ ! -f "$STKCODE_DIR/$BUILD_DIR-$ARCH/bin/supertuxkart" ]; then
        echo "Couldn't build $ARCH version."
        exit 1
    fi
    
    echo "Prepare package..."

    STK_PACKAGE_DIR="$STK_INSTALL_DIR/SuperTuxKart-$STK_VERSION-linux-$ARCH"
    
    if [ -f "$STK_PACKAGE_DIR" ]; then
        rm -rf "$STK_PACKAGE_DIR"
    fi
    
    mkdir -p "$STK_PACKAGE_DIR"
    mkdir -p "$STK_PACKAGE_DIR/bin"
    mkdir -p "$STK_PACKAGE_DIR/lib"
    
    schroot -c $SCHROOT_NAME -- "$0" copy_libraries "$ARCH" "$STK_PACKAGE_DIR/lib"
    
    find "$STK_PACKAGE_DIR/lib" -type f -exec strip -s {} \;
    
    if [ "$STATIC_GCC" -eq 0 ]; then
        mv "$STK_PACKAGE_DIR/lib/libgcc_s.so.1" "$STK_PACKAGE_DIR/lib/libgcc_s.so.1-orig"
        mv "$STK_PACKAGE_DIR/lib/libstdc++.so.6" "$STK_PACKAGE_DIR/lib/libstdc++.so.6-orig"
    fi
    
    write_run_game_sh "$STK_PACKAGE_DIR"
    
    cp "$STKCODE_DIR/$BUILD_DIR-$ARCH/bin/supertuxkart" "$STK_INSTALL_DIR/supertuxkart-$STK_VERSION-linux-$ARCH-symbols"
    cp "$STKEDITOR_DIR/$BUILD_DIR-$ARCH/bin/supertuxkart-editor" "$STK_INSTALL_DIR/supertuxkart-editor-$STK_VERSION-linux-$ARCH-symbols"
    
    cp -a "$STKCODE_DIR/$BUILD_DIR-$ARCH/bin/supertuxkart" "$STK_PACKAGE_DIR/bin/"
    cp -a "$STKEDITOR_DIR/$BUILD_DIR-$ARCH/bin/supertuxkart-editor" "$STK_PACKAGE_DIR/bin/"
    
    cp -a "$STKCODE_DIR/data/." "$STK_PACKAGE_DIR/data"
    cp -a "$STKASSETS_DIR/editor" "$STK_PACKAGE_DIR/data/"
    cp -a "$STKASSETS_DIR/karts" "$STK_PACKAGE_DIR/data/"
    cp -a "$STKASSETS_DIR/library" "$STK_PACKAGE_DIR/data/"
    cp -a "$STKASSETS_DIR/models" "$STK_PACKAGE_DIR/data/"
    cp -a "$STKASSETS_DIR/music" "$STK_PACKAGE_DIR/data/"
    cp -a "$STKASSETS_DIR/sfx" "$STK_PACKAGE_DIR/data/"
    cp -a "$STKASSETS_DIR/textures" "$STK_PACKAGE_DIR/data/"
    cp -a "$STKASSETS_DIR/tracks" "$STK_PACKAGE_DIR/data/"
    cp -a "$STKASSETS_DIR/licenses.txt" "$STK_PACKAGE_DIR/data/"
    
    strip --strip-debug "$STK_PACKAGE_DIR/bin/supertuxkart"
    strip --strip-debug "$STK_PACKAGE_DIR/bin/supertuxkart-editor"
    
    find "$STK_PACKAGE_DIR/bin" -type f -exec chrpath -d {} \;
    find "$STK_PACKAGE_DIR/lib" -type f -exec chrpath -d {} \;
    
    chmod a+rwx "$STK_PACKAGE_DIR" -R
    find "$STK_PACKAGE_DIR" -type f -exec chmod a-x {} \;
    find "$STK_PACKAGE_DIR/bin" -type f -exec chmod a+x {} \;
    chmod a+x "$STK_PACKAGE_DIR/run_game.sh"
    
    schroot -c $SCHROOT_NAME -- "$0" test_package "$STK_PACKAGE_DIR" "$BINARY_ARCH"
    
    # Compress package
    
    echo "Compress package..."
    
    cd "$STK_INSTALL_DIR"
    tar cf - "SuperTuxKart-$STK_VERSION-linux-$ARCH" | xz -T$THREADS_NUMBER -z -e -f - > "SuperTuxKart-$STK_VERSION-linux-$ARCH.tar.xz"
    cd -
}

# Handle clean command
if [ ! -z "$1" ] && [ "$1" = "clean" ]; then
    rm -rf "$DEPENDENCIES_DIR-"*
    rm -rf "$STKCODE_DIR/$BUILD_DIR-"*
    rm -rf "$STKEDITOR_DIR/$BUILD_DIR-"*
    rm -rf "$STK_INSTALL_DIR"
    exit 0
fi

# Handle build_stk command (internal only)
if [ ! -z "$1 " ] && [ "$1" = "build_stk" ]; then
    build_stk "$2" "$3"
    exit 0
fi

# Handle copy_libraries command (internal only)
if [ ! -z "$1 " ] && [ "$1" = "copy_libraries" ]; then
    copy_libraries "$2" "$3"
    exit 0
fi

# Handle test_package command (internal only)
if [ ! -z "$1 " ] && [ "$1" = "test_package" ]; then
    test_package "$2" "$3"
    exit 0
fi


# Building STK

create_package "$SCHROOT_32BIT_NAME" "x86" "elf32-i386"
create_package "$SCHROOT_64BIT_NAME" "x86_64" "elf64-x86-64"
create_package "$SCHROOT_ARMV7_NAME" "armv7" "elf32-littlearm"
create_package "$SCHROOT_ARM64_NAME" "arm64" "elf64-littleaarch64"

echo "Success."
