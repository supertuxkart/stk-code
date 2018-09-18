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
#include "audio/sfx_base.hpp"
#include "io/file_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart_model.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/server_config.hpp"
#include "network/stk_host.hpp"
#include "physics/triangle_mesh.hpp"
#include "states_screens/race_gui.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object_manager.hpp"

#include <algorithm>

const Vec3 g_kart_flag_offset(0.0, 0.2f, -0.5f);
const float g_capture_length = 2.0f;
const int g_captured_score = 10;

// ----------------------------------------------------------------------------
/** Constructor. Sets up the clock mode etc.
 */
CaptureTheFlag::CaptureTheFlag() : FreeForAll()
{
    m_red_flag_node = m_blue_flag_node = NULL;
    m_red_flag_indicator = m_blue_flag_indicator = NULL;
    m_red_flag_mesh = m_blue_flag_mesh = NULL;
    m_scored_sound = NULL;
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
    m_scored_sound = SFXManager::get()->createSoundSource("goal_scored");
#endif
}   // CaptureTheFlag

// ----------------------------------------------------------------------------
CaptureTheFlag::~CaptureTheFlag()
{
#ifndef SERVER_ONLY
    irr_driver->dropAllTextures(m_red_flag_mesh);
    irr_driver->dropAllTextures(m_blue_flag_mesh);
    irr_driver->removeMeshFromCache(m_red_flag_mesh);
    irr_driver->removeMeshFromCache(m_blue_flag_mesh);
    m_scored_sound->deleteSFX();
#endif
}   // ~CaptureTheFlag

// ----------------------------------------------------------------------------
void CaptureTheFlag::init()
{
    FreeForAll::init();
    m_orig_red_trans = Track::getCurrentTrack()->getRedFlag();
    m_orig_blue_trans = Track::getCurrentTrack()->getBlueFlag();

#ifndef SERVER_ONLY
    m_red_flag_node = irr_driver->addAnimatedMesh(m_red_flag_mesh, "red_flag");
    m_blue_flag_node = irr_driver->addAnimatedMesh(m_blue_flag_mesh,
        "blue_flag");
    assert(m_red_flag_node);
    assert(m_blue_flag_node);

    std::string red_path =
        file_manager->getAsset(FileManager::GUI_ICON, "red_arrow.png");
    std::string blue_path =
        file_manager->getAsset(FileManager::GUI_ICON, "blue_arrow.png");

    m_red_flag_indicator = irr_driver->addBillboard(
            core::dimension2df(1.5f, 1.5f), red_path, NULL);
    m_red_flag_indicator->setPosition(Vec3(
        m_orig_red_trans(Vec3(0.0f, 2.5f, 0.0f))).toIrrVector());
    m_blue_flag_indicator = irr_driver->addBillboard(
            core::dimension2df(1.5f, 1.5f), blue_path, NULL);
    m_blue_flag_indicator->setPosition(Vec3(
        m_orig_blue_trans(Vec3(0.0f, 2.5f, 0.0f))).toIrrVector());
#endif
}   // init

// ----------------------------------------------------------------------------
void CaptureTheFlag::reset()
{
    FreeForAll::reset();
    m_red_trans = m_orig_red_trans;
    m_blue_trans = m_orig_blue_trans;
    m_red_return_ticks = m_blue_return_ticks = m_red_scores =
        m_blue_scores = 0;
    m_red_holder = m_blue_holder = -1;
    updateFlagNodes();
}   // reset

