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

#include "modes/capture_the_flag.hpp"
#include "io/file_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "network/network_config.hpp"
#include "tracks/track.hpp"

#include <algorithm>

//-----------------------------------------------------------------------------
/** Constructor. Sets up the clock mode etc.
 */
CaptureTheFlag::CaptureTheFlag() : FreeForAll()
{
    m_red_flag_node = m_blue_flag_node = NULL;
    m_red_flag_mesh = m_blue_flag_mesh = NULL;
#ifndef SERVER_ONLY
    file_manager->pushTextureSearchPath(
        file_manager->getAsset(FileManager::MODEL,""), "models");
    m_red_flag_mesh = irr_driver->getAnimatedMesh
        (file_manager->getAsset(FileManager::MODEL, "red_flag.spm"));
    m_blue_flag_mesh = irr_driver->getAnimatedMesh
        (file_manager->getAsset(FileManager::MODEL, "blue_flag.spm"));
    assert(m_red_flag_mesh);
    assert(m_blue_flag_mesh);
    irr_driver->grabAllTextures(m_red_flag_mesh);
    irr_driver->grabAllTextures(m_blue_flag_mesh);
    file_manager->popTextureSearchPath();
#endif
}   // CaptureTheFlag

//-----------------------------------------------------------------------------
CaptureTheFlag::~CaptureTheFlag()
{
#ifndef SERVER_ONLY
    irr_driver->dropAllTextures(m_red_flag_mesh);
    irr_driver->dropAllTextures(m_blue_flag_mesh);
    irr_driver->removeMeshFromCache(m_red_flag_mesh);
    irr_driver->removeMeshFromCache(m_blue_flag_mesh);
#endif
}   // ~CaptureTheFlag

//-----------------------------------------------------------------------------
void CaptureTheFlag::init()
{
    FreeForAll::init();
    const btTransform& red_pos = Track::getCurrentTrack()->getRedFlag();
    const btTransform& blue_pos = Track::getCurrentTrack()->getBlueFlag();

#ifndef SERVER_ONLY
    m_red_flag_node = irr_driver->addAnimatedMesh(m_red_flag_mesh, "red_flag");
    m_blue_flag_node = irr_driver->addAnimatedMesh(m_blue_flag_mesh,
        "blue_flag");
    assert(m_red_flag_node);
    assert(m_blue_flag_node);

    m_red_flag_node->setPosition(Vec3(red_pos.getOrigin()).toIrrVector());
    Vec3 hpr;
    hpr.setHPR(red_pos.getRotation());
    m_red_flag_node->setRotation(hpr.toIrrHPR());

    m_blue_flag_node->setPosition(Vec3(blue_pos.getOrigin()).toIrrVector());
    hpr.setHPR(blue_pos.getRotation());
    m_blue_flag_node->setRotation(hpr.toIrrHPR());
#endif
}   // init
