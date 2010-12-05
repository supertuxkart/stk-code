//  $Id: animation_manager.cpp 1681 2008-04-09 13:52:48Z hikerstk $
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

#include "animations/animation_manager.hpp"

#include <string>

#include "animations/billboard_animation.hpp"
#include "animations/three_d_animation.hpp"
#include "io/xml_node.hpp"

AnimationManager::AnimationManager(const Track &track, const XMLNode &node)
{
    for(unsigned int i=0; i<node.getNumNodes(); i++)
    {
        const XMLNode *anim_node = node.getNode(i);
        std::string type = anim_node->getName();
        if(type=="anim_billboard")
            m_all_animations.push_back(new BillboardAnimation(*anim_node));
        else if(type=="animations-IPO")
            m_all_animations.push_back(new ThreeDAnimation(*anim_node));
        else
            fprintf(stderr, "Unknown animation type '%s' - ignored.\n", 
                    type.c_str());
    }   // for i<node.getNumNodes
}   // AnimationManager

// ----------------------------------------------------------------------------
/** Removes all animations from the scene node and memory. Called from
 *  Track::cleanup().
 */
AnimationManager::~AnimationManager()
{
    std::vector<AnimationBase*>::iterator i;
    for(i=m_all_animations.begin(); i!=m_all_animations.end(); i++)
        delete *i;
    m_all_animations.clear();
}   // ~AnimationManager

// ----------------------------------------------------------------------------
/** Resets all animations.
 */
void AnimationManager::reset()
{
    std::vector<AnimationBase*>::iterator i;
    for(i=m_all_animations.begin(); i!=m_all_animations.end(); i++)
        (*i)->reset();
}   // reset

// ----------------------------------------------------------------------------
/** Updates all animations. Called once per time step.
 *  \param dt Time since last call.
 */
void AnimationManager::update(float dt)
{
    std::vector<AnimationBase*>::iterator i;
    for(i=m_all_animations.begin(); i!=m_all_animations.end(); i++)
        (*i)->update(dt);
}   // update
