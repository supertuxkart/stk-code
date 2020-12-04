//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#ifndef GL_HEADER_HPP
#define GL_HEADER_HPP

#ifndef SERVER_ONLY
#include "glad/gl.h"
#else
typedef unsigned int GLuint;
typedef int GLint;
typedef unsigned int GLsync;
typedef unsigned int GLenum;
#endif   // server only

#ifndef GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX
#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#endif

#endif
