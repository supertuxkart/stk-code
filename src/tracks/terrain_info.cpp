//  $Id: terrain_info.cpp 1284 2007-11-08 12:31:54Z hikerstk $
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
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "tracks/terrain_info.hpp"

#include <math.h>

#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

TerrainInfo::TerrainInfo(int frequency)
{
    m_HoT_frequency = frequency;
    m_HoT_counter   = frequency;
    m_last_material = NULL;
}

//-----------------------------------------------------------------------------
TerrainInfo::TerrainInfo(const Vec3 &pos, int frequency)
{
    m_HoT_frequency = frequency;
    m_HoT_counter   = frequency;
    // initialise HoT
    update(pos);
}
//-----------------------------------------------------------------------------
void TerrainInfo::update(const Vec3& pos)
{
    m_HoT_counter++;
    if(m_HoT_counter>=m_HoT_frequency)
    {
        m_last_material = m_material;
        World::getWorld()->getTrack()->getTerrainInfo(pos, &m_HoT,
                                                      &m_normal, &m_material);
        m_normal.normalize();
        m_HoT_counter = 0;
    }
}   // update

// -----------------------------------------------------------------------------
/** Returns the pitch of the terrain depending on the heading
*/
float TerrainInfo::getTerrainPitch(float heading) const {
    if(m_HoT==Track::NOHIT) return 0.0f;

    const float X = sin(heading);
    const float Z = cos(heading);
    // Compute the angle between the normal of the plane and the line to
    // (x,0,z).  (x,0,z) is normalised, so are the coordinates of the plane,
    // simplifying the computation of the scalar product.
    float pitch = ( m_normal.getX()*X + m_normal.getZ()*Z );  // use (x, 0, z)

    // The actual angle computed above is between the normal and the (x, 0, z)
    // line, so to compute the actual angles 90 degrees must be subtracted.
    pitch = -acosf(pitch) + NINETY_DEGREE_RAD;
    return pitch;
}   // getTerrainPitch
