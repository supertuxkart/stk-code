//  $Id: Track.h,v 1.9 2004/08/14 23:25:19 grumbel Exp $
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

#ifndef HEADER_TRACK_H
#define HEADER_TRACK_H

#include <vector>
#include "plibwrap.h"

#define TRACKVIEW_SIZE 100.0f

class Track
{
private:
  sgVec2 min ;
  sgVec2 max ;
  sgVec2 center ;
  float  scale ;

  float  total_distance ;

  std::vector<sgVec3Wrap> driveline;

public:
  Track ( const char *drv_fname, int mirror, int reverse ) ;

  void glVtx ( sgVec2 v, float xoff, float yoff )
  {
    glVertex2f ( xoff + ( v[0] - center[0] ) * scale,
                 yoff + ( v[1] - center[1] ) * scale ) ;
  }

  int  absSpatialToTrack ( sgVec2 dst, sgVec3 xyz ) ;
  int  spatialToTrack ( sgVec2 last_pos, sgVec3 xyz, int hint ) ;
  void trackToSpatial ( sgVec3 xyz, int last_hint ) ;

  float getTrackLength () { return total_distance ; }
  void draw2Dview ( float x, float y ) ;
} ;


extern Track *curr_track ;

#endif

/* EOF */
