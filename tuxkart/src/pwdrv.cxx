//  $Id: pwdrv.cxx,v 1.5 2004/08/01 14:17:42 grumbel Exp $
//
//  TuxKart - a fun racing game with go-kart
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

#include "pwdrv.h"
#include "tuxkart.h"
#include "status.h"
#include <plib/pu.h>

static int width  = 800 ;
static int height = 600 ;
static int mouse_x ;
static int mouse_y ;
static int mouse_dx = 0 ;
static int mouse_dy = 0 ;
static int mouse_buttons = 0 ;
static char keyIsDown [ 512 ] ;
static unsigned int lastKeystroke = 0 ;

void pollEvents()
{
}

void swapBuffers()
{
  pwSwapBuffers();
}

void keystroke ( int key, int updown, int, int )
{
  if ( updown == PW_DOWN )
    lastKeystroke = key ;

  keyIsDown [ key ] = (updown == PW_DOWN) ;
}

int isKeyDown ( unsigned int k )
{
  return keyIsDown [ k ] ;
}

int getKeystroke ()
{
  int k = lastKeystroke ;
  lastKeystroke = 0 ;
  return k ;
}

void reshape ( int w, int h )
{
  width  = w ;
  height = h ;
  glViewport ( 0, 0, w, h ) ;
}

void motionfn ( int x, int y )
{
  mouse_x = x ;
  mouse_y = y ;
  mouse_dx += mouse_x - 320 ;
  mouse_dy += mouse_y - 240 ;
  puMouse ( x, y ) ;
}


void mousefn ( int button, int updown, int x, int y )
{
  mouse_x = x ;
  mouse_y = y ;

  if ( updown == PW_DOWN )
    mouse_buttons |= (1<<button) ;
  else
    mouse_buttons &= ~(1<<button) ;

  mouse_dx += mouse_x - 320 ;
  mouse_dy += mouse_y - 240 ;

  puMouse ( button, updown, x, y ) ;

  if ( updown == PW_DOWN )
    hide_status () ;
}

/*********************************\
*                                 *
* These functions capture mouse   *
* and keystrokes and pass them on *
* to PUI.                         *
*                                 *
\*********************************/

static void startupKeyFn ( int key, int updown, int, int )
{
  puKeyboard ( key, updown ) ;
}

static void startupMotionFn ( int x, int y )
{
  puMouse ( x, y ) ;
}

static void startupMouseFn ( int button, int updown, int x, int y )
{
  puMouse ( button, updown, x, y ) ;
}

void initVideo(int w, int h, bool fullscreen)
{
  width  = w;
  height = h;
  pwInit  ( 0, 0, getScreenWidth(), getScreenHeight(), FALSE, 
            "TuxKart " VERSION, 1, 0 ) ;
  pwSetCallbacks ( startupKeyFn, startupMouseFn, startupMotionFn, NULL, NULL ) ;

  for ( int i = 0 ; i < 512 ; i++ )
    keyIsDown [ i ] = FALSE ;
}

void shutdown()
{
  exit(0);
}

int getScreenWidth  () 
{
 return width  ; 
}

int getScreenHeight () 
{
 return height ; 
}

/* EOF */
