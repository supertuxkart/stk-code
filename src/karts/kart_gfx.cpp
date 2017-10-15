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

#include "karts/kart_gfx.hpp"

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/particle_emitter.hpp"
#include "graphics/particle_kind.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/skidding.hpp"
#include "physics/btKart.hpp"
#include "utils/log.hpp"
#include "race/race_manager.hpp"

#include <iostream>

KartGFX::KartGFX(const AbstractKart *kart, bool is_day)
{
    m_nitro_light = NULL;
    m_skidding_light_1 = NULL;
    m_skidding_light_2 = NULL;
    m_kart = kart;
    m_wheel_toggle = 0;
    m_skid_level = 0;

    const KartModel *km = m_kart->getKartModel();
    const float length = km->getLength();

    scene::ISceneNode *node = m_kart->getNode();
    // Create nitro light
    core::vector3df location(0.0f, 0.5f, -0.5f*length - 0.05f);
#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        m_nitro_light = irr_driver->addLight(location, /*force*/ 0.4f,
                                             /*radius*/ 5.0f, 0.0f, 0.4f, 1.0f,
                                             false, node);
        m_nitro_light->setVisible(false);
    #ifdef DEBUG
        m_nitro_light->setName( ("nitro emitter (" + m_kart->getIdent()
                                                   + ")").c_str() );
    #endif
    
        // Create skidding lights
        // For the first skidding level
        m_skidding_light_1 = 
            irr_driver->addLight(core::vector3df(0.0f, 0.1f, -0.5f * length - 
                                 0.05f), /* force */ 0.3f, /*radius*/ 3.0f,
                                 1.0f, 0.6f, 0.0f, false, node);
        m_skidding_light_1->setVisible(false);
        m_skidding_light_1->setName(("skidding emitter 1 (" + m_kart->getIdent() 
                                                            + ")").c_str() );
    
        // For the second skidding level
        m_skidding_light_2 =
            irr_driver->addLight(core::vector3df(0.0f, 0.1f, -0.5f * length - 
                                 0.05f), /* force */0.4f, /*radius*/4.0f,
                                 1.0f, 0.0f, 0.0f, false, node);
        m_skidding_light_2->setVisible(false);
        m_skidding_light_2->setName(("skidding emitter 2 (" + m_kart->getIdent()
                                                            + ")").c_str() );
        m_nitro_light->grab();
        m_skidding_light_1->grab();
        m_skidding_light_2->grab();
    }
#endif

    // Create particle effects
    Vec3 rear_left(kart->getWheelGraphicsPosition(3).getX(), 0.05f,
                   kart->getWheelGraphicsPosition(3).getZ()-0.1f    );
    Vec3 rear_right(kart->getWheelGraphicsPosition(2).getX(), 0.05f,
                    kart->getWheelGraphicsPosition(2).getZ()-0.1f   );

    Vec3 rear_center(0, kart->getKartHeight()*0.35f, -0.35f*length);

    Vec3 rear_nitro_center(0, kart->getKartHeight()*0.2f, -0.1f*length);

    // FIXME Used to match the emitter as seen in blender
    const Vec3 delta(0, 0, 0.6f);
    Vec3 rear_nitro_right = km->getNitroEmittersPositon(0) + delta;
    Vec3 rear_nitro_left  = km->getNitroEmittersPositon(1) + delta;
    if (!km->hasNitroEmitters())
        rear_nitro_right = rear_nitro_left = rear_nitro_center;

    // Create all effects. Note that they must be created
    // in the order of KartGFXType.
    addEffect(KGFX_NITRO1,      "nitro.xml",       rear_nitro_right, true );
    addEffect(KGFX_NITRO2,      "nitro.xml",       rear_nitro_left,  true );
    addEffect(KGFX_NITROSMOKE1, "nitro-smoke.xml", rear_nitro_left,  false);
    addEffect(KGFX_NITROSMOKE2, "nitro-smoke.xml", rear_nitro_right, false);
    addEffect(KGFX_ZIPPER,      "zipper_fire.xml", rear_center,      true );
    addEffect(KGFX_TERRAIN,     "smoke.xml",       Vec3(0, 0, 0),    false);
    addEffect(KGFX_SKID1L,      "skid1.xml",       rear_left,        true );
    addEffect(KGFX_SKID1R,      "skid1.xml",       rear_right,       true );
    addEffect(KGFX_SKID2L,      "skid2.xml",       rear_left,        true );
    addEffect(KGFX_SKID2R,      "skid2.xml",       rear_right,       true );
    if (!kart->getKartModel()->getExhaustXML().empty())
    {
        const std::string& ex = kart->getKartModel()->getExhaustXML();
        addEffect(KGFX_EXHAUST1, ex, rear_nitro_right, false);
        addEffect(KGFX_EXHAUST2, ex, rear_nitro_left, false);
    }
    else
    {
        m_all_emitters.push_back(NULL);
        m_all_emitters.push_back(NULL);
    }

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

