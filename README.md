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

## To do 
Here is a list of things to do or correct 

### Bug:
	* The second sub-mode of the zombie tag game (tag-zombie survivor powerful) doesn't work (the game ends much too early isRaceOver() method)
	* When gifts, banners, nitro are disabled, it doesn't always work 
	* Very recent bug: reloadable powers no longer work as they should. Just zips to infinity, which isn't normal. 
	* The code for the victory counters of the Team-Arena game modes (in world.cpp) depending on the team chosen is commented out as the code is causing problems. 
	* Victory screen information seems to be mixed up (inaccurate) 
	* Certain powers, such as the parachute and the anvil, mismanage the team condition (don't touch a member of your team).
	* When you choose your team online with the selection screen, the kart is always tux (we never managed to display the right kart)

### Improvement:
	* When hit (the Barril on the ground), the kart should explode instead of receiving a hitBanana() effect.
	* When a game is over, the victory animation is not played (only the defeat animation is played at all times).
	* Ensure that our game modes have acceptable default values for online mode (e.g.: nb the tag = 1, nb life =3, etc...)
	* Improve the third sub-game mode of team-arena-battle (player with the most points) (points player) to make it better and more fun to play.
	* In the overpowered survivor game mode, survivors should have lots of items at once, like 10-15 zippers, or 10-15 anvils, etc...
	* When a kart becomes a zombie, its kart should turn green. It works locally (a kart is recreated and the old one is deleted) but online it sometimes causes problems (the player is disconnected and reconnected very quickly). 

### Additional features:
	* Added new powers such as the tenis ball, the baseball, the voleyball or the oil puddle.
	* Added urn item. This item appears when a player is transformed into a zombie (when the survivor dies). The urn contains a reward. 
	* Added a mini surprise event system for our game mode. Like reducing the recharge time of player powers temporarily for 20s, or disabling the mini-map temporarily for 15s, or etc...
	* Power selection menu (which power will be available in the gift or in the banana)
	* Addition of a fog system or a darkening system (darkness with little light) to give a better atmosphere to the zombie game mode.
	* Added achievements related to our game modes or related to the deactivation of gifts, nitro, banners, etc.

## Important 
	* Create an installable (doesn't work after all) by (at most) December 17 in the afternoon.
