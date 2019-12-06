//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#ifndef HEADER_KEYBOARD_STATIC_KEYS_HPP
#define HEADER_KEYBOARD_STATIC_KEYS_CONFIG_HPP

#include <SKeyMap.h>

using namespace irr;

const int KBD_CTRL  = 0x100;
const int KBD_SHIFT = 0x200; 

//----------------------------------------
// Keys for normal mode
const int KBD_KEY_SCREENSHOT  = IRR_KEY_PRINT;
const int KBD_KEY_REWIND      = IRR_KEY_F11;
const int KBD_KEY_SAVE_REPLAY = IRR_KEY_F10;
const int KBD_KEY_SHOW_FPS    = IRR_KEY_F12;

//----------------------------------------
// Keys for artist debug mode
const int KBD_KEY_DEBUG_KART_FLY_UP                         = IRR_KEY_I;
const int KBD_KEY_DEBUG_KART_FLY_DOWN                       = IRR_KEY_K;
const int KBD_KEY_DEBUG_CAMERA_MOVE_UP                      = IRR_KEY_W;
const int KBD_KEY_DEBUG_CAMERA_MOVE_DOWN                    = IRR_KEY_S;
const int KBD_KEY_DEBUG_CAMERA_MOVE_RIGHT                   = IRR_KEY_R;
const int KBD_KEY_DEBUG_CAMERA_MOVE_LEFT                    = IRR_KEY_F;
const int KBD_KEY_DEBUG_CAMERA_MOVE_FORWARD                 = IRR_KEY_A;
const int KBD_KEY_DEBUG_CAMERA_MOVE_BACKWARD                = IRR_KEY_D;
const int KBD_KEY_DEBUG_CAMERA_ROTATE_CLOCKWISE             = IRR_KEY_Q;
const int KBD_KEY_DEBUG_CAMERA_ROTATE_COUNTER_CLOCKWISE     = IRR_KEY_E;
// - Following keys are used with Control
const int KBD_KEY_DEBUG_CAMERA_FREE              = KBD_CTRL + IRR_KEY_F1;
const int KBD_KEY_DEBUG_CAMERA_NORMAL            = KBD_CTRL + IRR_KEY_F2;
const int KBD_KEY_DEBUG_RELOAD_TEXTURE           = KBD_CTRL + IRR_KEY_F3;
const int KBD_KEY_DEBUG_FOLLOW_PREVIOUS_KART     = KBD_CTRL + IRR_KEY_F5;
const int KBD_KEY_DEBUG_FOLLOW_NEXT_KART         = KBD_CTRL + IRR_KEY_F6;

#endif
