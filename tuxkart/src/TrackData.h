//  $Id: TrackData.h,v 1.8 2004/09/24 15:45:02 matzebraun Exp $
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

#ifndef HEADER_TRACKDATA_H
#define HEADER_TRACKDATA_H

#include <string>
#include <vector>
#include <plib/sg.h>
#include "plibwrap.h"

/** */
class TrackData
{
public:
  std::string ident;
  std::string filename;
  std::string drv_filename;
  std::string loc_filename;

  std::string music_filename;

  std::string name;
  sgVec4 sky_color;

  bool   use_fog;
  sgVec4 fog_color;
  float  fog_density;
  float  fog_start;
  float  fog_end;

  // FIXME: Should we allow multiple lights?

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
  float total_distance;

private:
  void loadDriveline();

public:
  TrackData(const std::string& filename);
  ~TrackData();
};

#endif

/* EOF */
