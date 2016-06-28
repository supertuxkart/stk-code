# SuperTuxKart
[![Build Status](https://travis-ci.org/supertuxkart/stk-code.png?branch=master)](https://travis-ci.org/supertuxkart/stk-code)

SuperTuxKart is a free kart racing game. It focuses on fun and not on realistic kart physics. Instructions can be found on the in-game help page.

The SuperTuxKart homepage can be found at: <https://supertuxkart.net/>

The official SuperTuxKart forum is at <http://forum.freegamedev.net/viewforum.php?f=16>. If you need support, this would be the best place to start.

Hope you enjoy the game.

-- The SuperTuxKart development team.

## Hardware Requirements
* You need a 3D graphics card. (NVIDIA GeForce 8xxx and higher, ATI Radeon HD 4xxx and higher or Intel HD 3000 and higher.)
* You should have a CPU that's running at 1GHz or better.
* You'll need at least 512 MB of free VRAM (video memory).
* Disk space: 400MB
* Ideally, you want a joystick with at least 6 buttons.

## Compiling SuperTuxKart

### Windows
1. Install VS 2013 (or later). The free express versions work fine.
2. Download and install a source package - either a released package or from our [git/svn repositories](https://supertuxkart.net/Source_control).
3. Download the latest dependency package from [here](https://sourceforge.net/projects/supertuxkart/files/SuperTuxKart%20Dependencies/Windows/). Unzip it in the root directory, so that the dependencies directory is next to the src and data directories (if you are updating from a previous dependency package, you can delete the .dll files in the root directory, they are not needed anymore).
4. Download cmake and install it. Then start cmake-gui and select the STK root directory as 'Where is the source code', and a new directory in the root directory (next to src, data etc) as the build directory (for now I assume that this directory is called bld).
5. Click on configure. You will be asked to create the directory (yes), then for your VS version. Make sure you select the right version (be aware of the easy to confuse version numbers: VS 2013 = version 12). Click on configure, then generate. This will create the directory 'bld', and a VS solution in that directory.
6. In Visual Studio open the project file generated in the 'bld' folder.
7. Right click on the supertuxkart project in the solution explorer, and select "Set as StartUp Project".
8. Select Build->Build Solution (or press F7) to compile.

Compilation with cygwin is not officially supported, but this has been done (check with the forum for details).

### Mac OS X
The latest information about compilation on Mac are on our wiki: <https://supertuxkart.net/Building_and_packaging_on_OSX>

### UNIX
See [`INSTALL.md`](INSTALL.md) for details.

## License
This software is released under the GNU General Public License (GPL) which can be found in the file [`COPYING`](/COPYING) in the same directory as this file. Information about the licenses for artwork are contained in `data/licenses`.

## 3D coordinates
A reminder for those looking at the code and 3d models:

STK    : X right, Y up,       Z forwards

Blender: X right, Y forwards, Z up

The exporters perform the needed transform, so in Blender you just work with XY plane as ground, and things will appear fine in STK (using XZ as ground in the code, obviously).
