//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2012-2015 SuperTuxKart-Team
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

#ifndef HEADER_CONSTANTS_HPP
#define HEADER_CONSTANTS_HPP

/*
  All final units are in meters (or meters/sec or meters/sec^2)
  and degrees (or degrees/sec).
*/

/* Handy constants */
#define KILOMETER            1000.000f
#define HOUR                 (60.0f * 60.0f)

/*
  For convenience - here are some multipliers for other units.

  eg  30 * MILES_PER_HOUR  is 30mph expressed in m/sec
*/

#define KILOMETERS_PER_HOUR  (KILOMETER/HOUR)

/* M$ compilers don't define M_PI... */

#ifndef M_PI
#  define M_PI 3.14159265358979323846f  /* As in Linux's math.h */
#endif

#define NINETY_DEGREE_RAD  (M_PI/2.0f)
#define DEGREE_TO_RAD      (M_PI/180.0f)
#define RAD_TO_DEGREE      (180.0f/M_PI)

const unsigned int MAX_PLAYER_COUNT = 4;

extern const bool IS_LITTLE_ENDIAN;

#define DEFAULT_GROUP_NAME "standard"

extern const char STK_VERSION[];

#endif
