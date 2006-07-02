//  $Id: SkidMark.cxx 518 2006-01-12 04:54:56Z coz $
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

#include "world.hpp"
#include "skid_mark.hpp"

float SkidMark::globalTrackOffset = 0.005f;

SkidMark::SkidMark() {
  skidstate = new ssgSimpleState ();
  skidstate -> enable (GL_BLEND);
  //This is just for the skidmarks, so the ones drawn when the kart is in
  //reverse get drawn
  skidstate -> disable (GL_CULL_FACE);
  SkidMarking = false;
}   // SkidMark

// -----------------------------------------------------------------------------
SkidMark::~SkidMark() {
  if(!SkidMarks.empty()) {
    const unsigned int size = SkidMarks.size() -1;
    for(unsigned int i = 0; i < size; ++i) {
      ssgDeRefDelete(SkidMarks[i]);
    }   // for
  }   // if !empty
}   // ~SkidMark

// -----------------------------------------------------------------------------
void SkidMark::add(sgCoord* coord) {
  assert(SkidMarking);

  const int current = SkidMarks.size() - 1;
  SkidMarks[current]->add(coord);
}   // add

// -----------------------------------------------------------------------------
void SkidMark::addBreak(sgCoord* coord) {
  const unsigned int current = SkidMarks.size() - 1;
  if(SkidMarking)
    SkidMarks[current]->addEnd(coord);
  else {
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
  }   // if SkidMarking

  SkidMarking = !SkidMarking;
}   // addBreak

// -----------------------------------------------------------------------------
bool SkidMark::wasSkidMarking() const {
  return SkidMarking;
}   // wasSkidMarking


// =============================================================================
SkidMark::SkidMarkPos::SkidMarkPos() : ssgVtxTable ( GL_QUAD_STRIP,
                                                     new ssgVertexArray,
                                                     new ssgNormalArray,
                                                     new ssgTexCoordArray,
                                                     new ssgColourArray ) {
}   // SkidMarkPos

// -----------------------------------------------------------------------------
SkidMark::SkidMarkPos::SkidMarkPos( ssgVertexArray* vertices,
                                    ssgNormalArray* normals,
                                    ssgColourArray* colors,
                                    float global_track_offset) :
                                    ssgVtxTable ( GL_QUAD_STRIP,
                                                  vertices,
                                                  normals,
                                                  new ssgTexCoordArray,
                                                  colors )                {
  track_offset = global_track_offset;
}   // SkidMarkPos

// -----------------------------------------------------------------------------
SkidMark::SkidMarkPos::~SkidMarkPos() {
}   // ~SkidMarkPos
// -----------------------------------------------------------------------------
void SkidMark::SkidMarkPos::recalcBSphere() {
  bsphere . setRadius ( 1000.0f ) ;
  bsphere . setCenter ( 0, 0, 0 ) ;
}   // recalcBSphere
// -----------------------------------------------------------------------------
void SkidMark::SkidMarkPos::add(sgCoord* coord) {
  // Width of the skidmark
  const float width = 0.1f;

  static float a = 1.0f;
  sgVec3 pos;
  sgSetVec3(pos,
            coord->xyz[0] + sgSin(coord->hpr[0]+a*90) * width,
            coord->xyz[1] - sgCos(coord->hpr[0]+a*90) * width,
            coord->xyz[2] + track_offset);
  vertices->add(pos);

  sgSetVec3(pos,
            coord->xyz[0] + sgSin(coord->hpr[0]-a*90) * width,
            coord->xyz[1] - sgCos(coord->hpr[0]-a*90) * width,
            coord->xyz[2] + track_offset);
  vertices->add(pos);
  a = (a > 0.0f ? -1.0f : 1.0f);

  sgVec3 norm;
  sgSetVec3(norm, 0, 0, 1);
  normals->add(norm); normals->add(norm);

  sgVec4 color;
  sgSetVec4(color, 0.07f, 0.07f, 0.07f, 0.8f);
  colours->add(color); colours->add(color);
}   // add
// -----------------------------------------------------------------------------
void SkidMark::SkidMarkPos::addEnd(sgCoord* coord) {
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
}   // addEnd





/* EOF */
