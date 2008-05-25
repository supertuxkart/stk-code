//  $Id: terrain_info.hpp 1284 2007-11-08 12:31:54Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_TERRAIN_INFO_H
#define HEADER_TERRAIN_INFO_H

#include "vec3.hpp"
#include "material.hpp"

/** This class stores information about the triangle that's under an object, i.e.:
 *  the normal, a pointer to the material, and the height above th
 */
class TerrainInfo
{
private:
    int               m_HoT_frequency;      // how often hight of terrain is computed
    int               m_HoT_counter;        // compute HAT only every N timesteps
    Vec3              m_normal;             // normal of the triangle under the object
    const Material   *m_material;           // material of the triangle under the object
    float             m_HoT;                // height of terrain

public:
                     TerrainInfo(int frequency=1) {m_HoT_frequency=frequency;
                                                   m_HoT_counter=frequency;  }
                     TerrainInfo(const Vec3 &pos, int frequency=1);
    virtual         ~TerrainInfo() {};
    virtual void     update(const Vec3 &pos);
    float            getHoT()      const { return m_HoT;      }
    const Material  *getMaterial() const { return m_material; }
    const Vec3      &getNormal()   const { return m_normal;   }
    float            getTerrainPitch(float heading) const;

};   // TerrainInfo

#endif
