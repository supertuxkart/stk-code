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

#export NDK_CCACHE=ccache
export NDK_CPPFLAGS="-O3 -g"

export NDK_ABI_ARMV7=armeabi-v7a
export ARCH_ARMV7=arm
export HOST_ARMV7=arm-linux-androideabi
export NDK_PLATFORM_ARMV7=android-16
export MIN_SDK_VERSION_ARMV7=16
export TARGET_SDK_VERSION_ARMV7=29
export COMPILE_SDK_VERSION_ARMV7=29

export NDK_ABI_AARCH64=arm64-v8a
export ARCH_AARCH64=arm64
export HOST_AARCH64=aarch64-linux-android
export NDK_PLATFORM_AARCH64=android-21
export MIN_SDK_VERSION_AARCH64=21
export TARGET_SDK_VERSION_AARCH64=29
export COMPILE_SDK_VERSION_AARCH64=29

export NDK_ABI_X86=x86
export ARCH_X86=x86
export HOST_X86=i686-linux-android
export NDK_PLATFORM_X86=android-16
export MIN_SDK_VERSION_X86=16
export TARGET_SDK_VERSION_X86=29
export COMPILE_SDK_VERSION_X86=29

export NDK_ABI_X86_64=x86_64
export ARCH_X86_64=x86_64
export HOST_X86_64=x86_64-linux-android
export NDK_PLATFORM_X86_64=android-21
export MIN_SDK_VERSION_X86_64=21
export TARGET_SDK_VERSION_X86_64=29
export COMPILE_SDK_VERSION_X86_64=29

export APP_NAME_RELEASE="SuperTuxKart"
export PACKAGE_NAME_RELEASE="org.supertuxkart.stk"
export PACKAGE_CALLBACK_NAME_RELEASE="org_supertuxkart_stk"
export APP_DIR_NAME_RELEASE="supertuxkart"
export APP_ICON_RELEASE="$DIRNAME/icon.png"
export APP_ICON_ADAPTIVE_BG_RELEASE="$DIRNAME/icon_adaptive_bg.png"
export APP_ICON_ADAPTIVE_FG_RELEASE="$DIRNAME/icon_adaptive_fg.png"

export APP_NAME_BETA="SuperTuxKart Beta"
export PACKAGE_NAME_BETA="org.supertuxkart.stk_beta"
export PACKAGE_CALLBACK_NAME_BETA="org_supertuxkart_stk_1beta"
export APP_DIR_NAME_BETA="supertuxkart-beta"
export APP_ICON_BETA="$DIRNAME/icon-dbg.png"
export APP_ICON_ADAPTIVE_BG_BETA="$DIRNAME/icon_adaptive_bg-dbg.png"
export APP_ICON_ADAPTIVE_FG_BETA="$DIRNAME/icon_adaptive_fg-dbg.png"

export APP_NAME_DEBUG="SuperTuxKart Debug"
export PACKAGE_NAME_DEBUG="org.supertuxkart.stk_dbg"
export PACKAGE_CALLBACK_NAME_DEBUG="org_supertuxkart_stk_1dbg"
export APP_DIR_NAME_DEBUG="supertuxkart-dbg"
export APP_ICON_DEBUG="$DIRNAME/icon-dbg.png"
export APP_ICON_ADAPTIVE_BG_DEBUG="$DIRNAME/icon_adaptive_bg-dbg.png"
export APP_ICON_ADAPTIVE_FG_DEBUG="$DIRNAME/icon_adaptive_fg-dbg.png"


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
    rm -rf "$DIRNAME/bin"
    rm -rf "$DIRNAME/build"
    rm -rf "$DIRNAME/libs"
    rm -rf "$DIRNAME/obj"
    rm -rf "$DIRNAME/res"
    rm -rf "$DIRNAME/.gradle"
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
    export MIN_SDK_VERSION=$MIN_SDK_VERSION_ARMV7
    export TARGET_SDK_VERSION=$TARGET_SDK_VERSION_ARMV7
    export COMPILE_SDK_VERSION=$COMPILE_SDK_VERSION_ARMV7
elif [ "$COMPILE_ARCH" = "aarch64" ]; then
    export NDK_PLATFORM=$NDK_PLATFORM_AARCH64
    export NDK_ABI=$NDK_ABI_AARCH64
    export ARCH=$ARCH_AARCH64
    export HOST=$HOST_AARCH64
    export MIN_SDK_VERSION=$MIN_SDK_VERSION_AARCH64
    export TARGET_SDK_VERSION=$TARGET_SDK_VERSION_AARCH64
    export COMPILE_SDK_VERSION=$COMPILE_SDK_VERSION_AARCH64
