#!/bin/sh
#
# (C) 2016-2017 Dawid Gan, under the GPLv3
#
# A script that creates the apk build


export DIRNAME=$(realpath "$(dirname "$0")")

export NDK_PATH_DEFAULT="$DIRNAME/android-ndk"
export SDK_PATH_DEFAULT="$DIRNAME/android-sdk"

export NDK_TOOLCHAIN_PATH="$DIRNAME/obj/bin"
export NDK_BUILD_SCRIPT="$DIRNAME/Android.mk"
export PATH="$DIRNAME/obj/bin:$PATH"
export CROSS_SYSROOT="$DIRNAME/obj/sysroot"

#export NDK_CCACHE=ccache
export NDK_CPPFLAGS="-O3 -g"

export NDK_ABI_ARMV7=armeabi-v7a
export ARCH_ARMV7=arm
export HOST_ARMV7=arm-linux-androideabi
export NDK_PLATFORM_ARMV7=android-19
export SDK_VERSION_ARMV7=19

export NDK_ABI_X86=x86
export ARCH_X86=x86
export HOST_X86=i686-linux-android
export NDK_PLATFORM_X86=android-19
export SDK_VERSION_X86=19

export NDK_ABI_AARCH64=arm64-v8a
export ARCH_AARCH64=arm64
export HOST_AARCH64=aarch64-linux-android
export NDK_PLATFORM_AARCH64=android-21
export SDK_VERSION_AARCH64=21


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
    rm -rf bin
    rm -rf build
    rm -rf libs
    rm -rf obj
    rm -rf .gradle
    exit
fi

# Check if compilation for different platform has been started before
if [ -f "$DIRNAME/obj/compile_arch" ]; then
    PROJECT_ARCH=$(cat "$DIRNAME/obj/compile_arch") 
    
    if [ -z "$COMPILE_ARCH" ]; then
        COMPILE_ARCH="$PROJECT_ARCH"
    elif [ "$PROJECT_ARCH" != "$COMPILE_ARCH" ]; then
        echo "Error: Compilation for different platform has been already made."
        echo "Run './make.sh clean' first or set COMPILE_ARCH variable" \
             "to '$PROJECT_ARCH.'"
        exit
    fi
fi

# Update variables for selected architecture
if [ -z "$COMPILE_ARCH" ]; then
    COMPILE_ARCH="armv7"
fi

if [ "$COMPILE_ARCH" = "armv7" ]; then
    export NDK_PLATFORM=$NDK_PLATFORM_ARMV7
    export NDK_ABI=$NDK_ABI_ARMV7
    export ARCH=$ARCH_ARMV7
    export HOST=$HOST_ARMV7
    export SDK_VERSION=$SDK_VERSION_ARMV7
elif [ "$COMPILE_ARCH" = "x86" ]; then
    export NDK_PLATFORM=$NDK_PLATFORM_X86
    export NDK_ABI=$NDK_ABI_X86
    export ARCH=$ARCH_X86
    export HOST=$HOST_X86
    export SDK_VERSION=$SDK_VERSION_X86
elif [ "$COMPILE_ARCH" = "aarch64" ]; then
    export NDK_PLATFORM=$NDK_PLATFORM_AARCH64
    export NDK_ABI=$NDK_ABI_AARCH64
    export ARCH=$ARCH_AARCH64
    export HOST=$HOST_AARCH64
    export SDK_VERSION=$SDK_VERSION_AARCH64
else
    echo "Unknow COMPILE_ARCH: $COMPILE_ARCH. Possible values are: " \
         "armv7, aarch64, x86"
    exit
fi

# Update variables for selected build type
if [ -z "$BUILD_TYPE" ]; then
    BUILD_TYPE="debug"
fi

if [ "$BUILD_TYPE" = "debug" ] || [ "$BUILD_TYPE" = "Debug" ]; then
    export ANT_BUILD_TYPE="debug"
    export GRADLE_BUILD_TYPE="assembleDebug"
    export IS_DEBUG_BUILD=1
elif [ "$BUILD_TYPE" = "release" ] || [ "$BUILD_TYPE" = "Release" ]; then
    export ANT_BUILD_TYPE="release"
    export GRADLE_BUILD_TYPE="assembleRelease"
    export IS_DEBUG_BUILD=0
else
    echo "Unsupported BUILD_TYPE: $BUILD_TYPE. Possible values are: " \
         "debug, release"
    exit
fi

# Check selected build tool
if [ -z "$BUILD_TOOL" ]; then
    BUILD_TOOL="gradle"
fi

