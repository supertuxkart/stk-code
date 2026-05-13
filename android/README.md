# Building STK Android

## System requirements


To run SuperTuxKart on Android, you need a device that meets following requirements:

* Android 5.0 or later
* Processor compatible with armv7 or x86
* GPU that supports OpenGL ES 2.0
* 1 GB RAM (STK uses ~150 MB in minimal configuration)
* 300 MB of free space on internal storage
* Touch screen or external keyboard

## Building

The build scripts are designed to run under Linux. They may work under cygwin after some tweaks or WSL2 (Windows Subsystem for Linux) also.

### Dependencies

To build SuperTuxKart from source to Android, you'll need to install the following packages:

* autoconf
* automake
* make
* python
* packaging (python package)
* mako (python module)
* flex
* bison
* gradle
* imagemagick
* cmake
* meson
* vorbis-tools
* pngquant
* advancecomp
* libjpeg-progs
* optipng

Before build, you must download dependencies-android-src.tar.xz from: https://github.com/supertuxkart/dependencies/releases. Choose the corresponding STK version you are compiling, or use "preview" for 1.5 or git. It contains sources of libraries that are used in STK, but are not available in stk-code repository (curl, freetype, openal...).

These libraries are compiled and then statically linked with STK by the Android build script.

You need to extract that packed file to stk-code/lib directory, so that the directory will have following structure:
```
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
```

### Android SDK and NDK

You also need Android SDK for android-26 platform or later (SDL2 requirement) and Android NDK (the builds scripts are tested with NDK 28.1.13356709).

You need to create proper "android-sdk" and "android-ndk" symlinks in the directory with Android project, so that the compilation script will have access to the SDK and NDK.

#### Downloading it from Android Studio

The easiest way to download SDK and NDK is using the SDK Manager of Android Studio (you can download this app in https://developer.android.com/studio).

After installing Android Studio, go to More Actions (in the welcome screen) -> SDK Manager -> SDK Tools. In SDK Tools, install the lastest official version of "Android SDK Build-Tools" (for Android SDK), and any version of "NDK (Side by side)" (for Android NDK, recommended NDK 28.1.13356709).

### Generating assets and building dependencies

Before running the compilation, run the generate_assets.sh script, so the assets will be copied to "assets" directory, and then included in the .apk file.

You can select different karts and tracks by setting KARTS and TRACKS variables in the "./generate_assets.sh" script at the beginning of file also.

When you are creating the assets directory manually, note that the directories.txt file is urgently needed and it is used by the application for extracting assets.

If the assets directory is already prepared, you must run "./make_deps.sh" before run the make script (because you need to build the dependencies), and after this, you finally can run "./make.sh" command to build the project and create an .apk file. Note that all arguments are passed to the make command, so that you can run "./make.sh -j5" for multi-threaded build.

Basically if all dependencies are installed in the system, it should be enough to just run:

```bash
export SDK_PATH=/path/to/your/android/sdk 
export NDK_PATH=/path/to/your/android/ndk

# If you downloaded these deps in Android Studio, use this:
# export SDK_PATH=~/Android/Sdk
# export NDK_PATH=~/Android/Sdk/ndk

# Optional for STK_MIN_ANDROID_SDK, STK_TARGET_ANDROID_SDK and STK_NDK_VERSION
# If unset it will use the below values: 
# export STK_MIN_ANDROID_SDK=16
# export STK_TARGET_ANDROID_SDK=30
# export STK_NDK_VERSION=23.1.7779620

./generate_assets.sh
./make_deps.sh
./make.sh
```

#### Some environment variables

For more variables, read the .sh scripts (the lines starting with "export" specifically).

**COMPILE_ARCH**    - Allows one to choose CPU architecture for which the package will be compiled. Possible values: all, armv7, aarch64, x86, x86_64. Default is: all.

**BUILD_TYPE**      - Allows one to set build type. Possible values: debug, release, beta. Default is: debug.

**BUILD_TOOLS_VER** - Allows to override the SDK build-tools version.

**SDK_PATH**        - Path to SDK directory

**NDK_PATH**        - Path to NDK directory, it should include a list of installed NDK version folders

**PROJECT_VERSION** - Set Supertuxkart version number, for example "1.5" or "git" or whatever. The version must match with file assets/data/supertuxkart.$PROJECT_VERSION and that file must exist, because it is used for extracting and loading game data. Default is: git.

**PROJECT_CODE**    - Set Supertuxkart version code that is used in the manifest file. Default is: 1.

## Building the Release build

Making a release build is similar to typical compilation, but there are few additional things to do. 

You have to set PROJECT_VERSION variable. This is important, because assets manager in STK checks that value and detects if already extracted data files are up to date. So that when you will install new STK version, this will force new data extraction automatically.

The PROJECT_CODE variable typically should be set to a value higher than for previous release, so that users will receive the upgrade.

Later, you need a "keystore.jks" file also (to sign the .apk, required for Release or Beta build), so you have to put these commands (you need JDK installed): 

```bash
keytool -genkeypair -v -keystore keystore.jks -alias alias -keyalg RSA -keysize 2048 -validity 9125 
```

You must create a password, and put someinfo about you (or just press ENTER for default options) to create the file.

Later and Before compilation, you have to set:

```bash
export STK_STOREPASS="storepass"
export STK_KEYSTORE="./keysore.jks"
export STK_ALIAS="alias"
export BUILD_TYPE=release
```

And then, you make standard compilation with:

```bash
    ./generate_assets.sh
    ./make_deps.sh
    ./make.sh
```

## Troubleshooting

### "The source directory '/stk-code/android/deps-arm64-v8a/libadrenotools/lib/linkernsbypass' does not contain a CMakeLists.txt file." while executing make_deps.sh

For some reason, the library linkernsbypass doesn't come with the dependencies-android-src.tar.xz file, so execute this to fix this: 

```bash
git clone https://github.com/bylaws/liblinkernsbypass.git ../lib/libadrenotools/lib/linkernsbypass
```
### "java.lang.OutOfMemoryError / Java heap space execption" while executing make.sh

You need to add "org.gradle.jvmargs=-XX:MaxHeapSize=2048m -Xmx2048m" (without the quotes) to .gradle/gradle.properties. (or to gradle.properties in the android folder).
