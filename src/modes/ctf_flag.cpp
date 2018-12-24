//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#include "modes/ctf_flag.hpp"
#include "graphics/irr_driver.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "network/network_string.hpp"
#include "utils/mini_glm.hpp"

#include "LinearMath/btQuaternion.h"

// ============================================================================
// Position offset to attach in kart model
const Vec3 g_kart_flag_offset(0.0, 0.2f, -0.5f);
// ============================================================================
BareNetworkString* CTFFlag::saveState(std::vector<std::string>* ru)
{
    using namespace MiniGLM;
    ru->push_back(getUniqueIdentity());
    BareNetworkString* buffer = new BareNetworkString();
    buffer->addUInt8(m_flag_status);
    if (m_flag_status == OFF_BASE)
    {
        Vec3 normal = quatRotate(m_flag_trans.getRotation(),
            Vec3(0.0f, 1.0f, 0.0f));
        buffer->add(m_flag_trans.getOrigin());
        buffer->addUInt32(
            compressVector3(Vec3(normal.normalize()).toIrrVector()));
        buffer->addUInt16(m_ticks_since_off_base);
    }
    return buffer;
}   // saveState

// ----------------------------------------------------------------------------
void CTFFlag::restoreState(BareNetworkString* buffer, int count)
{
    using namespace MiniGLM;
    m_flag_status = buffer->getUInt8();
    if (m_flag_status == OFF_BASE)
    {
        Vec3 origin = buffer->getVec3();
        uint32_t normal_packed = buffer->getUInt32();
        Vec3 normal = decompressVector3(normal_packed);
        m_flag_trans.setOrigin(origin);
        m_flag_trans.setRotation(
            shortestArcQuat(Vec3(0.0f, 1.0f, 0.0f), normal));
        m_ticks_since_off_base = buffer->getUInt16();
    }
    updateFlagTrans(m_flag_trans);
}   // restoreState

// ----------------------------------------------------------------------------
void CTFFlag::updateFlagTrans(const btTransform& off_base_trans)
{
    if (getHolder() != -1)
    {
        AbstractKart* k = World::getWorld()->getKart(getHolder());
        m_flag_trans = k->getTrans();
        m_flag_trans.setOrigin(m_flag_trans(g_kart_flag_offset));
    }
    else if (m_flag_status == OFF_BASE)
    {
        m_flag_trans = off_base_trans;
    }
    else
        m_flag_trans = m_flag_base_trans;
}

// ----------------------------------------------------------------------------
void CTFFlag::update(int ticks)
{
    updateFlagTrans(m_flag_trans);

    // Check if not returning for too long
    if (m_flag_status != OFF_BASE)
        return;

    m_ticks_since_off_base += ticks;
    if (m_ticks_since_off_base > race_manager->getFlagReturnTicks())
    {
        resetToBase();
    }
}   // update

// ----------------------------------------------------------------------------
void CTFFlag::updateFlagGraphics(irr::scene::IAnimatedMeshSceneNode* flag_node)
{
    if (getHolder() != -1)
    {
        AbstractKart* k = World::getWorld()->getKart(getHolder());
        flag_node->setParent(k->getNode());
        flag_node->setPosition(g_kart_flag_offset.toIrrVector());
        flag_node->setRotation(core::vector3df(0.0f, 180.0f, 0.0f));
        flag_node->setAnimationSpeed(fabsf(k->getSpeed()) * 3.0f + 25.0f);
    }
    else
    {
        flag_node->setParent(
            irr_driver->getSceneManager()->getRootSceneNode());
        Vec3 hpr;
        flag_node->setPosition(
            Vec3(m_flag_trans.getOrigin()).toIrrVector());
        hpr.setHPR(m_flag_trans.getRotation());
        flag_node->setRotation(hpr.toIrrHPR());
        flag_node->setAnimationSpeed(25.0f);
    }
}   // updateFlagPosition
