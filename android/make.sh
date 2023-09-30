#!/bin/bash
#
# (C) 2016-2017 Dawid Gan, under the GPLv3
#
# A script that creates the apk build


export DIRNAME=$(realpath "$(dirname "$0")")

export NDK_PATH_DEFAULT="$DIRNAME/android-ndk"
export SDK_PATH_DEFAULT="$DIRNAME/android-sdk"

export NDK_BUILD_SCRIPT="$DIRNAME/Android.mk"

#export NDK_CCACHE=ccache
export CPU_CORE="-j$(($(nproc) + 1))"

if [ -z "$STK_MIN_ANDROID_SDK" ]; then
    export STK_MIN_ANDROID_SDK=16
fi

if [ -z "$STK_TARGET_ANDROID_SDK" ]; then
    export STK_TARGET_ANDROID_SDK=33
fi

if [ -z "$STK_NDK_VERSION" ]; then
    export STK_NDK_VERSION=23.1.7779620
fi

export APP_NAME_RELEASE="SuperTuxKart"
export PACKAGE_NAME_RELEASE="org.supertuxkart.stk"
export PACKAGE_CLASS_NAME_RELEASE="org/supertuxkart/stk"
export APP_DIR_NAME_RELEASE="supertuxkart"
export APP_ICON_RELEASE="$DIRNAME/icon.png"
export APP_ICON_ADAPTIVE_BG_RELEASE="$DIRNAME/icon_adaptive_bg.png"
export APP_ICON_ADAPTIVE_FG_RELEASE="$DIRNAME/icon_adaptive_fg.png"

export APP_NAME_BETA="SuperTuxKart Beta"
export PACKAGE_NAME_BETA="org.supertuxkart.stk_beta"
export PACKAGE_CLASS_NAME_BETA="org/supertuxkart/stk_beta"
export APP_DIR_NAME_BETA="supertuxkart-beta"
export APP_ICON_BETA="$DIRNAME/icon-dbg.png"
export APP_ICON_ADAPTIVE_BG_BETA="$DIRNAME/icon_adaptive_bg-dbg.png"
export APP_ICON_ADAPTIVE_FG_BETA="$DIRNAME/icon_adaptive_fg-dbg.png"

export APP_NAME_DEBUG="SuperTuxKart Debug"
export PACKAGE_NAME_DEBUG="org.supertuxkart.stk_dbg"
export PACKAGE_CLASS_NAME_DEBUG="org/supertuxkart/stk_dbg"
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
    rm -rf "$DIRNAME/.cxx"
    exit
fi

# Update variables for selected architecture
if [ -z "$COMPILE_ARCH" ]; then
    COMPILE_ARCH="all"
fi

if [ "$COMPILE_ARCH" = "armv7" ]; then
    COMPILE_ARCH="armeabi-v7a"
elif [ "$COMPILE_ARCH" = "aarch64" ]; then
    COMPILE_ARCH="arm64-v8a"
elif [ "$COMPILE_ARCH" != "x86" ] && [ "$COMPILE_ARCH" != "x86_64" ] && \
     [ "$COMPILE_ARCH" != "all" ]; then
    echo "Unknown COMPILE_ARCH: $COMPILE_ARCH. Possible values are:" \
         "all, armv7, aarch64, x86, x86_64"
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
    export PACKAGE_CLASS_NAME="$PACKAGE_CLASS_NAME_DEBUG"
    export APP_DIR_NAME="$APP_DIR_NAME_DEBUG"
    export APP_ICON="$APP_ICON_DEBUG"
    export APP_ICON_ADAPTIVE_BG="$APP_ICON_ADAPTIVE_BG_DEBUG"
    export APP_ICON_ADAPTIVE_FG="$APP_ICON_ADAPTIVE_FG_DEBUG"
elif [ "$BUILD_TYPE" = "release" ] || [ "$BUILD_TYPE" = "Release" ]; then
    export GRADLE_BUILD_TYPE="assembleRelease"
    export IS_DEBUG_BUILD=0
    export APP_NAME="$APP_NAME_RELEASE"
    export PACKAGE_NAME="$PACKAGE_NAME_RELEASE"
    export PACKAGE_CLASS_NAME="$PACKAGE_CLASS_NAME_RELEASE"
    export APP_DIR_NAME="$APP_DIR_NAME_RELEASE"
    export APP_ICON="$APP_ICON_RELEASE"
    export APP_ICON_ADAPTIVE_BG="$APP_ICON_ADAPTIVE_BG_RELEASE"
    export APP_ICON_ADAPTIVE_FG="$APP_ICON_ADAPTIVE_FG_RELEASE"
