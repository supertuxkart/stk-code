
//  $Id: dust_cloud.cpp 1681 2008-04-09 13:52:48Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#include "smoke.hpp"

#include "graphics/material_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "karts/kart.hpp"
#include "utils/constants.hpp"

Smoke::Smoke(Kart* kart) : m_kart(kart)
{
    const float particle_size = 0.5f;
    // Left wheel
    m_node_l = irr_driver->addParticleNode();
    m_node_l->setParent(m_kart->getNode());
    m_node_l->setPosition(core::vector3df(-m_kart->getKartWidth()*0.35f, particle_size*0.5f, -m_kart->getKartLength()*0.5f)); // Should use (behind) wheel pos
    Material *ml= material_manager->getMaterial("smoke.png");
    ml->setMaterialProperties(&(m_node_l->getMaterial(0)));
    m_node_l->setMaterialTexture(0, ml->getTexture());

    m_emitter_l = m_node_l->createPointEmitter(core::vector3df(0, 0, 0),   // velocity in m/ms
                                           5, 10,
                                           video::SColor(255,0,0,0),
                                           video::SColor(255,255,255,255),
                                           400, 400,
                                           20  // max angle
                                           );
    m_emitter_l->setMinStartSize(core::dimension2df(particle_size, particle_size));
    m_emitter_l->setMaxStartSize(core::dimension2df(particle_size, particle_size));
    m_node_l->setEmitter(m_emitter_l); // this grabs the emitter

    scene::IParticleAffector *afl = m_node_l->createFadeOutParticleAffector();
    m_node_l->addAffector(afl);
    afl->drop();

    // Right wheel
    m_node_r = irr_driver->addParticleNode();
    m_node_r->setParent(m_kart->getNode());
    m_node_r->setPosition(core::vector3df(m_kart->getKartWidth()*0.35f, particle_size*0.5f, -m_kart->getKartLength()*0.5f)); // Should use (behind) wheel pos
    Material *mr= material_manager->getMaterial("smoke.png");
    mr->setMaterialProperties(&(m_node_r->getMaterial(0)));
    m_node_r->setMaterialTexture(0, mr->getTexture());

    m_emitter_r = m_node_r->createPointEmitter(core::vector3df(0, 0, 0),   // velocity in m/ms
                                           5, 10, 
                                           video::SColor(255,0,0,0),
                                           video::SColor(255,255,255,255),
                                           400, 400,
                                           20  // max angle
                                           );  
    m_emitter_r->setMinStartSize(core::dimension2df(particle_size, particle_size));
    m_emitter_r->setMaxStartSize(core::dimension2df(particle_size, particle_size));
    m_node_r->setEmitter(m_emitter_r); // this grabs the emitter

    scene::IParticleAffector *afr = m_node_r->createFadeOutParticleAffector();
    m_node_r->addAffector(afr);
    afr->drop();
}   // KartParticleSystem

//-----------------------------------------------------------------------------
/** Destructor, removes
 */
Smoke::~Smoke()
{
    irr_driver->removeNode(m_node_l);
    irr_driver->removeNode(m_node_r);
}   // ~Smoke

//-----------------------------------------------------------------------------
void Smoke::update(float t)
{
    // No particles to emit, no need to change the speed
    if(m_emitter_l->getMinParticlesPerSecond()==0)
        return;
    // There seems to be no way to randomise the velocity for particles,
    // so we have to do this manually, by changing the default velocity.
    // Irrlicht expects velocity (called 'direction') in m/ms!!
    Vec3 dirl(cos(DEGREE_TO_RAD(rand()%180))*0.002f,
             sin(DEGREE_TO_RAD(rand()%180))*0.002f,
             sin(DEGREE_TO_RAD(rand()%100))*0.002f);
    m_emitter_l->setDirection(dirl.toIrrVector());
    Vec3 dirr(cos(DEGREE_TO_RAD(rand()%180))*0.002f,
             sin(DEGREE_TO_RAD(rand()%180))*0.002f,
             sin(DEGREE_TO_RAD(rand()%100))*0.002f);
    m_emitter_r->setDirection(dirr.toIrrVector());
}   // update
//-----------------------------------------------------------------------------
void Smoke::setCreationRate(float f)
{
    m_emitter_l->setMinParticlesPerSecond(int(f));
    m_emitter_l->setMaxParticlesPerSecond(int(f));
    m_emitter_r->setMinParticlesPerSecond(int(f));
    m_emitter_r->setMaxParticlesPerSecond(int(f));
}   // setCreationRate