// ----------------------------------------------------------------------------
void CaptureTheFlag::updateGraphics(float dt)
{
    FreeForAll::updateGraphics(dt);
    if (!NetworkConfig::get()->isNetworking() ||
        NetworkConfig::get()->isClient())
    {
        if (m_red_holder != -1)
        {
            m_red_trans = getKart(m_red_holder)->getSmoothedTrans();
            m_red_trans.setOrigin(m_red_trans(g_kart_flag_offset));
            m_red_flag_node->setAnimationSpeed(fabsf(getKart(m_red_holder)
                ->getSpeed()) * 3.0f + 25.0f);
        }
        else
            m_red_flag_node->setAnimationSpeed(25.0f);

        if (m_blue_holder != -1)
        {
            m_blue_trans = getKart(m_blue_holder)->getSmoothedTrans();
            m_blue_trans.setOrigin(m_blue_trans(g_kart_flag_offset));
            m_blue_flag_node->setAnimationSpeed(fabsf(getKart(m_blue_holder)
                ->getSpeed()) * 3.0f + 25.0f);
        }
        else
            m_blue_flag_node->setAnimationSpeed(25.0f);
        m_red_flag_indicator->setVisible(!isRedFlagInBase());
        m_blue_flag_indicator->setVisible(!isBlueFlagInBase());
    }
}   // updateGraphics

// ----------------------------------------------------------------------------
void CaptureTheFlag::update(int ticks)
{
    if (m_red_holder != -1 && m_blue_holder != -1 &&
        m_red_holder == m_blue_holder)
        Log::fatal("CaptureTheFlag", "Flag management messed up, abort.");

    FreeForAll::update(ticks);

    if (!NetworkConfig::get()->isNetworking() ||
        NetworkConfig::get()->isClient())
        return;

    // Update new flags position
    if (m_red_holder != -1)
    {
        m_red_trans = getKart(m_red_holder)->getTrans();
        m_red_trans.setOrigin(m_red_trans(g_kart_flag_offset));
    }
    if (m_blue_holder != -1)
    {
        m_blue_trans = getKart(m_blue_holder)->getTrans();
        m_blue_trans.setOrigin(m_blue_trans(g_kart_flag_offset));
    }

    const bool red_flag_in_base =
        m_red_trans.getOrigin() == m_orig_red_trans.getOrigin();
    const bool blue_flag_in_base =
        m_blue_trans.getOrigin() == m_orig_blue_trans.getOrigin();

    // Check if not returning for too long
    if (m_red_holder != -1 || red_flag_in_base)
        m_red_return_ticks = 0;
    else
        m_red_return_ticks++;

    if (m_blue_holder != -1 || blue_flag_in_base)
        m_blue_return_ticks = 0;
    else
        m_blue_return_ticks++;

    const int max_flag_return_timeout = stk_config->time2Ticks(
        ServerConfig::m_flag_return_timemout);
    if (m_red_holder == -1 && m_red_return_ticks > max_flag_return_timeout)
    {
        resetRedFlagToOrigin();
        m_red_return_ticks = 0;
        return;
    }
    if (m_blue_holder == -1 && m_blue_return_ticks > max_flag_return_timeout)
    {
        resetBlueFlagToOrigin();
        m_blue_return_ticks = 0;
        return;
    }

    if (m_red_holder != -1 && m_blue_holder == -1 &&
        blue_flag_in_base &&
        (m_orig_blue_trans.getOrigin() - m_red_trans.getOrigin()).length() <
        g_capture_length)
    {
        // Blue team scored
        NetworkString p(PROTOCOL_GAME_EVENTS);
        p.setSynchronous(true);
        p.addUInt8(GameEventsProtocol::GE_CTF_RESET)
            .addUInt8(1 << 1 | 0) // Reset red flag
            .addUInt8((int8_t)m_red_holder);
        STKHost::get()->sendPacketToAllPeers(&p, true);
        m_scores.at(m_red_holder) += g_captured_score;
        m_red_holder = -1;
        m_red_trans = m_orig_red_trans;
        m_blue_scores++;
        return;
    }
    else if (m_blue_holder != -1 && m_red_holder == -1 &&
        red_flag_in_base &&
        (m_orig_red_trans.getOrigin() - m_blue_trans.getOrigin()).length() <
        g_capture_length)
    {
        // Red team scored
        NetworkString p(PROTOCOL_GAME_EVENTS);
        p.setSynchronous(true);
        p.addUInt8(GameEventsProtocol::GE_CTF_RESET)
            .addUInt8(0 << 1 | 0)  // Reset blue flag
            .addUInt8((int8_t)m_blue_holder);
        STKHost::get()->sendPacketToAllPeers(&p, true);
        m_scores.at(m_blue_holder) += g_captured_score;
        m_blue_holder = -1;
        m_blue_trans = m_orig_blue_trans;
        m_red_scores++;
        return;
    }

    // Test if red or blue flag is touched
    for (auto& k : m_karts)
    {
        if (k->isEliminated() || k->getKartAnimation() || k->isSquashed())
            continue;

        if (m_red_holder == -1 &&
            (k->getXYZ() - m_red_trans.getOrigin()).length() <
            g_capture_length)
        {
            uint8_t kart_id = (uint8_t)k->getWorldKartId();
            if (getKartTeam(kart_id) == KART_TEAM_RED)
            {
                if (!red_flag_in_base)
                {
                    // Return the flag
                    resetRedFlagToOrigin();
                }
            }
            else
            {
                // Get the flag
                NetworkString p(PROTOCOL_GAME_EVENTS);
                p.setSynchronous(true);
                p.addUInt8(GameEventsProtocol::GE_CTF_ATTACH)
                    .addUInt8(1)  // Attach red flag
                    .addUInt8(kart_id);
                STKHost::get()->sendPacketToAllPeers(&p, true);
                m_red_holder = kart_id;
            }
        }
        if (m_blue_holder == -1 &&
            (k->getXYZ() - m_blue_trans.getOrigin()).length() <
            g_capture_length)
        {
            uint8_t kart_id = (uint8_t)k->getWorldKartId();
            if (getKartTeam(kart_id) == KART_TEAM_BLUE)
            {
                if (!blue_flag_in_base)
                {
                    // Return the flag
                    resetBlueFlagToOrigin();
                }
            }
            else
            {
                // Get the flag
                NetworkString p(PROTOCOL_GAME_EVENTS);
                p.setSynchronous(true);
                p.addUInt8(GameEventsProtocol::GE_CTF_ATTACH)
                    .addUInt8(0)  // Attach blue flag
                    .addUInt8(kart_id);
                STKHost::get()->sendPacketToAllPeers(&p, true);
                m_blue_holder = kart_id;
            }
        }
    }
}   // update

