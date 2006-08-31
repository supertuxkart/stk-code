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

#include <stdexcept>
#include <string>
#include <sstream>
#include "preprocessor.hpp"
#include "config.hpp"
#include "herring_manager.hpp"
#include "loader.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "kart.hpp"

// Simple shadow class, only used here for default herrings
class Shadow {
  ssgBranch *sh ;
 
public:
  Shadow ( float x1, float x2, float y1, float y2 ) ;
  ssgEntity *getRoot () { return sh ; }
};   // Shadow

// -----------------------------------------------------------------------------
Shadow::Shadow ( float x1, float x2, float y1, float y2 ) {
  ssgVertexArray   *va = new ssgVertexArray   () ; sgVec3 v ;
  ssgNormalArray   *na = new ssgNormalArray   () ; sgVec3 n ;
  ssgColourArray   *ca = new ssgColourArray   () ; sgVec4 c ;
  ssgTexCoordArray *ta = new ssgTexCoordArray () ; sgVec2 t ;

  sgSetVec4 ( c, 0.0f, 0.0f, 0.0f, 1.0f ) ; ca->add(c) ;
  sgSetVec3 ( n, 0.0f, 0.0f, 1.0f ) ; na->add(n) ;
 
  sgSetVec3 ( v, x1, y1, 0.10 ) ; va->add(v) ;
  sgSetVec3 ( v, x2, y1, 0.10 ) ; va->add(v) ;
  sgSetVec3 ( v, x1, y2, 0.10 ) ; va->add(v) ;
  sgSetVec3 ( v, x2, y2, 0.10 ) ; va->add(v) ;
 
  sgSetVec2 ( t, 0.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2 ( t, 1.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2 ( t, 0.0, 1.0 ) ; ta->add(t) ;
  sgSetVec2 ( t, 1.0, 1.0 ) ; ta->add(t) ;
 
  sh = new ssgBranch ;
  sh -> clrTraversalMaskBits ( SSGTRAV_ISECT|SSGTRAV_HOT ) ;
 
  sh -> setName ( "Shadow" ) ;
 
  ssgVtxTable *gs = new ssgVtxTable ( GL_TRIANGLE_STRIP, va, na, ta, ca ) ;
 
  gs -> clrTraversalMaskBits ( SSGTRAV_ISECT|SSGTRAV_HOT ) ;
  gs -> setState ( fuzzy_gst ) ;
  sh -> addKid ( gs ) ;
  sh -> ref () ; /* Make sure it doesn't get deleted by mistake */
}   // Shadow

// =============================================================================
HerringManager* herring_manager;

HerringManager::HerringManager() {
  for(int i=HE_RED; i<=HE_SILVER; i++) {
    allDefaultModels[i]=NULL;
    allTrackModels  [i]=NULL;
    allUserModels   [i]=NULL;
  }
}   // HerringManager

// -----------------------------------------------------------------------------
Herring* HerringManager::newHerring(herringType type, sgVec3 xyz) {
  ssgEntity* m;
  // This test determines the order in which models are used: user model
  // overwrite track models, which in turn overwrite default models.
  if(!allUserModels[type]) {
    m = allTrackModels[type] ? allTrackModels[type] : allDefaultModels[type];
  } else {
    m = allUserModels[type];
  }
  Herring* h = new Herring(type, xyz, m);
  allHerrings.push_back(h);
  return h;
}   // newHerring

// -----------------------------------------------------------------------------
void  HerringManager::hitHerring(Kart* kart) {
  for(AllHerringType::iterator i =allHerrings.begin(); 
                               i!=allHerrings.end();  i++) {
    if((*i)->wasEaten()) continue;
    if((*i)->hitKart(kart)) {
      (*i)->isEaten();
      kart->collectedHerring(*i);
    }   // if hit
  }   // for allHerrings
}   // hitHerring

// -----------------------------------------------------------------------------
// Remove all herring instances, and the track specific models. This is used
// just before a new track is loaded and a race is started
void HerringManager::cleanup() {
  for(AllHerringType::iterator i =allHerrings.begin(); 
                               i!=allHerrings.end();  i++) {
    delete *i;
  }
  allHerrings.clear();
  
  for(int i=HE_RED; i<=HE_SILVER; i++) {
    allTrackModels[i]=NULL;
  }  // for i
}   // cleanup

// -----------------------------------------------------------------------------
// Remove all herring instances, and the track specific models. This is used
// just before a new track is loaded and a race is started
void HerringManager::reset() {
  for(AllHerringType::iterator i =allHerrings.begin(); 
                               i!=allHerrings.end();  i++) {
    (*i)->reset();
  }  // for i
}   // reset
// -----------------------------------------------------------------------------
void HerringManager::update(float delta) {
  for(AllHerringType::iterator i =allHerrings.begin(); 
                               i!=allHerrings.end();  i++) {
    (*i)->update(delta);
  }   // for allHerrings
}   // delta

// -----------------------------------------------------------------------------
void HerringManager::loadAllHerrings() {
  // First load the default old models.
  sgVec3 yellow = { 1.0, 1.0, 0.4 }; CreateDefaultHerring(yellow, HE_GOLD  );
  sgVec3 cyan   = { 0.4, 1.0, 1.0 }; CreateDefaultHerring(cyan  , HE_SILVER);
  sgVec3 red    = { 0.8, 0.0, 0.0 }; CreateDefaultHerring(red   , HE_RED   );
  sgVec3 green  = { 0.0, 0.8, 0.0 }; CreateDefaultHerring(green , HE_GREEN );

  // The load the default style from the config file
  // This way if a herring is not defined in the herringstyle-file, the
  // default is used.
  std::string filename=config->herringStyle;
  loadHerringData(filename, ISUSERDATA);
}   // loadAllHerrings

// -----------------------------------------------------------------------------
void HerringManager::CreateDefaultHerring(sgVec3 colour, herringType type) {
  ssgVertexArray   *va = new ssgVertexArray   () ; sgVec3 v ;
  ssgNormalArray   *na = new ssgNormalArray   () ; sgVec3 n ;
  ssgColourArray   *ca = new ssgColourArray   () ; sgVec4 c ;
  ssgTexCoordArray *ta = new ssgTexCoordArray () ; sgVec2 t ;
  
  sgSetVec3(v, -0.5, 0.0, 0.0 ) ; va->add(v) ;
  sgSetVec3(v,  0.5, 0.0, 0.0 ) ; va->add(v) ;
  sgSetVec3(v, -0.5, 0.0, 0.5 ) ; va->add(v) ;
  sgSetVec3(v,  0.5, 0.0, 0.5 ) ; va->add(v) ;
  sgSetVec3(v, -0.5, 0.0, 0.0 ) ; va->add(v) ;
  sgSetVec3(v,  0.5, 0.0, 0.0 ) ; va->add(v) ;

  sgSetVec3(n,  0.0f,  1.0f,  0.0f ) ; na->add(n) ;

  sgCopyVec3 ( c, colour ) ; c[ 3 ] = 1.0f ; ca->add(c) ;
 
  sgSetVec2(t, 0.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2(t, 1.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2(t, 0.0, 1.0 ) ; ta->add(t) ;
  sgSetVec2(t, 1.0, 1.0 ) ; ta->add(t) ;
  sgSetVec2(t, 0.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2(t, 1.0, 0.0 ) ; ta->add(t) ;
 

  ssgLeaf *gset = new ssgVtxTable ( GL_TRIANGLE_STRIP, va, na, ta, ca ) ;
 
  gset->setState(material_manager->getMaterial("herring.rgb")->getState()) ;
 
  Shadow* sh = new Shadow ( -0.5, 0.5, -0.25, 0.25 ) ;
 
  ssgTransform* tr = new ssgTransform () ;
 
  tr -> addKid ( sh -> getRoot () ) ;
  tr -> addKid ( gset ) ;
  tr -> ref () ; /* Make sure it doesn't get deleted by mistake */
  preProcessObj(tr);
  allDefaultModels[type]=tr;

}   // CreateDefaultHerring
// -----------------------------------------------------------------------------
void HerringManager::loadHerringData(const std::string filename, int isUser) {
  if(filename.length()==0) return;
  const lisp::Lisp* root = 0;
  lisp::Parser parser;
  std::string tmp= "data/" + (std::string)filename + ".herring";
  root = parser.parse(loader->getPath(tmp.c_str()));

  const lisp::Lisp* herring_node = root->getLisp("herring");
  if(!herring_node) {
    delete root;
    std::stringstream msg;
    msg << "Couldn't load map '" << filename << "': no herring node.";
    throw std::runtime_error(msg.str());
  }
  loadHerringModel(herring_node, "red",    HE_RED,    isUser);
  loadHerringModel(herring_node, "green",  HE_GREEN,  isUser);
  loadHerringModel(herring_node, "gold",   HE_GOLD,   isUser);
  loadHerringModel(herring_node, "silver", HE_SILVER, isUser);
}   // loadHerringData
// -----------------------------------------------------------------------------
void HerringManager::loadHerringModel(const lisp::Lisp* herring_node,
				      char* colour, herringType type, 
				      int isUser) {

  const lisp::Lisp* data = herring_node->getLisp(colour);
  if(data) {
    std::string  name, model;
    data->get("name",  name);
    data->get("model", model);
    ssgEntity* e = ssgLoad(model.c_str(), loader);
    e->setName(name.c_str());
    preProcessObj(e);
    if(isUser==ISUSERDATA) {
      allUserModels[type] = e;
    } else {
      allTrackModels[type] = e;
    }
  }
}   // loadHerringModel
// -----------------------------------------------------------------------------
