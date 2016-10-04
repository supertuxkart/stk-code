//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2015 Joerg Henrichs
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

#include "utils/vec3.hpp"

class btTransform;
class Material;

/** This class stores information about the triangle that's under an object, i.e.:
 *  the normal, a pointer to the material, and the height above th
 * \ingroup tracks
 */
class TerrainInfo
{
private:
    /** Normal of the triangle under the object. */
    Vec3              m_normal;
    /** Material of the triangle under the object. */
    const Material   *m_material;
    /** The previous material a kart was on. */
    const Material   *m_last_material;
    /** The point that was hit. */
    Vec3              m_hit_point;

    /** DEBUG only: origin of raycast. */
    Vec3 m_origin_ray;

public:
             TerrainInfo();
             TerrainInfo(const Vec3 &pos);
    virtual ~TerrainInfo() {};

    bool     getSurfaceInfo(const Vec3 &from, Vec3 *position,
                            const Material **m);
    virtual void update(const btMatrix3x3 &rotation, const Vec3 &from);
    virtual void update(const Vec3 &from);
    virtual void update(const Vec3 &from, const Vec3 &towards);

    // ------------------------------------------------------------------------
    /** Simple wrapper with no offset. */
    virtual void update(const btMatrix3x3 &rotation)
    {
        update(rotation, Vec3(0,0,0));
    }
    // ------------------------------------------------------------------------
    /** Returns the height of the terrain. we're currently above */
    float getHoT()                       const {return m_hit_point.getY(); }
    // ------------------------------------------------------------------------
    /** Returns the current material the kart is on. */
    const Material *getMaterial()        const {return m_material;     }
    // ------------------------------------------------------------------------
    /** Returns the previous material the kart was one (which might be
     *  the same as getMaterial() ). */
    const Material *getLastMaterial()    const {return m_last_material;}
    // ------------------------------------------------------------------------
    /** Returns the normal of the terrain the kart is on. */
    const Vec3 &getNormal()              const {return m_normal;       }
    // ------------------------------------------------------------------------
    /** Returns the pitch of the terrain depending on the heading. */
    float getTerrainPitch(float heading) const;
    // ------------------------------------------------------------------------
    /** Returns the hit point of the raycast. */
    const btVector3& getHitPoint() const { return m_hit_point; }
    const Vec3& getOrigin() const { return m_origin_ray;  }

};  // TerrainInfo

#endif // HEADER_TERRAIN_INFO_HPP
