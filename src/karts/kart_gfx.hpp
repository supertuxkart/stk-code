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
     *  Nitro, zipper, and skidding effects. KGFX_COUNT
     *  is the number of entries and must therefore be last. */
    enum KartGFXType { KGFX_NITRO=0,
                       KGFX_ZIPPER,
                       KGFX_SKID,
                       KGFX_COUNT};

private:
    /** Vector of all particle kinds. */
    std::vector<ParticleKind*> m_all_particle_kinds;

    /** Vector of all particle emitters. */
    std::vector<ParticleEmitter*> m_all_emitters;

    /** Pointer to the owner of this kart. */
    const Kart *m_kart;

    void addEffect(KartGFXType type, const std::string &file_name, 
                   const Vec3 &position);

public:
         KartGFX(const Kart *kart);
        ~KartGFX();
    void reset();
    void update(float dt);
    void setCreationRateAbsolute(KartGFXType type, float f);
    void setCreationRateRelative(KartGFXType type, float f);
    void resizeBox(KartGFXType type, float speed, float dt);
};   // KartWGFX
#endif
