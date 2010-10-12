//  $Id: animation_base.cpp 1681 2008-04-09 13:52:48Z hikerstk $
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
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"

AnimationBase::AnimationBase(const XMLNode &node)
             : TrackObject(node)
{
    float fps=25;
    node.get("fps", &fps);
    for(unsigned int i=0; i<node.getNumNodes(); i++)
    {
        Ipo *ipo = new Ipo(*node.getNode(i), fps);
        m_all_ipos.push_back(ipo);
    }   // for i<getNumNodes()
    m_playing = true;

}   // AnimationBase
// ----------------------------------------------------------------------------
/** Removes all IPOs.
 */
AnimationBase::~AnimationBase()
{
    std::vector<Ipo*>::iterator i;
    for(i=m_all_ipos.begin(); i<m_all_ipos.end(); i++)
        delete *i;
    m_all_ipos.clear();
}   // ~AnimationBase

// ----------------------------------------------------------------------------
/** Stores the initial transform (in the IPOs actually). This is necessary
 *  for relative IPOs.
 *  \param xyz Position of the object.
 *  \param hpr Rotation of the object.
 */
void AnimationBase::setInitialTransform(const core::vector3df &xyz, 
                                        const core::vector3df &hpr)
{
    std::vector<Ipo*>::iterator i;
    for(i=m_all_ipos.begin(); i<m_all_ipos.end(); i++)
    {
        (*i)->setInitialTransform(xyz, hpr);
    }   // for i in m_all_ipos
}   // setTransform

// ----------------------------------------------------------------------------
/** Resets all IPOs for this animation.
 */
void AnimationBase::reset()
{
    std::vector<Ipo*>::iterator i;
    for(i=m_all_ipos.begin(); i<m_all_ipos.end(); i++)
    {
        (*i)->reset();
    }   // for i in m_all_ipos
}   // reset

// ----------------------------------------------------------------------------
/** Updates the time, position and rotation. Called once per framce.
 *  \param dt Time since last call.
 *  \param xyz Position to be updated.
 *  \param hpr Rotation to be updated.
 */
void AnimationBase::update(float dt, core::vector3df *xyz, 
                           core::vector3df *hpr, core::vector3df *scale)
{
    std::vector<Ipo*>::iterator i;
    for(i=m_all_ipos.begin(); i<m_all_ipos.end(); i++)
    {
        (*i)->update(dt, xyz, hpr, scale);
    }   // for i in m_all_ipos

}   // float dt
