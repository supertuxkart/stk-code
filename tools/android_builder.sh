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
export PROJECT_VERSION=git20181001
export PROJECT_CODE=48
export STOREPASS="xxx"
export KEYSTORE="/path/to/stk.keystore"
export ALIAS="alias"


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
    ./generate_assets.sh

    if [ ! -f "./assets/directories.txt" ]; then
        echo "Error: Couldn't generate assets"
        return
    fi

    if [ -f "./assets/data/supertuxkart.git" ]; then
        mv "./assets/data/supertuxkart.git" \
           "./assets/data/supertuxkart.$PROJECT_VERSION"
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

    ONLY_ASSETS=1        \
    TRACKS="all"         \
    TEXTURE_SIZE=512     \
    JPEG_QUALITY=95      \
    PNG_QUALITY=95       \
    PNGQUANT_QUALITY=95  \
    SOUND_QUALITY=112    \
    SOUND_MONO=0         \
    SOUND_SAMPLE=44100   \
    ./generate_assets.sh

    if [ ! -f "./assets/directories.txt" ]; then
        echo "Error: Couldn't generate full assets"
        return
    fi

    cd ./assets/data
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
    ./make.sh -j5
    cd -

    if [ ! -f ./android-$ARCH1/build/outputs/apk/release/android-$ARCH1-release-unsigned.apk ]; then
        echo "Error: Couldn't build apk for architecture $ARCH1"
        return
    fi

    cp ./android-$ARCH1/build/outputs/apk/release/android-$ARCH1-release-unsigned.apk \
       ./android-output/SuperTuxKart-$PROJECT_VERSION-$ARCH1-unaligned.apk

    cp ./android-$ARCH1/obj/local/$ARCH2/libmain.so \
       ./android-output/SuperTuxKart-$PROJECT_VERSION-$ARCH1-libmain.so

    cd ./android-output

    jarsigner -sigalg SHA1withRSA -digestalg SHA1                 \
              -keystore "$KEYSTORE"                               \
              -storepass "$STOREPASS"                             \
              SuperTuxKart-$PROJECT_VERSION-$ARCH1-unaligned.apk  \
              "$ALIAS"

    check_error

    zipalign -f 4 SuperTuxKart-$PROJECT_VERSION-$ARCH1-unaligned.apk \
                  SuperTuxKart-$PROJECT_VERSION-$ARCH1.apk

    check_error

    rm SuperTuxKart-$PROJECT_VERSION-$ARCH1-unaligned.apk

    cd -
}


# Handle clean command
if [ ! -z "$1" ] && [ "$1" = "clean" ]; then
    clean
    exit
fi

#Build packages
init_directories

generate_assets
generate_full_assets

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
