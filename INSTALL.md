# Building from source

In order to build SuperTuxKart from source, you'll need both the code and the assets (See <https://supertuxkart.net/Source_control> for more information):

```bash
git clone https://github.com/supertuxkart/stk-code stk-code
svn co https://svn.code.sf.net/p/supertuxkart/code/stk-assets stk-assets
```

## Building SuperTuxKart on Linux

### Dependencies

To build SuperTuxKart from source, you'll need to install the following packages:

* OpenAL (recommended: openal-soft-devel)
* Ogg (libogg-dev)
* Vorbis (libvorbis-dev)
* Freetype (libfreetype6-dev)
* Harfbuzz (libharfbuzz-dev)
* libcurl (libcurl-devel)
* libbluetooth (bluez-devel)
* openssl (openssl-dev)
* libpng (libpng-devel)
* zlib (zlib-devel)
* jpeg (libjpeg-turbo-devel)
* SDL2 (libsdl2-devel)

Fedora command:

```bash
sudo dnf install @development-tools angelscript-devel \
bluez-libs-devel cmake desktop-file-utils SDL2-devel \
freealut-devel freetype-devel \
gcc-c++ git-core libcurl-devel libjpeg-turbo-devel \
libpng-devel libsquish-devel libtool libvorbis-devel \
openal-soft-devel openssl-devel libcurl-devel harfbuzz-devel \
libogg-devel openssl-devel pkgconf \
wiiuse-devel zlib-devel
```

Mageia 6 command:

```bash
su -c 'urpmi gcc-c++ cmake openssl-devel libcurl-devel freetype-devel harfbuzz-devel \
libjpeg-turbo-devel libogg-devel openal-soft-devel SDL2-devel \
libpng-devel libvorbis-devel nettle-devel zlib-devel git subversion \
libbluez-devel libfreetype6-devel
```

openSUSE command:

```bash
sudo zypper install gcc-c++ cmake openssl-devel libcurl-devel libSDL2-devel \
freetype-devel harfbuzz-devel libogg-devel openal-soft-devel libpng-devel \
libvorbis-devel pkgconf zlib-devel enet-devel \
libjpeg-devel bluez-devel freetype2-devel
```

Debian-based Distributions command:

```bash
sudo apt-get install build-essential cmake libbluetooth-dev libsdl2-dev \
libcurl4-openssl-dev libenet-dev libfreetype6-dev libharfbuzz-dev \
libjpeg-dev libogg-dev libopenal-dev libpng-dev \
libssl-dev libvorbis-dev libmbedtls-dev pkg-config zlib1g-dev
```

Solus command:
```bash
sudo eopkg it cmake openal-soft-devel libogg-devel libvorbis-devel freetype2-devel \
harfbuzz-devel curl-devel bluez-devel openssl-devel libpng-devel zlib-devel \
libjpeg-turbo-devel sdl2-devel enet-devel libjpeg-turbo-devel bluez-devel curl-devel
```

### In-game recorder

