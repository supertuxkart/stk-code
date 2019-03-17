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

#include "tracks/check_line.hpp"

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/sp/sp_dynamic_draw_call.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "io/xml_node.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/linear_world.hpp"
#include "modes/profile_world.hpp"
#include "modes/world.hpp"
#include "network/network_string.hpp"
#include "race/race_manager.hpp"

#include "irrlicht.h"

#include <algorithm>
#include <cstdint>
#include <string>

/** Constructor for a checkline.
 *  \param node XML node containing the parameters for this checkline.
 *  \param index Index of this check structure in the check manager.
 */
CheckLine::CheckLine(const XMLNode &node,  unsigned int index)
         : CheckStructure(node, index)
{
    m_ignore_height = false;
    // Note that when this is called the karts have not been allocated
    // in world, so we can't call world->getNumKarts()
    m_previous_sign.resize(race_manager->getNumberOfKarts());
    std::string p1_string("p1");
    std::string p2_string("p2");

    // In case of a cannon in a reverse track, we have to use the target line
    // as check line
    if(getType()==CT_CANNON && race_manager->getReverseTrack())
    {
        p1_string = "target-p1";
        p2_string = "target-p2";
    }
    core::vector2df p1, p2;
    if(node.get(p1_string, &p1)   &&
        node.get(p2_string, &p2)  &&
        node.get("min-height", &m_min_height))
    {
        m_left_point  = Vec3(p1.X, m_min_height, p1.Y);
        m_right_point = Vec3(p2.X, m_min_height, p2.Y);
    }
    else
    {
        node.get(p1_string, &m_left_point);
        p1 = core::vector2df(m_left_point.getX(), m_left_point.getZ());
        node.get(p2_string, &m_right_point);
        p2 = core::vector2df(m_right_point.getX(), m_right_point.getZ());
        m_min_height = std::min(m_left_point.getY(), m_right_point.getY());
    }
    m_line.setLine(p1, p2);
    if(UserConfigParams::m_check_debug && !ProfileWorld::isNoGraphics())
    {
#ifndef SERVER_ONLY
        m_debug_dy_dc = std::make_shared<SP::SPDynamicDrawCall>
            (scene::EPT_TRIANGLE_STRIP,
            SP::SPShaderManager::get()->getSPShader("additive"),
            material_manager->getDefaultSPMaterial("additive"));
        SP::addDynamicDrawCall(m_debug_dy_dc);
        m_debug_dy_dc->getVerticesVector().resize(4);
        auto& vertices = m_debug_dy_dc->getVerticesVector();
        vertices[0].m_position = core::vector3df(p1.X,
            m_min_height - m_under_min_height, p1.Y);
        vertices[1].m_position = core::vector3df(p2.X,
            m_min_height - m_under_min_height, p2.Y);
        vertices[2].m_position = core::vector3df(p1.X,
            m_min_height + m_over_min_height, p1.Y);
        vertices[3].m_position = core::vector3df(p2.X,
            m_min_height + m_over_min_height, p2.Y);
        for(unsigned int i = 0; i < 4; i++)
        {
            vertices[i].m_color = m_active_at_reset
                               ? video::SColor(128, 255, 0, 0)
                               : video::SColor(128, 128, 128, 128);
        }
        m_debug_dy_dc->recalculateBoundingBox();
#endif
    }
}   // CheckLine

// ----------------------------------------------------------------------------
CheckLine::~CheckLine()
{
    if (m_debug_dy_dc)
    {
        m_debug_dy_dc->removeFromSP();
    }

}   // CheckLine
// ----------------------------------------------------------------------------
void CheckLine::reset(const Track &track)
{
    CheckStructure::reset(track);

    for (unsigned int i = 0; i<m_previous_sign.size(); i++)
    {
        core::vector2df p = m_previous_position[i].toIrrVector2d();
        m_previous_sign[i] = m_line.getPointOrientation(p) >= 0;
    }
}   // reset

// ----------------------------------------------------------------------------
void CheckLine::resetAfterKartMove(unsigned int kart_index)
{
    if (m_previous_position.empty()) return;
    AbstractKart *kart = World::getWorld()->getKart(kart_index);
    m_previous_position[kart_index] = kart->getXYZ();
}   // resetAfterKartMove

