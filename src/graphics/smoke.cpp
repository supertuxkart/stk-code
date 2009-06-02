
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

#include "material_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "karts/kart.hpp"

Smoke::Smoke(Kart* kart) : m_kart(kart)
{    
    m_node = irr_driver->addParticleNode();
    m_node->setParent(m_kart->getNode());
    m_node->setPosition(core::vector3df(0, 1, -1));
    m_node->setMaterialFlag(video::EMF_LIGHTING, false);
    m_node->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
    //const std::string s=file_manager->getTextureFile("smoke.png");
    video::ITexture *tex = material_manager->getMaterial("smoke.png")->getTexture();
    m_node->setMaterialTexture(0, tex);
    //m_node->setMaterialType(video::EMT_TRANSPARENT_VERTEX_ALPHA);
    //m_node->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);


    m_emitter = m_node->createBoxEmitter(core::aabbox3df(0, 0, 0, 0.3f, 0.3f, 1.3f),
                                         core::vector3df(0, 0, 0),
                                         20,   // minParticlesPerSecond,
                                         30  // maxParticlesPerSecond
                                           );
    m_node->setParticleSize(core::dimension2df(0.01f, 0.01f));
    m_node->setEmitter(m_emitter); // this grabs the emitter

    //scene::IParticleAffector *af = m_node->createFadeOutParticleAffector();
    //m_node->addAffector(af);
    //af->drop();
}   // KartParticleSystem

//-----------------------------------------------------------------------------
/** Destructor, removes
 */
Smoke::~Smoke()
{
}   // ~Smoke

//-----------------------------------------------------------------------------
void Smoke::update(float t)
{
    Vec3 dir = m_kart->getTrans().getBasis()*Vec3(0,-.01f,0);
    m_emitter->setDirection(dir.toIrrVector());
}   // update
//-----------------------------------------------------------------------------
void Smoke::setCreationRate(float f)
{
    f=0;
    m_emitter->setMaxParticlesPerSecond(int(f));
    m_emitter->setMaxParticlesPerSecond(int(f));
}   // setCreationRate
