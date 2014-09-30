//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2013 Steve Baker <sjbaker1@airmail.net>
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
#include "graphics/particle_emitter.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "items/projectile_manager.hpp"
#include "race/race_manager.hpp"
#include "utils/vec3.hpp"

#include <IParticleSystemSceneNode.h>

const float burst_time = 0.1f;

/** Creates an explosion effect. */
Explosion::Explosion(const Vec3& coord, const char* explosion_sound, const char * particle_file)
                     : HitSFX(coord, explosion_sound)
{
    // short emision time, explosion, not constant flame
    m_remaining_time  = burst_time;
    m_emission_frames = 0;

    ParticleKindManager* pkm = ParticleKindManager::get();
    ParticleKind* particles = pkm->getParticles(particle_file);
    m_emitter = new ParticleEmitter(particles, coord,  NULL);
}   // Explosion

//-----------------------------------------------------------------------------
/** Destructor stops the explosion sfx from being played and frees its memory.
 */
Explosion::~Explosion()
{
    if(m_emitter)
    {
        delete m_emitter;
    }
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

    m_emission_frames++;
    m_remaining_time -= dt;

    if (m_remaining_time < 0.0f && m_remaining_time >= -explosion_time)
    {
        scene::ISceneNode* node = m_emitter->getNode();
        
        const int intensity = (int)(255-(m_remaining_time/-explosion_time)*255);
        node->getMaterial(0).AmbientColor.setGreen(intensity);
        node->getMaterial(0).DiffuseColor.setGreen(intensity);
        node->getMaterial(0).EmissiveColor.setGreen(intensity);
        
        node->getMaterial(0).AmbientColor.setBlue(intensity);
        node->getMaterial(0).DiffuseColor.setBlue(intensity);
        node->getMaterial(0).EmissiveColor.setBlue(intensity);
        
        node->getMaterial(0).AmbientColor.setRed(intensity);
        node->getMaterial(0).DiffuseColor.setRed(intensity);
        node->getMaterial(0).EmissiveColor.setRed(intensity);
    }


    // Do nothing more if the animation is still playing
    if (m_remaining_time>0) return false;

    // Otherwise check that the sfx has finished, otherwise the
    // sfx will get aborted 'in the middle' when this explosion
    // object is removed.
    if (m_remaining_time > -explosion_time)
    {
        // if framerate is very low, emit for at least a few frames, in case
        // burst time is lower than the time of 1 frame
        if (m_emission_frames > 2)
        {
            // Stop the emitter and wait a little while for all particles to have time to fade out
            m_emitter->getNode()->getEmitter()->setMinParticlesPerSecond(0);
            m_emitter->getNode()->getEmitter()->setMaxParticlesPerSecond(0);
        }
    }
    else
    {
        // Sound and animation finished, node can be removed now.
        // Returning true will cause this node to be deleted by
        // the projectile manager.
        return true;   // finished
    }

    return false;  // not finished
}   // updateAndDelete
