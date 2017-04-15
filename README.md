# SuperTuxKart
[![Build Status](https://travis-ci.org/supertuxkart/stk-code.svg?branch=master)](https://travis-ci.org/supertuxkart/stk-code)
[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/supertuxkart/stk-code?svg=true&branch=master)](https://ci.appveyor.com/project/supertuxkart/stk-code)
[![#supertuxkart on the freenode IRC network](https://img.shields.io/badge/freenode-%23supertuxkart-brightgreen.svg)](https://webchat.freenode.net/?channels=supertuxkart)

SuperTuxKart is a free kart racing game. It focuses on fun and not on realistic kart physics. Instructions can be found on the in-game help page.

The SuperTuxKart homepage can be found at <https://supertuxkart.net/>. There is also our [FAQ](https://supertuxkart.net/FAQ) and information on how get in touch with the [community](https://supertuxkart.net/Community)

## Hardware Requirements
* You'll need a graphics card capable of 3D rendering - NVIDIA GeForce 8 series or newer (GeForce 8100 or newer), AMD/ATI Radeon HD 4000 series or newer, Intel HD Graphics 3000 or newer.
* You should have a CPU that's running at 1 GHz or faster. 
* You'll need at least 512 MB of free VRAM (video memory).
* Disk space: 400 MB
* Ideally, you want a joystick with at least 6 buttons.

## License
The software is released under the GNU General Public License (GPL) which can be found in the file [`COPYING`](/COPYING) in the same directory as this file. Information about the licenses for artwork are contained in `data/licenses`.

## 3D coordinates
A reminder for those who are looking at the code and 3D models:

SuperTuxKart: X right, Y up, Z forwards

Blender: X right, Y forwards, Z up

The export utilities  perform the needed transformation, so in Blender you just work with the XY plane as ground, and things will appear fine in STK (using XZ as ground in the code, obviously).

## Building from source

In order to build SuperTuxKart from source, you'll need both the code and the assets (See <https://supertuxkart.net/Source_control> for more information):

```bash
git clone https://github.com/supertuxkart/stk-code
svn checkout https://svn.code.sf.net/p/supertuxkart/code/stk-assets stk-assets
```

## Building on Linux

### Dependencies

Install the following packages:

  * OpenGL (mesa)
  * OpenAL (recommended: openal-soft-devel)
  * Ogg (libogg-dev)
  * Vorbis (libvorbis-dev)
  * Freetype (libfreetype6-dev)
  * libcurl (libcurl-devel)
  * libbluetooth (bluez-devel)
  * libpng (libpng-devel)
  * zlib (zlib-devel)
  * jpeg (libjpeg-turbo-devel)

Ubuntu command:

```bash
sudo apt-get install build-essential cmake libbluetooth-dev \
libcurl4-gnutls-dev libfreetype6-dev libfribidi-dev libgl1-mesa-dev \
libjpeg-dev libogg-dev libopenal-dev libpng-dev libvorbis-dev libxrandr-dev \
mesa-common-dev pkg-config zlib1g-dev
```

### Compiling

Compile SuperTuxKart:

```bash
mkdir cmake_build
cd cmake_build
cmake ..
make -j4
```
STK can then be run from the build directory with `bin/supertuxkart`

### Further options

To create a debug version of STK, use:

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


## Windows

1. Install Visual Studio 2013 (or later). The free express versions work fine.
2. Download and install a source package - either a released package or from our [git/svn repositories](https://supertuxkart.net/Source_control).
3. Download the latest dependency package from [here](https://sourceforge.net/projects/supertuxkart/files/SuperTuxKart%20Dependencies/Windows/). Unzip it in the root directory, so that the dependencies directory is next to the src and data directories (if you are updating from a previous dependency package, you can delete the .dll files in the root directory, they are not needed anymore).
4. Download cmake and install it. Then start cmake-gui and select the STK root directory as 'Where is the source code', and a new directory in the root directory (next to src, data etc) as the build directory (for now I assume that this directory is called bld).
5. Click on configure. You will be asked to create the directory (yes), then for your VS version. Make sure you select the right version (be aware of the easy to confuse version numbers: VS 2013 = version 12). Click on configure, then generate. This will create the directory 'bld', and a VS solution in that directory.
6. In Visual Studio open the project file generated in the 'bld' folder.
7. Right click on the supertuxkart project in the solution explorer, and select "Set as StartUp Project".
8. Select Build->Build Solution (or press F7) to compile.

## Windows (from PowerShell/Command line)
1. Download and install Visual Studio 2013 or newer, the community version works just fine. 
2. Download a source package from either [SuperTuxKart 0.9.2 download area - SourceForge.net](https://sourceforge.net/projects/supertuxkart/files/SuperTuxKart/0.9.2) or [SuperTuxKart.net - Source Control](https://supertuxkart.net/Source_control)
NOTE: the `stk-code` and `stk-assets` directories **must** be in the same directory 
3. Download the Windows dependencies package from either [SuperTuxKart download area - SourceForge.net](https://sourceforge.net/projects/supertuxkart/files/SuperTuxKart%20Dependencies/Windows/)
or [SuperTuxKart on GitHub - Dependencies](https://github.com/supertuxkart/dependencies)
and unpack the archive; once unpacked, copy the `windows` or `windows_64bit` directories into the `stk-code` directory
4. Download CMake from here: [CMake - download page](https://cmake.org/download/); and install it. Navigate to the `stk-code` directory; and create an directory called "build":
````cmd
mkdir build
cd build
````
once inside the build directory; run CMake to start the compilation process:
````cmd
cmake ..
````
*Visual Studio version references: Visual Studio 2013 is version 13. 2015 is version 14, 2017 is version 15*

once CMake finished configuring and creating the necessary files for the build, run the build command in the same directory:
````cmd
msbuild.exe SuperTuxKart.sln 
````
SuperTuxKart can now be run as `bin\Debug\supertuxkart.exe` or `bin\Release\supertuxkart.exe` 
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

```bash
mkdir cmake_build
cd cmake_build
cmake ..
make
```

With GCC:
```bash
mkdir cmake_build
cd cmake_build
cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DCMAKE_C_COMPILER=/usr/bin/gcc
make
```

Building on 10.10 with 10.9 compat:
```bash
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
