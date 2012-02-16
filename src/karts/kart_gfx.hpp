//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012  Joerg Henrichs
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

#ifndef HEADER_KART_GFX_HPP
#define HEADER_KART_GFX_HPP

/** \defgroup karts 
 *  This class implements all particle effects for a kart. */

#include <vector>
#include <string>

class Kart;
class ParticleEmitter;
class ParticleKind;
class Vec3;

class KartGFX
{
public:
    /** All particle effects supported by this object. 
     *  Nitro, zipper, terrain, and skidding effects.  Two different
     *  skid types are supported, but only one emitter node will be 
     *  created. So KGFX_SKID1/2 store the two types, and KGFX_SKID
     *  = KGFX_SKID1 stores the actual emitter node. KGFX_COUNT
     *  is the number of entries and must therefore be last. */
    enum KartGFXType { KGFX_NITRO=0,
                       KGFX_ZIPPER,
                       KGFX_TERRAIN,
                       KGFX_SKID,
                       KGFX_SKID1=KGFX_SKID,
                       KGFX_SKID2,
                       KGFX_COUNT};

private:
    /** The particle kind for skidding bonus level 1. */
    const ParticleKind *m_skid_kind1;

    /** The particle kind for skidding bonus level 2. */
    const ParticleKind *m_skid_kind2;

    /** Vector of all particle emitters. */
    std::vector<ParticleEmitter*> m_all_emitters;

    /** Pointer to the owner of this kart. */
    const Kart *m_kart;

    /** Used to alternate particle effects from the rear wheels. */
    int         m_wheel_toggle;

    void addEffect(KartGFXType type, const std::string &file_name, 
                   const Vec3 &position);

public:
         KartGFX(const Kart *kart);
        ~KartGFX();
    void reset();
    void setSkidLevel(const unsigned int level);
    void setParticleKind(const KartGFXType type, const ParticleKind *pk);
    void setXYZ(const KartGFXType type, const Vec3 &xyz);
    void setCreationRateAbsolute(const KartGFXType type, float f);
    void setCreationRateRelative(const KartGFXType type, float f);
    void resizeBox(const KartGFXType type, float speed, float dt);
    void updateTerrain(const ParticleKind *pk);
    void update(float dt);
};   // KartWGFX
#endif
