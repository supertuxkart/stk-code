# SuperTuxKart
[![Linux build status](https://github.com/supertuxkart/stk-code/actions/workflows/linux.yml/badge.svg)](https://github.com/supertuxkart/stk-code/actions/workflows/linux.yml)
[![Apple build status](https://github.com/supertuxkart/stk-code/actions/workflows/apple.yml/badge.svg)](https://github.com/supertuxkart/stk-code/actions/workflows/apple.yml)
[![Windows build status](https://github.com/supertuxkart/stk-code/actions/workflows/windows.yml/badge.svg)](https://github.com/supertuxkart/stk-code/actions/workflows/windows.yml)
[![Switch build status](https://github.com/supertuxkart/stk-code/actions/workflows/switch.yml/badge.svg)](https://github.com/supertuxkart/stk-code/actions/workflows/switch.yml)
[![#supertuxkart on the libera IRC network](https://img.shields.io/badge/libera-%23supertuxkart-brightgreen.svg)](https://web.libera.chat/?channels=#supertuxkart)

SuperTuxKart is a free kart racing game. It focuses on fun and not on realistic kart physics. Instructions can be found on the in-game help page.

The SuperTuxKart homepage can be found at <https://supertuxkart.net/>. There is also our [FAQ](https://supertuxkart.net/FAQ) and information on how get in touch with the [community](https://supertuxkart.net/Community).

Latest release binaries can be found [here](https://github.com/supertuxkart/stk-code/releases/latest), and preview release [here](https://github.com/supertuxkart/stk-code/releases/preview).

## Hardware Requirements
To run SuperTuxKart, make sure that your computer's specifications are equal or higher than the following specifications:

* A graphics card capable of 3D rendering - NVIDIA GeForce 470 GTX, AMD Radeon 6870 HD series card or Intel HD Graphics 4000 and newer. OpenGL >= 3.3
* You should have a dual-core CPU that's running at 1 GHz or faster.
* You'll need at least 512 MB of free VRAM (video memory).
* System memory: 1 GB
* Minimum disk space: 700 MB
* Ideally, you'll want a joystick with at least 6 buttons.

## License
The software is released under the GNU General Public License (GPL) which can be found in the file [`COPYING`](/COPYING) in the same directory as this file.

---

## 3D coordinates
A reminder for those who are looking at the code and 3D models:

SuperTuxKart: X right, Y up, Z forwards

Blender: X right, Y forwards, Z up

The export utilities  perform the needed transformation, so in Blender you just work with the XY plane as ground, and things will appear fine in STK (using XZ as ground in the code, obviously).

## Building from source

Building instructions can be found in [`INSTALL.md`](/INSTALL.md)
