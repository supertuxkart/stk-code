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

#include <string>
#include <vector>

#include "btBulletDynamicsCommon.h"

class Material;
class XMLNode;

/**
  * \ingroup items
  */
class PowerupManager
{
public:
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
                      POWERUP_MAX
    };

private:
    /** The icon for each powerup. */
    Material*     m_all_icons [POWERUP_MAX];

    /** A maximum distance for homing powerups. */
    float         m_all_max_distance[POWERUP_MAX];

    /** A force to steer a powerup towards a target. */
    float         m_all_force_to_target[POWERUP_MAX];

    /** Maximum turn angle for steering of homing powerups. */
    float         m_all_max_turn_angle[POWERUP_MAX];

    /** The mesh for each model (if the powerup has a model), e.g. a switch
        has none. */
    irr::scene::IMesh *m_all_meshes[POWERUP_MAX];

    /** Size of the corresponding mesh. */
    btVector3     m_all_extends[POWERUP_MAX];

    /** For each powerup the weight (probability) used depending on the
     *  number of players. */
    //std::vector<int> m_weight[POWERUP_MAX];

    PowerupType   getPowerupType(const std::string &name) const;

    void          loadWeights(const XMLNode &node);

public:
                  PowerupManager  ();
                 ~PowerupManager  ();
    void          loadAllPowerups ();
    void          removeTextures  ();
    void          LoadPowerup     (PowerupType type, const XMLNode &node);
    Material*     getIcon         (int type) const {return m_all_icons [type];      }
    /** Returns the mesh for a certain powerup. 
     *  \param type Mesh type for which the model is returned. */
    irr::scene::IMesh *getMesh         (int type) const {return m_all_meshes[type];      }
    float         getForceToTarget(int type) const {return m_all_force_to_target[type]; }
    float         getMaxDistance  (int type) const {return m_all_max_distance[type];}
    float         getMaxTurnAngle (int type) const {return m_all_max_turn_angle[type];}
    const btVector3& getExtend    (int type) const {return m_all_extends[type];     }
};

extern PowerupManager* powerup_manager;

#endif