elif [ "$COMPILE_ARCH" = "x86" ]; then
    export NDK_PLATFORM=$NDK_PLATFORM_X86
    export NDK_ABI=$NDK_ABI_X86
    export ARCH=$ARCH_X86
    export HOST=$HOST_X86
    export MIN_SDK_VERSION=$MIN_SDK_VERSION_X86
    export TARGET_SDK_VERSION=$TARGET_SDK_VERSION_X86
    export COMPILE_SDK_VERSION=$COMPILE_SDK_VERSION_X86
elif [ "$COMPILE_ARCH" = "x86_64" ]; then
    export NDK_PLATFORM=$NDK_PLATFORM_X86_64
    export NDK_ABI=$NDK_ABI_X86_64
    export ARCH=$ARCH_X86_64
    export HOST=$HOST_X86_64
    export MIN_SDK_VERSION=$MIN_SDK_VERSION_X86_64
    export TARGET_SDK_VERSION=$TARGET_SDK_VERSION_X86_64
    export COMPILE_SDK_VERSION=$COMPILE_SDK_VERSION_X86_64
else
    echo "Unknow COMPILE_ARCH: $COMPILE_ARCH. Possible values are: " \
         "armv7, aarch64, x86, x86_64"
    exit
fi

# Update variables for selected build type
if [ -z "$BUILD_TYPE" ]; then
    BUILD_TYPE="debug"
fi

if [ "$BUILD_TYPE" = "debug" ] || [ "$BUILD_TYPE" = "Debug" ]; then
    export GRADLE_BUILD_TYPE="assembleDebug"
    export IS_DEBUG_BUILD=1
    export APP_NAME="$APP_NAME_DEBUG"
    export PACKAGE_NAME="$PACKAGE_NAME_DEBUG"
    export PACKAGE_CALLBACK_NAME="$PACKAGE_CALLBACK_NAME_DEBUG"
    export APP_DIR_NAME="$APP_DIR_NAME_DEBUG"
    export APP_ICON="$APP_ICON_DEBUG"
    export APP_ICON_ADAPTIVE_BG="$APP_ICON_ADAPTIVE_BG_DEBUG"
    export APP_ICON_ADAPTIVE_FG="$APP_ICON_ADAPTIVE_FG_DEBUG"
elif [ "$BUILD_TYPE" = "release" ] || [ "$BUILD_TYPE" = "Release" ]; then
    export GRADLE_BUILD_TYPE="assembleRelease"
    export IS_DEBUG_BUILD=0
    export APP_NAME="$APP_NAME_RELEASE"
    export PACKAGE_NAME="$PACKAGE_NAME_RELEASE"
    export PACKAGE_CALLBACK_NAME="$PACKAGE_CALLBACK_NAME_RELEASE"
    export APP_DIR_NAME="$APP_DIR_NAME_RELEASE"
    export APP_ICON="$APP_ICON_RELEASE"
    export APP_ICON_ADAPTIVE_BG="$APP_ICON_ADAPTIVE_BG_RELEASE"
    export APP_ICON_ADAPTIVE_FG="$APP_ICON_ADAPTIVE_FG_RELEASE"
elif [ "$BUILD_TYPE" = "beta" ] || [ "$BUILD_TYPE" = "Beta" ]; then
    export GRADLE_BUILD_TYPE="assembleRelease"
    export IS_DEBUG_BUILD=0
    export APP_NAME="$APP_NAME_BETA"
    export PACKAGE_NAME="$PACKAGE_NAME_BETA"
    export PACKAGE_CALLBACK_NAME="$PACKAGE_CALLBACK_NAME_BETA"
    export APP_DIR_NAME="$APP_DIR_NAME_BETA"
    export APP_ICON="$APP_ICON_BETA"
    export APP_ICON_ADAPTIVE_BG="$APP_ICON_ADAPTIVE_BG_BETA"
    export APP_ICON_ADAPTIVE_FG="$APP_ICON_ADAPTIVE_FG_BETA"
else
    echo "Unsupported BUILD_TYPE: $BUILD_TYPE. Possible values are: " \
         "debug, release"
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

# Check if we have key for signing in release build
if [ "$GRADLE_BUILD_TYPE" = "assembleRelease" ]; then
    if [ -z "$STK_KEYSTORE" ]; then
        echo "Error: STK_KEYSTORE variable is empty."
        exit
    fi

    if [ ! -f "$STK_KEYSTORE" ]; then
        echo "Error: Couldn't find $STK_KEYSTORE file."
        exit
    fi

    if [ -z "$STK_STOREPASS" ]; then
        echo "Error: STK_STOREPASS variable is empty"
        exit
    fi

    if [ -z "$STK_ALIAS" ]; then
        echo "Error: STK_ALIAS variable is empty."
        exit
    fi
else
    STK_KEYSTORE="empty"
    STK_STOREPASS="empty"
    STK_ALIAS="empty"
