//  $Id: SkidMark.cxx,v 1.1 2004/08/13 17:25:50 grumbel Exp $
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

  vtxtable->ref();
  vertices->ref();
  normals->ref();
  colors->ref();

  // GL_TRIANGLE_STRIP
  vtxtable = new ssgVtxTable(GL_LINE_STRIP, vertices, normals, NULL, colors);
  vtxtable->ref();
 
  World::current()->scene->addKid(vtxtable);
}

SkidMark::~SkidMark()
{
}

void
SkidMark::add(float x, float y)
{
  sgVec3 pos;
  sgVec3 norm;
  sgVec4 color;

  sgSetVec3(pos,  x, y, 1);
  sgSetVec3(norm, 0, 0, 1);
  sgSetVec4(color, 1.0, 1.0, 1.0, 1.0);

  vertices->add(pos);
  normals->add(norm);
  colors->add(color);

  vtxtable->dirtyBSphere();
}

/* EOF */
