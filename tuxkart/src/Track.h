//  $Id: Track.h,v 1.11 2004/08/17 11:58:00 grumbel Exp $
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

#define TRACKVIEW_SIZE 150.0f

#include "TrackData.h"

class Track
{
private:
  TrackData track_data;
  sgVec2 min ;
  sgVec2 max ;
  sgVec2 center ;
  float  scale ;

  float  total_distance ;

public:
  Track ( const TrackData& track_data, int mirror, int reverse ) ;

  int  absSpatialToTrack ( sgVec2 dst, sgVec3 xyz ) ;
  int  spatialToTrack ( sgVec2 last_pos, sgVec3 xyz, int hint ) ;
  void trackToSpatial ( sgVec3 xyz, int last_hint ) ;

  float getTrackLength () { return total_distance ; }
  void draw2Dview ( float x, float y ) ;
};

#endif

/* EOF */
