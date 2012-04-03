//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012 Joerg Henrichs
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

#include "karts/cannon_animation.hpp"

#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"

#include "LinearMath/btTransform.h"

CannonAnimation::CannonAnimation(AbstractKart *kart, const Vec3 &target, 
                               float speed)
             : AbstractKartAnimation(kart)
{
    m_xyz       = m_kart->getXYZ();
	assert(speed>0);
	Vec3 delta  = target-m_kart->getXYZ();
	m_timer     = delta.length()/speed;
	m_velocity  = delta/m_timer;

    World::getWorld()->getPhysics()->removeKart(m_kart);
    
    m_curr_rotation.setHeading(m_kart->getHeading());
    m_curr_rotation.setPitch(m_kart->getPitch());
    m_curr_rotation.setRoll(m_kart->getRoll());

    m_add_rotation.setHeading(0);
    m_add_rotation.setPitch(  0);
    m_add_rotation.setRoll(   0);
}   // CannonAnimation

// ----------------------------------------------------------------------------
CannonAnimation::~CannonAnimation()
{
    btTransform trans = m_kart->getTrans();
    trans.setOrigin(m_xyz);
    m_kart->setTrans(trans);
    m_kart->getBody()->setCenterOfMassTransform(trans);
    World::getWorld()->getPhysics()->addKart(m_kart);
}   // ~CannonAnimation

// ----------------------------------------------------------------------------
/** Updates the kart animation.
 *  \param dt Time step size.
 *  \return True if the explosion is still shown, false if it has finished.
 */
void CannonAnimation::update(float dt)
{
    m_xyz += dt*m_velocity;
    m_kart->setXYZ(m_xyz);
    m_curr_rotation += dt*m_add_rotation;
    btQuaternion q(m_curr_rotation.getHeading(), m_curr_rotation.getPitch(),
                   m_curr_rotation.getRoll());
    m_kart->setRotation(q);

    AbstractKartAnimation::update(dt);
}   // update
