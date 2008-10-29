//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "btBulletDynamicsCommon.h"

class Material;
class ssgEntity;

// The anvil and parachute must be at the end of the enum, and the
// zipper just before them (see collectable::hitRedHerring).
enum CollectableType {COLLECT_NOTHING,
                      COLLECT_MISSILE, COLLECT_CAKE,
                      COLLECT_BOWLING, COLLECT_ZIPPER,
                      COLLECT_PARACHUTE, COLLECT_ANVIL,
                      COLLECT_MAX};

class CollectableManager
{
protected:
    Material*    m_all_icons [COLLECT_MAX];
    float        m_all_max_distance[COLLECT_MAX];    // if a target is closer than this
    float        m_all_force_to_target[COLLECT_MAX]; // apply this force to move towards
                                                     // the target
    float        m_all_max_turn_angle[COLLECT_MAX];  // maximum turn angle for homing
    ssgEntity*   m_all_models[COLLECT_MAX];
    btVector3    m_all_extends[COLLECT_MAX];
    void         LoadNode       (const lisp::Lisp* lisp, int collectType);
public:
    CollectableManager           ();
    void         loadCollectables();
    void         removeTextures  ();
    void         Load            (int collectType, const char* filename);
    Material*    getIcon         (int type) const {return m_all_icons [type];      }
    ssgEntity*   getModel        (int type) const {return m_all_models[type];      }
    float        getForceToTarget(int type) const {return m_all_force_to_target[type]; }
    float        getMaxDistance  (int type) const {return m_all_max_distance[type];}
    float        getMaxTurnAngle (int type) const {return m_all_max_turn_angle[type];}
    const btVector3& getExtend   (int type) const {return m_all_extends[type];     }
};

extern CollectableManager* collectable_manager;

#endif
