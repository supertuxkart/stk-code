//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
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

#ifndef HEADER_UTILS_H
#define HEADER_UTILS_H

#include <plib/ssg.h>

/*
  Some 3D geometry functions.
*/

void  pr_from_normal ( sgVec3 hpr, sgVec3 nrm ) ;
void hpr_from_normal ( sgVec3 hpr, sgVec3 nrm ) ;

bool canAccess ( char *fname ) ;

#endif

/* EOF */
