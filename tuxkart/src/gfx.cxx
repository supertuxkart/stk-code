//  $Id: gfx.cxx,v 1.21 2004/08/15 15:25:07 grumbel Exp $
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

#ifndef WIN32
#include <unistd.h>
#include <string.h>
#include <sys/io.h>
#include <sys/perm.h>                                                           
#endif

#include "tuxkart.h"
#include "Camera.h"
#include "gfx.h"
#include "TrackData.h"
#include "TrackManager.h"
#include "World.h"

GFX::GFX ( int _mirror )
{
  mirror = _mirror ;

  ssgSetFOV ( 75.0f, 0.0f ) ;
  ssgSetNearFar ( 0.05f, 1000.0f ) ;

  sgCoord cam ;
  sgSetVec3 ( cam.xyz, 0, 0, 0 ) ;
  sgSetVec3 ( cam.hpr, 0, 0, 0 ) ;
  ssgSetCamera ( & cam ) ;
}

/* Address of the parallel port. */

#define LPBASE 0x378L

/* -1 left-eye, +1 right-eye, 0 center (ie No Stereo) */

static int stereo = 0 ;


int stereoShift ()
{
  return stereo ;
}


void GFX::done ()
{
  swapBuffers();
  
  glBegin ( GL_POINTS ) ;
  glVertex3f ( 0, 0, 0 ) ;
  glEnd () ;
  glFlush () ;

#ifdef UL_LINUX
  static int firsttime = TRUE ;

  if ( firsttime )
  {
    firsttime = FALSE ;

    if ( getenv ( "TUXKART_STEREO" ) == NULL )
    {
      stereo = 0 ;
      return ;
    }

    fprintf ( stderr, "Requesting control of parallel printer port...\n" ) ;
 
    int res = ioperm ( LPBASE, 8, 1 ) ;
 
    if ( res != 0 )
    {
      perror ( "parport" ) ;
      fprintf ( stderr, "Need to run as 'root' to get stereo.\n" ) ;
      stereo = 0 ;
    }
    else
    {
      fprintf ( stderr, "Stereo Enabled!\n" ) ;
      stereo = -1 ;
    }
  }

  if ( stereo != 0 )
  {
    outb ( (stereo==-1) ? ~3 : ~2, LPBASE+2 ) ;
    stereo = -stereo ;
  }
#endif
}

/* EOF */
