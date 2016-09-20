# SuperTuxKart
[![Build Status](https://travis-ci.org/supertuxkart/stk-code.png?branch=master)](https://travis-ci.org/supertuxkart/stk-code)

SuperTuxKart is a free kart racing game. It focuses on fun and not on realistic kart physics. Instructions can be found on the in-game help page.

The SuperTuxKart homepage can be found at <https://supertuxkart.net/>. There is also our [FAQ](https://supertuxkart.net/FAQ) and information on how get in touch with the [community](https://supertuxkart.net/Community)

## Hardware Requirements
* You need a 3D graphics card. (NVIDIA GeForce 8xxx and higher, ATI Radeon HD 4xxx and higher or Intel HD 3000 and higher.)
* You should have a CPU that's running at 1GHz or better.
* You'll need at least 512 MB of free VRAM (video memory).
* Disk space: 400MB
* Ideally, you want a joystick with at least 6 buttons.

## License
This software is released under the GNU General Public License (GPL) which can be found in the file [`COPYING`](/COPYING) in the same directory as this file. Information about the licenses for artwork are contained in `data/licenses`.

## 3D coordinates
A reminder for those looking at the code and 3d models:

STK    : X right, Y up,       Z forwards

Blender: X right, Y forwards, Z up

The exporters perform the needed transform, so in Blender you just work with XY plane as ground, and things will appear fine in STK (using XZ as ground in the code, obviously).

## Building from source

First, you need both the code and the assets (See <https://supertuxkart.net/Source_control> for more information):

```
git clone https://github.com/supertuxkart/stk-code
svn checkout https://svn.code.sf.net/p/supertuxkart/code/stk-assets stk-assets
```

## Building on Linux

### Dependencies

Install the following packages:

  * OpenGL (mesa)
  * OpenAL (recommended: openal-soft-devel)
  * Ogg    (libogg-dev)
  * Vorbis (libvorbis-dev)
  * Freetype (libfreetype6-dev)
  * libcurl (libcurl-devel)
  * libbluetooth (bluez-devel)
  * libpng (libpng-devel)
  * zlib (zlib-devel)
  * jpeg (libjpeg-turbo-devel)

Ubuntu command:

```
sudo apt-get install build-essential cmake libbluetooth-dev \
libcurl4-gnutls-dev libfreetype6-dev libfribidi-dev libgl1-mesa-dev \
libjpeg-dev libogg-dev libopenal-dev libpng-dev libvorbis-dev libxrandr-dev \
mesa-common-dev pkg-config zlib1g-dev
```

### Compiling

Compile SuperTuxKart:

```
mkdir cmake_build
cd cmake_build
cmake ..
make -j4
```
STK can then be run from the build directory with `bin/supertuxkart`

### Further options

To create a debug version of STK, use:

```
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

You can install your build system-wide:

```
sudo make install
```

The default install location is `/usr/local`, i.e. the data files will
be written to `/usr/local/share/games/supertuxkart`, the executable
will be copied to `/usr/local/bin`. To change the default installation
location, specify `CMAKE_INSTALL_PREFIX` when running cmake, e.g.:
`cmake .. -DCMAKE_INSTALL_PREFIX=/opt/stk`


## Windows

1. Install VS 2013 (or later). The free express versions work fine.
2. Download and install a source package - either a released package or from our [git/svn repositories](https://supertuxkart.net/Source_control).
3. Download the latest dependency package from [here](https://sourceforge.net/projects/supertuxkart/files/SuperTuxKart%20Dependencies/Windows/). Unzip it in the root directory, so that the dependencies directory is next to the src and data directories (if you are updating from a previous dependency package, you can delete the .dll files in the root directory, they are not needed anymore).
4. Download cmake and install it. Then start cmake-gui and select the STK root directory as 'Where is the source code', and a new directory in the root directory (next to src, data etc) as the build directory (for now I assume that this directory is called bld).
5. Click on configure. You will be asked to create the directory (yes), then for your VS version. Make sure you select the right version (be aware of the easy to confuse version numbers: VS 2013 = version 12). Click on configure, then generate. This will create the directory 'bld', and a VS solution in that directory.
6. In Visual Studio open the project file generated in the 'bld' folder.
7. Right click on the supertuxkart project in the solution explorer, and select "Set as StartUp Project".
8. Select Build->Build Solution (or press F7) to compile.

## OS X

### Getting Started

Install developer tools, either from the OS X Install DVD or from Apple's website.

If you have never built anything before, you have create `/usr/local/include/` first:

```bash
sudo mkdir -p /usr/local/include/
```

Symlink the `include`-folder of OpenGL framework to `/usr/local/include/GL` (Unix programs have an easier time finding it this way):

```bash
sudo ln -s /System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/ /usr/local/include/GL
```

On OS X 10.9.5, you might need the following workaround:

```bash
sudo ln -s `xcrun --show-sdk-path`/usr/include/ /usr/include
sudo ln -s `xcrun --show-sdk-path`/System/Library/Frameworks/OpenGL.framework/Headers/ /usr/local/include/OpenGL
```

The first link is required in order to find libcurl, the second to find opengl.

Download pre-built dependencies from [here](https://sourceforge.net/projects/supertuxkart/files/SuperTuxKart%20Dependencies/OSX/) and put the frameworks in [hard disk root]/Library/Frameworks

### CMake

CMake is used to build STK. At this time CMake will not make a binary that is ready for distribution.

You'll have to run these commands inside your stk-code directory.

### Building

With clang:

```
mkdir cmake_build
cd cmake_build
cmake ..
make
```

With GCC :
```
mkdir cmake_build
cd cmake_build
cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DCMAKE_C_COMPILER=/usr/bin/gcc
make
```

Building on 10.10 with 10.9 compat
```
cmake .. -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk  -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9
```

### Xcode

Place an additional copy of the dependencies into `Users/<YOUR_USERNAME>/Library/Frameworks`.
Then cd to your cloned stk-code directory and execute the following commands:

```bash
mkdir xcode_build && cd xcode_build
cmake .. -GXcode
```

Use Finder to navigate to your stk-code/xcode_build folder and open the newly generated Xcode project (SuperTuxKart.xcodeproj).

You can then build the project in Xcode using Product -> Build

Note: Xcode is much less well tested than makefiles, so there may be issues when trying to use Xcode.
