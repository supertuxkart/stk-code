//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015  Joerg Henrichs
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

#include "animations/animation_base.hpp"

#include "animations/ipo.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "utils/vs.hpp"

#include <algorithm>
#include <cmath>


AnimationBase::AnimationBase(const XMLNode &node)
{
    float fps=25;
    node.get("fps", &fps);
    for(unsigned int i=0; i<node.getNumNodes(); i++)
    {
        if (node.getNode(i)->getName() == "animated-texture")
            continue;
        Ipo *ipo = new Ipo(*node.getNode(i), fps);
        m_all_ipos.push_back(ipo);
    }
    m_playing   = true;
    m_anim_type = ATT_CYCLIC;

    if (m_all_ipos.size() == 0)
    {
        // Throw to avoid construction completely
        throw std::runtime_error("Empty IPO, discard.");
    }
    reset();
    calculateAnimationDuration();
}   // AnimationBase
// ----------------------------------------------------------------------------
/** Special constructor which takes one IPO (or curve). This is used by the
 */
AnimationBase::AnimationBase(Ipo *ipo)
{
    m_anim_type    = ATT_CYCLIC_ONCE;
    m_playing      = true;
    m_all_ipos.push_back(ipo);
    reset();
    calculateAnimationDuration();
}   // AnimationBase(Ipo)

// ----------------------------------------------------------------------------
AnimationBase::~AnimationBase()
{
    for (Ipo* ipo : m_all_ipos)
        delete ipo;
}   // ~AnimationBase

// ----------------------------------------------------------------------------
void AnimationBase::calculateAnimationDuration()
{
    m_animation_duration = -1.0f;
    for (const Ipo* currIpo : m_all_ipos)
    {
        m_animation_duration = std::max(m_animation_duration,
            currIpo->getEndTime());
    }
}   // calculateAnimationDuration

// ----------------------------------------------------------------------------
/** Stores the initial transform (in the IPOs actually). This is necessary
 *  for relative IPOs.
 *  \param xyz Position of the object.
 *  \param hpr Rotation of the object.
 */
void AnimationBase::setInitialTransform(const Vec3 &xyz,
                                        const Vec3 &hpr)
{
    for (Ipo* curr : m_all_ipos)
    {
        curr->setInitialTransform(xyz, hpr);
    }
}   // setTransform

// ----------------------------------------------------------------------------
/** Resets all IPOs for this animation.
 */
void AnimationBase::reset()
{
    m_current_time = 0;
    for (Ipo* curr : m_all_ipos)
    {
        curr->reset();
    }
}   // reset

// ----------------------------------------------------------------------------
/** Updates the time, position and rotation. Called once per frame.
 *  \param dt Time since last call.
 *  \param xyz Position to be updated.
 *  \param hpr Rotation to be updated.
 */
void AnimationBase::update(float dt, Vec3 *xyz, Vec3 *hpr, Vec3 *scale)
{
    assert(!std::isnan(m_current_time));

    // Don't do anything if the animation is disabled
    if(!m_playing) return;
    m_current_time += dt;

    assert(!std::isnan(m_current_time));

    for (Ipo* curr : m_all_ipos)
    {
        curr->update(m_current_time, xyz, hpr, scale);
    }
}   // update

// ----------------------------------------------------------------------------
/** Return the time, position and rotation at the specified time. It does not
 *  update the internal timer as update() does.
 *  \param dt Time since last call.
 *  \param xyz Position to be updated.
 *  \param hpr Rotation to be updated.
 */
void AnimationBase::getAt(float time, Vec3 *xyz, Vec3 *hpr, Vec3 *scale)
{
    assert(!std::isnan(time));

    // Don't do anything if the animation is disabled
    if (!m_playing) return;

    for (Ipo* curr : m_all_ipos)
    {
        curr->update(time, xyz, hpr, scale);
    }
}   // getAt

// ----------------------------------------------------------------------------
/** Returns the derivative at the specified point.
 *  \param time The time for which to determine the derivative.
 *  \param xyz Float pointer to store the result.
 */
void AnimationBase::getDerivativeAt(float time, Vec3 *xyz)
{
    for (Ipo* curr : m_all_ipos)
    {
        curr->getDerivative(time, xyz);
    }
    xyz->normalize();
}
