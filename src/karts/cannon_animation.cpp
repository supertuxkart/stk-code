//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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
                                 const Vec3 &start_left, const Vec3 &start_right,
                                 const Vec3 &end_left,   const Vec3 &end_right   )
               : AbstractKartAnimation(kart, "CannonAnimation")
{
    m_curve  = new AnimationBase(ipo);
    m_timer  = ipo->getEndTime();
    
    // First make sure that left and right points are indeed correct
    // -------------------------------------------------------------
    Vec3 my_start_left = start_left;
    Vec3 my_start_right = start_right;
    Vec3 p0, p1;
    // Define a plane that goes through the middle of the start line
    // (the curve's origin must be in the middle of the line.
    m_curve->getAt(0, &p0);
    m_curve->getAt(0.1f, &p1);
    Vec3 p2 = 0.5f*(p0 + p1) + m_kart->getNormal();
    if (start_left.sideofPlane(p0, p1, p2) < 0)
    {
        // Left and right start line needs to be swapped
        my_start_left = start_right;
        my_start_right = start_left; 
    }

    // First adjust start and end points to take on each side half the kart
    // width into account:
    Vec3 direction = my_start_right - my_start_left;
    direction.normalize();

    float kw = m_kart->getKartModel()->getWidth();
    Vec3 adj_start_left  = my_start_left  + (0.5f*kw) * direction;
    Vec3 adj_start_right = my_start_right - (0.5f*kw) * direction;

    // Store the length of the start and end line, which is used
    // during update() to adjust the distance to center
    m_start_line_length = (adj_start_left - adj_start_right).length();
    m_end_line_length = (end_left - end_right).length() - kw;

    // The current kart position is divided into three components:
    // kart.xyz = curve.xyz + parallel_to_start_line_component + rest
    // 1) curve.xyz: The point at the curve at t=0.
    // 2) parallel_to_start_line_component:
    //    A component parallel to the start line. This component is scaled
    //    depending on time and length of start- and end-line (e.g. if the 
    //    end line is twice as long as the start line, this will make sure
    //    that a kart starting at the very left of the start line will end
    //    up at the very left of the end line). This component can also be
    //    adjusted by steering while in the air. This is done by modifying
    //    m_fraction_of_line, which is multiplied with the current width
    //    vector.
    // 3) rest: The amoount that the kart is ahead and above the
    //    start line. This is stored in m_delta and will be added to the
    //    newly computed curve xyz coordinates.
    // 
    // Compute the delta between the kart position and the start of the curve.
    // This delta is rotated with the kart and added to the interpolated curve
    // position to get the actual kart position during the animation.
    Vec3 curve_xyz;
    m_curve->update(0, &curve_xyz);
    m_delta = kart->getXYZ() - curve_xyz;
    
    // Compute on which fraction of the start line the kart is, to get the
    // second component of the kart position: distance along start line
    Vec3 v = adj_start_left - adj_start_right;
    float l = v.length();
    v /= l;

    float f = v.dot(adj_start_left - kart->getXYZ());
    if (f <= 0)
        f = 0;
    else if (f >= l)
        f = l;
    else
        f = f / l;
    // Now f is in [0,1] - 0 in case of left side, 1 if the kart is at the
    // very right. Convert this to [-1,1] assuming that the ipo for the
    // cannon is in the middle of the start and end line
    m_fraction_of_line = 2.0f*f - 1.0f;

    Vec3 delta = 0.5f*m_fraction_of_line * (adj_start_right - adj_start_left);
    // Subtract the horizontal difference, to get the constant offset the
    // kart has from the curve.
    m_delta = m_delta - delta;
    
    // The previous call to m_curve->update will set the internal timer
    // of the curve to dt. Reset it to 0 to make sure the timer is in
    // synch with the timer of the CanonAnimation
    m_curve->reset();
}   // CannonAnimation

// ----------------------------------------------------------------------------
CannonAnimation::~CannonAnimation()
{
    delete m_curve;

    btTransform pos = m_kart->getTrans();
    m_kart->getBody()->setCenterOfMassTransform(pos);
    Vec3 v(0, 0, m_kart->getKartProperties()->getEngineMaxSpeed());
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

    // First compute the current rotation
    // -----------------------------------
    // Get the tangent = derivative at the current point to compute the
    // new orientation of the kart
    Vec3 tangent;
    m_curve->getDerivativeAt(m_curve->getAnimationDuration() - m_timer,
                             &tangent);
    // Get the current kart orientation
    Vec3 forward = m_kart->getTrans().getBasis().getColumn(2);
    forward.normalize();
    
    // Only adjust the heading.
    // ------------------------
    // I tried to also adjust pitch at the same time, but that adds a strong
    // roll to the kart on some cannons
    Vec3 v1(tangent), v2(forward);
    v1.setY(0); v2.setY(0);
    btQuaternion q = m_kart->getRotation()*shortestArcQuatNormalize2(v2, v1);

    // While start and end line have to have the same 'up' vector, karts can 
    // sometimes be not parallel to them. So slowly adjust this over time
    Vec3 up = m_kart->getTrans().getBasis().getColumn(1);
    up.normalize();
    Vec3 gravity = -m_kart->getBody()->getGravity();
    gravity.normalize();
    // Adjust only 5% towards the real up vector. This will smoothly
    // adjust the kart.
    Vec3 target_up_vector = (gravity*0.05f + up*0.95f).normalize();
    btQuaternion q_up = shortestArcQuat(up, target_up_vector);

    m_kart->setRotation(q_up * q);

    // Then compute the new location of the kart
    // -----------------------------------------
    // The timer counts backwards, so the fraction goes from 1 to 0
    float f = m_timer / m_curve->getAnimationDuration();
    float f_current_width = m_start_line_length * f
                          + m_end_line_length   * (1.0f - f);

    // Adjust the horizontal location based on steering
    m_fraction_of_line += m_kart->getSteerPercent()*dt*2.0f;
    btClamp(m_fraction_of_line, -1.0f, 1.0f);

    // horiz_delta is in kart coordinates, the rotation by q will
    // transform it to the global coordinate system
    Vec3 horiz_delta = Vec3(0.5f*m_fraction_of_line  * f_current_width, 0, 0);

    Vec3 rotated_delta = quatRotate(q, m_delta + horiz_delta);

    Vec3 curve_xyz;
    m_curve->update(dt, &curve_xyz);
    m_kart->setXYZ(curve_xyz+rotated_delta);

    AbstractKartAnimation::update(dt);
}   // update
