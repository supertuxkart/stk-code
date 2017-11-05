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
#include "items/flyable.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "modes/world.hpp"

#include "LinearMath/btTransform.h"

/** The constructor for the cannon animation. 
 *  \param kart The kart to be animated. Can also be NULL if a basket ball
 *         etc is animated (e.g. cannon animation).
 *  \param ipo The IPO (blender interpolation curve) which the kart
 *         should follow.
 *  \param start_left, start_right: Left and right end points of the line
 *         that the kart just crossed.
 *  \param end_left, end_right: Left and right end points of the line at
 *         which the kart finishes.
 *  \param skid_rot Visual rotation of the kart due to skidding (while this
 *         value can be queried, the AbstractkartAnimation constructor
 *         resets the value to 0, so it needs to be passed in.
 */
CannonAnimation::CannonAnimation(AbstractKart *kart, Ipo *ipo,
                                 const Vec3 &start_left, const Vec3 &start_right,
                                 const Vec3 &end_left,   const Vec3 &end_right,
                                 float skid_rot)
               : AbstractKartAnimation(kart, "CannonAnimation")
{
    m_flyable = NULL;
    init(ipo, start_left, start_right, end_left, end_right, skid_rot);
}   // CannonAnimation

// ----------------------------------------------------------------------------
/** Constructor for a flyable. It sets the kart data to NULL.
 */
CannonAnimation::CannonAnimation(Flyable *flyable, Ipo *ipo,
                                 const Vec3 &start_left, const Vec3 &start_right,
                                 const Vec3 &end_left, const Vec3 &end_right   )
               : AbstractKartAnimation(NULL, "CannonAnimation")
{
    m_flyable = flyable;
    init(ipo, start_left, start_right, end_left, end_right, /*skid_rot*/0);
}   // CannonAnimation(Flyable*...)

// ----------------------------------------------------------------------------
/** Common initialisation for kart-based and flyable-based animations.
 *  \param ipo The IPO (blender interpolation curve) which the kart
 *         should follow.
 *  \param start_left, start_right: Left and right end points of the line
 *         that the kart just crossed.
 *  \param end_left, end_right: Left and right end points of the line at
 *         which the kart finishes.
 *  \param skid_rot Visual rotation of the kart due to skidding (while this
 *         value can be queried, the AbstractkartAnimation constructor
 *         resets the value to 0, so it needs to be passed in.
 */
void CannonAnimation::init(Ipo *ipo, const Vec3 &start_left,
    const Vec3 &start_right, const Vec3 &end_left,
    const Vec3 &end_right, float skid_rot)
{
    m_curve = new AnimationBase(ipo);
    m_timer = ipo->getEndTime();

    // First make sure that left and right points are indeed correct
    // -------------------------------------------------------------
    Vec3 my_start_left = start_left;
    Vec3 my_start_right = start_right;
    Vec3 p0, p1;
    // Define a plane that goes through the middle of the start line
    // (the curve's origin must be in the middle of the line.
    m_curve->getAt(0, &p0);
    m_curve->getAt(0.1f, &p1);
    Vec3 p2;
    if (m_kart)
        p2 = 0.5f*(p0 + p1) + m_kart->getNormal();
    else
        p2 = 0.5f*(p0 + p1) + m_flyable->getNormal();

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

    float kw = m_kart ? m_kart->getKartModel()->getWidth()
                      : m_flyable->getExtend().getX();
    Vec3 adj_start_left = my_start_left + (0.5f*kw) * direction;
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
    Vec3 xyz = m_kart ? m_kart->getXYZ() : m_flyable->getXYZ();
    m_curve->update(0, &curve_xyz);
    m_delta = xyz - curve_xyz;

    // Compute on which fraction of the start line the kart is, to get the
    // second component of the kart position: distance along start line
    Vec3 v = adj_start_left - adj_start_right;
    float l = v.length();
    v /= l;

    float f = v.dot(adj_start_left - xyz);
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

    // Compute the original heading of the kart. At the end of the cannon,
    // the kart should be parallel to the curve, but at the beginning it 
    // the kart should be parallel to the curve and facing forwards, but
    // at the beginning it might not be. The initial rotation between the
    // tangent of the curce and the kart is stored as a m_delta_heading,
    // which will be applied to the curve orientation in update, but reduced
    // over time till it becomes 0 at the end of the curve. The effect is that
    // initially (t=0) the kart will keep its (non-orhtogonal) rotation,
    // but smoothly this will adjusted until at the end the kart will be
    // facing forwards again.
    Vec3 tangent;
    m_curve->getDerivativeAt(0, &tangent);
    // Get the current kart orientation
    const btTransform &trans = m_kart ? m_kart->getTrans()
                                      : m_flyable->getBody()->getWorldTransform();
    Vec3 forward = trans.getBasis().getColumn(2);
    Vec3 v1(tangent), v2(forward);
    v1.setY(0); v2.setY(0);
    m_delta_heading = shortestArcQuatNormalize2(v1, v2)
        * btQuaternion(Vec3(0, 1, 0), skid_rot);


    // The previous call to m_curve->update will set the internal timer
    // of the curve to dt. Reset it to 0 to make sure the timer is in
    // synch with the timer of the CanonAnimation
    m_curve->reset();
}   // init

