//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015  Joerg Henrichs
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

#include "tracks/check_cylinder.hpp"

#include <string>
#include <stdio.h>

#include "io/xml_node.hpp"
#include "items/item.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"

CheckCylinder::CheckCylinder(const XMLNode &node, unsigned int index, TriggerItemListener* listener)
           : CheckStructure(node, index)
{
    m_radius2 = 1;
    m_height = 0;
    m_listener = listener;
    node.get("height", &m_height);
    node.get("radius", &m_radius2);
    m_radius2 *= m_radius2;
    node.get("xyz", &m_center_point);
    unsigned int num_karts = race_manager->getNumberOfKarts();
    m_is_inside.resize(num_karts);
    m_distance2.resize(num_karts);
    for (unsigned int i=0; i<num_karts; i++)
    {
        m_is_inside[i] = false;
    }
}   // CheckCylinder

// ----------------------------------------------------------------------------
/** True if going from old_pos to new_pos enters or leaves this cylinder. This
 *  function is called from update (of the checkline structure). It also
 *  updates the flag about which karts are inside
 *  \param old_pos  Position in previous frame.
 *  \param new_pos  Position in current frame.
 *  \param kart_id  Index of the kart, can be used to store kart specific
 *                  additional data.
 */
bool CheckCylinder::isTriggered(const Vec3 &old_pos, const Vec3 &new_pos,
                                int kart_id)
{
    // TODO: this is the code for a sphere, rewrite for cylinder
    Vec3 old_pos_xz(old_pos.x(), 0.0f, old_pos.z());
    Vec3 new_pos_xz(new_pos.x(), 0.0f, new_pos.z());
    Vec3 center_xz(m_center_point.x(), 0.0f, m_center_point.z());
    float old_dist2 = (old_pos_xz - center_xz).length2();
    float new_dist2 = (new_pos_xz - center_xz).length2();
    m_is_inside[kart_id] = new_dist2<m_radius2;
    m_distance2[kart_id] = new_dist2;
    // Trigger if the kart goes from outside (or border) to inside,
    // or inside ro outside (or border).
    bool triggered = (old_dist2>=m_radius2 && new_dist2 < m_radius2) ||
           (old_dist2< m_radius2 && new_dist2 >=m_radius2);

    if (triggered && m_listener != NULL)
        m_listener->onTriggerItemApproached();

    return triggered;
}   // isTriggered
