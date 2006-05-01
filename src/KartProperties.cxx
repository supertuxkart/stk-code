//  $Id: KartProperties.cxx,v 1.5 2005/08/10 07:58:10 joh Exp $
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

#include <iostream>
#include <stdexcept>
#include <plib/ssg.h>
#include "MaterialManager.h"
#include "lisp/Parser.h"
#include "lisp/Lisp.h"
#include "Loader.h"
#include "preprocessor.h"
#include "StringUtils.h"
#include "KartProperties.h"

KartProperties::KartProperties() {
  init_defaults();
}   // KartProperties

// -----------------------------------------------------------------------------
KartProperties::KartProperties(const std::string& filename,
			       char *node)
              : icon_material(0), model(0) {
  init_defaults();

  const lisp::Lisp* root = 0;
  ident = StringUtils::basename(StringUtils::without_extension(filename));

  try {
    lisp::Parser parser;
    root = parser.parse(loader->getPath(filename));
    
    const lisp::Lisp* lisp = root->getLisp(node);
    if(!lisp) {
      std::string s="No ";
      s+=node;
      s+=" node found";
      throw std::runtime_error(s);
    }
  
    lisp->get("name",   name);
    lisp->get("model",  model_file);
    lisp->get("icon",   icon_file);
    lisp->get("shadow", shadow_file);
    lisp->get("red",     color[0]);
    lisp->get("green",   color[1]);
    lisp->get("blue",    color[2]);

    lisp->get("tire-grip",      tire_grip);
    lisp->get("corn-f",         corn_f);
    lisp->get("corn-r",         corn_r);
    lisp->get("mass",           mass);
    lisp->get("inertia",        inertia);
    lisp->get("turn-speed",     turn_speed);
    lisp->get("max-wheel-turn", max_wheel_turn);
    lisp->get("wheel-base",     wheel_base);
    lisp->get("heightCOG",      heightCOG);
    lisp->get("engine-power",   engine_power);
    lisp->get("air-friction",   air_friction);
  } catch(std::exception& err) {
    std::cout << "Error while parsing KartProperties '" << filename
              << ": " << err.what() << "\n";
  }
  delete root;

  // load material
  icon_material = material_manager->getMaterial(icon_file.c_str());
}   // KartProperties

// -----------------------------------------------------------------------------
KartProperties::~KartProperties() {
  ssgDeRefDelete(model);
}   // ~KartProperties

// -----------------------------------------------------------------------------
void KartProperties::init_defaults() {
  // Default to a standard Tux configuration in case anything goes wrong
  name          = "Tux";
  ident         = "tux";
  model_file    = "tuxkart.ac";
  icon_file     = "tuxicon.png";
  shadow_file   = "tuxkartshadow.png";
  icon_material = NULL;

  color[0] = 1.0f;
  color[1] = 0.0f;
  color[2] = 0.0f;
    
  wheel_base      = 1.2f;
  heightCOG       = 0.5f;
  engine_power    = 100.0f;
  roll_resistance = 4.8f;
  mass            = 90;
  air_friction    = 0.8257;
  tire_grip       = 4.0f;

  corn_f          = -7.2f;
  corn_r          = -5.0;
  inertia         = 13;
  turn_speed      = M_PI;
  max_wheel_turn  = M_PI/2;
  
  model = NULL;
}   // init_defaults

// -----------------------------------------------------------------------------
void KartProperties::loadModel() {
  model = ssgLoadAC ( model_file.c_str(), loader ) ;
  preProcessObj(model, 0);
  model->ref();
}   // loadModel

/* EOF */
