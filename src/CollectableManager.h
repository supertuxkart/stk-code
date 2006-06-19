//  $Id: Collectable.h,v 1.4 2005/08/19 20:51:56 joh Exp $
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

#ifndef HEADER_COLLECTABLEMANAGER_H
#define HEADER_COLLECTABLEMANAGER_H

#include "lisp/Parser.h"
#include "lisp/Lisp.h"

class Material;
class ssgEntity;

enum collectableType {COLLECT_NOTHING, COLLECT_MISSILE, 
		      COLLECT_SPARK,   COLLECT_HOMING_MISSILE,
		      COLLECT_ZIPPER,  COLLECT_MAGNET, 
		      COLLECT_MAX};

class CollectableManager {
 protected:
  Material*    allIcons [COLLECT_MAX];
  float        allSpeeds[COLLECT_MAX];
  ssgEntity*   allModels[COLLECT_MAX];
  void         LoadNode       (const lisp::Lisp* lisp, int collectType);
 public:
  CollectableManager          (){}
  void         loadCollectable();
  void         Load           (int collectType, const char* filename);
  Material*    getIcon        (int type) {return allIcons [type];}
  float        getSpeed       (int type) {return allSpeeds[type];}
  ssgEntity*   getModel       (int type) {return allModels[type];}
};

extern CollectableManager* collectable_manager;

#endif
