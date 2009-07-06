//  $Id: check_structure.hpp 1681 2008-04-09 13:52:48Z hikerstk $
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

#ifndef HEADER_CHECK_STRUCTURE_HPP
#define HEADER_CHECK_STRUCTURE_HPP

#include <vector>

#include "utils/vec3.hpp"

class XMLNode;
class Track;

/** Virtual base class for a check structure. */
class CheckStructure
{
protected:
	/** Stores the previous position of all karts. This is needed to detect
     *  when e.g. a check point is reached the first time, or a checkline is
     *  crossed. */
    std::vector<Vec3> m_previous_position;
public:
                CheckStructure();
    virtual    ~CheckStructure() {};
    virtual void update(float dt);
    /** True if going from old_pos to new_pos crosses this checkline. This function
     *  is called from update (of the checkline structure).
     *  \param old_pos  Position in previous frame.
     *  \param new_pos  Position in current frame.
     *  \param indx     Index of the kart, can be used to store kart specific
     *                  additional data.
     */
    virtual bool isTriggered(const Vec3 &old_pos, const Vec3 &new_pos, int indx)=0;
    virtual void trigger();
    virtual void reset(const Track &track);
};   // CheckStructure

#endif

