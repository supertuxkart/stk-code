//  $Id$
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

#include "explosion.hpp"

#include "material.hpp"
#include "material_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "items/projectile_manager.hpp"
#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "graphics/scene.hpp"
#include "utils/vec3.hpp"

Explosion::Explosion(const Vec3& coord, const int explosion_sound)
{
    m_remaining_time = 1.5f;
    m_node = irr_driver->addParticleNode();
    m_node->setPosition(coord.toIrrVector());
    Material *m = material_manager->getMaterial("explode.png");
    m_node->setMaterialTexture(0, m->getTexture());
    m->setMaterialProperties(&(m_node->getMaterial(0)));
    //m_node->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF);

    scene::IParticleEmitter* em = m_node->createPointEmitter();
    em->setDirection(core::vector3df(0.0f,0.0006f,0.0f));  // velocity in m/ms(!!)
    em->setMinParticlesPerSecond(1);
    em->setMaxParticlesPerSecond(5);
    em->setMinStartSize(core::dimension2df(0.1f, 0.1f));
    em->setMaxStartSize(core::dimension2df(0.5f, 0.5f));
    m_node->setEmitter(em); // this grabs the emitter
    em->drop(); // so we can drop it here without deleting it

    scene::IParticleAffector* paf = m_node->createFadeOutParticleAffector();
    m_node->addAffector(paf); // same goes for the affector
    paf->drop();

    //scene::IParticleAffector *paf = 
    //    m_node->createGravityAffector(Vec3(0, 0, -5).toIrrVector());
    //m_node->addAffector(paf);
    //paf->drop();


    m_explode_sound = sfx_manager->newSFX( (SFXManager::SFXType)explosion_sound );
    init(coord);
}   // Explosion

//-----------------------------------------------------------------------------
Explosion::~Explosion()
{
    // FIXME LEAK: Explosion that are still playing when a race ends might
    // not get freed correctly (ssgCutout is removed when removing this
    // from the scene node).
    sfx_manager->deleteSFX(m_explode_sound);
    // cut will be cleaned up when the explosion is rerefed by plib
}
//-----------------------------------------------------------------------------
void Explosion::init(const Vec3& coord)
{
    m_explode_sound->position(coord);
    m_explode_sound->play();
}   // init

//-----------------------------------------------------------------------------
void Explosion::update(float dt)
{
    m_remaining_time -=dt;

    // Do nothing more if the animation is still playing
    if(m_remaining_time>0) return;

    // Otherwise check that the sfx has finished, otherwise the
    // sfx will get aborted 'in the middle' when this explosion
    // object is removed.
    if(m_explode_sound->getStatus() == SFXManager::SFX_PLAYING)
    {
        m_remaining_time = 0;
    }
    else
    {
        // Sound and animation finished --> remove node
        irr_driver->removeNode(m_node);
        projectile_manager->FinishedExplosion();
        return;
    }
}
