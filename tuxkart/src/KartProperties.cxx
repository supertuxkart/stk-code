//  $Id: KartProperties.cxx,v 1.16 2004/09/05 21:26:24 matzebraun Exp $
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
#include <stdexcept>
#include <plib/ssg.h>
#include <math.h>
#include "material.h"
#include "lisp/Parser.h"
#include "lisp/Lisp.h"
#include "Loader.h"
#include "preprocessor.h"
#include "StringUtils.h"
#include "KartProperties.h"

KartProperties::KartProperties()
{
  init_defaults();
}

KartProperties::KartProperties(const std::string& filename)
    : icon_material(0), model(0)
{
  init_defaults();

  const lisp::Lisp* root = 0;
  ident = StringUtils::basename(StringUtils::without_extension(filename));

  try {
    lisp::Parser parser;
    root = parser.parse(loader->getPath(filename));
    
    const lisp::Lisp* lisp = root->getLisp("tuxkart-kart");
    if(!lisp)
        throw std::runtime_error("No tuxkart-kart node found");
  
    lisp->get("name",   name);
    lisp->get("model",  model_file);
    lisp->get("icon",   icon_file);
    lisp->get("shadow", shadow_file);
    lisp->get("red",     color[0]);
    lisp->get("green",   color[1]);
    lisp->get("blue",    color[2]);

    lisp->get("max-grip",       max_grip);
    lisp->get("corn-f",         corn_f);
    lisp->get("corn-r",         corn_r);
    lisp->get("mass",           mass);
    lisp->get("inertia",        inertia);
    lisp->get("turn-speed",     turn_speed);
    lisp->get("max-wheel-turn", max_wheel_turn);
    lisp->get("engine-power",   engine_power);
    lisp->get("max-throttle",   max_throttle);
    lisp->get("air-friction",   air_friction);
  } catch(std::exception& err) {
    std::cout << "Error while parsing KartProperties '" << filename
              << ": " << err.what() << "\n";
  }
  delete root;

  // load material
  icon_material = getMaterial(icon_file.c_str());

  // load model
  model = ssgLoadAC ( model_file.c_str(), loader ) ;
  preProcessObj(model, 0);
  model->ref();
}

KartProperties::~KartProperties()
{
  ssgDeRefDelete(model);
}

void
KartProperties::init_defaults()
{
  // Default to a standard Tux configuration in case anything goes wrong
  name = "Tux";
  ident = "tux";
  model_file = "tuxkart.ac";
  icon_file = "tuxicon.png";
  shadow_file = "tuxkartshadow.png";
  icon_material = NULL;

  color[0] = 1.0f;
  color[1] = 0.0f;
  color[2] = 0.0f;
  
  max_throttle    = 100;
  engine_power    = 60;
  corn_f          = -7.2f;
  corn_r          = -5.0;
  mass            = 90;
  inertia         = 13;
  turn_speed      = M_PI;
  max_wheel_turn  = M_PI/2;
  max_grip        = 4.0f;
  air_friction    = 0.8257;
  system_friction = 4.8f;
  
  model = NULL;
}

Material*
KartProperties::getIconMaterial() const
{
  return icon_material;
}

ssgEntity*
KartProperties::getModel() const
{
  return model;
}

/* EOF */
