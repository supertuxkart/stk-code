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
    m_node = irr_driver->addParticleNode();
        scene::IParticleEmitter* em = m_node->createBoxEmitter(
                core::aabbox3d<f32>(-7,0,-7,7,1,7), // emitter size
                core::vector3df(0.0f,0.06f,0.0f),   // initial direction
                80,100,                             // emit rate
                video::SColor(0,255,255,255),       // darkest color
                video::SColor(0,255,255,255),       // brightest color
                800,2000,0,                         // min and max age, angle
                core::dimension2df(10.f,10.f),         // min size
                core::dimension2df(20.f,20.f));        // max size

        m_node->setEmitter(em); // this grabs the emitter
        em->drop(); // so we can drop it here without deleting it

        scene::IParticleAffector* paf = m_node->createFadeOutParticleAffector();

        m_node->addAffector(paf); // same goes for the affector
        paf->drop();




    //scene::IParticleEmitter *em = 
    //    m_node->createPointEmitter(Vec3(0, 0, 1).toIrrVector(),
    //                               5, 10     // min and max particles per second
    //                               );
    //m_node->setEmitter(em);
    //em->drop();

    //scene::IParticleAffector *paf = 
    //    m_node->createGravityAffector(Vec3(0, 0, -5).toIrrVector());
    //m_node->addAffector(paf);
    //paf->drop();
    //paf = m_node->createFadeOutParticleAffector();
    //m_node->addAffector(paf);
    //paf->drop();


    m_node->setPosition(coord.toIrrVector());
    m_node->setPosition(core::vector3df(5, 5, 5));
    m_node->setScale(core::vector3df(2,2,2));
    m_node->setMaterialFlag(video::EMF_LIGHTING, false);
    m_node->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
    Material *m = material_manager->getMaterial("lava.png");
    m_node->setMaterialTexture(0, m->getTexture());
    m_node->setMaterialType(video::EMT_TRANSPARENT_VERTEX_ALPHA);

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

    m_has_ended = false;
}

//-----------------------------------------------------------------------------
void Explosion::update(float dt)
{
#ifndef HAVE_IRRLICHT
    //fprintf(stderr, "Explosion: update: ");
    if(++m_step >= m_seq->getNumKids())
    {
        //be sure that the sound is not prematurely stopped
        if(m_explode_sound->getStatus() != SFXManager::SFX_PLAYING)
        {
            //fprintf(stderr, "Sound finished. Removing.\n");
            stk_scene->remove((ssgTransform*)this);
            projectile_manager->FinishedExplosion();
            m_has_ended = true;
            return;
        }
        else
        {
            //fprintf(stderr, "Waiting for sound to finish.\n");
        }
    }
    else
    {
        //fprintf(stderr, "Step.\n");
        m_seq->selectStep(m_step);
    }
#endif
}
