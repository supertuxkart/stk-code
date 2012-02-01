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

#include "karts/kart_gfx.hpp"

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "graphics/particle_emitter.hpp"
#include "graphics/particle_kind.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "karts/kart.hpp"
#include "physics/btKart.hpp"

#include <iostream>

KartGFX::KartGFX(const Kart *kart)
{
    if(!UserConfigParams::m_graphical_effects)
    {
        for(unsigned int i=0; i<KGFX_COUNT; i++)
            m_all_emitters.push_back(NULL);
        return;
    }

    m_kart = kart;

    Vec3 rear_center(0, kart->getKartHeight()*0.35f, 
                       -kart->getKartLength()*0.35f);

    // Create all effects. Note that they must be created
    // in the order of KartGFXType.
    addEffect(KGFX_NITRO,   "nitro.xml",       rear_center);
    addEffect(KGFX_ZIPPER,  "zipper_fire.xml", rear_center);
    addEffect(KGFX_TERRAIN, "smoke.xml",       Vec3(0,0,0));
    addEffect(KGFX_SKID1,   "skid1.xml",       rear_center);
    addEffect(KGFX_SKID2,   "skid2.xml",       rear_center);

}   // KartGFX

// ----------------------------------------------------------------------------
/** Destructor. Frees all particle effects and kinds.
 */
KartGFX::~KartGFX()
{
    for(unsigned int i=0; i<KGFX_COUNT; i++)
    {
        if(m_all_emitters[i])
            delete m_all_emitters[i];
    }   // for i < KGFX_COUNT

}   // ~KartGFX

// ----------------------------------------------------------------------------
/** Creates a new particle node with the specified particle kind read from
 *  the given file.
 *  \param type The KGFX_ type of this type.
 *  \param file_name The file name of the particle specification.
 *  \param position Where on the kart the particles should be emitted.
 */
void KartGFX::addEffect(KartGFXType type, const std::string &file_name, 
                        const Vec3 &position)
{
    const ParticleKind *kind    = NULL;
    ParticleEmitter    *emitter = NULL;
    try
    {

        kind = ParticleKindManager::get()->getParticles(file_name);
        //kind    = new ParticleKind(file_manager->getGfxFile(file_name));
        // Skid2 is only used to store the emitter type, and a wheeless
        // kart has no terrain effects.
        if(type==KGFX_SKID2 || (type==KGFX_TERRAIN && m_kart->isWheeless()) )
            emitter = NULL;
        else if(type==KGFX_TERRAIN)
            // Terrain is NOT a child of the kart, since bullet returns the
            // raycast info in world coordinates
            emitter = new ParticleEmitter(kind, position);
        else
            emitter = new ParticleEmitter(kind, position, m_kart->getNode());
    }
    catch (std::runtime_error& e)
    {
        // If an error happens, mark this emitter as non existant
        // by adding a NULL to the list (which is tested for in all
        // cases). C++ guarantees that all memory allocated in the
        // constructor is properly freed.
        std::cerr << e.what() << std::endl;
        kind    = NULL;
        emitter = NULL;
    }
    assert(m_all_emitters.size()==type);
    m_all_emitters.push_back(emitter);
    if(type==KGFX_SKID1)
        m_skid_kind1 = kind;
    else if (type==KGFX_SKID2)
        m_skid_kind2 = kind;
}   // addEffect

// ----------------------------------------------------------------------------
/** Resets all particle emitters. Used at the (re)start of a race.
 */
void KartGFX::reset()
{
    m_wheel_toggle = 1;
    for(unsigned int i=0; i<m_all_emitters.size(); i++)
    {
        if(m_all_emitters[i])
        {
            m_all_emitters[i]->setCreationRateAbsolute(0);
            m_all_emitters[i]->clearParticles();
        }
    }
}   // reset

// ----------------------------------------------------------------------------
/** Selects the correct skidding particle type depending on skid bonus level.
 *  \param type Must be either KGFX_SKID1 or KGFX_SKID2 - the particle type
 *         to use corresponding to the bonus level.
 */
void KartGFX::setSkidLevel(const unsigned int level)
{
    assert(level >= 1);
    assert(level <= 2);
    const ParticleKind *pk = level==1 ? m_skid_kind1 : m_skid_kind2;
    if(m_all_emitters[KGFX_SKID1])
        m_all_emitters[KGFX_SKID1]->setParticleType(pk);
}   // setSkidLevel

// ----------------------------------------------------------------------------
/** Sets a new particle type to be used. Note that the memory of this
 *  kind must be managed by the caller.
 *  \param type The emitter type for which to set the new particle type.
 *  \param pk The particle kind to use.
 */
