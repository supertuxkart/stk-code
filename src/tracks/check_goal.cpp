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

#include "tracks/check_goal.hpp"

#include "animations/animation_base.hpp"
#include "io/xml_node.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include <stdio.h>

/** Constructor for a check goal line.
 *  \param node XML node containing the parameters for this checkline.
 *  \param index Index of this check structure in the check manager.
 */
CheckGoal::CheckGoal(const XMLNode &node,  unsigned int index) 
           : CheckLine(node, index)
{
}   // CheckGoal

// ----------------------------------------------------------------------------
/** Called when the check line is triggered. This function  creates a cannon 
 *  animation object and attaches it to the kart.
 *  \param kart_index The index of the kart that triggered the check line.
 */
void CheckGoal::trigger(unsigned int kart_index)
{
    printf("*** DEBUG TEST ***\n");
}   // CheckGoal
