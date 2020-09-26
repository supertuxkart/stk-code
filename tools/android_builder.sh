#!/bin/sh
#
# (C) 2018 Dawid Gan, under the GPLv3
#
# A script that builds Android APKs for all architectures
#
# The script assumes that you know what you are doing. It allows to generate all
# packages for Google Play Store with single command. If you just want to build 
# STK for your own use, then use android/make.sh script instead.

export BUILD_TYPE=Beta
export PROJECT_VERSION=git20200827
export PROJECT_CODE=192
export STK_STOREPASS="xxx"
export STK_KEYSTORE="/path/to/stk.keystore"
export STK_ALIAS="alias"


check_error()
{
    if [ $? -gt 0 ]; then
        echo "Error ocurred."
        exit
    fi
}

clean()
{
    echo "Clean everything"

    rm -rf ./android-armv7
    rm -rf ./android-aarch64
    rm -rf ./android-x86
    rm -rf ./android-x86_64
    rm -rf ./android/assets
    rm -rf ./android-output

    cd android
    ./make.sh clean
    cd -
}

init_directories()
{
    echo "Init directories"

    if [ ! -d "./android-armv7" ]; then
        echo "Creating android-armv7 directory"

        mkdir android-armv7
        cd android-armv7
    
        ln -s ../android/Android.mk
        ln -s ../android/AndroidManifest.xml
        ln -s ../android/banner.png
        ln -s ../android/build.gradle
        ln -s ../android/gradle
        ln -s ../android/gradlew
        ln -s ../android/icon.png
        ln -s ../android/icon-dbg.png
        ln -s ../android/icon_adaptive_fg.png
        ln -s ../android/icon_adaptive_fg-dbg.png
        ln -s ../android/make.sh
        ln -s ../android/android-ndk
        ln -s ../android/android-sdk
        ln -s ../android/assets
        ln -s ../android/src
    
        cd -
    fi

    if [ ! -d "./android-aarch64" ]; then
        echo "Creating android-aarch64 directory"
        cp -a ./android-armv7 ./android-aarch64
    fi

    if [ ! -d "./android-x86" ]; then
        echo "Creating android-x86 directory"
        cp -a ./android-armv7 ./android-x86
    fi

    if [ ! -d "./android-x86_64" ]; then
        echo "Creating android-x86_64 directory"
        cp -a ./android-armv7 ./android-x86_64
    fi

    if [ ! -d "./android-output" ]; then
        echo "Creating android-output directory"
        mkdir ./android-output
    fi
}

generate_assets()
{
    echo "Generate assets"

    if [ -d "./android/assets" ]; then
        echo "Assets already found in ./android/assets"
        return
    fi

    cd ./android
    DECREASE_QUALITY=0                              \
    CONVERT_TO_JPG=0                                \
    ASSETS_PATHS="../android-output/assets-lq/data" \
    ./generate_assets.sh

    if [ ! -f "./assets/files.txt" ]; then
        echo "Error: Couldn't generate assets"
        return
    fi

    if [ -f "./assets/data/supertuxkart.git" ]; then
        mv "./assets/data/supertuxkart.git" \
           "./assets/data/supertuxkart.$PROJECT_VERSION"
        sed -i "s/data\/supertuxkart.git/data\/supertuxkart.$PROJECT_VERSION/g" \
           "./assets/files.txt"
    fi

    cd -
}