fi

# Find newest build-tools version
if [ -z "$BUILD_TOOLS_VER" ]; then
    BUILD_TOOLS_DIRS=`ls -1 "$SDK_PATH/build-tools" | sort -V -r`
   
    for DIR in $BUILD_TOOLS_DIRS; do
        if [ "$DIR" = "`echo $DIR | sed 's/[^0-9,.]//g'`" ]; then
            BUILD_TOOLS_VER="$DIR"
            break
        fi
    done
fi

if [ -z "$BUILD_TOOLS_VER" ] || [ ! -d "$SDK_PATH/build-tools/$BUILD_TOOLS_VER" ]; then
    echo "Error: Couldn't detect build-tools version."
    exit
fi

# Set project version and code
if [ -f "$DIRNAME/obj/project_version" ]; then
    PROJECT_VERSION_PREV=$(cat "$DIRNAME/obj/project_version") 
    
    if [ -z "$PROJECT_VERSION" ]; then
        export PROJECT_VERSION="$PROJECT_VERSION_PREV"
    elif [ "$PROJECT_VERSION" != "$PROJECT_VERSION_PREV" ]; then
        echo "Different project version has been set. Forcing recompilation..."
        touch -c "$DIRNAME/Android.mk"
    fi
fi

if [ -z "$PROJECT_VERSION" ]; then
    if [ $IS_DEBUG_BUILD -ne 0 ]; then
        export PROJECT_VERSION="git"
    else
        echo "Error: Variable PROJECT_VERSION is not set. It must have unique" \
             "value for release build."
        exit
    fi
fi

if [ -z "$PROJECT_CODE" ]; then
    if [ $IS_DEBUG_BUILD -ne 0 ]; then
        PROJECT_CODE="1"
    else
        echo "Error: Variable PROJECT_CODE is not set."
        exit
    fi
fi

if [ -d "$DIRNAME/assets/data" ]; then
    if [ ! -f "$DIRNAME/assets/data/supertuxkart.$PROJECT_VERSION" ]; then
        echo "Error: supertuxkart.$PROJECT_VERSION doesn't exist in" \
             "assets/data directory."
        exit
    fi
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

# Last ndk supporting android 14 is 17c which has gcc-4.9, and most
# dependencies require clang to build
if [ -f "$DIRNAME/obj/bin/$HOST-gcc-4.9" ]; then
    cp "$DIRNAME/obj/bin/clang" "$DIRNAME/obj/bin/$HOST-gcc"
    cp "$DIRNAME/obj/bin/clang++" "$DIRNAME/obj/bin/$HOST-g++"
fi

echo "$PROJECT_VERSION" > "$DIRNAME/obj/project_version"

# Zlib
if [ ! -f "$DIRNAME/obj/zlib.stamp" ]; then
    echo "Compiling zlib"
    mkdir -p "$DIRNAME/obj/zlib"
    cp -a -f "$DIRNAME/../lib/zlib/"* "$DIRNAME/obj/zlib"

    cd "$DIRNAME/obj/zlib"
    cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
            -DHOST=$HOST -DARCH=$ARCH -DCMAKE_C_FLAGS="-fpic" &&
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
    MLIBRARY="$DIRNAME/obj/sysroot/usr/lib/$HOST/libm.a"
    # For ndk 17c
    if [ ! -f "$MLIBRARY" ]; then
        MLIBRARY="$DIRNAME/obj/sysroot/usr/lib/libm.a"
    fi
    cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
            -DHOST=$HOST -DARCH=$ARCH                                     \
            -DZLIB_LIBRARY="$DIRNAME/obj/zlib/libz.a"                     \
            -DZLIB_INCLUDE_DIR="$DIRNAME/obj/zlib/"                       \
            -DM_LIBRARY="$MLIBRARY"                                       \
            -DPNG_TESTS=0 -DCMAKE_C_FLAGS="-fpic" &&
    make $@
    check_error
    touch "$DIRNAME/obj/libpng.stamp"
fi

# Freetype bootstrap
if [ ! -f "$DIRNAME/obj/freetype_bootstrap.stamp" ]; then
    echo "Compiling freetype bootstrap"
    mkdir -p "$DIRNAME/obj/freetype/build"
    cp -a -f "$DIRNAME/../lib/freetype/"* "$DIRNAME/obj/freetype"
    cd "$DIRNAME/obj/freetype/build"
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
             -DHOST=$HOST -DARCH=$ARCH                                     \
             -DZLIB_LIBRARY="$DIRNAME/obj/zlib/libz.a"                     \
             -DZLIB_INCLUDE_DIR="$DIRNAME/obj/zlib/"                       \
             -DPNG_LIBRARY="$DIRNAME/obj/libpng/libpng.a"                  \
             -DPNG_PNG_INCLUDE_DIR="$DIRNAME/obj/libpng/"                  \
             -DFT_WITH_HARFBUZZ=OFF -DFT_WITH_BZIP2=OFF                    \
             -DFT_WITH_BROTLI=OFF -DFT_WITH_ZLIB=ON -DFT_WITH_PNG=ON       \
             -DCMAKE_C_FLAGS="-fpic" &&
    make $@
    check_error
    # We need to rebuild freetype after harfbuzz is compiled
    touch "$DIRNAME/obj/freetype_bootstrap.stamp"
