//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2006 Steve Baker <sjbaker1@airmail.net>
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

#ifndef HEADER_PLIBDRV_H
#define HEADER_PLIBDRV_H

#define MAXKEYS 512

void InitPlib();
void keyfn ( int key, int updown, int x, int );
void gui_mousefn ( int button, int updown, int x, int y );
void gui_motionfn ( int button, int updown );
void keystroke ( int key, int updown, int x, int y ) ;
int  isKeyDown ( unsigned int k ) ;
int  getKeystroke () ;
void pollEvents() ;
void printkeys();
#endif
