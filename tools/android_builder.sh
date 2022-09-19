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
export PROJECT_VERSION=git20211004
export PROJECT_CODE=299
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

    rm -rf ./android/assets
    rm -rf ./android-output

    cd android
    ./make.sh clean
    cd -
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

# Handle clean command
if [ ! -z "$1" ] && [ "$1" = "clean" ]; then
    clean
    exit
fi

#Build packages

generate_lq_assets
generate_full_assets
generate_assets

if [ -f "./android-output/SuperTuxKart-$PROJECT_VERSION.apk" ]; then
    echo "Package for architecture $ARCH1 is already built"
    #exit
fi

cd ./android
./make_deps.sh
check_error
./make.sh
cd -

if [ ! -f ./android/build/outputs/apk/release/android-release.apk ]; then
    echo "Error: Couldn't build apk"
    exit
fi

if [ ! -f ./android/build/outputs/bundle/release/android-release.aab ]; then
    echo "Error: Couldn't build app bundle"
    exit
fi

cp ./android/build/outputs/apk/release/android-release.apk \
   ./android-output/SuperTuxKart-$PROJECT_VERSION.apk

cp ./android/build/outputs/bundle/release/android-release.aab \
   ./android-output/SuperTuxKart-$PROJECT_VERSION.aab

for arch in $(ls ./android/build/intermediates/ndkBuild/release/obj/local); do
    cp ./android/build/intermediates/ndkBuild/release/obj/local/$arch/libmain.so \
    ./android-output/SuperTuxKart-$PROJECT_VERSION-$arch-libmain.so
    cp ./android/build/intermediates/ndkBuild/release/obj/local/$arch/libSDL2.so \
    ./android-output/SuperTuxKart-$PROJECT_VERSION-$arch-libSDL2.so
done