elif [ "$BUILD_TYPE" = "beta" ] || [ "$BUILD_TYPE" = "Beta" ]; then
    export GRADLE_BUILD_TYPE="assembleRelease"
    export IS_DEBUG_BUILD=0
    export APP_NAME="$APP_NAME_BETA"
    export PACKAGE_NAME="$PACKAGE_NAME_BETA"
    export PACKAGE_CLASS_NAME="$PACKAGE_CLASS_NAME_BETA"
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

NDK_PATH="$(realpath "$NDK_PATH")/${STK_NDK_VERSION}"
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

BUILD_TOOLS_FULL=(${BUILD_TOOLS_VER//./ })
export COMPILE_SDK_VERSION="${BUILD_TOOLS_FULL[0]}"

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
    mkdir "$DIRNAME/obj"
    touch "$DIRNAME/obj/make_standalone_toolchain.stamp"
fi

echo "$PROJECT_VERSION" > "$DIRNAME/obj/project_version"

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
        return
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

find "$DIRNAME/assets/data/po" -type f -name '*.po' | while read -r f; do create_translation "$f"; done

ADAPTIVE_ICON_FILE="$DIRNAME/res/drawable-anydpi-v26/icon.xml"

echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>"                      >  "$ADAPTIVE_ICON_FILE"
echo "<adaptive-icon"                                                  >> "$ADAPTIVE_ICON_FILE"
echo "  xmlns:android=\"http://schemas.android.com/apk/res/android\">" >> "$ADAPTIVE_ICON_FILE"
echo "    <background android:drawable=\"@drawable/icon_bg\" />"       >> "$ADAPTIVE_ICON_FILE"
echo "    <foreground android:drawable=\"@drawable/icon_fg\" />"       >> "$ADAPTIVE_ICON_FILE"
echo "</adaptive-icon>"                                                >> "$ADAPTIVE_ICON_FILE"

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
cp -f "$DIRNAME/../lib/sdl2/android-project/app/src/main/java/org/libsdl/app/SDLSurface.java" \
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

export ANDROID_HOME="$SDK_PATH"
./gradlew -Pcompile_sdk_version="$COMPILE_SDK_VERSION"   \
          -Pmin_sdk_version="$STK_MIN_ANDROID_SDK"       \
          -Ptarget_sdk_version="$STK_TARGET_ANDROID_SDK" \
          -Pstorepass="$STK_STOREPASS"                   \
          -Pkeystore="$STK_KEYSTORE"                     \
          -Palias="$STK_ALIAS"                           \
          -Pndk_version="$STK_NDK_VERSION"               \
          -Pcompile_arch="$COMPILE_ARCH"                 \
          -Pcpu_core="$CPU_CORE"                         \
          -Ppackage_name="$PACKAGE_NAME"                 \
          -Pversion_name="$PROJECT_VERSION"              \
          -Pversion_code="$PROJECT_CODE"                 \
          $GRADLE_BUILD_TYPE

if [ "$GRADLE_BUILD_TYPE" = "assembleRelease" ]; then
./gradlew -Pcompile_sdk_version="$COMPILE_SDK_VERSION"   \
          -Pmin_sdk_version="$STK_MIN_ANDROID_SDK"       \
          -Ptarget_sdk_version="$STK_TARGET_ANDROID_SDK" \
          -Pstorepass="$STK_STOREPASS"                   \
          -Pkeystore="$STK_KEYSTORE"                     \
          -Palias="$STK_ALIAS"                           \
          -Pndk_version="$STK_NDK_VERSION"               \
          -Pcompile_arch="$COMPILE_ARCH"                 \
          -Pcpu_core="$CPU_CORE"                         \
          -Ppackage_name="$PACKAGE_NAME"                 \
          -Pversion_name="$PROJECT_VERSION"              \
          -Pversion_code="$PROJECT_CODE"                 \
          "bundleRelease"
fi

check_error
