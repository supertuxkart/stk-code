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
#include "constants.hpp"
#include "material_manager.hpp"
#include "karts/kart.hpp"

Nitro::Nitro(Kart* kart)
        : ParticleSystem(200, 0.0f, true, 0.5f),
        m_kart(kart)
{
#ifdef DEBUG
    setName("nitro");
#endif
    bsphere.setCenter(0, 0, 0);
    bsphere.setRadius(1000.0f);
    dirtyBSphere();

    m_nitro_fire = new ssgSimpleState ();
    m_nitro_fire->setTexture(material_manager->getMaterial("nitro-particle.rgb")->getState()->getTexture());
    m_nitro_fire -> setTranslucent    () ;
    m_nitro_fire -> enable            ( GL_TEXTURE_2D ) ;
    m_nitro_fire -> setShadeModel     ( GL_SMOOTH ) ;
    m_nitro_fire -> disable           ( GL_CULL_FACE ) ;
    m_nitro_fire -> enable            ( GL_BLEND ) ;
    m_nitro_fire -> disable           ( GL_ALPHA_TEST ) ;
    m_nitro_fire -> enable            ( GL_LIGHTING ) ;
    m_nitro_fire -> setColourMaterial ( GL_EMISSION ) ;
    m_nitro_fire -> setMaterial       ( GL_AMBIENT, 0, 0, 0, 1 ) ;
    m_nitro_fire -> setMaterial       ( GL_DIFFUSE, 0, 0, 0, 1 ) ;
    m_nitro_fire -> setMaterial       ( GL_SPECULAR, 0, 0, 0, 1 ) ;
    m_nitro_fire -> setShininess      (  0 ) ;
    m_nitro_fire->ref();

    setState(m_nitro_fire);

}   // KartParticleSystem

//-----------------------------------------------------------------------------
Nitro::~Nitro()
{
    ssgDeRefDelete(m_nitro_fire);
}   // ~Nitro

//-----------------------------------------------------------------------------
void Nitro::update(float t)
{
    ParticleSystem::update(t);
}   // update

//-----------------------------------------------------------------------------
void Nitro::particle_create(int, Particle *p)
{
    sgSetVec4(p->m_col, 1, 1, 1, 1 ); /* initially white */
    sgSetVec3(p->m_vel, 0, 0, 0 );
    sgSetVec3(p->m_acc, 0, 0, 2.0f ); /* Gravity */
    p->m_size         = 0.5f;
    p->m_time_to_live = 0.8f;

    Vec3 xyz       = m_kart->getXYZ();
    const Vec3 vel = -m_kart->getVelocity()*0.2f;
    sgCopyVec3(p->m_vel, vel.toFloat());
    sgCopyVec3(p->m_pos, xyz.toFloat());
    p->m_vel[0] += 2*cos(DEGREE_TO_RAD(rand()% 180));
    p->m_vel[1] += 2*sin(DEGREE_TO_RAD(rand()% 180));
    p->m_vel[2] = 0;
}   // particle_create

//-----------------------------------------------------------------------------
void Nitro::particle_update(float delta, int,
                            Particle * particle)
{
    particle->m_size    -= delta*.2f;
    particle->m_col[3]  -= delta * 2.0f;
}  // particle_update


