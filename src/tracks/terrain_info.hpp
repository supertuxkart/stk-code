//  $Id: terrain_info.hpp 1284 2007-11-08 12:31:54Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTe ABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_TERRAIN_INFO_HPP
#define HEADER_TERRAIN_INFO_HPP

#include "graphics/material.hpp"
#include "utils/vec3.hpp"

/** This class stores information about the triangle that's under an object, i.e.:
 *  the normal, a pointer to the material, and the height above th
 * \ingroup tracks
 */
class TerrainInfo
{
private:
    int               m_HoT_frequency;      // how often hight of terrain is computed
    int               m_HoT_counter;        // compute HAT only every N timesteps
    Vec3              m_normal;             // normal of the triangle under the object
    const Material   *m_material;           // material of the triangle under the object
    const Material   *m_last_material;      // the previous material a kart was on
    float             m_HoT;                // height of terrain

public:
    TerrainInfo(int frequency=1);
    TerrainInfo(const Vec3 &pos, int frequency=1);
    virtual ~TerrainInfo() {};

    virtual void update(const Vec3 &pos);

    /** Returns the height of the terrain. we're currently above */
    float getHoT()                       const {return m_HoT;          }
    
    /** Returns the current material the kart is on. */
    const Material *getMaterial()        const {return m_material;     }
    /** Returns the previous material the kart was one (which might be
     *  the same as getMaterial() ). */
    const Material *getLastMaterial()    const {return m_last_material;}
    /** Returns the normal of the terrain the kart is on. */
    const Vec3 &getNormal()              const {return m_normal;       }
    /** Returns the pitch of the terrain depending on the heading. */
    float getTerrainPitch(float heading) const;
};  // TerrainInfo

#endif // HEADER_TERRAIN_INFO_HPP
