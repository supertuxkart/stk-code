================================================================================

 SUPERTUXKART

================================================================================



--------------------------------------------------------------------------------
 SYSTEM REQUIREMENTS
--------------------------------------------------------------------------------

To run SuperTuxKart on Android, you need a device that meets following
requirements:

- Android 4.1 or later
- Processor compatible with armv7 or x86
- GPU that supports OpenGL ES 2.0
- 1 GB RAM (STK uses ~150 MB in minimal configuration)
- 300 MB of free space on internal storage
- Touch screen or external keyboard



--------------------------------------------------------------------------------
 COMPILATION
--------------------------------------------------------------------------------

The build scripts are designed to run under linux. They may work under cygwin
after some tweaks, but atm. only linux is supported.

Dependencies list (may be incomplete):

    autoconf, automake, make, python, gradle, imagemagick, cmake, 
    vorbis-tools, pngquant

Additionally some dependencies for optimize_data script:

    advancecomp, libjpeg-progs, optipng

Before compilation you must download dependencies-android-src.tar.xz from:
https://github.com/supertuxkart/dependencies/releases
Choose the corresponding STK version you are compiling, use "preview" for git.
It contains sources of libraries that are used in STK, but are not available
in stk-code repository (curl, freetype, openal...).

These libraries are compiled and then statically linked with STK by the Android
build script.

You need to extract that packed file to stk-code/lib directory, so that the
directory will have following structure:
> stk-code
  > build
  > data
  > lib
    > angelscript
    > bullet
    > curl
    > enet
    > freetype
    > ...
  > src
  > ...

You also need Android SDK for android-26 platform or later (SDL2 requirement) and
Android NDK. Version r23 is recommended, because it's known that it works
without issues. r24 or later is not compatible because it removes Android 4.1
support (https://github.com/android/ndk/wiki/Changelog-r23#announcements)

You need to create proper "android-sdk" and "android-ndk" symlinks in the
directory with Android project, so that the compilation script will have access
to the SDK and NDK.

Before running the compilation, run the generate_assets script, so that
selected assets will be copied to "assets" directory, and then included in the
apk file.

You can select different karts and tracks by setting KARTS and TRACKS variables
in the generate_assets.sh script at the beginning of file.

When you are creating the assets directory manually, note that the
directories.txt file is urgently needed and it is used by the application for
extracting assets.

If the assets directory is already prepared, you can run "./make.sh" command to
build the project and create an apk file. Note that all arguments are passed to
the make command, so that you can run "./make.sh -j5" for multi-threaded build.

Basically if all dependencies are installed in the system, it should be enough
to just run:

    export SDK_PATH=/path/to/your/android/sdk
    export NDK_PATH=/path/to/your/android/ndk
    # Optional for STK_MIN_ANDROID_SDK, STK_TARGET_ANDROID_SDK and STK_NDK_VERSION
    # If unset it will use the below values
    export STK_MIN_ANDROID_SDK=16
    export STK_TARGET_ANDROID_SDK=30
    export STK_NDK_VERSION=23.1.7779620
    ./generate_assets.sh
    ./make_deps.sh
    ./make.sh

You may need to add org.gradle.jvmargs=-XX:MaxHeapSize=2048m -Xmx2048m to
.gradle/gradle.properties if you see java.lang.OutOfMemoryError / Java heap
space execption error.

--------------------------------------------------------------------------------
 ENVIRONMENT VARIABLES
--------------------------------------------------------------------------------

COMPILE_ARCH    - Allows one to choose CPU architecture for which the package will
                  be compiled. 
                  Possible values: all, armv7, aarch64, x86, x86_64.
                  Default is: all.

BUILD_TYPE      - Allows one to set build type.
                  Possible values: debug, release, beta.
                  Default is: debug.

BUILD_TOOLS_VER - Allows to override the SDK build-tools version.

SDK_PATH        - Path to SDK directory

NDK_PATH        - Path to NDK directory, it should include a list of installed
                  NDK version folders

PROJECT_VERSION - Set Supertuxkart version number, for example "0.9.3" or 
                  "git20170409" or whatever. The version must match with file
                  assets/data/supertuxkart.$PROJECT_VERSION
                  and that file must exist, because it is used for extracting 
                  and loading game data.
                  Default is: git.

PROJECT_CODE    - Set Supertuxkart version code that is used in the manifest 
                  file.
                  Default is: 1.



--------------------------------------------------------------------------------
 RELEASE BUILD
--------------------------------------------------------------------------------

Making a release build is similar to typical compilation, but there are few
additional things to do. 

You have to set PROJECT_VERSION variable. This is important, because assets 
manager in STK checks that value and detects if already extracted data files are
up to date. So that when you will install new STK version, this will force new 
data extraction automatically.

The PROJECT_CODE variable typically should be set to a value higher than for
previous release, so that users will receive the upgrade.

Before compilation you have to set:

    export BUILD_TYPE=release

It's also needed to set STK_STOREPASS, STK_KEYSTORE and STK_ALIAS environment
variables, so that the apk files can be signed.

and then you make standard compilation with:

    ./generate_assets.sh
    ./make_deps.sh
    ./make.sh