fi

# Harfbuzz
if [ ! -f "$DIRNAME/obj/harfbuzz.stamp" ]; then
    echo "Compiling harfbuzz"
    mkdir -p "$DIRNAME/obj/harfbuzz/build"
    cp -a -f "$DIRNAME/../lib/harfbuzz/"* "$DIRNAME/obj/harfbuzz"

    cd "$DIRNAME/obj/harfbuzz/build"
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake    \
             -DHOST=$HOST -DARCH=$ARCH -DBUILD_SHARED_LIBS=OFF                \
             -DFREETYPE_LIBRARY="$DIRNAME/obj/freetype/build/libfreetype.a $DIRNAME/obj/libpng/libpng.a $DIRNAME/obj/zlib/libz.a" \
             -DFREETYPE_INCLUDE_DIRS="$DIRNAME/obj/freetype/include/"         \
             -DHB_HAVE_GLIB=OFF -DHB_HAVE_GOBJECT=OFF -DHB_HAVE_ICU=OFF       \
             -DHB_HAVE_FREETYPE=ON -DHB_BUILD_SUBSET=OFF                      \
             -DCMAKE_C_FLAGS="-fpic" -DCMAKE_CXX_FLAGS="-std=gnu++0x -fpic" &&
    make $@
    check_error
    mkdir -p "$DIRNAME/obj/harfbuzz/include/harfbuzz"
    cp $DIRNAME/obj/harfbuzz/src/*.h "$DIRNAME/obj/harfbuzz/include/harfbuzz"
    touch "$DIRNAME/obj/harfbuzz.stamp"
fi

# Freetype
if [ ! -f "$DIRNAME/obj/freetype.stamp" ]; then
    echo "Compiling freetype"
    cd "$DIRNAME/obj/freetype/build"
    rm -rf *
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake    \
             -DHOST=$HOST -DARCH=$ARCH                                        \
             -DZLIB_LIBRARY="$DIRNAME/obj/zlib/libz.a"                        \
             -DZLIB_INCLUDE_DIR="$DIRNAME/obj/zlib/"                          \
             -DPNG_LIBRARY="$DIRNAME/obj/libpng/libpng.a"                     \
             -DPNG_PNG_INCLUDE_DIR="$DIRNAME/obj/libpng/"                     \
             -DHarfBuzz_LIBRARY="$DIRNAME/obj/harfbuzz/build/libharfbuzz.a"   \
             -DHarfBuzz_INCLUDE_DIR="$DIRNAME/obj/harfbuzz/src/"              \
             -DFT_WITH_HARFBUZZ=ON -DFT_WITH_BZIP2=OFF                        \
             -DFT_WITH_BROTLI=OFF -DFT_WITH_ZLIB=ON -DFT_WITH_PNG=ON          \
             -DCMAKE_C_FLAGS="-fpic" &&
    make $@
    check_error
    touch "$DIRNAME/obj/freetype.stamp"
fi

# Openal
if [ ! -f "$DIRNAME/obj/openal.stamp" ]; then
    echo "Compiling openal"
    mkdir -p "$DIRNAME/obj/openal"
    cp -a -f "$DIRNAME/../lib/openal/"* "$DIRNAME/obj/openal"

    cd "$DIRNAME/obj/openal"
    cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake   \
            -DHOST=$HOST -DARCH=$ARCH  -DALSOFT_UTILS=0 -DALSOFT_EXAMPLES=0 \
            -DLIBTYPE=STATIC -DOPENSL_LIBRARY="-lOpenSLES"                  \
            -DOPENSL_INCLUDE_DIR="$DIRNAME/obj/sysroot/usr/include/SLES/"   \
            -DOPENSL_ANDROID_INCLUDE_DIR="$DIRNAME/obj/sysroot/usr/include/SLES/" \
            -DCMAKE_C_FLAGS="-fpic" -DCMAKE_CXX_FLAGS="-fpic" &&
    make $@
    check_error
    touch "$DIRNAME/obj/openal.stamp"
fi

# MbedTLS
if [ ! -f "$DIRNAME/obj/mbedtls.stamp" ]; then
    echo "Compiling mbedtls"
    mkdir -p "$DIRNAME/obj/mbedtls"
    cp -a -f "$DIRNAME/../lib/mbedtls/"* "$DIRNAME/obj/mbedtls"

    cd "$DIRNAME/obj/mbedtls"
    cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
            -DHOST=$HOST -DARCH=$ARCH -DBUILD_SHARED_LIBS=OFF             \
            -DENABLE_TESTING=OFF -DENABLE_PROGRAMS=OFF                    \
            -DCMAKE_C_FLAGS="-fpic" &&
    make $@
    check_error
    touch "$DIRNAME/obj/mbedtls.stamp"
fi

# Curl
if [ ! -f "$DIRNAME/obj/curl.stamp" ]; then
    echo "Compiling curl"
    mkdir -p "$DIRNAME/obj/curl"
    cp -a -f "$DIRNAME/../lib/curl/"* "$DIRNAME/obj/curl"

    cd "$DIRNAME/obj/curl"
    cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake       \
            -DHOST=$HOST -DARCH=$ARCH -DBUILD_SHARED_LIBS=OFF                   \
            -DZLIB_LIBRARY="$DIRNAME/obj/zlib/libz.a"                           \
            -DZLIB_INCLUDE_DIR="$DIRNAME/obj/zlib/"                             \
            -DMBEDCRYPTO_LIBRARY="$DIRNAME/obj/mbedtls/library/libmbedcrypto.a" \
            -DMBEDTLS_LIBRARY="$DIRNAME/obj/mbedtls/library/libmbedtls.a"       \
            -DMBEDX509_LIBRARY="$DIRNAME/obj/mbedtls/library/libmbedx509.a"     \
            -DMBEDTLS_INCLUDE_DIRS="$DIRNAME/obj/mbedtls/include/"              \
            -DBUILD_TESTING=OFF -DBUILD_CURL_EXE=OFF                            \
            -DCMAKE_USE_MBEDTLS=ON -DUSE_ZLIB=ON -DCMAKE_USE_OPENSSL=OFF        \
            -DCMAKE_USE_LIBSSH=OFF -DCMAKE_USE_LIBSSH2=OFF                      \
            -DCMAKE_USE_GSSAPI=OFF -DUSE_NGHTTP2=OFF -DUSE_QUICHE=OFF           \
            -DHTTP_ONLY=ON -DCURL_CA_BUNDLE=none -DCURL_CA_PATH=none            \
            -DENABLE_THREADED_RESOLVER=ON &&
    make $@
    check_error
    touch "$DIRNAME/obj/curl.stamp"
fi

# Libjpeg
if [ ! -f "$DIRNAME/obj/libjpeg.stamp" ]; then
    echo "Compiling libjpeg"
    mkdir -p "$DIRNAME/obj/libjpeg"
    cp -a -f "$DIRNAME/../lib/libjpeg/"* "$DIRNAME/obj/libjpeg"

    cd "$DIRNAME/obj/libjpeg"
    cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
            -DHOST=$HOST -DARCH=$ARCH -DCMAKE_C_FLAGS="-fpic" &&
    make $@
    check_error
    touch "$DIRNAME/obj/libjpeg.stamp"
fi

# Libogg
if [ ! -f "$DIRNAME/obj/libogg.stamp" ]; then
    echo "Compiling libogg"
    mkdir -p "$DIRNAME/obj/libogg"
    cp -a -f "$DIRNAME/../lib/libogg/"* "$DIRNAME/obj/libogg"

    cd "$DIRNAME/obj/libogg"
    cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
            -DHOST=$HOST -DARCH=$ARCH -DCMAKE_C_FLAGS="-fpic" &&
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
    cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
            -DHOST=$HOST -DARCH=$ARCH -DCMAKE_C_FLAGS="-fpic"             \
            -DOGG_LIBRARY="$DIRNAME/obj/libogg/libogg.a"                  \
            -DOGG_INCLUDE_DIR="$DIRNAME/obj/libogg/include" &&
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
    APP_STL=c++_static                   \
    NDK_DEBUG=$IS_DEBUG_BUILD

check_error

# Build apk
echo "Building APK"

mkdir -p "$DIRNAME/res/drawable/"
mkdir -p "$DIRNAME/res/drawable-anydpi-v26/"
mkdir -p "$DIRNAME/res/drawable-mdpi/"
mkdir -p "$DIRNAME/res/drawable-hdpi/"
mkdir -p "$DIRNAME/res/drawable-xhdpi/"
mkdir -p "$DIRNAME/res/drawable-xxhdpi/"
mkdir -p "$DIRNAME/res/drawable-xxxhdpi/"
rm -rf "$DIRNAME/res/values*"
mkdir -p "$DIRNAME/res/values/"

STYLES_FILE="$DIRNAME/res/values/styles.xml"

echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>"       >  "$STYLES_FILE"
echo "<!--Generated by make.sh-->"                      >>  "$STYLES_FILE"
echo "<resources>"                                      >> "$STYLES_FILE"
echo "    <style name=\"Theme.STKSplashScreen\" parent=\"android:style/Theme.Holo\">" >> "$STYLES_FILE"
echo "         <item name=\"android:windowBackground\">#A8A8A8</item>" >> "$STYLES_FILE"
echo "         <item name=\"android:windowFullscreen\">true</item>" >> "$STYLES_FILE"
echo "         <item name=\"android:windowNoTitle\">true</item>" >> "$STYLES_FILE"
echo "         <item name=\"android:windowLayoutInDisplayCutoutMode\">shortEdges</item>" >> "$STYLES_FILE"
echo "         <item name=\"android:windowContentOverlay\">@null</item>" >> "$STYLES_FILE"
echo "    </style>"                                     >> "$STYLES_FILE"
echo "</resources>"                                     >> "$STYLES_FILE"

STRINGS_FILE="$DIRNAME/res/values/strings.xml"

# Strings used in stk android ui (when extracting game data first time)
PO_EXTRACT_GAME_DATA="po_extract_game_data"
PO_EXTRACT_GAME_DATA_STR="Extracting game data..."
PO_EXTRACT_ERROR="po_extract_error"
PO_EXTRACT_ERROR_STR="Game data extraction error"
PO_EXTRACT_ERROR_MSG="po_extract_error_msg"
PO_EXTRACT_ERROR_MSG_STR="Check remaining device space or reinstall SuperTuxKart."
PO_QUIT="po_quit"
PO_QUIT_STR="Quit"

echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>"       >  "$STRINGS_FILE"
echo "<!--Generated by make.sh-->"                      >>  "$STRINGS_FILE"
echo "<resources>"                                      >> "$STRINGS_FILE"
echo "    <string name=\"app_name\">$APP_NAME</string>" >> "$STRINGS_FILE"
echo "    <string name=\"$PO_EXTRACT_GAME_DATA\">$PO_EXTRACT_GAME_DATA_STR</string>" >> "$STRINGS_FILE"
echo "    <string name=\"$PO_EXTRACT_ERROR\">$PO_EXTRACT_ERROR_STR</string>" >> "$STRINGS_FILE"
echo "    <string name=\"$PO_EXTRACT_ERROR_MSG\">$PO_EXTRACT_ERROR_MSG_STR</string>" >> "$STRINGS_FILE"
echo "    <string name=\"$PO_QUIT\">$PO_QUIT_STR</string>" >> "$STRINGS_FILE"
echo "</resources>"                                     >> "$STRINGS_FILE"

translate_str()
{
    echo $(grep -A 1 -e "msgid \"$1\"" "$2" \
        | sed -n 's/msgstr "\(.*\)"/\1/p' | sed "s/'/\\\'/g")
}

create_translation()
{
    PO="$1"
    CUR_LANG=$(basename -- "$PO" | cut -f 1 -d '.')
    # Skip english po file
    if [ "$CUR_LANG" = "en" ]; then
        continue
    fi
    # Fix some difference in language code
    if [ "$CUR_LANG" = "he" ]; then
        # Hebrew
        CUR_LANG="iw"
    fi
    if [ "$CUR_LANG" = "id" ]; then
        # Indonesian
        CUR_LANG="in"
    fi
    if [ "$CUR_LANG" = "yi" ]; then
        # Yiddish
        CUR_LANG="ji"
    fi
    CUR_LANG=$(echo "$CUR_LANG" | sed 's/_/-r/g')
    EXTRACT_GAME_DATA_STR=$(translate_str "$PO_EXTRACT_GAME_DATA_STR" "$PO")
    EXTRACT_ERROR_STR=$(translate_str "$PO_EXTRACT_ERROR_STR" "$PO")
    EXTRACT_ERROR_MSG_STR=$(translate_str "$PO_EXTRACT_ERROR_MSG_STR" "$PO")
    QUIT_STR=$(translate_str "$PO_QUIT_STR" "$PO")
    if [ -n "$EXTRACT_GAME_DATA_STR" ] \
    || [ -n "$EXTRACT_ERROR_STR" ] \
    || [ -n "$EXTRACT_ERROR_MSG_STR" ] \
    || [ -n "$QUIT_STR" ]; then
        mkdir -p "$DIRNAME/res/values-$CUR_LANG"
        TRANSLATION="$DIRNAME/res/values-$CUR_LANG/strings.xml"
        echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>"       >  "$TRANSLATION"
        echo "<!--Generated by make.sh-->"                      >>  "$TRANSLATION"
        echo "<resources>"                                      >> "$TRANSLATION"
        if [ -n "$EXTRACT_GAME_DATA_STR" ] ; then
            echo "    <string name=\"$PO_EXTRACT_GAME_DATA\">$EXTRACT_GAME_DATA_STR</string>" >> "$TRANSLATION"
        fi
        if [ -n "$EXTRACT_ERROR_STR" ] ; then
            echo "    <string name=\"$PO_EXTRACT_ERROR\">$EXTRACT_ERROR_STR</string>" >> "$TRANSLATION"
        fi
        if [ -n "$EXTRACT_ERROR_MSG_STR" ] ; then
            echo "    <string name=\"$PO_EXTRACT_ERROR_MSG\">$EXTRACT_ERROR_MSG_STR</string>" >> "$TRANSLATION"
        fi
        if [ -n "$QUIT_STR" ] ; then
            echo "    <string name=\"$PO_QUIT\">$QUIT_STR</string>" >> "$TRANSLATION"
        fi
        echo "</resources>"                                     >> "$TRANSLATION"
    fi
}

find "$DIRNAME/assets/data/po" -type f -name '*.po' | while read f; do create_translation "$f"; done

ADAPTIVE_ICON_FILE="$DIRNAME/res/drawable-anydpi-v26/icon.xml"

echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>"                      >  "$ADAPTIVE_ICON_FILE"
echo "<adaptive-icon"                                                  >> "$ADAPTIVE_ICON_FILE"
echo "  xmlns:android=\"http://schemas.android.com/apk/res/android\">" >> "$ADAPTIVE_ICON_FILE"
echo "    <background android:drawable=\"@drawable/icon_bg\" />"       >> "$ADAPTIVE_ICON_FILE"
echo "    <foreground android:drawable=\"@drawable/icon_fg\" />"       >> "$ADAPTIVE_ICON_FILE"
echo "</adaptive-icon>"                                                >> "$ADAPTIVE_ICON_FILE"

sed -i "s/minSdkVersion=\".*\"/minSdkVersion=\"$MIN_SDK_VERSION\"/g" \
       "$DIRNAME/AndroidManifest.xml"
       
sed -i "s/targetSdkVersion=\".*\"/targetSdkVersion=\"$TARGET_SDK_VERSION\"/g" \
       "$DIRNAME/AndroidManifest.xml"
       
sed -i "s/package=\".*\"/package=\"$PACKAGE_NAME\"/g" \
       "$DIRNAME/AndroidManifest.xml"

sed -i "s/package org.supertuxkart.*/package $PACKAGE_NAME;/g" \
       "$DIRNAME/src/main/java/STKEditText.java"