To build the in-game recorder for STK, you have to install
libopenglrecorder from your distribution, or compile it yourself from [here](https://github.com/Benau/libopenglrecorder).
Compilation instruction is explained there. If you don't need this feature, pass `-DBUILD_RECORDER=off` to cmake.

### Shaderc for Vulkan support

You need to compile [Shaderc](https://github.com/google/shaderc) for vulkan support in SuperTuxKart if you are not building for Windows or macOS. If you don't need this feature, pass `-DNO_SHADERC=on` to cmake.

### Compiling

To compile SuperTuxKart, run the following commands inside `stk-code` directory

```bash
# go into the stk-code directory
cd stk-code

# create and enter the cmake_build directory
mkdir cmake_build
cd cmake_build

# run cmake to generate the makefile
cmake ..

# compile
make -j$(nproc)
```

STK can then be run from the build directory with `bin/supertuxkart`

#### Keeping your build up to date

To recompile the latest code without redownloading the entire source, first run the ```svn up``` command inside the 'stk-assets' directory, then run the following commands inside the 'stk-code' directory:

```bash
git pull
cd cmake_build
cmake ..
make -j$(nproc)
```

##### Build Speed Optimization

"-j$(nproc)" is an example, for a faster build, use "-jx" instead, where "x" is the amount of CPU threads you have, minus one.

### Further options

To create a debug version of STK, run:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

You can install your build system-wide:

```bash
sudo make install
```

The default install location is `/usr/local`, i.e. the data files will
be written to `/usr/local/share/games/supertuxkart`, the executable
will be copied to `/usr/local/bin`. To change the default installation
location, specify `CMAKE_INSTALL_PREFIX` when running cmake, e.g.:
`cmake .. -DCMAKE_INSTALL_PREFIX=/opt/stk`

## Building SuperTuxKart on Windows

To Build SuperTuxKart on Windows follow these instructions:

1. Download and install Visual Studio from here: [Visual Studio - Download](https://www.visualstudio.com/downloads/). The free Visual Studio Community edition works fine. Remember to select "Desktop development with C++" in the installer.

2. If you want the stable version, download the SuperTuxKart source package from the latest stable version [SuperTuxKart on GitHub](https://github.com/supertuxkart/stk-code/releases) and unpack it.

3. If you want the development version, you will need a Git client and a SVN client. More information can be found here: [SuperTuxKart.net - Source Control](https://supertuxkart.net/Source_control).
Open your file browser and find somewhere you want to put the development version of SuperTuxKart. For example in C:\Users\<Your Username> as the Git and SVN clients will have write permissions there, and you should create its own directory, for example SuperTuxKart-dev. Enter that directory, and create a directory inside called stk-assets, and enter it. If you installed TortoiseSVN, right-click, select TortoiseSVN -> Checkout... and paste the corresponding URL found in [SuperTuxKart.net - Source Control](https://supertuxkart.net/Source_control). While it is downloading the game assets, go back to your file browser and one level up. Right-click again somewhere empty and select "Git clone..." and paste the corresponding link found in [SuperTuxKart.net - Source Control](https://supertuxkart.net/Source_control).
*Note: Both `stk-code` and `stk-assets` **must** be in the same directory, otherwise the build will likely fail!*

4. If you got the stable version, download the Windows dependencies package from [SuperTuxKart on GitHub - Dependencies Releases](https://github.com/supertuxkart/dependencies/releases), find the stk-code version there and download the `dependencies(arch).zip` as needed and unpack the archive into the `stk-code` directory.

5. If you got the development version go to SuperTuxKart-dev in your file browser, then visit [SuperTuxKart on GitHub - Dependencies latest preview release](https://github.com/supertuxkart/dependencies/releases/tag/preview)
and unpack the archive into the `stk-code` directory. Download `i686` if you use Win32 generator of MSVC, `x86_64` for x64, `armv7` for ARM and `aarch64` for ARM64.

6. Download CMake from here: [CMake - download page](https://cmake.org/download/), install it; once CMake is installed, double click on the CMake icon on your desktop, and point it towards your `stk-code` directory in the 'Where is the source code' field, and point 'Where to build the binaries' to a new directory called `build` or `bld` inside the stk-code directory.

7. Press 'Configure'; CMake will ask you if it is OK to create the aforementioned directory, press `Yes`. CMake will then ask you about your version of Visual Studio.

    Confirm your selection; *Please look at the table below to avoid confusion between version numbers and releases of Visual Studio*; CMake will begin creating the required files for the build in the directory. If you want to do a 64-bit build, select the version of Visual Studio you installed with "Win64" appended. Press 'Generate' button.

8. Navigate to your build directory and open the `SuperTuxKart.sln` file; Visual Studio will now load the solution.

9. In the 'Solution Explorer', right click on the `supertuxkart` project and select "Set as StartUp project".

10. Open the 'Build' menu and select 'Build Solution'; or, press the default keyboard shortcut: `CTRL + SHIFT + B` to build the solution.

*Note: To avoid confusion between releases and versions, refer to this table:*

Visual Studio Release | Version
----------------------|------------
Visual Studio 2019| 16
Visual Studio 2017| 15
Visual Studio 2015| 14
Visual Studio 2013| 13

## Building SuperTuxKart on Windows (from PowerShell/Command line)

1. Download and install Visual Studio from here: [Visual Studio - Download](https://www.visualstudio.com/downloads/), the free Visual Studio Community edition works fine.

2. Download a source package from either [SuperTuxKart on GitHub](https://github.com/supertuxkart/stk-code/releases) or [SuperTuxKart.net - Source Control](https://supertuxkart.net/Source_control)
NOTE: the `stk-code` and `stk-assets` directories **must** be in the same directory, `stk-assets` is not needed if you download the full source tarball `(SuperTuxKart-version-src.tar.xz)`.
3. Download the Windows dependencies package from [SuperTuxKart on GitHub - Dependencies latest preview release](https://github.com/supertuxkart/dependencies/releases/tag/preview)
and unpack the archive into the `stk-code` directory. Download `i686` if you use Win32 generator of MSVC, `x86_64` for x64, `armv7` for ARM and `aarch64` for ARM64.
4. Download CMake from here: [CMake - download page](https://cmake.org/download/); and install it. Navigate to the `stk-code` directory; and create an directory called "build":

    ```cmd
    mkdir build
    cd build
    ```

5. Once inside the build directory; run CMake to start the compilation process:

    ```cmd
    cmake ..
    ```

6. Now that CMake finished configuring and creating the necessary files for the build, run the build command in the same directory:

    ```cmd
    msbuild.exe SuperTuxKart.sln
    ```

SuperTuxKart can now be run as `bin\Debug\supertuxkart.exe` or `bin\Release\supertuxkart.exe`

## Building SuperTuxKart on Windows using LLVM MinGW

1. Get the LLVM Mingw archive [here](https://github.com/mstorsjo/llvm-mingw/releases/latest), get the `*-msvcrt-i686.zip or` `*-msvcrt-x86_64.zip` depending on whether you have an Intel / AMD 32 or 64-bit Windows. If you are using ARM64 Windows get the `*-msvcrt-i686.zip` should be fine (untested). After downloading extract it as `C:\llvm-mingw` so `C:\llvm-mingw` contains `bin`, `include`, `lib`, etc.
2. Get Ninja [here](https://github.com/ninja-build/ninja/releases/latest), download the `ninja-win.zip` and extract the `ninja.exe` from the archive to `C:\llvm-mingw`. If you are not using Intel / AMD 64-bit Windows use [this link](https://packages.msys2.org/package/mingw-w64-i686-ninja) and extract `mingw32\bin\ninja.exe` inside the `tar.zst`.
3. Download a source package from either [SuperTuxKart on GitHub](https://github.com/supertuxkart/stk-code/releases) or [SuperTuxKart.net - Source Control](https://supertuxkart.net/Source_control)
NOTE: the `stk-code` and `stk-assets` directories **must** be in the same directory, `stk-assets` is not needed if you download the full source tarball `(SuperTuxKart-version-src.tar.xz)`. Also make sure they lie within the C drive.
4. Download the Windows dependencies package from [SuperTuxKart on GitHub - Dependencies latest preview release](https://github.com/supertuxkart/dependencies/releases/tag/preview)
and unpack the archive into the `stk-code` directory. Download `i686` if you compile for Intel / AMD 32-bit, `x86_64` for Intel / AMD 64-bit, `armv7` for ARM 32-bit and `aarch64` for ARM 64-bit version of Windows.
6. Download CMake from here: [CMake - download page](https://cmake.org/download/), install it; once CMake is installed, double click on the CMake icon on your desktop, and point it towards your `stk-code` directory in the 'Where is the source code' field, and point 'Where to build the binaries' to a new directory called `build` inside the stk-code directory.
7. Press the `Add Entry` button and add the values below:
* Name: `LLVM_ARCH` Type: `STRING`  Value: `i686`, `x86_64`, `armv7` or `aarch64`
* Name: `LLVM_PREFIX` Type: `STRING`  Value: `C:/llvm-mingw`
* Name: `CMAKE_MAKE_PROGRAM` Type: `STRING` Value: `C:/llvm-mingw/ninja.exe`
* Name: `USE_WIIUSE` Type: `BOOL`  Value: `Empty (unchecked)`
8. Press 'Configure'; CMake will ask you if it is OK to create the aforementioned directory, press `Yes`. Choose `Ninja` from `Specify the generator for this project`, choose `Specify toolchain file for cross-compiling` then press `Next`. Specify the toolchain file which is located in `stk-code\cmake\Toolchain-llvm-mingw.cmake` and press `Finish`. If no error appears then press 'Generate'
9. Once inside the build directory using command line `cmd.exe` or PowerShell:
    ```cmd
    C:\llvm-mingw\ninja.exe
    ```

SuperTuxKart can now be run as `bin\supertuxkart.exe`.

## Building SuperTuxKart on macOS

### Getting Started

Install the developer tools, either from the OS X Install DVD or from Apple's website.

Download `dependencies-macosx.tar.xz` from `Assets` section [here](https://github.com/supertuxkart/dependencies/releases) and extract it inside stk-code directory, use `preview` version for git stk-code.

Build STK

```bash
cd /path/to/stk-code
mkdir cmake_build
cd cmake_build
cmake .. -DCMAKE_FIND_ROOT_PATH=$(pwd)/../dependencies-macosx -DUSE_CRYPTO_OPENSSL=FALSE
make
```

Add ` -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9` for 10.9 compatibility.

#### (Optional) packaging for distribution

By default, the executable that is produced is not ready for distribution. Install <https://github.com/auriamg/macdylibbundler> and run:

```bash
dylibbundler -od -b -x ./bin/SuperTuxKart.app/Contents/MacOS/supertuxkart -d ./bin/SuperTuxKart.app/Contents/libs/ -p @executable_path/../libs/ -s ../dependencies-macosx/lib
```

Add `-ns` to disable ad-hoc codesigning

Afterwards, copy the contents of `stk-assets` into `/SuperTuxKart.app/Contents/Resources/data`.
