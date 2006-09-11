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

#ifndef HEADER_HERRINGMANAGER_H
#define HEADER_HERRINGMANAGER_H


#include <vector>
#include <map>
#include "herring.hpp"
#include "lisp/lisp.hpp"

class Kart;
class ssgEntity;

class HerringManager{

private:
  // The vector of all herrings of the current track
  typedef std::vector<Herring*> AllHerringType;
  AllHerringType allHerrings;

  // This stores all herring models defined in the models/herring
  // subdirectory.
  ssgEntity *herringModel[HE_SILVER+1];

  // This is the active model. It gets determined by first loading the
  // default, then track models, user models, cup models. This means, that
  // a herring style specified in a track overwrites a command line option.
  std::map<std::string,ssgEntity*>allModels;

  std::string userFilename;
  void CreateDefaultHerring(sgVec3 colour, std::string name);
  void setDefaultHerringStyle();
  void setHerring(const lisp::Lisp *herring_node, char *colour, 
		  herringType type);

public:
  HerringManager();
  void        loadDefaultHerrings();
  void        loadHerringStyle(const std::string filename);
  Herring*    newHerring      (herringType type, sgVec3* xyz);
  void        update          (float delta);
  void        hitHerring      (Kart* kart);
  void        cleanup         ();
  void        reset           ();
  void        setUserFilename (char *s) {userFilename=s;}
};

extern HerringManager* herring_manager;


#endif