#ifndef SERVER_ONLY    
    if (CVS->isGLSL())
    {
        m_nitro_light->drop();
        m_skidding_light_1->drop();
        m_skidding_light_2->drop();
    }
#endif

}   // ~KartGFX

// ----------------------------------------------------------------------------
/** Creates a new particle node with the specified particle kind read from
 *  the given file.
 *  \param type The KGFX_ type of this type.
 *  \param file_name The file name of the particle specification.
 *  \param position Where on the kart the particles should be emitted.
 */
void KartGFX::addEffect(KartGFXType type, const std::string &file_name,
                        const Vec3 &position, bool important)
{
#ifndef SERVER_ONLY
    if (((UserConfigParams::m_graphical_effects < 2 || !CVS->isGLSL()) &&
        (!important || m_kart->getType() == RaceManager::KT_AI ||
        m_kart->getType() == RaceManager::KT_SPARE_TIRE)) ||
        UserConfigParams::m_graphical_effects < 1)
    {
        m_all_emitters.push_back(NULL);
        return;
    }

    const ParticleKind *kind    = NULL;
    ParticleEmitter    *emitter = NULL;
    try
    {

        kind = ParticleKindManager::get()->getParticles(file_name);
        //kind    = new ParticleKind(file_manager->getGfxFile(file_name));
        // Skid2 is only used to store the emitter type, and a wheeless
        // kart has no terrain effects.
        if(type==KGFX_SKID2L || type==KGFX_SKID2R ||
            (type==KGFX_TERRAIN && m_kart->isWheeless()) )
            emitter = NULL;
        else if(type==KGFX_TERRAIN)
            // Terrain is NOT a child of the kart, since bullet returns the
            // raycast info in world coordinates
            emitter = new ParticleEmitter(kind, position, NULL, false,
                                          important);
        else
            emitter = new ParticleEmitter(kind, position, m_kart->getNode(),
                                          false, important);
    }
    catch (std::runtime_error& e)
    {
        // If an error happens, mark this emitter as non existant
        // by adding a NULL to the list (which is tested for in all
        // cases). C++ guarantees that all memory allocated in the
        // constructor is properly freed.
        Log::error("[KartGFX]", "%s",e.what());
        kind    = NULL;
        emitter = NULL;
    }
    assert((int)m_all_emitters.size()==type);
    m_all_emitters.push_back(emitter);
    if(type==KGFX_SKID1L || type==KGFX_SKID1R)
        m_skid_kind1 = kind;
    else if (type==KGFX_SKID2L || type==KGFX_SKID2R)
        m_skid_kind2 = kind;
#endif
}   // addEffect

// ----------------------------------------------------------------------------
/** Resets all particle emitters. Used at the (re)start of a race.
 */
void KartGFX::reset()
{
    m_wheel_toggle = 1;
#ifndef SERVER_ONLY
    for(unsigned int i=0; i<m_all_emitters.size(); i++)
    {
        if(m_all_emitters[i])
        {
            m_all_emitters[i]->setCreationRateAbsolute(0);
        }
    }
#endif
}   // reset

// ----------------------------------------------------------------------------
/** Selects the correct skidding particle type depending on skid bonus level.
 *  \param level Must be 1 (accumulated enough for level 1 bonus) or 2 
 *         (accumulated enough for level 2 bonus).
 */
void KartGFX::setSkidLevel(const unsigned int level)
{
    assert(level >= 1);
    assert(level <= 2);
    m_skid_level = level;
    const ParticleKind *pk = level==1 ? m_skid_kind1 : m_skid_kind2;
#ifndef SERVER_ONLY
    if(m_all_emitters[KGFX_SKID1L])
        m_all_emitters[KGFX_SKID1L]->setParticleType(pk);
    if(m_all_emitters[KGFX_SKID1R])
        m_all_emitters[KGFX_SKID1R]->setParticleType(pk);
    // Relative 0 means it will emitt the minimum rate, i.e. the rate
    // set to indicate that the bonus is now available.
    setCreationRateRelative(KartGFX::KGFX_SKIDL, 0.0f);
    setCreationRateRelative(KartGFX::KGFX_SKIDR, 0.0f);
#endif
}   // setSkidLevel

// ----------------------------------------------------------------------------
/** Sets a new particle type to be used. Note that the memory of this
 *  kind must be managed by the caller.
 *  \param type The emitter type for which to set the new particle type.
 *  \param pk The particle kind to use.
 */
