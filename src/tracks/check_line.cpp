//  $Id: check_line.cpp 1681 2008-04-09 13:52:48Z hikerstk $
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

#include "tracks/check_line.hpp"

#include <string>

#include "io/xml_node.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"

/** Constructor for a checkline. 
 *  \param check_manager Pointer to the check manager, which is needed when
 *         resetting e.g. new lap counters. 
 *  \param node XML node containing the parameters for this checkline.
 */
CheckLine::CheckLine(CheckManager *check_manager, const XMLNode &node, 
                     unsigned int index) 
         : CheckStructure(check_manager, node, index)
{
    // Note that when this is called the karts have not been allocated
    // in world, so we can't call world->getNumKarts()
    m_previous_sign.resize(race_manager->getNumberOfKarts());
    core::vector2df p1, p2;
    node.get("p1", &p1);
    node.get("p2", &p2);
    node.get("min-height", &m_min_height);
    m_line.setLine(p1, p2);
}   // CheckLine

// ----------------------------------------------------------------------------
void CheckLine::reset(const Track &track)
{
    CheckStructure::reset(track);
    for(unsigned int i=0; i<m_previous_sign.size(); i++)
    {
        core::vector2df p = m_previous_position[i].toIrrVector2d();
        m_previous_sign[i] = m_line.getPointOrientation(p)>=0;
    }
}   // reset

// ----------------------------------------------------------------------------
/** Returns the center point of this checkline.
 */
Vec3 CheckLine::getCenterPoint() const
{
    core::vector2df c=m_line.getMiddle();
    Vec3 xyz(c.X, m_min_height, c.Y);
    return xyz;
}   // getCenterPoint

// ----------------------------------------------------------------------------
/** True if going from old_pos to new_pos crosses this checkline. This function
 *  is called from update (of the checkline structure).
 *  \param old_pos  Position in previous frame.
 *  \param new_pos  Position in current frame.
 *  \param indx     Index of the kart, can be used to store kart specific
 *                  additional data.
 */
bool CheckLine::isTriggered(const Vec3 &old_pos, const Vec3 &new_pos, int indx)
{
    core::vector2df p=new_pos.toIrrVector2d();
    bool sign = m_line.getPointOrientation(p)>=0;
    bool result=sign!=m_previous_sign[indx];
    // If the sign has changed, i.e. the infinite line was crossed somewhere,
    // check if the finite line was actually crossed:
    core::vector2df out;
    if(sign!=m_previous_sign[indx] &&
        m_line.intersectWith(core::line2df(old_pos.toIrrVector2d(), 
                                           new_pos.toIrrVector2d()), out) )
    {
        // Now check the minimum height: the kart position must be within a
        // reasonable distance in the Z axis - 'reasonable' for now to be
        // between -1 and 4 units (negative numbers are unlikely, but help
        // in case that there is 'somewhat' inside of the track, or the
        // checklines are a bit off in Z direction.
        result = new_pos.getY()-m_min_height<4.0f   && 
                 new_pos.getY()-m_min_height>-1.0f;
        if(UserConfigParams::m_check_debug && !result)
        {
            printf("CHECK: Kart %s crosses line, but wrong height (%f vs %f).\n",
                    World::getWorld()->getKart(indx)->getIdent().c_str(),
                    new_pos.getY(), m_min_height);
        }
    }
    else
        result = false;
    m_previous_sign[indx] = sign;
    return result;
}   // isTriggered
