//  $Id: pwdrv.h,v 1.2 2004/07/31 11:35:14 grumbel Exp $
// 
//  TuxKart - A Fun Gokart Game
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#ifndef HEADER_PWDRW_H
#define HEADER_PWDRV_H

#include <plib/pw.h>

void pollEvents();
void swapBuffers();
void keystroke( int key, int updown, int, int );
int  isKeyDown( unsigned int k );
int  getKeystroke();
void reshape( int w, int h );
void initVideo(int w, int h, bool fullscreen);
void shutdown();
int getScreenWidth  ();
int getScreenHeight ();
void motionfn ( int x, int y ) ;
void mousefn ( int button, int updown, int x, int y ) ;

#endif

/* EOF */
