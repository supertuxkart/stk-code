//  $Id: TrackData.h,v 1.2 2004/08/10 16:59:19 grumbel Exp $
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
#include <plib/sg.h>

/** */
class TrackData
{
public:
  std::string name;
  sgVec3 skycolor;

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

public:
  TrackData();
};

#endif

/* EOF */
