//  $Id: utils.cxx,v 1.5 2004/08/25 13:26:14 grumbel Exp $
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

#include <plib/sg.h>
#include "tuxkart.h"

void pr_from_normal ( sgVec3 hpr, sgVec3 nrm )
{
  float sy = sin ( -hpr [ 0 ] * SG_DEGREES_TO_RADIANS ) ;
  float cy = cos ( -hpr [ 0 ] * SG_DEGREES_TO_RADIANS ) ;

  hpr[2] =  SG_RADIANS_TO_DEGREES * atan2 ( nrm[0] * cy - nrm[1] * sy, nrm[2] ) ;
  hpr[1] = -SG_RADIANS_TO_DEGREES * atan2 ( nrm[1] * cy + nrm[0] * sy, nrm[2] ) ;
}


void hpr_from_normal ( sgVec3 hpr, sgVec3 nrm )
{
  pr_from_normal ( hpr, nrm ) ;
  hpr[0] = -SG_RADIANS_TO_DEGREES * atan2 ( nrm[0], nrm[1] ) ;
}


bool canAccess ( char *fname )
{
#ifdef WIN32 
  return _access ( fname, 04 ) == 0 ;
#else
  return access ( fname, F_OK ) == 0 ;
#endif
}


bool chDir ( char *dir )
{
#ifdef WIN32 
  return _chdir ( dir ) == -1 ;
#else
  return chdir ( dir ) == -1 ;
#endif
}


void secondSleep ( int s )
{
#ifdef WIN32 
  Sleep ( 1000 * s ) ;
#else
  sleep ( s ) ;
#endif
}


