//  $Id: plibwrap.h,v 1.2 2004/08/15 13:57:55 grumbel Exp $
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

#ifndef HEADER_PLIBWRAP_H
#define HEADER_PLIBWRAP_H

#include <plib/sg.h>

struct sgVec3Wrap
{
  sgVec3 vec;

  inline sgVec3Wrap() {}

  inline sgVec3Wrap(const sgVec3& vec_)
  {
    sgCopyVec3(vec, vec_);
  }

  inline operator float*() { 
    return vec; 
  }
};

#endif

/* EOF */
