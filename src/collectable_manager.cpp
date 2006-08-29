//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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
#include "collectable_manager.hpp"
#include "loader.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "preprocessor.hpp"

typedef struct {
  collectableType collectable;
  const char*const dataFile;
} initCollectableType;

initCollectableType ict[]={
  {COLLECT_ZIPPER,         "zipper.collectable"       },
  {COLLECT_MAGNET,         "magnet.collectable"       },
  {COLLECT_SPARK,          "spark.projectile"         },
  {COLLECT_MISSILE,        "missile.projectile"       },
  {COLLECT_HOMING_MISSILE, "homingmissile.projectile" },
  {COLLECT_MAX,            ""                         },
};

CollectableManager* collectable_manager=0;

// -----------------------------------------------------------------------------
void CollectableManager::loadCollectable() {
  for(int i=0; ict[i].collectable != COLLECT_MAX; i++) {
    Load(ict[i].collectable, ict[i].dataFile);
  }
}  // loadCollectable

// -----------------------------------------------------------------------------
void CollectableManager::Load(int collectType, const char* filename) {
  const lisp::Lisp* root = 0;
  try {
    lisp::Parser parser;
    std::string tmp= "data/" + (std::string)filename;
    root = parser.parse(loader->getPath(tmp.c_str()));
    
    const lisp::Lisp* lisp = root->getLisp("tuxkart-collectable");
    if(!lisp) {
      std::string s="No 'tuxkart-collectable node found";
      throw std::runtime_error(s);
    }
    LoadNode(lisp, collectType);
  } catch(std::exception& err) {
    std::cout << "Error while parsing collectable '" << filename
              << ": " << err.what() << "\n";
  }
  delete root;

}   // Load

// -----------------------------------------------------------------------------
void CollectableManager::LoadNode(const lisp::Lisp* lisp, int collectType ) {
  std::string sName, sModel, sIconFile;
  int dummy;
  lisp->get("name",   sName                 ); // the name is actually ignored
  lisp->get("model",  sModel                );
  lisp->get("icon",   sIconFile             );
  // If the speed value is an integer (e.g. no "."), an uninitialised
  // value will be returned. In this case try reading the speed as
  // an integer value.
  if(!lisp->get("speed",  allSpeeds[collectType])) {
    dummy=-1;
    lisp->get("speed",  dummy);
    allSpeeds[collectType]=dummy;
  }

  // load material
  allIcons [collectType] = material_manager->getMaterial(sIconFile.c_str());

  //FIXME: something probably forgets to disable GL_CULL_FACE after enabling it,
  //this is just a quick fix.
  if(collectType == COLLECT_SPARK) allIcons[COLLECT_SPARK]->getState()->disable ( GL_CULL_FACE ) ;

  if(sModel!="") {
    ssgEntity* e = ssgLoadAC(sModel.c_str(), loader);
    allModels[collectType] = e;
    preProcessObj(e, 0);
    e->ref();
  } else {
    allModels[collectType] = 0;
  }
}   // LoadNode

