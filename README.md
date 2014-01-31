#SuperTuxKart
[![Build Status](https://travis-ci.org/supertuxkart/stk-code.png?branch=master)](https://travis-ci.org/supertuxkart/stk-code)

SuperTuxKart is a free kart racing game. It is focusing on fun and
not on realistic kart physics. Instruction can be found on the
in-game help page.

The SuperTuxKart homepage can be found at: <http://supertuxkart.sourceforge.net>

The official SuperTuxKart forum is at <http://supertuxkart.sourceforge.net/forum>. If you need support,
this would be the best place to start.

Hope you enjoy the game.

-- The SuperTuxKart development team.


##Hardware Requirements
* You need a 3D graphics card. (NVIDIA GeForce 8xxx and higher, ATI Radeon HD 4xxx and higher or Intel HD 3000 and higher)
* You should have a CPU that's running at 1GHz or better.
* You'll need at least 512 MB of free VRAM (video memory).
* Disk space: 400MB
* Ideally, you want a joystick with at least 6 buttons.


##Compiling SuperTuxKart

###Windows
A project file for Visual Studio 9 (e.g. the free 2008 express 
edition) is included in the sources in `src/ide/vc9`. A separate
dependency package is available on SuperTuxKart's sourceforge 
page, which includes all necessary libraries, header files, 
and dlls to compile and run the source code.

While compilation with cygwin is not officially supported,
this has been done (check with the forum for details).

###Mac OS X
The latest information about compilation on Mac are on our wiki:
<http://supertuxkart.sourceforge.net/Building_and_packaging_on_OSX>
The Xcode project file is in `/src/ide/Xcode/`, but it still 
requires that all dependencies are installed as explained on the wiki.

###UNIX
See `INSTALL` for details.


##License
This software is released under the GNU General Public License (GPL) which
can be found in the file `LICENSE` in the same directory as this file.
Information about the licenses for artwork are contained in 
`data/licenses`.


##3D coordinates
A reminder for those looking at the code and 3d models:

STK    : X right, Y up,       Z forwards

Blender: X right, Y forwards, Z up

The exporters perform the needed transform, so in Blender you just work
with XY plane as ground, and things will appear fine in STK (using XZ
as ground in the code, obviously).