if [ "$BUILD_TOOL" != "gradle" ] && [ "$BUILD_TOOL" != "ant" ]; then
    echo "Unsupported BUILD_TOOL: $BUILD_TOOL. Possible values are: " \
         "gradle, ant"
    exit
fi

# Check if we have access to the Android NDK and SDK
if [ -z "$NDK_PATH" ]; then
    export NDK_PATH="$NDK_PATH_DEFAULT"
fi

if [ -z "$SDK_PATH" ]; then
    export SDK_PATH="$SDK_PATH_DEFAULT"
fi

NDK_PATH=$(realpath "$NDK_PATH")
SDK_PATH=$(realpath "$SDK_PATH")

if [ ! -d "$NDK_PATH" ]; then
    echo "Error: Couldn't find $NDK_PATH directory. Please create a symlink" \
         "to your Android NDK installation in the $NDK_PATH_DEFAULT or set"  \
         "proper path in the NDK_PATH variable"
    exit
fi

if [ ! -d "$SDK_PATH" ]; then
    echo "Error: Couldn't find $SDK_PATH directory. Please create a symlink" \
         "to your Android SDK installation in the $SDK_PATH_DEFAULT or set"  \
         "proper path in the SDK_PATH variable"
    exit
fi

# Find newest build-tools version
if [ -z "$BUILD_TOOLS_VER" ]; then
    BUILD_TOOLS_DIRS=`ls -1 "$SDK_PATH/build-tools" | sort -V -r`
   
    for DIR in $BUILD_TOOLS_DIRS; do
        if [ "$DIR" = `echo $DIR | sed 's/[^0-9,.]//g'` ]; then
            BUILD_TOOLS_VER="$DIR"
            break
        fi
    done
fi

if [ -z "$BUILD_TOOLS_VER" ] || [ ! -d "$SDK_PATH/build-tools/$BUILD_TOOLS_VER" ]; then
    echo "Error: Couldn't detect build-tools version."
    exit
fi


# Standalone toolchain
if [ ! -f "$DIRNAME/obj/make_standalone_toolchain.stamp" ]; then
    echo "Creating standalone toolchain"
    rm -rf "$DIRNAME/obj"
    ${NDK_PATH}/build/tools/make-standalone-toolchain.sh \
        --platform=$NDK_PLATFORM                         \
        --install-dir="$DIRNAME/obj/"                    \
        --arch=$ARCH
    check_error
    touch "$DIRNAME/obj/make_standalone_toolchain.stamp"
    echo $COMPILE_ARCH > "$DIRNAME/obj/compile_arch"
fi

# Freetype
if [ ! -f "$DIRNAME/obj/freetype.stamp" ]; then
    echo "Compiling freetype"
    mkdir -p "$DIRNAME/obj/freetype"
    cp -a -f "$DIRNAME/../lib/freetype/"* "$DIRNAME/obj/freetype"

    cd "$DIRNAME/obj/freetype"
    ./configure --host=$HOST          \
                --without-zlib        \
                --without-png         \
                --without-harfbuzz &&
    make $@
    check_error
    touch "$DIRNAME/obj/freetype.stamp"
fi

# Zlib
if [ ! -f "$DIRNAME/obj/zlib.stamp" ]; then
    echo "Compiling zlib"
    mkdir -p "$DIRNAME/obj/zlib"
    cp -a -f "$DIRNAME/../lib/zlib/"* "$DIRNAME/obj/zlib"

    cd "$DIRNAME/obj/zlib"
    cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
            -DHOST=$HOST &&
    make $@
    check_error
    touch "$DIRNAME/obj/zlib.stamp"
fi

# Libpng
if [ ! -f "$DIRNAME/obj/libpng.stamp" ]; then
    echo "Compiling libpng"
    mkdir -p "$DIRNAME/obj/libpng"
    mkdir -p "$DIRNAME/obj/libpng/lib"
    cp -a -f "$DIRNAME/../lib/libpng/"* "$DIRNAME/obj/libpng"

    cd "$DIRNAME/obj/libpng"
    cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
            -DHOST=$HOST                                                  \
            -DZLIB_LIBRARY="$DIRNAME/obj/zlib/libz.a"                     \
            -DZLIB_INCLUDE_DIR="$DIRNAME/obj/zlib/"                       \
            -DPNG_TESTS=0 &&
    make $@
    check_error
    touch "$DIRNAME/obj/libpng.stamp"
fi

