//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
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
#include <limits.h>

#include "utils.h"
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
  hpr[0] = -SG_RADIANS_TO_DEGREES * atan2 ( nrm[0], nrm[1] ) ;
  pr_from_normal ( hpr, nrm ) ;
}

float getHeightAndNormal(ssgBranch* branch, sgVec3 my_position, sgVec3 normal)
{
  /* Look for the nearest polygon *beneath* my_position */

  ssgHit *results ;
  int num_hits ;

  float hot ;        /* H.O.T == Height Of Terrain */
  sgVec3 HOTvec ;

  sgMat4 invmat ;
  sgMakeIdentMat4 ( invmat ) ;
  invmat[3][0] = - my_position [0] ;
  invmat[3][1] = - my_position [1] ;
  invmat[3][2] = 0.0 ;

  sgSetVec3 ( HOTvec, 0.0f, 0.0f, my_position [ 2 ] ) ;

  num_hits = ssgHOT (branch, HOTvec, invmat, &results ) ;
  
  hot = - FLT_MAX ;

  for ( int i = 0 ; i < num_hits ; i++ )
  {
    ssgHit *h = &results [ i ] ;

    float hgt = - h->plane[3] / h->plane[2] ;

    if ( hgt >= hot )
    {
      hot = hgt ;

      if ( normal != NULL )
        sgCopyVec3 ( normal, h->plane ) ;
    }
  }

  return hot ;
}

/* EOF */
