//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 Joerg Henrichs
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


#ifndef HEADER_ALIGNED_ARRAY_HPP
#define HEADER_ALIGNED_ARRAY_HPP

// Optimised windows compilation should not use std::vector, but
// btAlignedObjectArray to enable bullet SSE optimisations.
// On the other hand, std::vector gives much better debugging features.
// So SSE is disabled in bullet on windows debug
#if !defined(DEBUG) && defined(WIN32)
#  undef USE_ALIGNED
#else
#  undef  USE_ALIGNED
#endif

#ifdef USE_ALIGNED
#  include "LinearMath/btAlignedObjectArray.h"
#  define AlignedArray btAlignedObjectArray
#else
#  include <vector>
#  define AlignedArray std::vector
#endif


#endif