void KartGFX::setParticleKind(const KartGFXType type, const ParticleKind *pk)
{
#ifndef SERVER_ONLY
    ParticleEmitter *pe = m_all_emitters[KGFX_TERRAIN];
    if(!pe) return;

    pe->setParticleType(pk);
#endif
}   // setParticleKind

// ----------------------------------------------------------------------------
/** Defines the new position of the specified emitter.
 *  \param type The emitter to set a new position for.
 *  \param xyz The new position of the emitter.
 */
void KartGFX::setXYZ(const KartGFXType type, const Vec3 &xyz)
{
#ifndef SERVER_ONLY
    ParticleEmitter *pe = m_all_emitters[KGFX_TERRAIN];
    if(!pe) return;
    pe->setPosition(xyz);
#endif
}   // setXYZ

// ----------------------------------------------------------------------------
/** Sets the absolute creation rate for the specified particle type.
 *  \param type The particle effect for which to set the
 *         creation rate (in particles per seconds).
 *  \param f The new creation rate.
 */
void KartGFX::setCreationRateAbsolute(KartGFXType type, float f)
{
#ifndef SERVER_ONLY
    if (!m_all_emitters[type])
        return;
        
    if (m_all_emitters[type]->getCreationRateFloat() == f)
        return;

    m_all_emitters[type]->setCreationRateAbsolute(f);
#endif
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
#ifndef SERVER_ONLY
    if(m_all_emitters[type])
    {
        if(f<0)
            m_all_emitters[type]->setCreationRateAbsolute(0);
        else
            m_all_emitters[type]->setCreationRateRelative(f);
    }
#endif
}   // setCreationRateRelative

// ----------------------------------------------------------------------------
/** Resize the area from which the particles are emitted: the emitter box
 *  should spread from last frame's position to the current position if
 *  we want the particles to be emitted in a smooth, continuous flame and not
 *  in blobs.
 *  \param type The particle effect for which to resize the emitting box.
 *  \param new_size New size of the box, typically speed*dt.
 */
void KartGFX::resizeBox(KartGFXType type, float new_size)
{
#ifndef SERVER_ONLY
    if(m_all_emitters[type])
        m_all_emitters[type]->resizeBox(std::max(0.25f, new_size));
#endif
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
#ifndef SERVER_ONLY
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
    const float skidding = m_kart->getSkidding()->getSkidFactor();
    // Only create particles when the kart is actually on ground
    bool on_ground       = m_kart->isOnGround() &&
                           m_kart->getSkidding()->getGraphicalJumpOffset()==0;
    if (skidding > 1.0f && on_ground)
        rate = fabsf(m_kart->getControls().getSteer()) > 0.8 ? skidding - 1 : 0;
    else if (speed >= 0.5f && on_ground)
        rate = speed/m_kart->getKartProperties()->getEngineMaxSpeed();
    else
    {
        pe->setCreationRateAbsolute(0);
        return;
    }
    // m_skidding can be > 2, and speed > maxSpeed (if powerups are used).
    if(rate>1.0f) rate = 1.0f;
    pe->setCreationRateRelative(rate);
#endif
}   // updateTerrain

// ----------------------------------------------------------------------------
/** Updates all gfx.
 *  \param dt Time step size.
 */
void KartGFX::update(float dt)
{
    m_wheel_toggle = 1 - m_wheel_toggle;

    for (unsigned int i = 0; i < m_all_emitters.size(); i++)
    {
        if (m_all_emitters[i])
            m_all_emitters[i]->update(dt);
    }

}  // update

// ----------------------------------------------------------------------------
/** Updates nitro dependent particle effects.
 *  \param nitro_frac Nitro fraction/
 */
void KartGFX::updateNitroGraphics(float nitro_frac)
{
#ifndef SERVER_ONLY
    // Upate particle effects (creation rate, and emitter size
    // depending on speed)
    // --------------------------------------------------------
    if (nitro_frac > 0)
    {
        setCreationRateRelative(KartGFX::KGFX_NITRO1, nitro_frac);
        setCreationRateRelative(KartGFX::KGFX_NITRO2, nitro_frac);
        setCreationRateRelative(KartGFX::KGFX_NITROSMOKE1, nitro_frac);
        setCreationRateRelative(KartGFX::KGFX_NITROSMOKE2, nitro_frac);
        
        if (CVS->isGLSL())
            m_nitro_light->setVisible(true);
    }
    else
    {
        setCreationRateAbsolute(KartGFX::KGFX_NITRO1,      0);
        setCreationRateAbsolute(KartGFX::KGFX_NITRO2,      0);
        setCreationRateAbsolute(KartGFX::KGFX_NITROSMOKE1, 0);
        setCreationRateAbsolute(KartGFX::KGFX_NITROSMOKE2, 0);
        
        if (CVS->isGLSL())
            m_nitro_light->setVisible(false);
    }
    
    // Exhaust is always emitting
    setCreationRateRelative(KartGFX::KGFX_EXHAUST1, 1.0);
    setCreationRateRelative(KartGFX::KGFX_EXHAUST2, 1.0);
#endif
}  // updateGraphics

