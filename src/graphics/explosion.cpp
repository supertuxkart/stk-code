//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#include "graphics/explosion.hpp"

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "items/projectile_manager.hpp"
#include "race/race_manager.hpp"
#include "utils/vec3.hpp"

#include <IParticleSystemSceneNode.h>

const float burst_time = 0.1f;

/** Creates an explosion effect. */
Explosion::Explosion(const Vec3& coord, const char* explosion_sound)
                     : HitSFX(coord, explosion_sound)
{
    // short emision time, explosion, not constant flame
    m_remaining_time  = burst_time; 
    m_node            = irr_driver->addParticleNode();
    
#ifdef DEBUG
    m_node->setName("explosion");
#endif
    m_node->setPosition(coord.toIrrVector());
    Material* m = material_manager->getMaterial("explode.png");
    m_node->setMaterialTexture(0, m->getTexture());
    m->setMaterialProperties(&(m_node->getMaterial(0)), NULL);
    m_node->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR );

    scene::IParticleEmitter* em = 
        m_node->createSphereEmitter(core::vector3df(0.0f,0.0f,0.0f), 0.5f,
              /* velocity in m/ms */core::vector3df(0.0f,0.005f,0.0f), 
                                     600, 900, // min max particles per sec
                                     video::SColor(0, 0, 0, 0), // min colour
                                     video::SColor(0, 0, 0, 0), // max colour
                                     (int)((burst_time + explosion_time)
                                             *1000.0f), // min life ms
                                     (int)((burst_time + explosion_time)
                                             *1000.0f), // max max life ms
                                     90, // max angle
                                     // min and max start size
                                     core::dimension2df(0.3f, 0.3f), 
                                     core::dimension2df(0.75f, 0.75f)
                                     );
    m_node->setEmitter(em); // this grabs the emitter
    em->drop(); // so we can drop it here without deleting it

    scene::IParticleAffector* scale_affector = 
        m_node->createScaleParticleAffector(core::dimension2df(3.0f, 3.0f));
    m_node->addAffector(scale_affector); // same goes for the affector
    scale_affector->drop();
}   // Explosion

//-----------------------------------------------------------------------------
/** Destructor stops the explosion sfx from being played and frees its memory.
 */
Explosion::~Explosion()
{
}   // ~Explosion

//-----------------------------------------------------------------------------
/** Updates the explosion, called one per time step.
 *  \param dt Time step size.
 *  \return true If the explosion is finished.
 */
bool Explosion::updateAndDelete(float dt)
{
    // The explosion sfx is shorter than the particle effect,
    // so no need to save the result of the update call.
    HitSFX::updateAndDelete(dt);

    m_remaining_time -= dt;
    
    if (m_remaining_time < 0.0f && m_remaining_time >= -explosion_time)
    {
        
        const int intensity = (int)(255-(m_remaining_time/-explosion_time)*255);
        m_node->getMaterial(0).AmbientColor.setGreen(intensity);
        m_node->getMaterial(0).DiffuseColor.setGreen(intensity);
        m_node->getMaterial(0).EmissiveColor.setGreen(intensity);
        
        m_node->getMaterial(0).AmbientColor.setBlue(intensity);
        m_node->getMaterial(0).DiffuseColor.setBlue(intensity);
        m_node->getMaterial(0).EmissiveColor.setBlue(intensity);
        
        m_node->getMaterial(0).AmbientColor.setRed(intensity);
        m_node->getMaterial(0).DiffuseColor.setRed(intensity);
        m_node->getMaterial(0).EmissiveColor.setRed(intensity);
         
    }
    
    
    // Do nothing more if the animation is still playing
    if (m_remaining_time>0) return false;

    // Otherwise check that the sfx has finished, otherwise the
    // sfx will get aborted 'in the middle' when this explosion
    // object is removed.
    if (m_remaining_time > -explosion_time)
    {
        // Stop the emitter and wait a little while for all particles to have time to fade out
        m_node->getEmitter()->setMinParticlesPerSecond(0);
        m_node->getEmitter()->setMaxParticlesPerSecond(0);
    }
    else
    {
        // Sound and animation finished --> remove node
        irr_driver->removeNode(m_node);
        m_node = NULL;
        return true;   // finished
    }

    return false;  // not finished
}   // updateAndDelete