sed -i "s/import org.supertuxkart.*/import $PACKAGE_NAME.STKInputConnection;/g" \
       "$DIRNAME/src/main/java/STKEditText.java"

sed -i "s/package org.supertuxkart.*/package $PACKAGE_NAME;/g" \
       "$DIRNAME/src/main/java/STKInputConnection.java"

sed -i "s/import org.supertuxkart.*.STKEditText;/import $PACKAGE_NAME.STKEditText;/g" \
       "$DIRNAME/src/main/java/STKInputConnection.java"

sed -i "s/package org.supertuxkart.*/package $PACKAGE_NAME;/g" \
       "$DIRNAME/src/main/java/SuperTuxKartActivity.java"

sed -i "s/import org.supertuxkart.*/import $PACKAGE_NAME.STKEditText;/g" \
       "$DIRNAME/src/main/java/SuperTuxKartActivity.java"

sed -i "s/versionName=\".*\"/versionName=\"$PROJECT_VERSION\"/g" \
       "$DIRNAME/AndroidManifest.xml"
       
sed -i "s/versionCode=\".*\"/versionCode=\"$PROJECT_CODE\"/g" \
       "$DIRNAME/AndroidManifest.xml"

cp -f "$DIRNAME/../lib/sdl2/android-project/app/src/main/java/org/libsdl/app/HIDDevice.java" \
       "$DIRNAME/src/main/java/"