// ----------------------------------------------------------------------------
void CheckLine::changeDebugColor(bool is_active)
{
    assert(m_debug_dy_dc);
    video::SColor color = is_active ? video::SColor(192, 255, 0, 0)
                                    : video::SColor(192, 128, 128, 128);
    for(unsigned int i = 0; i < 4; i++)
    {
        m_debug_dy_dc->getVerticesVector()[i].m_color = color;
    }
    m_debug_dy_dc->setUpdateOffset(0);
}   // changeDebugColor

// ----------------------------------------------------------------------------
/** True if going from old_pos to new_pos crosses this checkline. This function
 *  is called from update (of the checkline structure).
 *  \param old_pos   Position in previous frame.
 *  \param new_pos   Position in current frame.
 *  \param kart_indx Index of the kart, can be used to store kart specific
 *                   additional data. If set to a negative number it will
 *                   be ignored (used for e.g. soccer ball, and basket ball).
 */
bool CheckLine::isTriggered(const Vec3 &old_pos, const Vec3 &new_pos,
                            int kart_index)
{
    World* w = World::getWorld();
    core::vector2df p=new_pos.toIrrVector2d();
    bool sign = m_line.getPointOrientation(p)>=0;
    bool result;

    bool previous_sign;

    if (kart_index < 0)
    {
        core::vector2df p = old_pos.toIrrVector2d();
        previous_sign = (m_line.getPointOrientation(p) >= 0);
    }
    else
    {
        previous_sign = m_previous_sign[kart_index];
    }

    // If the sign has changed, i.e. the infinite line was crossed somewhere,
    // check if the finite line was actually crossed:
    core::vector2df cross_point;
    if (sign != previous_sign &&
        m_line.intersectWith(core::line2df(old_pos.toIrrVector2d(),
                                           new_pos.toIrrVector2d()),
                             cross_point) )
    {
        // Now check the minimum height: the kart position must be within a
        // reasonable distance in the Z axis - 'reasonable' for now to be
        // between -1 and 4 units (negative numbers are unlikely, but help
        // in case that the kart is 'somewhat' inside of the track, or the
        // checklines are a bit off in Z direction.
        result = m_ignore_height || 
                        (new_pos.getY()-m_min_height<m_over_min_height   &&
                         new_pos.getY()-m_min_height>-m_under_min_height     );
        if (UserConfigParams::m_check_debug && !result)
        {
            if (kart_index >= 0)
                Log::info("CheckLine", "Kart %s crosses line, but wrong height "
                          "(%f vs %f).",
                          World::getWorld()->getKart(kart_index)->getIdent().c_str(),
                          new_pos.getY(), m_min_height);
            else if (!result)
                Log::info("CheckLine", "Object crosses line, but wrong height "
                          "(%f vs %f).",
                          new_pos.getY(), m_min_height);
        }
    }
    else
        result = false;

    if (kart_index >= 0)
    {
        m_previous_sign[kart_index] = sign;
        if (result)
        {
            LinearWorld* lw = dynamic_cast<LinearWorld*>(w);
            if (triggeringCheckline() && lw != NULL)
                lw->setLastTriggeredCheckline(kart_index, m_index);
        }
    }
    return result;
}   // isTriggered

// ----------------------------------------------------------------------------
void CheckLine::saveCompleteState(BareNetworkString* bns)
{
    CheckStructure::saveCompleteState(bns);
    World* world = World::getWorld();
    for (unsigned int i = 0; i < world->getNumKarts(); i++)
        bns->addUInt8(m_previous_sign[i] ? 1 : 0);
}   // saveCompleteState

// ----------------------------------------------------------------------------
void CheckLine::restoreCompleteState(const BareNetworkString& b)
{
    CheckStructure::restoreCompleteState(b);
    m_previous_sign.clear();
    World* world = World::getWorld();
    for (unsigned int i = 0; i < world->getNumKarts(); i++)
    {
        bool previous_sign = b.getUInt8() == 1;
        m_previous_sign.push_back(previous_sign);
    }
}   // restoreCompleteState
