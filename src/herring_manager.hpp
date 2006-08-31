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
#include "herring.hpp"
#include "lisp/lisp.hpp"

class Kart;
class ssgEntity;

class HerringManager{
 public:
  enum {ISUSERDATA, ISTRACKDATA};

 private:
  typedef std::vector<Herring*> AllHerringType;
  AllHerringType allHerrings;

  /* This object stores up to three models for each herring type:
     1) A user chosen model via command line option --hering
     2) A track specific model
     3) The default model (old herring style)
     When a herring is needed, these three models will be tested in this 
     order: if no model is found, the next 'level' is used. For example,
     if the user does not define a model for red herring, the track model
     is tested, and if there is no track model, the default model (which 
     always exists) is used. This way, the user can overwrite the track 
     model, and the track can overwrite the default model                 */

  ssgEntity *allDefaultModels[HE_SILVER+1];
  ssgEntity *allTrackModels  [HE_SILVER+1];
  ssgEntity *allUserModels   [HE_SILVER+1];
  void CreateDefaultHerring(sgVec3 colour, herringType type);
  void loadHerringModel(const lisp::Lisp* herring_node,
			char  *colour, herringType type, int isUser);
public:
  HerringManager();
  void        loadAllHerrings();
  void        loadHerringData(const std::string filename, int isUser);
  Herring*    newHerring     (herringType type, sgVec3 xyz);
  void        update         (float delta);
  void        hitHerring     (Kart* kart);
  void        cleanup        ();
  void        reset          ();
};

extern HerringManager* herring_manager;


#endif