cp -f "$DIRNAME/../lib/sdl2/android-project/app/src/main/java/org/libsdl/app/HIDDeviceBLESteamController.java" \
       "$DIRNAME/src/main/java/"
cp -f "$DIRNAME/../lib/sdl2/android-project/app/src/main/java/org/libsdl/app/HIDDeviceManager.java" \
       "$DIRNAME/src/main/java/"
cp -f "$DIRNAME/../lib/sdl2/android-project/app/src/main/java/org/libsdl/app/HIDDeviceUSB.java" \
       "$DIRNAME/src/main/java/"
cp -f "$DIRNAME/../lib/sdl2/android-project/app/src/main/java/org/libsdl/app/SDLActivity.java" \
       "$DIRNAME/src/main/java/"
cp -f "$DIRNAME/../lib/sdl2/android-project/app/src/main/java/org/libsdl/app/SDLAudioManager.java" \
       "$DIRNAME/src/main/java/"
cp -f "$DIRNAME/../lib/sdl2/android-project/app/src/main/java/org/libsdl/app/SDLControllerManager.java" \
       "$DIRNAME/src/main/java/"
cp -f "$DIRNAME/../lib/sdl2/android-project/app/src/main/java/org/libsdl/app/SDL.java" \
       "$DIRNAME/src/main/java/"

