//  $Id: Track.cxx,v 1.17 2004/09/24 15:45:02 matzebraun Exp $
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

#include "tuxkart.h"
#include "Loader.h"
#include "TrackData.h"
#include "Track.h"

Track::Track (const TrackData* track_data_)
  : track_data(track_data_)
{
}

Track::~Track()
{
}

extern int check_hint ;

int Track::spatialToTrack ( sgVec3 res, sgVec3 xyz, int hint )
{
  size_t nearest = 0 ;
  float d ;
  float nearest_d = 99999 ;

  /*
    If we don't have a good hint, search all the
    points on the track to find our nearest centerline point
  */

/*
if ( check_hint )
fprintf(stderr,"ih=%d ", hint ) ;
  if ( hint < 0 || hint >= npoints )
*/
  {
    for (size_t i = 0 ; i < track_data->driveline.size() ; ++i )
    {
      d = sgDistanceVec2 ( track_data->driveline[i], xyz ) ;

      if ( d < nearest_d )
      {
        nearest_d = d ;
        nearest = i ;
      }
    }

    hint = nearest ;
/*
if ( check_hint )
fprintf(stderr,"hint=%d\n", hint ) ;
*/
  }

  /*
    Check the two points on the centerline either side
    of the hint.
  */

/*
  int hp = ( hint <=      0     ) ? (npoints-1) : (hint-1) ;
  int hn = ( hint >= (npoints-1)) ?      0      : (hint+1) ;

  float dp = sgDistanceVec2 ( track_data.driveline[ hp ], xyz ) ;
  float d0 = sgDistanceVec2 ( track_data.driveline[hint], xyz ) ;
  float dn = sgDistanceVec2 ( track_data.driveline[ hn ], xyz ) ;

if ( check_hint )
fprintf(stderr,"d=(%f->%f->%f), %d/%d/%d ", dp,d0,dn, hp,hint,hn ) ;

  if ( d0 < dp && d0 < dn )
  {
    nearest   = hint ;
    nearest_d =  d0  ;
  }
  else
  if ( dp < dn )
  {
    nearest   = hp ;
    nearest_d = dp ;
  }
  else
  {
    nearest   = hn ;
    nearest_d = dn ;
  }

if ( check_hint )
fprintf(stderr,"new hint=%d\n", nearest ) ;
*/
  /*
    OK - so we have the closest point
  */

  size_t    prev,  next ;
  float dprev, dnext ;

  prev = ( nearest   ==   0     ) 
          ? track_data->driveline.size() - 1 : (nearest - 1) ;
  next = ( nearest+1 >= track_data->driveline.size() ) 
          ?      0        : nearest + 1 ;

  dprev = sgDistanceVec2 ( track_data->driveline[prev], xyz ) ;
  dnext = sgDistanceVec2 ( track_data->driveline[next], xyz ) ;

  size_t p1, p2 ;
  float d1, d2 ;

  if ( dnext < dprev )
  {
    p1 = nearest   ; p2 =  next ;
    d1 = nearest_d ; d2 = dnext ;
  }
  else
  {
    p1 =  prev ; p2 = nearest   ;
    d1 = dprev ; d2 = nearest_d ;
  }

  sgVec3 line_eqn ;
  sgVec3 tmp ;

  sgMake2DLine ( line_eqn, track_data->driveline[p1], track_data->driveline[p2] ) ;

  res [ 0 ] = sgDistToLineVec2 ( line_eqn, xyz ) ;

  sgAddScaledVec2 ( tmp, xyz, line_eqn, -res [0] ) ;

  res [ 1 ] = sgDistanceVec2 ( tmp, track_data->driveline[p1] ) + track_data->driveline[p1][2] ;

  return nearest ;
}

int Track::absSpatialToTrack ( sgVec3 res, sgVec3 xyz )
{
  return spatialToTrack ( res, xyz, 100000 ) ;
}


void Track::trackToSpatial ( sgVec3 xyz, int hint )
{
  sgCopyVec3 ( xyz, track_data->driveline [ hint ] ) ;
}

void Track::draw2Dview ( float x, float y, float w, float h, bool stretch )
{
  sgVec2 sc ;
  sgSubVec2 ( sc, track_data->driveline_max, track_data->driveline_center ) ;

  float scale_x = w / sc[0] ;
  float scale_y = h / sc[1] ;

  if(stretch == false)
    scale_x = scale_y = std::min(scale_x, scale_y);
 
  glBegin ( GL_LINE_LOOP ) ;
  for ( size_t i = 0 ; i < track_data->driveline.size() ; ++i )
    {
      glVertex2f ( x + ( track_data->driveline[i][0] - track_data->driveline_center[0] ) * scale_x,
                   y + ( track_data->driveline[i][1] - track_data->driveline_center[1] ) * scale_y ) ;
    }
  glEnd () ;
}

/* EOF */
