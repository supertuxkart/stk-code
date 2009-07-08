//  $Id: checkline.cpp 1681 2008-04-09 13:52:48Z hikerstk $
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

#include "tracks/checkline.hpp"

#include <string>

#include "io/xml_node.hpp"
#include "race/race_manager.hpp"

Checkline::Checkline(const XMLNode &node) : CheckStructure(node)
{
    m_previous_sign.resize(race_manager->getNumKarts());
    core::vector2df p1, p2;
	node.get("p1", &p1);
	node.get("p2", &p2);
	node.get("min-height", &m_min_height);
    m_line.setLine(p1, p2);
}   // Checkline

// ----------------------------------------------------------------------------
void Checkline::reset(const Track &track)
{
    CheckStructure::reset(track);
    for(unsigned int i=0; i<m_previous_sign.size(); i++)
    {
        core::vector2df p = m_previous_position[i].toIrrVector2d();
        m_previous_sign[i] = m_line.getPointOrientation(p)>=0;
    }
}   // reset

// ----------------------------------------------------------------------------
/** True if going from old_pos to new_pos crosses this checkline. This function
 *  is called from update (of the checkline structure).
 *  \param old_pos  Position in previous frame.
 *  \param new_pos  Position in current frame.
 *  \param indx     Index of the kart, can be used to store kart specific
 *                  additional data.
 */
bool Checkline::isTriggered(const Vec3 &old_pos, const Vec3 &new_pos, int indx)
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
        result = new_pos.getZ()-m_min_height<4.0f   && 
                 new_pos.getZ()-m_min_height>-1.0f;
    }
    else
        result = false;
    m_previous_sign[indx] = sign;
    return result;
}   // update
