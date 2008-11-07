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

#ifndef HEADER_POWERUPMANAGER_H
#define HEADER_POWERUPMANAGER_H

#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "btBulletDynamicsCommon.h"

class Material;
class ssgEntity;

// The anvil and parachute must be at the end of the enum, and the
// zipper just before them (see Powerup::hitBonusBox).
enum PowerupType {POWERUP_NOTHING,
                  POWERUP_BUBBLEGUM, POWERUP_CAKE,
                  POWERUP_BOWLING, POWERUP_ZIPPER,
                  POWERUP_PARACHUTE, POWERUP_ANVIL,
                  POWERUP_MAX};

class PowerupManager
{
protected:
    Material*    m_all_icons [POWERUP_MAX];
    float        m_all_max_distance[POWERUP_MAX];    // if a target is closer than this
    float        m_all_force_to_target[POWERUP_MAX]; // apply this force to move towards
                                                     // the target
    float        m_all_max_turn_angle[POWERUP_MAX];  // maximum turn angle for homing
    ssgEntity*   m_all_models[POWERUP_MAX];
    btVector3    m_all_extends[POWERUP_MAX];
    void         LoadNode       (const lisp::Lisp* lisp, int collectType);
public:
    PowerupManager           ();
    void         loadPowerups();
    void         removeTextures  ();
    void         Load            (int collectType, const char* filename);
    Material*    getIcon         (int type) const {return m_all_icons [type];      }
    ssgEntity*   getModel        (int type) const {return m_all_models[type];      }
    float        getForceToTarget(int type) const {return m_all_force_to_target[type]; }
    float        getMaxDistance  (int type) const {return m_all_max_distance[type];}
    float        getMaxTurnAngle (int type) const {return m_all_max_turn_angle[type];}
    const btVector3& getExtend   (int type) const {return m_all_extends[type];     }
};

extern PowerupManager* powerup_manager;

#endif
