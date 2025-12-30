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

#include "karts/boost_observer.hpp"

#include <vector>
#include <string>

class Kart;
class ParticleEmitter;
class ParticleKind;
class Vec3;

#ifndef SERVER_ONLY
class ParticleEmitter;
#endif

namespace irr {
    namespace scene {
        class ISceneNode;
    }
}

/** Implements all particle effects for a kart.
 *  Also implements IBoostObserver to receive boost activation events
 *  from MaxSpeed for event-driven particle bursts.
 */
class KartGFX : public IBoostObserver
{
public:
    /** All particle effects supported by this object.
     *  Nitro, zipper, terrain, and skidding effects.  Three different
     *  skid types are supported, but only one emitter node will be
     *  created. So KGFX_SKID1/2/3 store the two types, and KGFX_SKID
     *  = KGFX_SKID1 stores the actual emitter node. KGFX_COUNT
     *  is the number of entries and must therefore be last. */
    enum KartGFXType { KGFX_NITRO1=0,
                       KGFX_NITRO2,
                       KGFX_NITROHACK1,
                       KGFX_NITROHACK2,
                       KGFX_NITROSMOKE1,
                       KGFX_NITROSMOKE2,
                       KGFX_ZIPPER,
                       KGFX_TERRAIN,
                       KGFX_SKIDL,
                       KGFX_SKIDR,
                       KGFX_SKIDL2,
                       KGFX_SKIDR2,
                       KGFX_SKID1L = KGFX_SKIDL,
                       KGFX_SKID1R = KGFX_SKIDR,
                       KGFX_SKID2L = KGFX_SKIDL2,
                       KGFX_SKID2R = KGFX_SKIDR2,
                       KGFX_SKID3L,
                       KGFX_SKID3R,
                       KGFX_SKID0L,
                       KGFX_SKID0R,
                       KGFX_EXHAUST1,
                       KGFX_EXHAUST2,
                       KGFX_BOOST_BURST1,  // Dedicated boost activation particles
                       KGFX_BOOST_BURST2,
                       KGFX_COUNT};

private:
    /** The particle kind for skidding bonus level 0 (no boost yet). */
    const ParticleKind *m_skid_kind0;

    /** The particle kind for skidding bonus level 1 (yellow). */
    const ParticleKind *m_skid_kind1;

    /** The particle kind for skidding bonus level 2 (red). */
    const ParticleKind *m_skid_kind2;

    /** The particle kind for skidding bonus level 3 (purple). */
    const ParticleKind *m_skid_kind3;

    /** Vector of all particle emitters. */
    std::vector<ParticleEmitter*> m_all_emitters;

    /** Pointer to the owner of this kart. */
    const Kart *m_kart;

    /** Used to alternate particle effects from the rear wheels. */
    int         m_wheel_toggle;
    
    /** A skid level that is currently in use */
    int m_skid_level;

    /** A light that's shown when the kart uses nitro. */
    irr::scene::ISceneNode* m_nitro_light;

    /** A light that's shown when the kart uses nitro with "nitro-hack" on. */
    irr::scene::ISceneNode* m_nitro_hack_light;

    // ========================================================================
    // Boost-triggered particle effect timers
    // Set by onBoostActivated(), decay to 0.0 over time for particle bursts
    // NOTE: Edge detection is now handled by MaxSpeed observer notifications
    // ========================================================================
    float m_nitro_effect_timer = 0.0f;
    float m_zipper_effect_timer = 0.0f;
    float m_skid_bonus_effect_timer = 0.0f;

    // Effect durations in seconds
    static constexpr float NITRO_EFFECT_DURATION = 1.5f;
    static constexpr float ZIPPER_EFFECT_DURATION = 2.0f;
    static constexpr float SKID_BONUS_EFFECT_DURATION = 1.2f;

    /** Light that is shown when the kart is skidding. */
    irr::scene::ISceneNode* m_skidding_light_1;

    /** A light that's shown on the second skid-level with another color. */
    irr::scene::ISceneNode* m_skidding_light_2;

    /** A light that's shown on the third skid-level with another color. */
    irr::scene::ISceneNode* m_skidding_light_3;

    void addEffect(KartGFXType type, const std::string &file_name,
                   const Vec3 &position, bool important);
    void resizeBox(const KartGFXType type, float new_size);

    bool supportsLight() const;
public:
         KartGFX(const Kart *kart, bool is_day);
        ~KartGFX();
    void reset();
    void setSkidLevel(const unsigned int level, const unsigned int upcoming_level);
    void setParticleKind(const KartGFXType type, const ParticleKind *pk);
    void setXYZ(const KartGFXType type, const Vec3 &xyz);
    void setCreationRateAbsolute(const KartGFXType type, float f);
    void setCreationRateRelative(const KartGFXType type, float f);
    void updateTerrain(const ParticleKind *pk);
    void update(float dt);
    void updateNitroGraphics(float f, bool isNitroHackOn, bool activeNitro);
    void updateBoostParticleEffects(float dt);
    void updateSkidLight(unsigned int level);
    void getGFXStatus(int* nitro, bool* zipper,
                      int* skidding, bool* red_skidding, bool* purple_skidding) const;
    void setGFXFromReplay(int nitro, bool zipper,
                          int skidding, bool red_skidding, bool purple_skidding);
    void setGFXInvisible();

    // ========================================================================
    // IBoostObserver implementation
    // ========================================================================
    /** Called by MaxSpeed when a boost activates.
     *  Triggers particle burst effects for ALL karts (particles visible to everyone).
     *  @param kart The kart that activated the boost
     *  @param category The boost type (MS_INCREASE_*)
     *  @param add_speed The speed increase value
     *  @param duration_ticks How long the boost lasts
     */
    void onBoostActivated(Kart* kart, unsigned int category,
                          float add_speed, int duration_ticks) override;

};   // KartGFX
#endif
