//  $Id: KartProperties.cxx,v 1.8 2004/08/09 11:25:35 grumbel Exp $
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

#include <iostream>
#include <math.h>
#include "material.h"
#include "lispreader.h"
#include "Loader.h"
#include "KartProperties.h"

KartProperties::KartProperties()
{
  init_defaults();
}

KartProperties::KartProperties(const std::string& filename)
{
  init_defaults();

  try {
    LispReader* kart = LispReader::load(loader ? loader->getPath(filename) : filename, "tuxkart-kart");
    assert(kart);
  
    LispReader reader(kart->get_lisp());

    reader.read_string("name",   name);
    reader.read_string("model",  model_file);
    reader.read_string("icon",   icon_file);
    reader.read_string("shadow", shadow_file);
    reader.read_float("red",     color[0]);
    reader.read_float("green",   color[1]);
    reader.read_float("blue",    color[2]);

    reader.read_float("max-grip",       max_grip);
    reader.read_float("corn-f",         corn_f);
    reader.read_float("corn-r",         corn_r);
    reader.read_float("mass",           mass);
    reader.read_float("inertia",        inertia);
    reader.read_float("turn-speed",     turn_speed);
    reader.read_float("max-wheel-turn", max_wheel_turn);
    reader.read_float("engine-power",   engine_power);
    reader.read_float("max-throttle",   max_throttle);
    reader.read_float("air-friction",   air_friction);

    delete kart;
  } catch(LispReaderException& err) {
    std::cout << "LispReaderException: " << err.message << std::endl;
  } catch(std::exception& err) {
    std::cout << "Catched std::exception: " << err.what() << std::endl;
  }
}

void
KartProperties::init_defaults()
{
  // Default to a standard Tux configuration in case anything goes wrong
  name = "Tux";
  model_file = "tuxkart.ac";
  icon_file = "tuxicon.rgb";
  shadow_file = "tuxkartshadow.rgb";
  icon_material = NULL;

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
