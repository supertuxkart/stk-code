//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012  Joerg Henrichs
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

#ifndef HEADER_CHECK_GOAL_HPP
#define HEADER_CHECK_GOAL_HPP

#include "animations/animation_base.hpp"
#include "tracks/check_line.hpp"
#include "utils/cpp2011.h"

class CheckManager;
class XMLNode;

/** 
 *  \brief Implements a simple checkline that will score a point when the
 *         soccer ball crosses it.
 *
 * \ingroup tracks
 */
class CheckGoal : public CheckLine
{
private:
    bool    m_first_goal;
public:
             CheckGoal(const XMLNode &node, unsigned int index);
    virtual ~CheckGoal() {}
    virtual void update(float dt) OVERRIDE;
    virtual void trigger(unsigned int kart_index);
};   // CheckLine

#endif
