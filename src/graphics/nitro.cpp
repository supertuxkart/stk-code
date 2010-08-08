//  $Id: nitro.cpp 1681 2008-04-09 13:52:48Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008  Joerg Henrichs
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

#include "graphics/nitro.hpp"

#include "graphics/material_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "karts/kart.hpp"
#include "utils/constants.hpp"

Nitro::Nitro(Kart* kart) : m_kart(kart)
{
    const float particle_size = 0.25f;
    m_node = irr_driver->addParticleNode();
#ifdef DEBUG
    std::string debug_name = m_kart->getIdent()+" (nitro)";
    m_node->setName(debug_name.c_str());
#endif
    m_node->setParent(m_kart->getNode());
    m_node->setPosition(core::vector3df(0, particle_size*0.25f, -m_kart->getKartLength()*0.5f));
    Material *m= material_manager->getMaterial("nitro-particle.png");
    m->setMaterialProperties(&(m_node->getMaterial(0)));
    m_node->setMaterialTexture(0, m->getTexture());

    m_emitter = m_node->createBoxEmitter(core::aabbox3df(-m_kart->getKartWidth()*0.3f, 0.1f,                                -m_kart->getKartLength()*0.1f,
                                                          m_kart->getKartWidth()*0.3f, 0.1f + m_kart->getKartHeight()*0.5f, -m_kart->getKartLength()*0.3f),
                                         core::vector3df(0.0f, 0.03f, 0.0f),
                                         5, 10,
                                         video::SColor(255,0,0,0),
                                         video::SColor(255,255,255,255),
                                         150, 250, // Min max life milisec
                                         40, // Angle
                                         core::dimension2df(particle_size/2.0f, particle_size/2.0f),
                                         core::dimension2df(particle_size*2.0f, particle_size*2.0f)
                                         );
    m_emitter->setMinStartSize(core::dimension2df(particle_size/2.0f, particle_size/2.0f));
    m_emitter->setMaxStartSize(core::dimension2df(particle_size*2.0f, particle_size*2.0f));
    m_node->setEmitter(m_emitter); // this grabs the emitter
    m_emitter->drop();             // so we can drop our reference

    scene::IParticleAffector *af = m_node->createFadeOutParticleAffector(video::SColor(0, 0, 0, 0), 2500);
    m_node->addAffector(af);
    af->drop();
}   // KartParticleSystem

//-----------------------------------------------------------------------------
Nitro::~Nitro()
{
    irr_driver->removeNode(m_node);
}   // ~Nitro

//-----------------------------------------------------------------------------
void Nitro::update(float t)
{
    // No particles to emit, no need to change the speed
    if(m_emitter->getMinParticlesPerSecond()==0)
        return;
    // There seems to be no way to randomise the velocity for particles,
    // so we have to do this manually, by changing the default velocity.
    // Irrlicht expects velocity (called 'direction') in m/ms!!
    Vec3 dir(cos(DEGREE_TO_RAD*(rand()%180))*0.001f,
             sin(DEGREE_TO_RAD*(rand()%180))*0.001f,
             sin(DEGREE_TO_RAD*(rand()%100))*0.001f);
    m_emitter->setDirection(dir.toIrrVector());
}   // update

//-----------------------------------------------------------------------------
void Nitro::setCreationRate(float f)
{
    m_emitter->setMinParticlesPerSecond(int(f));
    m_emitter->setMaxParticlesPerSecond(int(f));
}   // setCreationRate

