//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015  Joerg Henrichs
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

#ifndef HEADER_CHECK_LAP_HPP
#define HEADER_CHECK_LAP_HPP

#include "tracks/check_structure.hpp"

class XMLNode;
class CheckManager;

/**
 *  \brief Implements a simple lap test. A new lap is detected
 *  when the distance along the track reduces by a certain amount
 *  of time.
 * \ingroup tracks
 */
class CheckLap : public CheckStructure
{
private:
    /** Store the previous distance along track. */
    std::vector<float> m_previous_distance;

public:
                 CheckLap(const XMLNode &node, unsigned int index);
    virtual     ~CheckLap() {};
    virtual bool isTriggered(const Vec3 &old_pos, const Vec3 &new_pos,
                             int indx);
    virtual void reset(const Track &track);
};   // CheckLine

#endif

