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

class SkidMark {

 public:
        SkidMark();
       ~SkidMark();
  void add           (sgCoord* coord);  // Add a position where the skidmark is
  void addBreak      (sgCoord *coord);   //Begin or finish an skidmark
  bool wasSkidMarking() const;

private:

  class SkidMarkPos : public ssgVtxTable {
  public:
         SkidMarkPos   ();
         SkidMarkPos  (ssgVertexArray* vertices,
		       ssgNormalArray* normals,
		       ssgColourArray* colors,
		       float global_track_offset);
         ~SkidMarkPos ();
    void recalcBSphere();
    void add          (sgCoord* coord); // Add a position where the skidmark is
    void addEnd       (sgCoord *coord);
  private:
    float track_offset;                 // Amount of which the skidmark is lifted
                                        // above the track to avoid z-buffer errors
  };  // SkidMarkPos

  bool SkidMarking;
  static float globalTrackOffset;       // This is to make Z-fighting between 
                                        // skidmarks less likely, specially the
                                        // skidmarks made by the AI

  std::vector <SkidMarkPos *> SkidMarks;
  ssgSimpleState *skidstate;
};

#endif

/* EOF */
