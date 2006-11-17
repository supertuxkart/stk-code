//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, Steve Baker
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

#ifndef HEADER_KARTCONTROL_H
#define HEADER_KARTCONTROL_H
#include <plib/js.h>

struct KartControl
{
    float data [ _JS_MAX_AXES ];
    int   buttons;
    int   presses;
    int   releases;
    float lr;
    float accel;
    bool  brake;
    bool  wheelie;
    bool  jump;
    bool  rescue;
    bool  fire;

    KartControl() : buttons(0), presses(0),
            releases(0), lr(0.0f), accel(0.0f), brake(false),
    wheelie(false), jump(false),  rescue(false), fire(false){}}
;

#endif