// ----------------------------------------------------------------------------
CannonAnimation::~CannonAnimation()
{
    delete m_curve;
    if (m_kart)
    {
        btTransform pos = m_kart->getTrans();
        m_kart->getBody()->setCenterOfMassTransform(pos);
        Vec3 v(0, 0, m_kart->getKartProperties()->getEngineMaxSpeed());
        m_kart->setVelocity(pos.getBasis()*v);
    }
    else
    {
        m_flyable->setAnimation(NULL);
    }
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
    AbstractKartAnimation::update(dt);

    // First compute the current rotation
    // ==================================
    // Get the tangent = derivative at the current point to compute the
    // new orientation of the kart
    Vec3 tangent;
    m_curve->getDerivativeAt(m_curve->getAnimationDuration() - m_timer,
                             &tangent);
    // Get the current kart orientation
    const btTransform &trans = m_kart ? m_kart->getTrans()
                                      : m_flyable->getBody()->getWorldTransform();
    Vec3 forward = trans.getBasis().getColumn(2);
    
    // Heading
    // -------
    // I tried to also adjust pitch at the same time, but that adds a strong
    // roll to the kart on some cannons while it is in the air (caused by
    // the rotation axis returned shortestArc not being orthogonal to the 
    // up vector).
    Vec3 v1(tangent), v2(forward);
    v1.setY(0); v2.setY(0);
    btQuaternion heading = shortestArcQuatNormalize2(v2, v1);

    // Align to up-vector
    // ------------------
    // While start and end line have to have the same 'up' vector, karts can 
    // sometimes be not parallel to them. So slowly adjust this over time
    Vec3 up = trans.getBasis().getColumn(1);
    up.normalize();
    Vec3 gravity = m_kart ? -m_kart->getBody()->getGravity()
                          : -m_flyable->getBody()->getGravity();
    if (gravity.length2() > 0)
        gravity.normalize();
    else
        gravity.setValue(0, 1, 0);
    // Adjust only 5% towards the real up vector. This will smoothly
    // adjust the kart while the kart is in the air
    Vec3 target_up_vector = (gravity*0.05f + up*0.95f).normalize();
    btQuaternion q_up = shortestArcQuat(up, target_up_vector);

    // Additional kart rotation
    // ------------------------
    // Apply any additional rotation the kart had when crossing the start
    // line. This rotation will be reduced the closer the kart gets to
    // the end line, with the result that at the start line the kart will
    // be not rotated at all (so the visuals from physics to cannon will
    // be smoothed), and at the end line the kart will face in the 
    // forward direction.

    // The timer counts backwards, so the fraction goes from 1 to 0
    float f = m_timer / m_curve->getAnimationDuration();
    float f_current_width = m_start_line_length * f
                          + m_end_line_length   * (1.0f - f);

    // Then compute the new location of the kart
    // -----------------------------------------
    btQuaternion all_heading;
    if (m_kart)
    {
        btQuaternion zero(gravity, 0);
        btQuaternion current_delta_heading = zero.slerp(m_delta_heading, f);
        all_heading = m_kart->getRotation()*current_delta_heading*heading;
        m_kart->setRotation(q_up * all_heading);

        // Adjust the horizontal location based on steering
        m_fraction_of_line += m_kart->getSteerPercent()*dt*2.0f;
        btClamp(m_fraction_of_line, -1.0f, 1.0f);
    }   // if m_kart
    else
    {
        // If a rubber ball is in this cannon, reduce its height over
        // time so that it starts closer to the ground when released
        float height = m_delta.getY();
        float radius = m_flyable->getExtend().getY();
        height = (height - radius) * 0.95f + radius;
        m_delta.setY(height);
        all_heading.setValue(0, 0, 0, 1);
    }

    // Determine direction orthogonal to the curve for the sideway movement
    // of the kart.
    Vec3 sideways = gravity.cross(tangent);

    Vec3 rotated_delta = sideways*(0.5f*m_fraction_of_line  * f_current_width)
                       + quatRotate(all_heading, m_delta);

    Vec3 curve_xyz;
    m_curve->update(dt, &curve_xyz);
    if (m_kart)
        m_kart->setXYZ(curve_xyz + rotated_delta);
    else
        m_flyable->setXYZ(curve_xyz + rotated_delta);
}   // update