cp "banner.png" "$DIRNAME/res/drawable/banner.png"
cp "$APP_ICON" "$DIRNAME/res/drawable/icon.png"
convert -scale 48x48 "$APP_ICON" "$DIRNAME/res/drawable-mdpi/icon.png"
convert -scale 72x72 "$APP_ICON" "$DIRNAME/res/drawable-hdpi/icon.png"
convert -scale 96x96 "$APP_ICON" "$DIRNAME/res/drawable-xhdpi/icon.png"
convert -scale 144x144 "$APP_ICON" "$DIRNAME/res/drawable-xxhdpi/icon.png"
convert -scale 192x192 "$APP_ICON" "$DIRNAME/res/drawable-xxxhdpi/icon.png"

#convert -scale 108x108 "$APP_ICON_ADAPTIVE_BG" "$DIRNAME/res/drawable-mdpi/icon_bg.png"
#convert -scale 162x162 "$APP_ICON_ADAPTIVE_BG" "$DIRNAME/res/drawable-hdpi/icon_bg.png"
#convert -scale 216x216 "$APP_ICON_ADAPTIVE_BG" "$DIRNAME/res/drawable-xhdpi/icon_bg.png"
#convert -scale 324x324 "$APP_ICON_ADAPTIVE_BG" "$DIRNAME/res/drawable-xxhdpi/icon_bg.png"
#convert -scale 432x432 "$APP_ICON_ADAPTIVE_BG" "$DIRNAME/res/drawable-xxxhdpi/icon_bg.png"

