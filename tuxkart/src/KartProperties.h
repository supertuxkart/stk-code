//  $Id: KartProperties.h,v 1.1 2004/08/08 10:43:42 grumbel Exp $
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

#ifndef HEADER_KARTPROPERTIES_H
#define HEADER_KARTPROPERTIES_H

#include <string>

class KartProperties
{
public:
  /** Filename of the 3d model that is used for things kart */
  std::string model_file;

  /** Filename of the icon that represents the kart in the
      statusbar and the character select screen */
  std::string icon_file;

  /** Filename of the image file that contains the shadow for this
      kart */
  std::string shadow_file;

  /** Color the represents the kart in the status bar and on the
      track-view */
  float       color[3];
  
  /** Physic properties */
  float max_grip;
  float corn_f;
  float corn_r;
  float mass;
  float inertia;
  float turn_speed;
  float max_wheel_turn;
  float engine_power;
  float max_throttle;
  float air_friction;

  KartProperties();
};

extern KartProperties kart_properties;

#endif

/* EOF */