generate_full_assets()
{
    echo "Generate zip file with full assets"

    if [ -f "./android-output/stk-assets.zip" ]; then
        echo "Full assets already found in ./android-output/stk-assets.zip"
        return
    fi

    cp -a ./android/generate_assets.sh ./android-output/

    cd ./android-output/

    ONLY_ASSETS=1           \
    TRACKS="all"            \
    TEXTURE_SIZE=512        \
    JPEG_QUALITY=95         \
    PNG_QUALITY=95          \
    PNGQUANT_QUALITY=95     \
    SOUND_QUALITY=112       \
    SOUND_MONO=0            \
    SOUND_SAMPLE=44100      \
    OUTPUT_PATH="assets-hq" \
    ./generate_assets.sh

    if [ ! -f "./assets-hq/files.txt" ]; then
        echo "Error: Couldn't generate full assets"
        return
    fi

    cd ./assets-hq/data
    zip -r ../../stk-assets.zip ./*
    cd ../../

    rm ./generate_assets.sh
    
    if [ ! -f "./stk-assets.zip" ]; then
        echo "Error: Couldn't generate full assets"
        return
    fi

    FULL_ASSETS_SIZE=`du -b ./stk-assets.zip | cut -f1`
    sed -i "s/stk_assets_size = .*\;/stk_assets_size = $FULL_ASSETS_SIZE\;/g" \
           "../src/utils/download_assets_size.hpp"
    
    cd ../
}

generate_lq_assets()
{
    echo "Generate zip file with lq assets"

    if [ -f "./android-output/stk-assets-lq.zip" ]; then
        echo "Full assets already found in ./android-output/stk-assets-lq..zip"
        return
    fi

    cp -a ./android/generate_assets.sh ./android-output/

    cd ./android-output/

    ONLY_ASSETS=1           \
    OUTPUT_PATH="assets-lq" \
    ./generate_assets.sh

    if [ ! -f "./assets-lq/files.txt" ]; then
        echo "Error: Couldn't generate lq assets"
        return
    fi

    cd ./assets-lq/data
    zip -r ../../stk-assets-lq.zip ./*
    cd ../../

    rm ./generate_assets.sh
    
    if [ ! -f "./stk-assets-lq.zip" ]; then
        echo "Error: Couldn't generate lq assets"
        return
    fi
    
    cd ../
}

build_package()
{
    export ARCH1=$1
    export ARCH2=$2

    echo "Build package for $ARCH1"

    if [ -f "./android-output/SuperTuxKart-$PROJECT_VERSION-$ARCH1.apk" ]; then
        echo "Package for architecture $ARCH1 is already built"
        #return
    fi

    export COMPILE_ARCH=$ARCH1

    cd ./android-$ARCH1
    ./make.sh -j $(($(nproc) + 1))
    cd -

    if [ ! -f ./android-$ARCH1/build/outputs/apk/release/android-$ARCH1-release.apk ]; then
        echo "Error: Couldn't build apk for architecture $ARCH1"
        return
    fi

    if [ ! -f ./android-$ARCH1/build/outputs/bundle/release/android-$ARCH1.aab ]; then
        echo "Error: Couldn't build app bundle for architecture $ARCH1"
        return
    fi

    cp ./android-$ARCH1/build/outputs/apk/release/android-$ARCH1-release.apk \
       ./android-output/SuperTuxKart-$PROJECT_VERSION-$ARCH1.apk

    cp ./android-$ARCH1/build/outputs/bundle/release/android-$ARCH1.aab \
       ./android-output/SuperTuxKart-$PROJECT_VERSION-$ARCH1.aab

    cp ./android-$ARCH1/obj/local/$ARCH2/libmain.so \
       ./android-output/SuperTuxKart-$PROJECT_VERSION-$ARCH1-libmain.so

}


# Handle clean command
if [ ! -z "$1" ] && [ "$1" = "clean" ]; then
    clean
    exit
fi

#Build packages
init_directories

generate_lq_assets
generate_full_assets
generate_assets

if [ -z "$1" ] || [ "$1" = "armv7" ]; then
    build_package armv7 armeabi-v7a
fi

PROJECT_CODE=$(($PROJECT_CODE + 1))

if [ -z "$1" ] || [ "$1" = "aarch64" ]; then
    build_package aarch64 arm64-v8a
fi

PROJECT_CODE=$(($PROJECT_CODE + 1))

if [ -z "$1" ] || [ "$1" = "x86" ]; then
    build_package x86 x86
fi

PROJECT_CODE=$(($PROJECT_CODE + 1))

if [ -z "$1" ] || [ "$1" = "x86_64" ]; then
    build_package x86_64 x86_64
fi