convert -scale 108x108 xc:"rgba(255,255,255,255)" "$DIRNAME/res/drawable-mdpi/icon_bg.png"
convert -scale 162x162 xc:"rgba(255,255,255,255)" "$DIRNAME/res/drawable-hdpi/icon_bg.png"
convert -scale 216x216 xc:"rgba(255,255,255,255)" "$DIRNAME/res/drawable-xhdpi/icon_bg.png"
convert -scale 324x324 xc:"rgba(255,255,255,255)" "$DIRNAME/res/drawable-xxhdpi/icon_bg.png"
convert -scale 432x432 xc:"rgba(255,255,255,255)" "$DIRNAME/res/drawable-xxxhdpi/icon_bg.png"

convert -scale 108x108 "$APP_ICON_ADAPTIVE_FG" "$DIRNAME/res/drawable-mdpi/icon_fg.png"
convert -scale 162x162 "$APP_ICON_ADAPTIVE_FG" "$DIRNAME/res/drawable-hdpi/icon_fg.png"
convert -scale 216x216 "$APP_ICON_ADAPTIVE_FG" "$DIRNAME/res/drawable-xhdpi/icon_fg.png"
convert -scale 324x324 "$APP_ICON_ADAPTIVE_FG" "$DIRNAME/res/drawable-xxhdpi/icon_fg.png"
convert -scale 432x432 "$APP_ICON_ADAPTIVE_FG" "$DIRNAME/res/drawable-xxxhdpi/icon_fg.png"

if [ -f "/usr/lib/jvm/java-8-openjdk-amd64/bin/java" ]; then
    export JAVA_HOME="/usr/lib/jvm/java-8-openjdk-amd64"
    export PATH=$JAVA_HOME/bin:$PATH
fi

export ANDROID_HOME="$SDK_PATH"
./gradlew -Pcompile_sdk_version=$COMPILE_SDK_VERSION \
          -Pbuild_tools_ver="$BUILD_TOOLS_VER"       \
          -Pstorepass="$STK_STOREPASS"               \
          -Pkeystore="$STK_KEYSTORE"                 \
          -Palias="$STK_ALIAS"                       \
          $GRADLE_BUILD_TYPE

if [ "$GRADLE_BUILD_TYPE" = "assembleRelease" ]; then
./gradlew -Pcompile_sdk_version=$COMPILE_SDK_VERSION \
          -Pbuild_tools_ver="$BUILD_TOOLS_VER"       \
          -Pstorepass="$STK_STOREPASS"               \
          -Pkeystore="$STK_KEYSTORE"                 \
          -Palias="$STK_ALIAS"                       \
          "bundleRelease"
fi

check_error
