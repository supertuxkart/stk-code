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

#include <iostream>

#include "World.h"
#include "SkidMark.h"

float SkidMark::globalTrackOffset = 0.005f;

SkidMark::SkidMark()
#if 0
: ssgVtxTable ( GL_QUAD_STRIP,
                  new ssgVertexArray,
                  new ssgNormalArray,
                  new ssgTexCoordArray,
                  new ssgColourArray )
#endif
{
  skidstate = new ssgSimpleState ();
  skidstate -> enable (GL_BLEND);
  //This is just for the skidmarks, so the ones drawn when the kart is in
  //reverse get drawn
  skidstate -> disable (GL_CULL_FACE);
#if 1
  SkidMarking = false;

#endif
#if 0
  newSkidmark = 1;
#endif
}

SkidMark::~SkidMark()
{
#if 1
  if(!SkidMarks.empty())
  {
      const unsigned int size = SkidMarks.size() -1;
      for(unsigned int i = 0; i < size; ++i)
      {
          ssgDeRefDelete(SkidMarks[i]);
      }
  }
#endif
}

SkidMark::SkidMarkPos::SkidMarkPos() : ssgVtxTable ( GL_QUAD_STRIP,
                                                     new ssgVertexArray,
                                                     new ssgNormalArray,
                                                     new ssgTexCoordArray,
                                                     new ssgColourArray )
{
}

SkidMark::SkidMarkPos::SkidMarkPos( ssgVertexArray* vertices,
                                    ssgNormalArray* normals,
                                    ssgColourArray* colors,
                                    float global_track_offset) :
                                    ssgVtxTable ( GL_QUAD_STRIP,
                                                  vertices,
                                                  normals,
                                                  new ssgTexCoordArray,
                                                  colors )
{
  track_offset = global_track_offset;
}

SkidMark::SkidMarkPos::~SkidMarkPos()
{
}

void
SkidMark::add(sgCoord* coord)
{
#if 1
  assert(SkidMarking);

  const int current = SkidMarks.size() - 1;
  SkidMarks[current]->add(coord);
}

void
SkidMark::SkidMarkPos::add(sgCoord* coord)
{
  // Width of the skidmark
  const float width = 0.1f;

  sgVec3 pos;
  sgSetVec3(pos,
            coord->xyz[0] + sgSin(coord->hpr[0]-90) * width,
            coord->xyz[1] - sgCos(coord->hpr[0]-90) * width,
            coord->xyz[2] + track_offset);
  vertices->add(pos);

  sgSetVec3(pos,
            coord->xyz[0] + sgSin(coord->hpr[0]+90) * width,
            coord->xyz[1] - sgCos(coord->hpr[0]+90) * width,
            coord->xyz[2] + track_offset);
  vertices->add(pos);

  sgVec3 norm;
  sgSetVec3(norm, 0, 0, 1);
  normals->add(norm); normals->add(norm);

  sgVec4 color;
  sgSetVec4(color, 0.07f, 0.07f, 0.07f, 0.8f);
  colours->add(color); colours->add(color);
#endif
#if 0
  sgVec3 pos;
  sgVec3 norm;
  sgVec4 color;
  // Amount of which the skidmark is lifted above the track to avoid
  // z-buffer errors
  float track_offset = 0.03f;

  // Width of the skidmark
  float width = 0.1f;

  sgSetVec3(pos,
            coord->xyz[0] + sgSin(coord->hpr[0]-90) * width,
            coord->xyz[1] - sgCos(coord->hpr[0]-90) * width,
            coord->xyz[2] + track_offset);
  vertices->add(pos);

  sgSetVec3(pos,
            coord->xyz[0] + sgSin(coord->hpr[0]+90) * width,
            coord->xyz[1] - sgCos(coord->hpr[0]+90) * width,
            coord->xyz[2] + track_offset);
  vertices->add(pos);


  sgSetVec3(norm, 0, 0, 1);
  sgSetVec4(color, 0, 0, 0, .5);

  normals->add(norm); normals->add(norm);
  colours->add(color); colours->add(color);
#endif
}

