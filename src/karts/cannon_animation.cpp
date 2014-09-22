//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2013 Joerg Henrichs
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

CannonAnimation::CannonAnimation(AbstractKart *kart, Ipo *ipo)
             : AbstractKartAnimation(kart, "CannonAnimation")
{
    m_curve  = new AnimationBase(ipo);
    m_timer  = ipo->getEndTime();

    // Compute the delta between the kart position and the start of the curve.
    // This delta is rotated with the kart and added to the interpolated curve
    // position to get the actual kart position during the animation.
    m_curve->update(0, &m_previous_orig_xyz);
    m_delta = kart->getXYZ() - m_previous_orig_xyz;

    // Now the delta vector needs to be rotated back, so that it will point
    // in the right direction when it is (in update) rotated to be the same
    // as the kart's heading. To estimate the angle at the start, use the
    // interpolated value at t=dt:
    const float dt = 0.1f;
    Vec3 xyz1;
    m_curve->update(dt, &xyz1);
    core::vector3df rot = (m_previous_orig_xyz-xyz1).toIrrVector()
                                                    .getHorizontalAngle();
    btQuaternion q(Vec3(0,1,0),rot.Y*DEGREE_TO_RAD);
    btMatrix3x3 m(q);
    m_delta = m * m_delta;

    // The previous call to m_curve->update will set the internal timer
    // of the curve to dt. Reset it to 0 to make sure the timer is in
    // synch with the timer of the CanonAnimation
    m_curve->reset();
}   // CannonAnimation

// ----------------------------------------------------------------------------
CannonAnimation::~CannonAnimation()
{
    delete m_curve;

    btTransform pos;
    pos.setOrigin(m_kart->getXYZ());
    pos.setRotation(btQuaternion(btVector3(0.0f, 1.0f, 0.0f),
                                 m_kart->getHeading()        ));

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

    Vec3 xyz;
    m_curve->update(dt, &xyz);

    // It can happen that the same position is returned, e.g. if the end of
    // the curve is reached, but due to floating point differences the
    // end is not detected in the above test. To avoid that the kart then
    // rotates to a heading of 0, do not rotate in this case at all, i.e.
    // the previous rotation is kept.
    if(xyz!=m_previous_orig_xyz)
    {
        btQuaternion prev_rot = m_kart->getRotation();
        core::vector3df rot = (xyz-m_previous_orig_xyz).toIrrVector()
                                                       .getHorizontalAngle();
        btQuaternion q(Vec3(0,1,0),rot.Y*DEGREE_TO_RAD);
        m_kart->setRotation(prev_rot.slerp(q,0.1f));
    }
    m_previous_orig_xyz = xyz;

    Vec3 rotated_delta = m_kart->getTrans().getBasis()*m_delta;
    m_kart->setXYZ(xyz + rotated_delta);

    AbstractKartAnimation::update(dt);
}   // update
