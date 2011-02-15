//  $Id$
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
#include "physics/triangle_mesh.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

/** Constructor to initialise terrain data.
 */
TerrainInfo::TerrainInfo()
{
    m_last_material = NULL;
}   // TerrainInfo

//-----------------------------------------------------------------------------
/** Constructor to initialise terrain data at a given position
 *  \param pos The position to get the data from.
 */
TerrainInfo::TerrainInfo(const Vec3 &pos)
{
    // initialise HoT
    update(pos);
}   // TerrainInfo
//-----------------------------------------------------------------------------
/** Update the terrain information based on the latest position.
 *  \param Position from which to start the rayast from. 
 */
void TerrainInfo::update(const Vec3& pos)
{
    m_last_material = m_material;
    btVector3 to(pos);
    to.setY(-100000.f);

    const TriangleMesh &tm = World::getWorld()->getTrack()->getTriangleMesh();
    tm.castRay(pos, to, &m_hit_point, &m_material, &m_normal);
}   // update

// -----------------------------------------------------------------------------
/** Does a raycast upwards from the given position
If the raycast indicated that the kart is 'under something' (i.e. a 
 *  specially marked terrain), to another raycast up to detect under whic
 *  mesh the kart is. This is using the special gfx effect mesh only.
 *  This is used e.g. to detect if a kart is under water, and then to 
 *  get the proper position for water effects. Note that the TerrainInfo
 *  objects keeps track of the previous raycast position.
 */
bool TerrainInfo::getSurfaceInfo(const Vec3 &from, Vec3 *position, 
                                 const Material **material)
{
    Vec3 to=from+Vec3(0, 10000, 0);
    const TriangleMesh &tm = World::getWorld()->getTrack()->getGFXEffectMesh();
    return tm.castRay(from, to, position, material);
}   // getSurfaceInfo

// -----------------------------------------------------------------------------
/** Returns the pitch of the terrain depending on the heading
*/
float TerrainInfo::getTerrainPitch(float heading) const {
    if(!m_material) return 0.0f;

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
