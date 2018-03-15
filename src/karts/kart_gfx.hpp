//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015  Joerg Henrichs
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

class AbstractKart;
class ParticleEmitter;
class ParticleKind;
class Vec3;

namespace irr {
    namespace scene {
        class ISceneNode;
    }
}

class KartGFX
{
public:
    /** All particle effects supported by this object.
     *  Nitro, zipper, terrain, and skidding effects.  Two different
     *  skid types are supported, but only one emitter node will be
     *  created. So KGFX_SKID1/2 store the two types, and KGFX_SKID
     *  = KGFX_SKID1 stores the actual emitter node. KGFX_COUNT
     *  is the number of entries and must therefore be last. */
    enum KartGFXType { KGFX_NITRO1=0,
                       KGFX_NITRO2,
                       KGFX_NITROSMOKE1,
                       KGFX_NITROSMOKE2,
                       KGFX_ZIPPER,
                       KGFX_TERRAIN,
                       KGFX_SKIDL,
                       KGFX_SKIDR,
                       KGFX_SKID1L = KGFX_SKIDL,
                       KGFX_SKID1R = KGFX_SKIDR,
                       KGFX_SKID2L,
                       KGFX_SKID2R,
                       KGFX_EXHAUST1,
                       KGFX_EXHAUST2,
                       KGFX_COUNT};

private:
    /** The particle kind for skidding bonus level 1. */
    const ParticleKind *m_skid_kind1;

    /** The particle kind for skidding bonus level 2. */
    const ParticleKind *m_skid_kind2;

    /** Vector of all particle emitters. */
    std::vector<ParticleEmitter*> m_all_emitters;

    /** Pointer to the owner of this kart. */
    const AbstractKart *m_kart;

    /** Used to alternate particle effects from the rear wheels. */
    int         m_wheel_toggle;
    
    /** A skid level that is currently in use */
    int m_skid_level;

    /** A light that's shown when the kart uses nitro. */
    irr::scene::ISceneNode* m_nitro_light;

    /** Light that is shown when the kart is skidding. */
    irr::scene::ISceneNode* m_skidding_light_1;

    /** A light that's shown on the second skid-level with another color. */
    irr::scene::ISceneNode* m_skidding_light_2;

    void addEffect(KartGFXType type, const std::string &file_name,
                   const Vec3 &position, bool important);
    void resizeBox(const KartGFXType type, float new_size);

public:
         KartGFX(const AbstractKart *kart, bool is_day);
        ~KartGFX();
    void reset();
    void setSkidLevel(const unsigned int level);
    void setParticleKind(const KartGFXType type, const ParticleKind *pk);
    void setXYZ(const KartGFXType type, const Vec3 &xyz);
    void setCreationRateAbsolute(const KartGFXType type, float f);
    void setCreationRateRelative(const KartGFXType type, float f);
    void updateTerrain(const ParticleKind *pk);
    void update(float dt);
    void updateNitroGraphics(float f);
    void updateSkidLight(unsigned int level);
    void getGFXStatus(int* nitro, bool* zipper,
                      int* skidding, bool* red_skidding) const;
    void setGFXFromReplay(int nitro, bool zipper,
                          int skidding, bool red_skidding);
    void setGFXInvisible();

};   // KartWGFX
#endif
