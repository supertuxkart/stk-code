//  $Id: SkidMark.cxx,v 1.2 2004/08/13 18:57:04 grumbel Exp $
//
//  TuxKart - a fun racing game with go-kart
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
{
  vertices = new ssgVertexArray;
  normals  = new ssgNormalArray;
  colors   = new ssgColourArray;

  vertices->ref();
  normals->ref();
  colors->ref();

  //GL_TRIANGLE_STRIP
  vtxtable = new ssgVtxTable(GL_LINE_STRIP, vertices, normals, NULL, colors);
  vtxtable->ref();
 
  World::current()->scene->addKid(vtxtable);
  
  detail = 0;
}

SkidMark::~SkidMark()
{
}

void
SkidMark::add(sgCoord* coord)
{
  if (detail != 0)
    {
      detail -= 1;
      return;
    }

  detail = 2;
  sgVec3 pos;
  sgVec3 norm;
  sgVec4 color;
  // Amount of which the skidmark is lifted above the track to avoid
  // z-buffer errors
  float track_offset = 0.003f;
  
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
  sgSetVec4(color, 1, 1, 1, 1);

  normals->add(norm); normals->add(norm);
  colors->add(color); colors->add(color);

  vtxtable->dirtyBSphere();
}

/* EOF */