void
SkidMark::addBreak(sgCoord* coord)
{
#if 1
  const unsigned int current = SkidMarks.size() - 1;
  if(SkidMarking)
    SkidMarks[current]->addEnd(coord);
  else
  {
    globalTrackOffset += 0.005f;
    if(globalTrackOffset > 0.05f) globalTrackOffset = 0.01f;

    // Width of the skidmark
    const float width = 0.1f;

    sgVec3 pos;
    sgSetVec3(pos,
            coord->xyz[0] + sgSin(coord->hpr[0]-90) * width,
            coord->xyz[1] - sgCos(coord->hpr[0]-90) * width,
            coord->xyz[2] + globalTrackOffset);
    ssgVertexArray* SkidMarkVertices = new ssgVertexArray;
    SkidMarkVertices->add(pos);

    sgSetVec3(pos,
            coord->xyz[0] + sgSin(coord->hpr[0]+90) * width,
            coord->xyz[1] - sgCos(coord->hpr[0]+90) * width,
            coord->xyz[2] + globalTrackOffset);
    SkidMarkVertices->add(pos);

    sgVec3 norm;
    sgSetVec3(norm, 0, 0, 1);

    ssgNormalArray* SkidMarkNormals = new ssgNormalArray;
    SkidMarkNormals->add(norm);
    SkidMarkNormals->add(norm);

    sgVec4 color;
    sgSetVec4(color, 0, 0, 0, 1);

    ssgColourArray* SkidMarkColors = new ssgColourArray;
    SkidMarkColors->add(color);
    SkidMarkColors->add(color);

    SkidMarkPos* newSkidMark = new SkidMarkPos( SkidMarkVertices,
                                              SkidMarkNormals,
                                              SkidMarkColors,
                                              globalTrackOffset);
    newSkidMark->ref();
    world->scene->addKid(newSkidMark);
    newSkidMark-> setState (skidstate);

    SkidMarks.push_back(newSkidMark);
  }

  SkidMarking = !SkidMarking;
}

void
SkidMark::SkidMarkPos::addEnd(sgCoord* coord)
{
  // Width of the skidmark
  const float width = 0.1f;

  sgVec3 pos;
  sgSetVec3(pos,
            coord->xyz[0] + sgSin(coord->hpr[0]-90) * width,
            coord->xyz[1] - sgCos(coord->hpr[0]-90) * width,
            coord->xyz[2] + track_offset);
  vertices->add(pos);

  sgSetVec3(pos,
            coord->xyz[0] + sgSin(coord->hpr[0]+90) * width,
            coord->xyz[1] - sgCos(coord->hpr[0]+90) * width,
            coord->xyz[2] + track_offset);
  vertices->add(pos);

  sgVec3 norm;
  sgSetVec3(norm, 0, 0, 1);
  normals->add(norm); normals->add(norm);

  sgVec4 color;
  sgSetVec4(color, 0.15f, 0.15f, 0.15f, 0.0f);
  colours->add(color); colours->add(color);
}
#endif
#if 0
  sgVec3 pos;
  sgVec3 norm;
  sgVec4 color;
  // Amount of which the skidmark is lifted above the track to avoid
  // z-buffer errors
  float track_offset = 0.03f;

  // Width of the skidmark
  float width = 0.1f;

  sgSetVec3(pos,
            coord->xyz[0] + sgSin(coord->hpr[0]-90) * width,
            coord->xyz[1] - sgCos(coord->hpr[0]-90) * width,
            coord->xyz[2] + track_offset);
  vertices->add(pos);

  sgSetVec3(pos,
            coord->xyz[0] + sgSin(coord->hpr[0]+90) * width,
            coord->xyz[1] - sgCos(coord->hpr[0]+90) * width,
            coord->xyz[2] + track_offset);
  vertices->add(pos);


  sgSetVec3(norm, 0, 0, 1);
  sgSetVec4(color, 0, 0, 0, .0);

  normals->add(norm); normals->add(norm);
  colours->add(color); colours->add(color);
}
#endif

#if 0
void
SkidMark::recalcBSphere()
{
  bsphere . setRadius ( 1000.0f ) ;
  bsphere . setCenter ( 0, 0, 0 ) ;
}
#endif

#if 1
void
SkidMark::SkidMarkPos::recalcBSphere()
{
  bsphere . setRadius ( 1000.0f ) ;
  bsphere . setCenter ( 0, 0, 0 ) ;
}

bool
SkidMark::wasSkidMarking() const
{
  return SkidMarking;
}
#endif
/* EOF */
