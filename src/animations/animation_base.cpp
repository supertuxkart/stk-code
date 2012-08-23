//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009  Joerg Henrichs
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
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"

#include <algorithm>

AnimationBase::AnimationBase(const XMLNode &node)
             : TrackObject(node)
{
    float fps=25;
    node.get("fps", &fps);
    for(unsigned int i=0; i<node.getNumNodes(); i++)
    {
        Ipo *ipo = new Ipo(*node.getNode(i), fps);
        m_all_ipos.push_back(ipo);
    }
    m_playing   = true;
    m_anim_type = ATT_CYCLIC;

    if (m_all_ipos.size() == 0) // this will happen for some separate but non-animated objects
    {
        m_playing = false;
    }

}   // AnimationBase
// ----------------------------------------------------------------------------
/** Special constructor which takes one IPO (or curve). This is used by the
 */
AnimationBase::AnimationBase(Ipo *ipo) : TrackObject()
{
    m_anim_type    = ATT_CYCLIC_ONCE;
    m_playing      = true;
    m_all_ipos.push_back(ipo);
    reset();
}   // AnimationBase(Ipo)

// ----------------------------------------------------------------------------
/** Stores the initial transform (in the IPOs actually). This is necessary
 *  for relative IPOs.
 *  \param xyz Position of the object.
 *  \param hpr Rotation of the object.
 */
void AnimationBase::setInitialTransform(const Vec3 &xyz, 
                                        const Vec3 &hpr)
{
    Ipo* curr;
    for_in (curr, m_all_ipos)
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
    Ipo* curr;
    for_in (curr, m_all_ipos)
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
    TrackObject::update(dt);
    
    // Don't do anything if the animation is disabled
    if(!m_playing) return;
    m_current_time += dt;

    Ipo* curr;
    for_in (curr, m_all_ipos)
    {
        curr->update(m_current_time, xyz, hpr, scale);
    }
}   // update