// ----------------------------------------------------------------------------
void CaptureTheFlag::resetRedFlagToOrigin()
{
    NetworkString p(PROTOCOL_GAME_EVENTS);
    p.setSynchronous(true);
    p.addUInt8(GameEventsProtocol::GE_CTF_RESET)
        .addUInt8(1 << 1 | 0)  // Reset red flag to original
        .addUInt8(((int8_t)-1));
    STKHost::get()->sendPacketToAllPeers(&p, true);
    m_red_trans = m_orig_red_trans;
}   // resetRedFlagToOrigin

// ----------------------------------------------------------------------------
void CaptureTheFlag::resetBlueFlagToOrigin()
{
    NetworkString p(PROTOCOL_GAME_EVENTS);
    p.setSynchronous(true);
    p.addUInt8(GameEventsProtocol::GE_CTF_RESET)
        .addUInt8(0 << 1 | 0)  // Reset blue flag to original
        .addUInt8(((int8_t)-1));
    STKHost::get()->sendPacketToAllPeers(&p, true);
    m_blue_trans = m_orig_blue_trans;
}   // resetBlueFlagToOrigin

// ----------------------------------------------------------------------------
void CaptureTheFlag::updateFlagNodes()
{
#ifndef SERVER_ONLY
    Vec3 hpr;
    if (m_red_holder == -1)
    {
        m_red_flag_node->setPosition(
            Vec3(m_red_trans.getOrigin()).toIrrVector());
        hpr.setHPR(m_red_trans.getRotation());
        m_red_flag_node->setRotation(hpr.toIrrHPR());
    }

    if (m_blue_holder == -1)
    {
        m_blue_flag_node->setPosition(
            Vec3(m_blue_trans.getOrigin()).toIrrVector());
        hpr.setHPR(m_blue_trans.getRotation());
        m_blue_flag_node->setRotation(hpr.toIrrHPR());
    }
#endif
}   // updateFlagNodes

