//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011  Joerg Henrichs, Marianne Gagnon
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

#include "graphics/particle_emitter.hpp"

#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "utils/constants.hpp"

ParticleEmitter::ParticleEmitter(float particleSize, core::vector3df position, Material* material,
                                 int minParticlesPerSecond, int maxParticlesPerSecond,
                                 video::SColor minStartColor, video::SColor maxStartColor,
                                 int lifeTimeMin, int lifeTimeMax, int maxAngle, int fadeOutTime,
                                 float directionMultiplier, float minSize, float maxSize, scene::ISceneNode* parent)
{
    m_direction_multiplier = directionMultiplier;
    m_node = irr_driver->addParticleNode();
    
#ifdef DEBUG
    std::string debug_name = std::string("particles(") + material->getTexture()->getName().getPath().c_str() + ")";
    m_node->setName(debug_name.c_str());
#endif
    
    if (parent != NULL)
    {
        m_node->setParent(parent);
    }
    
    // Note: the smoke system is NOT child of the kart, since bullet
    // gives the position of the wheels on the ground in world coordinates.
    // So it's easier not to move the particle system with the kart, and set 
    // the position directly from the wheel coordinates.
    m_node->setPosition(position);
    material->setMaterialProperties(&(m_node->getMaterial(0)));
    m_node->setMaterialTexture(0, material->getTexture());
    
    // FIXME: does the maxAngle param work at all??
    
    m_emitter = m_node->createPointEmitter(core::vector3df(0.0f, 0.3f, 0.0f),   // velocity in m/ms
                                           minParticlesPerSecond, maxParticlesPerSecond,
                                           minStartColor, maxStartColor,
                                           lifeTimeMin, lifeTimeMax,
                                           maxAngle
                                           );
    m_emitter->setMinStartSize(core::dimension2df(minSize, minSize));
    m_emitter->setMaxStartSize(core::dimension2df(maxSize, maxSize));
    m_node->setEmitter(m_emitter); // this grabs the emitter
    m_emitter->drop();             // so we can drop our references
    
    // FIXME: fade-out color doesn't seem to quite work
    scene::IParticleFadeOutAffector *af = m_node->createFadeOutParticleAffector(video::SColor(0, 255, 0, 0), fadeOutTime);
    m_node->addAffector(af);
    af->drop();
    
}   // KartParticleSystem

//-----------------------------------------------------------------------------
/** Destructor, removes
 */
ParticleEmitter::~ParticleEmitter()
{
    irr_driver->removeNode(m_node);
}   // ~ParticleEmitter

//-----------------------------------------------------------------------------
void ParticleEmitter::update()
{
    // No particles to emit, no need to change the speed
    if (m_emitter->getMinParticlesPerSecond() == 0) return;
    
    // There seems to be no way to randomise the velocity for particles,
    // so we have to do this manually, by changing the default velocity.
    // Irrlicht expects velocity (called 'direction') in m/ms!!
    Vec3 dir(cos(DEGREE_TO_RAD*(rand()%180))*m_direction_multiplier,
             sin(DEGREE_TO_RAD*(rand()%100))*m_direction_multiplier,
             sin(DEGREE_TO_RAD*(rand()%180))*m_direction_multiplier);
    
    m_emitter->setDirection(dir.toIrrVector());
}   // update

//-----------------------------------------------------------------------------

void ParticleEmitter::setCreationRate(float f)
{
    m_emitter->setMinParticlesPerSecond(int(f));
    m_emitter->setMaxParticlesPerSecond(int(f));
}   // setCreationRate

//-----------------------------------------------------------------------------

void ParticleEmitter::setPosition(core::vector3df pos)
{
    m_node->setPosition(pos);
}
