//  $Id: sdldrv.h,v 1.8 2004/08/08 16:35:27 jamesgregory Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 James Gregory <james.gregory@btinternet.com>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_SDLDRV_H
#define HEADER_SDLDRV_H

#include <SDL.h>
#include <map>

enum KartControl {KC_LEFT, KC_RIGHT, KC_UP, KC_DOWN, KC_WHEELIE, KC_JUMP, KC_RESCUE, KC_FIRE};

struct ControlConfig
{
	std::map<KartControl, SDLKey> keys;
	bool useJoy;
};

extern Uint8 *keyState;

void initVideo (int w, int h, bool fullscreen);
void shutdownVideo();
void pollEvents();
void kartInput();
void swapBuffers();
int  getScreenWidth();
int  getScreenHeight();

void setupControls();

#endif

/* EOF */
