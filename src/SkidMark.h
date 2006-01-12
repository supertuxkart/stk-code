//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_SKIDMARK_CXX
#define HEADER_SKIDMARK_CXX

#include <vector>

#include <plib/ssg.h>

#if 0
class SkidMark : public ssgVtxTable
#endif
class SkidMark
{
public:
  SkidMark();
  ~SkidMark();

  // Add a position where the skidmark is
  void add(sgCoord* coord);
  //Begin or finish an skidmark
  void addBreak (sgCoord *coord);

#if 0
  void recalcBSphere();

  int newSkidmark;
#endif
#if 1
  bool wasSkidMarking() const;
private:

  class SkidMarkPos : public ssgVtxTable
  {
  public:
    SkidMarkPos();
    SkidMarkPos( ssgVertexArray* vertices,
                 ssgNormalArray* normals,
                 ssgColourArray* colors,
                 float global_track_offset);
    ~SkidMarkPos();

    void recalcBSphere();

    // Add a position where the skidmark is
    void add(sgCoord* coord);
    void addEnd (sgCoord *coord);
  private:
    //Amount of which the skidmark is lifted above the track to avoid
    //z-buffer errors
    float track_offset;
  };

  bool SkidMarking;
  //This is to make Z-fighting between skidmarks less likely, specially
  //the skidmarks made by the AI
  static float globalTrackOffset;
  std::vector <SkidMarkPos *> SkidMarks;
  ssgSimpleState *skidstate;
#endif
};

#endif

/* EOF */
