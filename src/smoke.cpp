
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
#include "kart.hpp"

Smoke::Smoke(Kart* kart_,
                                       int num, float _create_rate, int _ttf,
                                       float sz, float bsphere_size)
        : ParticleSystem (num, _create_rate, _ttf, sz, bsphere_size),
        m_kart(kart_)
{
    getBSphere () -> setCenter ( 0, 0, 0 ) ;
    getBSphere () -> setRadius ( 1000.0f ) ;
    dirtyBSphere();
}   // KartParticleSystem

//-----------------------------------------------------------------------------
void Smoke::update ( float t )
{
#if 0
    std::cout << "BSphere: r:" << getBSphere()->radius
    << " ("  << getBSphere()->center[0]
    << ", "  << getBSphere()->center[1]
    << ", "  << getBSphere()->center[2]
    << ")"
    << std::endl;
#endif
    getBSphere () -> setRadius ( 1000.0f ) ;
    ParticleSystem::update(t);
}   // update

//-----------------------------------------------------------------------------
void Smoke::particle_create(int, Particle *p)
{
    sgSetVec4 ( p -> m_col, 1, 1, 1, 1 ) ; /* initially white */
    sgSetVec3 ( p -> m_pos, 0, 0, 0 ) ;    /* start off on the ground */
    sgSetVec3 ( p -> m_vel, 0, 0, 0 ) ;
    sgSetVec3 ( p -> m_acc, 0, 0, 2.0f ) ; /* Gravity */
    p -> m_size = .5f;
    p -> m_time_to_live = 0.5 ;            /* Droplets evaporate after 5 seconds */

    const sgCoord* POS = m_kart->getCoord();
    const btVector3 VEL = m_kart->getVelocity();

    const float X_DIRECTION = sgCos (POS->hpr[0] - 90.0f); // Point at the rear
    const float Y_DIRECTION = sgSin (POS->hpr[0] - 90.0f); // Point at the rear

    sgCopyVec3 (p->m_pos, POS->xyz);

    p->m_pos[0] += X_DIRECTION * 0.7f;
    p->m_pos[1] += Y_DIRECTION * 0.7f;

    const float ABS_VEL = sqrt((VEL.getX() * VEL.getX()) + (VEL.getY() * VEL.getY()));

    p->m_vel[0] = X_DIRECTION * -ABS_VEL/2;
    p->m_vel[1] = Y_DIRECTION * -ABS_VEL/2;

    p->m_vel[0] += sgCos ((float)(rand()%180));
    p->m_vel[1] += sgSin ((float)(rand()%180));
    p->m_vel[2] += sgSin ((float)(rand()%100));

    getBSphere () -> setCenter ( POS->xyz[0], POS->xyz[1], POS->xyz[2] ) ;
}   // particle_create

//-----------------------------------------------------------------------------
void Smoke::particle_update (float delta, int,
        Particle * particle)
{
    particle->m_size    += delta*2.0f;
    particle->m_col[3]  -= delta * 2.0f;

    particle->m_pos[0] += particle->m_vel[0] * delta;
    particle->m_pos[1] += particle->m_vel[1] * delta;
    particle->m_pos[2] += particle->m_vel[2] * delta;
}  // particle_update

//-----------------------------------------------------------------------------
void Smoke::particle_delete (int , Particle* )
{}   // particle_delete

