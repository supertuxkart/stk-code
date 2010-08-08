//  $Id: smoke.cpp 1681 2008-04-09 13:52:48Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009  Joerg Henrichs
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

#include "graphics/smoke.hpp"

#include "graphics/material_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "karts/kart.hpp"
#include "physics/btKart.hpp"
#include "utils/constants.hpp"

Smoke::Smoke(Kart* kart) : m_kart(kart), m_particle_size(0.33f)
{
    m_node = irr_driver->addParticleNode();
#ifdef DEBUG
    std::string debug_name = m_kart->getIdent()+" (smoke)";
    m_node->setName(debug_name.c_str());
#endif
    // Note: the smoke system is NOT child of the kart, since bullet
    // gives the position of the wheels on the ground in world coordinates.
    // So it's easier not to move the particle system with the kart, and set 
    // the position directly from the wheel coordinates.
    m_node->setPosition(core::vector3df(-m_kart->getKartWidth()*0.35f, 
                                        m_particle_size*0.25f, 
                                        -m_kart->getKartLength()*0.5f));
    Material *m= material_manager->getMaterial("smoke.png");
    m->setMaterialProperties(&(m_node->getMaterial(0)));
    m_node->setMaterialTexture(0, m->getTexture());

    m_emitter = m_node->createPointEmitter(core::vector3df(0, 0, 0),   // velocity in m/ms
                                           5, 10,
                                           video::SColor(255,0,0,0),
                                           video::SColor(255,255,255,255),
                                           300, 500,
                                           20  // max angle
                                           );
    m_emitter->setMinStartSize(core::dimension2df(m_particle_size/1.5f, m_particle_size/1.5f));
    m_emitter->setMaxStartSize(core::dimension2df(m_particle_size*1.5f, m_particle_size*1.5f));
    m_node->setEmitter(m_emitter); // this grabs the emitter
    m_emitter->drop();             // so we can drop our references

    scene::IParticleFadeOutAffector *af = m_node->createFadeOutParticleAffector(video::SColor(0, 255, 0, 0), 500);
    m_node->addAffector(af);
    af->drop();

}   // KartParticleSystem

//-----------------------------------------------------------------------------
/** Destructor, removes
 */
Smoke::~Smoke()
{
    irr_driver->removeNode(m_node);
}   // ~Smoke

//-----------------------------------------------------------------------------
void Smoke::update(float t)
{
    // No particles to emit, no need to change the speed
    if(m_emitter->getMinParticlesPerSecond()==0)
        return;
    static int left=1;
    left = 1-left;
    const btWheelInfo &wi = m_kart->getVehicle()->getWheelInfo(2+left);
    Vec3 c=wi.m_raycastInfo.m_contactPointWS;

    // FIXME: the X position is not yet always accurate.
    m_node->setPosition(core::vector3df(c.getX()+m_particle_size*0.25f * (left?+1:-1),
                                        c.getY(),
                                        c.getZ()+m_particle_size*0.25f));

    // There seems to be no way to randomise the velocity for particles,
    // so we have to do this manually, by changing the default velocity.
    // Irrlicht expects velocity (called 'direction') in m/ms!!
    Vec3 dir(cos(DEGREE_TO_RAD*(rand()%180))*0.002f,
             sin(DEGREE_TO_RAD*(rand()%100))*0.002f,
             sin(DEGREE_TO_RAD*(rand()%180))*0.002f);
    m_emitter->setDirection(dir.toIrrVector());
}   // update

//-----------------------------------------------------------------------------
void Smoke::setCreationRate(float f)
{
    m_emitter->setMinParticlesPerSecond(int(f));
    m_emitter->setMaxParticlesPerSecond(int(f));
}   // setCreationRate
