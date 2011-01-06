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
#include "graphics/particle_kind.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "utils/constants.hpp"

ParticleEmitter::ParticleEmitter(ParticleKind* type, core::vector3df position,
                                 scene::ISceneNode* parent)
{
    m_node = irr_driver->addParticleNode();
    m_particle_type = type;
    
    Material* material    = type->getMaterial();
    const float minSize   = type->getMinSize();
    const float maxSize   = type->getMaxSize();
    const int lifeTimeMin = type->getMinLifetime();
    const int lifeTimeMax = type->getMaxLifetime();
    
    assert(material->getTexture() != NULL);
    assert(maxSize >= minSize);
    assert(lifeTimeMax >= lifeTimeMin);
    
#ifdef DEBUG
    std::string debug_name = std::string("particles(") + material->getTexture()->getName().getPath().c_str() + ")";
    m_node->setName(debug_name.c_str());
#endif
    
    
    if (parent != NULL)
    {
        m_node->setParent(parent);
    }
    
    m_node->setPosition(position);
    material->setMaterialProperties(&(m_node->getMaterial(0)));
    m_node->setMaterialTexture(0, material->getTexture());
    
    //m_node->getMaterial(0).MaterialType = video::EMT_ONETEXTURE_BLEND ;
    //m_node->getMaterial(0).MaterialTypeParam = pack_texureBlendFunc(video::EBF_SRC_ALPHA, video::EBF_ONE_MINUS_SRC_ALPHA,
    //                                                                video::EMFN_MODULATE_1X, video::EAS_TEXTURE | video::EAS_VERTEX_COLOR); 
    m_node->getMaterial(0).ZWriteEnable = false; // disable z-buffer writes
    
    switch (type->getShape())
    {
        case EMITTER_POINT:
        {
            // FIXME: does the maxAngle param work at all??
            // FIXME: the min and max color params don't appear to work
            m_emitter = m_node->createPointEmitter(core::vector3df(0.0f, 0.0f, 5.0f),   // velocity in m/ms
                                                   type->getMinRate(), type->getMaxRate(),
                                                   type->getMinColor(), type->getMaxColor(),
                                                   lifeTimeMin, lifeTimeMax,
                                                   0 /* angle */
                                                   );
            break;
        }
            
        case EMITTER_BOX:
        {
            // FIXME: does the maxAngle param work at all??
            // FIXME: the min and max color params don't appear to work
            const float box_size_x = type->getBoxSizeX()/2.0f;
            const float box_size_y = type->getBoxSizeY()/2.0f;
            const float box_size_z = type->getBoxSizeZ()/2.0f;
            m_emitter = m_node->createBoxEmitter(core::aabbox3df(-box_size_x, -box_size_y, -box_size_z,
                                                                  box_size_x,  box_size_y,  box_size_z),
                                                 core::vector3df(0.0f, 0.0f, 0.0f),   // velocity in m/ms
                                                 type->getMinRate(), type->getMaxRate(),
                                                 type->getMinColor(), type->getMaxColor(),
                                                 lifeTimeMin, lifeTimeMax,
                                                 0 /* angle */
                                                 );
            
            //irr_driver->getSceneManager()->addCubeSceneNode(2.0f, parent, -1, position, core::vector3df(0, 0, 0) /* rotation */,
            //                                                core::vector3df(box_size_x, box_size_y, box_size_z));
            
            break;
        }
        
        default:
        {
            fprintf(stderr, "[ParticleEmitter] Unknown shape\n");
            return;
        }
    }
    
    m_emitter->setMinStartSize(core::dimension2df(minSize, minSize));
    m_emitter->setMaxStartSize(core::dimension2df(maxSize, maxSize));
    m_node->setEmitter(m_emitter); // this grabs the emitter
    m_emitter->drop();             // so we can drop our references
    
    // FIXME: this is ridiculous, the fadeout time should be equal to the lifetime, except that the
    //        lifetime is random...
    scene::IParticleFadeOutAffector *af = m_node->createFadeOutParticleAffector(video::SColor(0, 255, 255, 255),
                                                                                type->getFadeoutTime());
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
    const int x = m_particle_type->getAngleSpreadX();
    const int y = m_particle_type->getAngleSpreadY();
    const int z = m_particle_type->getAngleSpreadZ();
    Vec3 dir(cos(DEGREE_TO_RAD*(rand()%x - x/2))*m_particle_type->getVelocityX(),
             sin(DEGREE_TO_RAD*(rand()%y - x/2))*m_particle_type->getVelocityY(),
             sin(DEGREE_TO_RAD*(rand()%z - x/2))*m_particle_type->getVelocityZ());
    
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
