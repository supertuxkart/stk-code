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

/** Visual studio workarounds in one place
 *  Note that Visual Studio 2013 does have the maths functions defined,
 *  so we define the work arounds only for compiler versions before 18.00
 */

#if defined(WIN32) && defined(_MSC_VER) && _MSC_VER < 1800
#  include <math.h>

#  define isnan _isnan
#  define roundf(x) (floorf(x + 0.5f))
#  define round(x)  (floorf(x + 0.5))
#endif

#ifdef __MINGW32__
	#include <cmath>
	using std::isnan;
#endif
