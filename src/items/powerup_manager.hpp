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

#ifndef HEADER_POWERUPMANAGER_HPP
#define HEADER_POWERUPMANAGER_HPP

#include "irrlicht.h"

#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "btBulletDynamicsCommon.h"

class Material;
class XMLNode;

// The anvil and parachute must be at the end of the enum, and the
// zipper just before them (see Powerup::hitBonusBox).
enum PowerupType {POWERUP_NOTHING,
                  POWERUP_FIRST, 
                  POWERUP_BUBBLEGUM = POWERUP_FIRST, 
                  POWERUP_CAKE,
                  POWERUP_BOWLING, POWERUP_ZIPPER, POWERUP_PLUNGER,
                  POWERUP_SWITCH,
                  POWERUP_PARACHUTE, 
                  POWERUP_ANVIL,      //powerup.cpp assumes these two come last
                  POWERUP_LAST=POWERUP_ANVIL,
                  POWERUP_MAX};

class PowerupManager
{
private:
    Material*     m_all_icons [POWERUP_MAX];
    float         m_all_max_distance[POWERUP_MAX];    // if a target is closer than this
    float         m_all_force_to_target[POWERUP_MAX]; // apply this force to move towards
                                                     // the target
    float         m_all_max_turn_angle[POWERUP_MAX];  // maximum turn angle for homing
    scene::IMesh *m_all_meshes[POWERUP_MAX];
    btVector3     m_all_extends[POWERUP_MAX];
    void          LoadNode       (const lisp::Lisp* lisp, int collectType);
    PowerupType   getPowerupType(const std::string &name);
public:
                  PowerupManager  ();
                 ~PowerupManager  ();
    void          loadAllPowerups ();
    void          removeTextures  ();
    void          LoadPowerup     (PowerupType type, const XMLNode &node);
    Material*     getIcon         (int type) const {return m_all_icons [type];      }
    /** Returns the mesh for a certain powerup. 
     *  \param type Mesh type for which the model is returned. */
    scene::IMesh *getMesh         (int type) const {return m_all_meshes[type];      }
    float         getForceToTarget(int type) const {return m_all_force_to_target[type]; }
    float         getMaxDistance  (int type) const {return m_all_max_distance[type];}
    float         getMaxTurnAngle (int type) const {return m_all_max_turn_angle[type];}
    const btVector3& getExtend   (int type) const {return m_all_extends[type];     }
};

extern PowerupManager* powerup_manager;

#endif
