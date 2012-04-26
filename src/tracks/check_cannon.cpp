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

#include "tracks/check_cannon.hpp"

#include "animations/animation_base.hpp"
#include "animations/ipo.hpp"
#include "io/xml_node.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/cannon_animation.hpp"
#include "modes/world.hpp"


/** Constructor for a check cannon. 
 *  \param node XML node containing the parameters for this checkline.
 *  \param index Index of this check structure in the check manager.
 */
CheckCannon::CheckCannon(const XMLNode &node,  unsigned int index) 
         : CheckLine(node, index)
{
    core::vector3df p1, p2;
    if(!node.get("target-p1", &p1) ||
       !node.get("target-p2", &p2)    )
    {
        printf("CheckCannon has no target line specified.\n");
        exit(-1);
    }
    m_target.setLine(p1, p2);
    m_curve = new Ipo(*(node.getNode("curve")));
}   // CheckCannon

// ----------------------------------------------------------------------------
/** Destructor, frees the curve data (which the cannon animation objects only
 *  have a read-only copy of).
 */
CheckCannon::~CheckCannon()
{
    delete m_curve;
}   // ~CheckCannon

// ----------------------------------------------------------------------------
/** Called when the check line is triggered. This function  creates a cannon 
 *  animation object and attaches it to the kart.
 *  \param kart_index The index of the kart that triggered the check line.
 */
void CheckCannon::trigger(unsigned int kart_index)
{
    Vec3 target(m_target.getMiddle());
    AbstractKart *kart = World::getWorld()->getKart(kart_index);
    if(kart->getKartAnimation()) return;

    new CannonAnimation(kart, m_curve->clone());
}   // CheckCannon
