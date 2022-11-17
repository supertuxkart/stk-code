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
#include "modes/world.hpp"
#include "network/network_string.hpp"
#include "race/race_manager.hpp"
#include "tracks/graph.hpp"
#include "tracks/quad.hpp"

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
    m_previous_sign.resize(RaceManager::get()->getNumberOfKarts());
    std::string p1_string("p1");
    std::string p2_string("p2");

    // In case of a cannon in a reverse track, we have to use the target line
    // as check line
    if(getType()==CT_CANNON && RaceManager::get()->getReverseTrack())
    {
        p1_string = "target-p1";
        p2_string = "target-p2";
    }
    float min_height = 0.0f;
    Vec3 normal(0, 1, 0);
    Vec3 center;
    core::vector2df p1, p2;
    if(node.get(p1_string, &p1)   &&
        node.get(p2_string, &p2)  &&
        node.get("min-height", &min_height))
    {
        m_left_point  = Vec3(p1.X, min_height, p1.Y);
        m_right_point = Vec3(p2.X, min_height, p2.Y);
        center = (m_left_point + m_right_point) * 0.5f;
    }
    else
    {
        node.get(p1_string, &m_left_point);
        node.get(p2_string, &m_right_point);
        center = (m_left_point + m_right_point) * 0.5f;
        int sector = -1;
        if (Graph::get())
            Graph::get()->findRoadSector(center, &sector);
        if (sector != -1)
            normal = Graph::get()->getQuad(sector)->getNormal();
    }

    // How much a kart is allowed to be under the minimum height of a
    // quad and still considered to be able to cross it.
    float under_min_height = 1.0f;

    // How much a kart is allowed to be over the minimum height of a
    // quad and still considered to be able to cross it.
    float over_min_height = 4.0f;

    m_check_plane[0].pointA = Vec3(m_left_point + (normal *
        under_min_height * -1.0f)).toIrrVector();
    m_check_plane[0].pointB = Vec3(m_right_point + (normal *
        under_min_height * -1.0f)).toIrrVector();
    m_check_plane[0].pointC = Vec3(m_left_point + (normal *
        over_min_height)).toIrrVector();
    m_check_plane[1].pointA = Vec3(m_left_point + (normal *
        over_min_height)).toIrrVector();
    m_check_plane[1].pointB = Vec3(m_right_point + (normal *
        under_min_height * -1.0f)).toIrrVector();
    m_check_plane[1].pointC = Vec3(m_right_point + (normal *
        over_min_height)).toIrrVector();

    // 2, 3 are scaled for testing ignoring height (used only by basketball atm)
    // Only scale upwards a little because basket ball cannot be too high
    // Also to avoid check plane overlapping
    over_min_height = 10.0f;
    m_check_plane[2].pointA = Vec3(m_left_point + (normal *
        under_min_height * -1.0f)).toIrrVector();
    m_check_plane[2].pointB = Vec3(m_right_point + (normal *
        under_min_height * -1.0f)).toIrrVector();
    m_check_plane[2].pointC = Vec3(m_left_point + (normal *
        over_min_height)).toIrrVector();
    m_check_plane[3].pointA = Vec3(m_left_point + (normal *
        over_min_height)).toIrrVector();
    m_check_plane[3].pointB = Vec3(m_right_point + (normal *
        under_min_height * -1.0f)).toIrrVector();
    m_check_plane[3].pointC = Vec3(m_right_point + (normal *
        over_min_height)).toIrrVector();

    if(UserConfigParams::m_check_debug && !GUIEngine::isNoGraphics())
    {
#ifndef SERVER_ONLY
        m_debug_dy_dc = std::make_shared<SP::SPDynamicDrawCall>
            (scene::EPT_TRIANGLE_STRIP,
            SP::SPShaderManager::get()->getSPShader("additive"),
            material_manager->getDefaultSPMaterial("additive"));
        SP::addDynamicDrawCall(m_debug_dy_dc);
        m_debug_dy_dc->getVerticesVector().resize(4);
        auto& vertices = m_debug_dy_dc->getVerticesVector();
        vertices[0].m_position = m_check_plane[0].pointA;
        vertices[1].m_position = m_check_plane[0].pointB;
        vertices[2].m_position = m_check_plane[0].pointC;
        vertices[3].m_position = m_check_plane[1].pointC;
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
        m_previous_sign[i] = m_previous_position[i].sideofPlane(
            m_check_plane[0].pointA,
            m_check_plane[0].pointB, m_check_plane[0].pointC) >= 0;
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
    if (!m_debug_dy_dc)
        return;
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
    // Sign here is for old client (<= 1.2) in networking, it's not used
    // anymore now
    bool sign = new_pos.sideofPlane(m_check_plane[0].pointA,
        m_check_plane[0].pointB, m_check_plane[0].pointC) >= 0;
    bool result = false;

    bool ignore_height = m_ignore_height;
    bool check_line_debug = false;
start:
    irr::core::triangle3df* check_plane =
        ignore_height ? &m_check_plane[2] : &m_check_plane[0];
    core::line3df test(old_pos.toIrrVector(), new_pos.toIrrVector());
    core::vector3df intersect;
    if (check_plane[0].getIntersectionWithLimitedLine(test, intersect) ||
        check_plane[1].getIntersectionWithLimitedLine(test, intersect))
    {
        if (UserConfigParams::m_check_debug && check_line_debug)
        {
            if (kart_index >= 0)
            {
                Log::info("CheckLine", "Kart %s crosses line, but wrong height.",
                    World::getWorld()->getKart(kart_index)->getIdent().c_str());
            }
            else
            {
                Log::info("CheckLine", "Object crosses line, but wrong height.");
            }
        }
        else
            result = true;
    }
    else if (UserConfigParams::m_check_debug && !ignore_height &&
        !check_line_debug)
    {
        check_line_debug = true;
        ignore_height = true;
        goto start;
    }

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
