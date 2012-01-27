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
#include "karts/kart.hpp"

#include <iostream>

KartGFX::KartGFX(const Kart *kart)
{
    if(!UserConfigParams::m_graphical_effects)
    {
        for(unsigned int i=0; i<KGFX_COUNT; i++)
        {
            m_all_emitters.push_back(NULL);
            m_all_particle_kinds.push_back(NULL);
        }
        return;
    }

    m_kart = kart;

    Vec3 position(0, kart->getKartHeight()*0.35f, 
                    -kart->getKartLength()*0.35f);

    // Create all effects. Note that they must be created
    // in the order of KartGFXType.
    addEffect(KGFX_NITRO,  "nitro.xml",       position);
    addEffect(KGFX_ZIPPER, "zipper_fire.xml", position);
    addEffect(KGFX_SKID,   "nitro.xml",       position);

}   // KartGFX

// ----------------------------------------------------------------------------
KartGFX::~KartGFX()
{
    for(unsigned int i=0; i<KGFX_COUNT; i++)
    {
        if(m_all_emitters[i])
            delete m_all_emitters[i];
        if(m_all_particle_kinds[i])
            delete m_all_particle_kinds[i];
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
    ParticleKind    *kind    = NULL;
    ParticleEmitter *emitter = NULL;
    try
    {
        kind    = new ParticleKind(file_manager->getGfxFile(file_name));
        emitter = new ParticleEmitter(kind, 
                                      position, 
                                      m_kart->getNode()                );
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
    assert(m_all_particle_kinds.size()==type);
    m_all_particle_kinds.push_back(kind);
}   // addEffect

// ----------------------------------------------------------------------------
void KartGFX::reset()
{
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
/** Updates all gfx.
 *  \param dt Time step size.
 */
void KartGFX::update(float dt)
{
    if(!UserConfigParams::m_graphical_effects) return;

    for(unsigned int i=0; i<m_all_emitters.size(); i++)
    {
        if(m_all_emitters[i])
            m_all_emitters[i]->update(dt);
    }

}  // update

// ----------------------------------------------------------------------------
/** Sets the creation rate for the specified particle type relative to the
 *  given minimum and maximum particle rate.
 *  \param type The particle effect for which to set the 
 *         creation rate.
 *  \param f The new relative creation rate.
 */
void KartGFX::setCreationRateRelative(KartGFXType type, float f)
{
    if(m_all_emitters[type])
        m_all_emitters[type]->setCreationRateRelative(f);
}   // setCreationRate

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
}   // setCreationRate

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