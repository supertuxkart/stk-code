//  $Id$
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

#include <plib/sg.h>

class Track
{
public:
  std::string ident;

  std::string name;
  std::string music_filename;

  sgVec4 sky_color;

  bool   use_fog;
  sgVec4 fog_color;
  float  fog_density;
  float  fog_start;
  float  fog_end;

  /** Position of the sun */
  sgVec3 sun_position;

  sgVec4 ambientcol;
  sgVec4 specularcol;
  sgVec4 diffusecol;

  /** sgVec3 is a float[3] array, so unfortunately we can't put it in a
   * std::vector because it lacks a copy-constructor, this hack should help...
   */
  class sgVec3Wrapper
  {
  private:
    sgVec3 vec;

  public:
    sgVec3Wrapper(const sgVec3& o) {
      sgCopyVec3(vec, o);
    }

    operator const float* () const {
      return vec;
    }

    operator float* () {
      return vec;
    }
  };
  std::vector<sgVec3Wrapper> driveline;

  sgVec2 driveline_min;
  sgVec2 driveline_max;
  sgVec2 driveline_center;

private:
  float total_distance;

public:
  Track(const std::string& filename);
  ~Track();

  int  absSpatialToTrack ( sgVec2 dst, sgVec3 xyz ) const ;
  int  spatialToTrack ( sgVec2 last_pos, sgVec3 xyz, int hint ) const ;
  void trackToSpatial ( sgVec3 xyz, int last_hint ) const ;

  float getTrackLength () const
  { return total_distance ; }

  const std::string& getIdent() const
  { return ident; }

  /* Draw track preview.   Warning: w and h is not in pixels. */
  void draw2Dview ( float x, float y, float w, float h, bool stretch ) const ;

private:
  void loadTrack(const std::string& filename);
  void loadDriveline();
};

#endif
