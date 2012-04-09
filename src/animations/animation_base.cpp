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
    if(m_all_ipos.size()==0)
    {
        printf("Warning: empty animation curve.\n");
        return;
        //exit(-1);
    }

    m_playing = true;
}   // AnimationBase
// ----------------------------------------------------------------------------

AnimationBase::~AnimationBase()
{
}   // ~AnimationBase

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
    m_current_time += dt;

    if ( UserConfigParams::m_graphical_effects )
    {
        Ipo* curr;
        for_in (curr, m_all_ipos)
        {
            curr->update(m_current_time, xyz, hpr, scale);
        }
    }
}   // update

// ----------------------------------------------------------------------------
/** Approximates the overall length of each segment (curve between two points)
 *  and the overall length of the curve.
 */
void AnimationBase::computeLengths()
{
    Ipo* curr;
    // First determine the maximum number of points among all IPOs
    unsigned int max_points =0;
    float       max_time    = 0;
    for_in (curr, m_all_ipos)
    {
        const std::vector<Vec3>& points = curr->getPoints();
        max_points = std::max(max_points, (unsigned int) points.size());
        max_time   = std::max(max_time, curr->getEndTime());
    }

    // Divide (on average) each segment into STEPS points, and use
    // a simple linear approximation for this part of the curve
    const float STEPS = 100.0f * (max_points-1);
    float x           = 0;
    float dx          = max_time / STEPS ;
    float distance    = 0;

    // Initialise xyz_old with the point at time 0
    Vec3 xyz_old(0,0,0), hpr, scale;
    for_in(curr, m_all_ipos)
    {
        curr->update(/*time*/0, &xyz_old, &hpr, &scale);
    }

    for(unsigned int i=1; i<(unsigned int)STEPS; i++)
    {
        // Interpolations always start at time 0
        float x = dx * i;
        Vec3 xyz(0,0,0);
        for_in(curr, m_all_ipos)
        {
            // hpr is not needed, so just reuse old variable
            curr->update(x, &xyz, &hpr, &scale);
        }   // for curr in m_all_ipos
        distance += (xyz-xyz_old).length();
        xyz_old = xyz;
    }   // for i in m_points
}   // computeLengths
