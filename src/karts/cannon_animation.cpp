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

#include "animations/animation_base.hpp"
#include "animations/ipo.hpp"
#include "animations/three_d_animation.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "modes/world.hpp"

#include "LinearMath/btTransform.h"

CannonAnimation::CannonAnimation(AbstractKart *kart, Ipo *ipo, 
                                 const Vec3 &delta)
             : AbstractKartAnimation(kart, "CannonAnimation")
{
    m_curve  = new AnimationBase(ipo);
    m_timer  = ipo->getEndTime();
    Vec3 xyz = m_kart->getXYZ();
    Vec3 hpr, scale;
    // Get the curve position at t=0
    m_curve->update(0, &xyz, &hpr, &scale);
    m_offset  = m_kart->getXYZ() - xyz-delta;
    m_delta   = delta;
}   // CannonAnimation

// ----------------------------------------------------------------------------
CannonAnimation::~CannonAnimation()
{
    delete m_curve;
    float epsilon = 0.5f * m_kart->getKartHeight();

    btTransform pos;
    pos.setOrigin(m_kart->getXYZ()+btVector3(0, m_kart->getKartHeight() + epsilon,
                                           0));
    pos.setRotation(btQuaternion(btVector3(0.0f, 1.0f, 0.0f), m_kart->getHeading()));

    m_kart->getBody()->setCenterOfMassTransform(pos);
    Vec3 v(0, 0, m_kart->getKartProperties()->getMaxSpeed());
    m_kart->setVelocity(pos.getBasis()*v);
}   // ~CannonAnimation

// ----------------------------------------------------------------------------
/** Updates the kart animation.
 *  \param dt Time step size.
 *  \return True if the explosion is still shown, false if it has finished.
 */
void CannonAnimation::update(float dt)
{
    if(m_timer < dt)
    {
        AbstractKartAnimation::update(dt);
        return;
    }
    Vec3 xyz     = m_kart->getXYZ();
    core::vector3df old_xyz = xyz.toIrrVector();
    Vec3 hpr, scale;
    m_curve->update(dt, &xyz, &hpr, &scale);

    Vec3 rotated_delta = m_kart->getTrans().getBasis()*m_delta;
    rotated_delta = Vec3(0,0,0);
    Vec3 new_xyz = xyz+rotated_delta+m_offset;
    m_kart->setXYZ(new_xyz);

    core::vector3df rot = (new_xyz.toIrrVector()-old_xyz).getHorizontalAngle();
    btQuaternion q(Vec3(0,1,0),rot.Y*DEGREE_TO_RAD);
    m_kart->setRotation(q);
    AbstractKartAnimation::update(dt);
}   // update