void KartGFX::setParticleKind(const KartGFXType type, const ParticleKind *pk)
{
    ParticleEmitter *pe = m_all_emitters[KGFX_TERRAIN];
    if(!pe) return;

    pe->setParticleType(pk);
}   // setParticleKind

// ----------------------------------------------------------------------------
/** Defines the new position of the specified emitter.
 *  \param type The emitter to set a new position for.
 *  \param xyz The new position of the emitter.
 */
void KartGFX::setXYZ(const KartGFXType type, const Vec3 &xyz)
{
    ParticleEmitter *pe = m_all_emitters[KGFX_TERRAIN];
    if(!pe) return;
    pe->setPosition(xyz);
}   // setXYZ

// ----------------------------------------------------------------------------
/** Sets the absolute creation rate for the specified particle type. 
 *  \param type The particle effect for which to set the 
 *         creation rate (in particles per seconds).
 *  \param f The new creation rate.
 */
void KartGFX::setCreationRateAbsolute(KartGFXType type, float f)
{
    if(m_all_emitters[type])
        m_all_emitters[type]->setCreationRateAbsolute(f);
}   // setCreationRateAbsolute

// ----------------------------------------------------------------------------
/** Sets the creation rate for the specified particle type relative to the
 *  given minimum and maximum particle rate. If a negative value is 
 *  specified, the creation rate will be set to 0 (absolute).
 *  \param type The particle effect for which to set the 
 *         creation rate (<0 means no more particles).
 *  \param f The new relative creation rate.
 */
void KartGFX::setCreationRateRelative(KartGFXType type, float f)
{
    if(m_all_emitters[type])
    {
        if(f<0)
            m_all_emitters[type]->setCreationRateAbsolute(0);
        else
            m_all_emitters[type]->setCreationRateRelative(f);
    }
}   // setCreationRateRelative

// ----------------------------------------------------------------------------
/** Resize the area from which the particles are emitted: the emitter box 
 *  should spread from last frame's position to the current position if 
 *  we want the particles to be emitted in a smooth, continuous flame and not 
 *  in blobs.
 *  \param type The particle effect for which to resize the emitting box.
 *  \param speed Current speed of the kart.
 *  \param dt Time step size.
 */
void KartGFX::resizeBox(KartGFXType type, float speed, float dt)
{
    if(m_all_emitters[type])
        m_all_emitters[type]->resizeBox(std::max(0.25f, speed*dt));
}   // resizeBox

// ----------------------------------------------------------------------------
/** If necessary defines a new particle type for the terrain emitter. Then 
 *  adjusts the location of the terrain emitter to be in synch with the 
 *  current wheel position, and defines the emission rate depending on speed,
 *  steering, and skidding.
 *  \param pk Particle type to use.
 */
void KartGFX::updateTerrain(const ParticleKind *pk)
{
   ParticleEmitter *pe = m_all_emitters[KGFX_TERRAIN];
    if(!pe) return;

    pe->setParticleType(pk);
 
    const btWheelInfo &wi = m_kart->getVehicle()
                                  ->getWheelInfo(2+m_wheel_toggle);
    Vec3 xyz = wi.m_raycastInfo.m_contactPointWS;
    // FIXME: the X and Z position is not always accurate.
    xyz.setX(xyz.getX()+ 0.06f * (m_wheel_toggle ? +1 : -1));
    xyz.setZ(xyz.getZ()+0.06f);
    pe->setPosition(xyz);

    // Now compute the particle creation rate:
    float rate           = 0;
    const float speed    = fabsf(m_kart->getSpeed());
    const float skidding = m_kart->getSkidding();
    if (skidding > 1.0f)
        rate = fabsf(m_kart->getControls().m_steer) > 0.8 ? skidding - 1 : 0;
    else if (speed >= 0.5f)
        rate = speed/m_kart->getKartProperties()->getMaxSpeed();
    else
    {
        pe->setCreationRateAbsolute(0);
        return;
    }
    // m_skidding can be > 2, and speed > maxSpeed (if powerups are used).
    if(rate>1.0f) rate = 1.0f;
    pe->setCreationRateRelative(rate);
}   // updateTerrain

// ----------------------------------------------------------------------------
/** Updates all gfx.
 *  \param dt Time step size.
 */
void KartGFX::update(float dt)
{
    if(!UserConfigParams::m_graphical_effects) return;

    m_wheel_toggle = 1 - m_wheel_toggle;

    for(unsigned int i=0; i<m_all_emitters.size(); i++)
    {
        if(m_all_emitters[i])
            m_all_emitters[i]->update(dt);
    }

}  // update

