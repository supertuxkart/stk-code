//  $Id: constants.h,v 1.3 2004/07/31 23:46:18 grumbel Exp $
//
//  TuxKart - a fun racing game with go-kart
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

#ifndef HEADER_CONSTANTS_H
#define HEADER_CONSTANTS_H

/*
  All final units are in meters (or meters/sec or meters/sec^2)
  and degrees (or degrees/sec).
*/

/* Handy constants */

#define GRAVITY              (4.0f * 9.80665f)
#define MILE                 1609.344f
#define KILOMETER            1000.000f
#define HOUR                 (60.0f * 60.0f)

/*
  For convenience - here are some multipliers for other units.

  eg  30 * MILES_PER_HOUR  is 30mph expressed in m/sec
*/

#define MILES_PER_HOUR       (MILE/HOUR)
#define KILOMETERS_PER_HOUR  (KILOMETER/HOUR)

/* M$ compilers don't define M_PI... */

#ifndef M_PI
#  define M_PI 3.14159265358979323846  /* As in Linux's math.h */
#endif

#endif