# Openal
if [ ! -f "$DIRNAME/obj/openal.stamp" ]; then
    echo "Compiling openal"
    mkdir -p "$DIRNAME/obj/openal"
    cp -a -f "$DIRNAME/../lib/openal/"* "$DIRNAME/obj/openal"

    cd "$DIRNAME/obj/openal"
    cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
            -DHOST=$HOST                                                  \
            -DALSOFT_UTILS=0                                              \
            -DALSOFT_EXAMPLES=0                                           \
            -DALSOFT_TESTS=0                                              \
            -DLIBTYPE=STATIC &&
    make $@
    check_error
    touch "$DIRNAME/obj/openal.stamp"
fi

# OpenSSL
if [ ! -f "$DIRNAME/obj/openssl.stamp" ]; then
    echo "Compiling openssl"
    mkdir -p "$DIRNAME/obj/openssl"
    cp -a -f "$DIRNAME/../lib/openssl/"* "$DIRNAME/obj/openssl"

    cd "$DIRNAME/obj/openssl"
    ./Configure android --cross-compile-prefix="$HOST-"
    make $@
    check_error
    touch "$DIRNAME/obj/openssl.stamp"
fi

# Curl
if [ ! -f "$DIRNAME/obj/curl.stamp" ]; then
    echo "Compiling curl"
    mkdir -p "$DIRNAME/obj/curl"
    cp -a -f "$DIRNAME/../lib/curl/"* "$DIRNAME/obj/curl"

    cd "$DIRNAME/obj/curl"
    CPPFLAGS="-I$DIRNAME/obj/openssl/include $CPPFLAGS" \
    LDFLAGS="-L$DIRNAME/obj/openssl/ $LDFLAGS"          \
    ./configure --host=$HOST                            \
                --with-ssl                              \
                --disable-shared                        \
                --enable-static                         \
                --enable-threaded-resolver &&
    make $@
    check_error
    touch "$DIRNAME/obj/curl.stamp"
fi

# Jpeglib
if [ ! -f "$DIRNAME/obj/jpeglib.stamp" ]; then
    echo "Compiling jpeglib"
    mkdir -p "$DIRNAME/obj/jpeglib"
    cp -a -f "$DIRNAME/../lib/jpeglib/"* "$DIRNAME/obj/jpeglib"

    cd "$DIRNAME/obj/jpeglib"
    cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
            -DHOST=$HOST &&
    make $@
    check_error
    touch "$DIRNAME/obj/jpeglib.stamp"
fi

# Libogg
if [ ! -f "$DIRNAME/obj/libogg.stamp" ]; then
    echo "Compiling libogg"
    mkdir -p "$DIRNAME/obj/libogg"
    cp -a -f "$DIRNAME/../lib/libogg/"* "$DIRNAME/obj/libogg"

    cd "$DIRNAME/obj/libogg"
    ./configure --host=$HOST &&
    make $@
    check_error
    touch "$DIRNAME/obj/libogg.stamp"
fi

# Libvorbis
if [ ! -f "$DIRNAME/obj/libvorbis.stamp" ]; then
    echo "Compiling libvorbis"
    mkdir -p "$DIRNAME/obj/libvorbis"
    cp -a -f "$DIRNAME/../lib/libvorbis/"* "$DIRNAME/obj/libvorbis"

    cd "$DIRNAME/obj/libvorbis"
    CPPFLAGS="-I$DIRNAME/obj/libogg/include $CPPFLAGS" \
    LDFLAGS="-L$DIRNAME/obj/libogg/src/.libs $LDFLAGS" \
    ./configure --host=$HOST &&
    make $@
    check_error
    touch "$DIRNAME/obj/libvorbis.stamp"
fi

# STK
echo "Compiling STK"
cd "$DIRNAME"
${NDK_PATH}/ndk-build $@                 \
    APP_BUILD_SCRIPT="$NDK_BUILD_SCRIPT" \
    APP_ABI="$NDK_ABI"                   \
    APP_PLATFORM="$NDK_PLATFORM"         \
    APP_CPPFLAGS="$NDK_CPPFLAGS"         \
    APP_STL=gnustl_static                \
    NDK_DEBUG=$IS_DEBUG_BUILD

check_error

# Build apk
echo "Building APK"

sed -i "s/minSdkVersion=\".*\"/minSdkVersion=\"$SDK_VERSION\"/g" \
       "$DIRNAME/AndroidManifest.xml"


if [ "$BUILD_TOOL" = "gradle" ]; then
    export ANDROID_HOME="$SDK_PATH"
    gradle -Psdk_version=$SDK_VERSION           \
           -Pbuild_tools_ver="$BUILD_TOOLS_VER" \
           $GRADLE_BUILD_TYPE
elif [ "$BUILD_TOOL" = "ant" ]; then
    ant -Dsdk.dir="$SDK_PATH"  \
        -Dtarget=$NDK_PLATFORM \
        $ANT_BUILD_TYPE
fi

check_error