// ----------------------------------------------------------------------------
void CaptureTheFlag::attachFlag(NetworkString& ns)
{
#ifndef SERVER_ONLY
    bool attach_red_flag = ns.getUInt8() == 1;
    unsigned kart_id = ns.getUInt8();
    core::stringw get_msg;
    const core::stringw& name = getKart(kart_id)->getController()
        ->getName();
    if (attach_red_flag)
    {
        m_red_holder = kart_id;
        m_red_flag_node->setParent(getKart(kart_id)->getNode());
        m_red_flag_node->setPosition(g_kart_flag_offset.toIrrVector());
        m_red_flag_node->setRotation(core::vector3df(0.0f, 180.0f, 0.0f));
        // I18N: Show when a player gets the flag in CTF
        get_msg = _("%s has the red flag!", name);
    }
    else
    {
        m_blue_holder = kart_id;
        m_blue_flag_node->setParent(getKart(kart_id)->getNode());
        m_blue_flag_node->setPosition(g_kart_flag_offset.toIrrVector());
        m_blue_flag_node->setRotation(core::vector3df(0.0f, 180.0f, 0.0f));
        // I18N: Show when a player gets the flag in CTF
        get_msg = _("%s has the blue flag!", name);
    }
    if (getKart(kart_id)->getController()->isLocalPlayerController())
        SFXManager::get()->quickSound("wee");
    m_race_gui->addMessage(get_msg, NULL, 1.5f);
#endif
}   // attachFlag

// ----------------------------------------------------------------------------
void CaptureTheFlag::resetFlag(NetworkString& ns)
{
#ifndef SERVER_ONLY
    uint8_t reset_info = ns.getUInt8();
    bool reset_red_flag = (reset_info >> 1 & 1) == 1;
    bool with_custom_transform = (reset_info & 1) == 1;
    int8_t kart_id = ns.getUInt8();
    if (kart_id != -1)
    {
        core::stringw scored_msg;
        AbstractKart* kart = getKart(kart_id);
        const core::stringw& name = kart->getController()->getName();
        if (reset_red_flag)
        {
            m_scores.at(kart_id) += g_captured_score;
            m_red_holder = -1;
            m_red_trans = m_orig_red_trans;
            // I18N: Show when a player captured the flag in CTF
            scored_msg = _("%s captured the red flag!", name);
            m_red_flag_node->setParent(
                irr_driver->getSceneManager()->getRootSceneNode());
            m_blue_scores++;
        }
        else
        {
            m_scores.at(kart_id) += g_captured_score;
            m_blue_holder = -1;
            m_blue_trans = m_orig_blue_trans;
            // I18N: Show when a player captured the flag in CTF
            scored_msg = _("%s captured the blue flag!", name);
            m_blue_flag_node->setParent(
                irr_driver->getSceneManager()->getRootSceneNode());
            m_red_scores++;
        }
        m_race_gui->addMessage(scored_msg, NULL, 3.0f);
        kart->getKartModel()
            ->setAnimation(KartModel::AF_WIN_START, true/* play_non_loop*/);
        m_scored_sound->play();
    }
    else
    {
        core::stringw returned_msg;
        if (reset_red_flag)
        {
            btTransform t = m_orig_red_trans;
            // I18N: Show when the red flag is returned to its base in CTF
            if (!with_custom_transform)
                returned_msg = _("The red flag has returned!");
            else
            {
                t.setOrigin(ns.getVec3());
                t.setRotation(ns.getQuat());
            }
            m_red_holder = -1;
            m_red_trans = t;
            m_red_flag_node->setParent(
                irr_driver->getSceneManager()->getRootSceneNode());
        }
        else
        {
            btTransform t = m_orig_blue_trans;
            // I18N: Show when the blue flag is returned to its base in CTF
            if (!with_custom_transform)
                returned_msg = _("The blue flag has returned!");
            else
            {
                t.setOrigin(ns.getVec3());
                t.setRotation(ns.getQuat());
            }
            m_blue_holder = -1;
            m_blue_trans = t;
            m_blue_flag_node->setParent(
                irr_driver->getSceneManager()->getRootSceneNode());
        }
        if (!returned_msg.empty())
            m_race_gui->addMessage(returned_msg, NULL, 1.5f);
    }
    updateFlagNodes();
#endif
}   // resetFlag