// ----------------------------------------------------------------------------
/** Updates the skiddng light (including disabling it).
 *  \param level Which level of light to display: 0 no light at all,
 *         1: level 1, 2 level 2.
 */
void KartGFX::updateSkidLight(unsigned int level)
{
#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        m_skidding_light_1->setVisible(level == 1);
        m_skidding_light_2->setVisible(level > 1);
    }
#endif
}   // updateSkidLight

// ----------------------------------------------------------------------------
void KartGFX::getGFXStatus(int* nitro, bool* zipper,
                           int* skidding, bool* red_skidding) const
{
#ifndef SERVER_ONLY
    int n = 0;
    bool z = false;
    int s = 0;
    bool r = false;

    if (m_all_emitters[KGFX_NITRO1])
    {
        n = m_all_emitters[KGFX_NITRO1]->getCreationRate();
    }

    if (m_all_emitters[KGFX_ZIPPER])
    {
        z = m_all_emitters[KGFX_ZIPPER]->getCreationRate() > 0;
    }

    if (m_all_emitters[KGFX_SKIDL])
    {
        s = m_all_emitters[KGFX_SKIDL]->getCreationRate();
        r = m_skid_level == 2;
    }

    *nitro = n;
    *zipper = z;
    *skidding = s;
    *red_skidding = r;
#endif
}   // getGFXStatus

// ----------------------------------------------------------------------------
void KartGFX::setGFXFromReplay(int nitro, bool zipper,
                               int skidding, bool red_skidding)
{
#ifndef SERVER_ONLY
    if (nitro > 0)
    {
        setCreationRateAbsolute(KartGFX::KGFX_NITRO1,      (float)nitro);
        setCreationRateAbsolute(KartGFX::KGFX_NITRO2,      (float)nitro);
        setCreationRateAbsolute(KartGFX::KGFX_NITROSMOKE1, (float)nitro);
        setCreationRateAbsolute(KartGFX::KGFX_NITROSMOKE2, (float)nitro);
        
        if (CVS->isGLSL())
            m_nitro_light->setVisible(true);
    }
    else
    {
        setCreationRateAbsolute(KartGFX::KGFX_NITRO1,      0.0f);
        setCreationRateAbsolute(KartGFX::KGFX_NITRO2,      0.0f);
        setCreationRateAbsolute(KartGFX::KGFX_NITROSMOKE1, 0.0f);
        setCreationRateAbsolute(KartGFX::KGFX_NITROSMOKE2, 0.0f);
        
        if (CVS->isGLSL())
            m_nitro_light->setVisible(false);
    }

    if (zipper)
        setCreationRateAbsolute(KartGFX::KGFX_ZIPPER, 800.0f);

    if (skidding > 0)
    {
        const ParticleKind* skid_kind = red_skidding ? m_skid_kind2 
                                                     : m_skid_kind1;

        if (m_all_emitters[KGFX_SKID1L])
            m_all_emitters[KGFX_SKID1L]->setParticleType(skid_kind);
        if (m_all_emitters[KGFX_SKID1R])
            m_all_emitters[KGFX_SKID1R]->setParticleType(skid_kind);

        if (CVS->isGLSL())
        {
            m_skidding_light_1->setVisible(!red_skidding);
            m_skidding_light_2->setVisible(red_skidding);
        }
        
        setCreationRateAbsolute(KartGFX::KGFX_SKIDL, (float)skidding);
        setCreationRateAbsolute(KartGFX::KGFX_SKIDR, (float)skidding);
    }
    else
    {
        setCreationRateAbsolute(KartGFX::KGFX_SKIDL, 0.0f);
        setCreationRateAbsolute(KartGFX::KGFX_SKIDR, 0.0f);
        
        if (CVS->isGLSL())
        {
            m_skidding_light_1->setVisible(false);
            m_skidding_light_2->setVisible(false);
        }
    }
#endif
}   // setGFXFromReplay

// ----------------------------------------------------------------------------
void KartGFX::setGFXInvisible()
{
#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        m_nitro_light->setVisible(false);
        m_skidding_light_1->setVisible(false);
        m_skidding_light_2->setVisible(false);
        m_kart->getKartModel()->toggleHeadlights(false);
    }
#endif
}   // setGFXInvisible
