
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
#include "constants.hpp"
#include "material_manager.hpp"
#include "karts/kart.hpp"

Smoke::Smoke(Kart* kart)
        : ParticleSystem(200, 0.0f, true, 0.75f),
        m_kart(kart)
{
#ifdef DEBUG
    setName("smoke");
#endif
    bsphere.setCenter(0, 0, 0);
    bsphere.setRadius(1000.0f);
    dirtyBSphere();

    m_smokepuff = new ssgSimpleState ();
    m_smokepuff->setTexture(material_manager->getMaterial("smoke.rgb")->getState()->getTexture());
    m_smokepuff -> setTranslucent    () ;
    m_smokepuff -> enable            ( GL_TEXTURE_2D ) ;
    m_smokepuff -> setShadeModel     ( GL_SMOOTH ) ;
    m_smokepuff -> disable           ( GL_CULL_FACE ) ;
    m_smokepuff -> enable            ( GL_BLEND ) ;
    m_smokepuff -> enable            ( GL_LIGHTING ) ;
    m_smokepuff -> setColourMaterial ( GL_EMISSION ) ;
    m_smokepuff -> setMaterial       ( GL_AMBIENT, 0, 0, 0, 1 ) ;
    m_smokepuff -> setMaterial       ( GL_DIFFUSE, 0, 0, 0, 1 ) ;
    m_smokepuff -> setMaterial       ( GL_SPECULAR, 0, 0, 0, 1 ) ;
    m_smokepuff -> setShininess      (  0 ) ;
    m_smokepuff->ref();

    setState(m_smokepuff);

}   // KartParticleSystem

//-----------------------------------------------------------------------------
Smoke::~Smoke()
{
    ssgDeRefDelete(m_smokepuff);
}   // ~Smoke
//-----------------------------------------------------------------------------
void Smoke::update(float t)
{
    ParticleSystem::update(t);
}   // update

//-----------------------------------------------------------------------------
void Smoke::particle_create(int, Particle *p)
{
    sgSetVec4(p->m_col, 1, 1, 1, 1 ); /* initially white */
    sgSetVec3(p->m_vel, 0, 0, 0 );
    sgSetVec3(p->m_acc, 0, 0, 2.0f ); /* Gravity */
    p->m_size         = 0.5f;
    p->m_time_to_live = 0.4f;

    // Change from left to right wheel and back for each new particle
    static int wheel_number = 2;
    wheel_number            = 5 - wheel_number;
    Vec3 xyz=m_kart->getVehicle()->getWheelInfo(wheel_number).m_raycastInfo.m_contactPointWS;

    sgCopyVec3 (p->m_pos, xyz.toFloat());
    p->m_vel[0] += cos(DEGREE_TO_RAD(rand()%180));
    p->m_vel[1] += sin(DEGREE_TO_RAD(rand()%180));
    p->m_vel[2] += sin(DEGREE_TO_RAD(rand()%100));

    bsphere.setCenter ( xyz.getX(), xyz.getY(), xyz.getZ() ) ;
}   // particle_create

//-----------------------------------------------------------------------------
void Smoke::particle_update(float delta, int,
                            Particle * particle)
{
    particle->m_size    -= delta*.2f;
    particle->m_col[3]  -= delta * 2.0f;
}  // particle_update