// ----------------------------------------------------------------------------
bool CaptureTheFlag::getDroppedFlagTrans(const btTransform& kt,
                                         btTransform* out) const
{
    Vec3 from = kt.getOrigin() + kt.getBasis().getColumn(1);
    Vec3 to = kt.getOrigin() + kt.getBasis().getColumn(1) * -10000.0f;
    Vec3 hit_point, normal;
    const Material* material = NULL;

    // From TerrainInfo::update
    const TriangleMesh &tm = Track::getCurrentTrack()->getTriangleMesh();
    bool ret = tm.castRay(from, to, &hit_point, &material, &normal,
        /*interpolate*/false);
    if (!ret || material == NULL)
        return false;

    Track::getCurrentTrack()->getTrackObjectManager()->castRay
        (from, to, &hit_point, &material, &normal, /*interpolate*/false);
    *out = btTransform(shortestArcQuat(Vec3(0, 1, 0), normal),
        hit_point);
    return true;
}   // getDroppedFlagTrans

// ----------------------------------------------------------------------------
video::SColor CaptureTheFlag::getColor(unsigned int kart_id) const
{
    return getKartTeam(kart_id) == KART_TEAM_RED ?
        video::SColor(255, 255, 0, 0) : video::SColor(255, 0, 0, 255);
}   // getColor

// ----------------------------------------------------------------------------
bool CaptureTheFlag::isRaceOver()
{
    if (m_unfair_team)
        return true;

    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient())
        return false;

    if ((m_count_down_reached_zero && race_manager->hasTimeTarget()) ||
        (m_red_scores >= race_manager->getHitCaptureLimit() ||
        m_blue_scores >= race_manager->getHitCaptureLimit()))
        return true;

    return false;
}   // isRaceOver

// ----------------------------------------------------------------------------
bool CaptureTheFlag::kartHit(int kart_id, int hitter)
{
    if (!FreeForAll::kartHit(kart_id, hitter))
        return false;
    if (m_red_holder == kart_id || m_blue_holder == kart_id)
    {
        bool reset_red_flag = m_red_holder == kart_id;
        btTransform dropped_trans = reset_red_flag ?
            m_orig_red_trans : m_orig_blue_trans;
        bool succeed = getDroppedFlagTrans(
            getKart(kart_id)->getTrans(), &dropped_trans);
        NetworkString p(PROTOCOL_GAME_EVENTS);
        p.setSynchronous(true);
        // If reset red flag
        uint8_t reset_info = reset_red_flag ? 1 : 0;
        reset_info <<= 1;
        // With custom transform
        if (succeed)
            reset_info |= 1;
        p.addUInt8(GameEventsProtocol::GE_CTF_RESET).addUInt8(reset_info)
            .addUInt8(((int8_t)-1));
        if (succeed)
        {
            p.add(Vec3(dropped_trans.getOrigin()))
                .add(dropped_trans.getRotation());
        }
        STKHost::get()->sendPacketToAllPeers(&p, true);
        if (reset_red_flag)
        {
            m_red_holder = -1;
            m_red_trans = dropped_trans;
        }
        else
        {
            m_blue_holder = -1;
            m_blue_trans = dropped_trans;
        }
    }
    return true;
}   // kartHit

//-----------------------------------------------------------------------------
unsigned int CaptureTheFlag::getRescuePositionIndex(AbstractKart *kart)
{
    return m_kart_position_map.at(kart->getWorldKartId());
}   // getRescuePositionIndex
