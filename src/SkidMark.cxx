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

#include "World.h"
#include "SkidMark.h"

SkidMark::SkidMark()
  : ssgVtxTable ( GL_QUAD_STRIP, 
                  new ssgVertexArray,
                  new ssgNormalArray,
                  new ssgTexCoordArray,
                  new ssgColourArray )
{
  ssgSimpleState *skidstate;
  skidstate = new ssgSimpleState ();
  skidstate -> enable (GL_BLEND);
  this -> setState (skidstate);
  
  newSkidmark = 2;
}

SkidMark::~SkidMark()
{
}

void
SkidMark::add(sgCoord* coord)
{
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
}

void
SkidMark::addBreak(sgCoord* coord)
{
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

void
SkidMark::recalcBSphere()
{
  bsphere . setRadius ( 1000.0f ) ;
  bsphere . setCenter ( 0, 0, 0 ) ;
}

/* EOF */
