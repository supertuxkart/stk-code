//  $Id: KartProperties.cxx,v 1.4 2004/08/08 12:17:59 grumbel Exp $
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

#include <math.h>
#include "material.h"
#include "KartProperties.h"

KartProperties::KartProperties()
{
  model_file = "tuxkart.ac";

  color[0] = 1.0f;
  color[1] = 0.0f;
  color[2] = 0.0f;
  
  max_throttle   = 100;
  engine_power   = 60;
  corn_f         = -7.2f;
  corn_r         = -5.0;
  mass           = 90;
  inertia        = 13;
  turn_speed     = M_PI;
  max_wheel_turn = M_PI/2;
  max_grip       = 4.0f;
  air_friction   = 0.8257;

  icon_file = "tuxicon.rgb";

  icon_material = NULL;
}

Material*
KartProperties::getIconMaterial()
{
  if (icon_material)
    {
      return icon_material; 
    }
  else
    {
      char* icon_file_c = strdup(icon_file.c_str());
      icon_material = getMaterial(icon_file_c);
      free(icon_file_c);
      return icon_material;
    }
}

/* EOF */
