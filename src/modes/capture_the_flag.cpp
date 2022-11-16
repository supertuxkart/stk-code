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
#include "items/powerup.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart_model.hpp"
#include "modes/ctf_flag.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/server_config.hpp"
#include "network/stk_host.hpp"
#include "physics/triangle_mesh.hpp"
#include "states_screens/race_gui.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <algorithm>
#include <IAnimatedMeshSceneNode.h>
#include <ISceneManager.h>

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
    if (GUIEngine::isNoGraphics())
        return;

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
    if (GUIEngine::isNoGraphics())
        return;

    m_red_flag_node->drop();
    m_blue_flag_node->drop();
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
    const btTransform& orig_red = Track::getCurrentTrack()->getRedFlag();
    const btTransform& orig_blue = Track::getCurrentTrack()->getBlueFlag();

    m_red_flag = std::make_shared<CTFFlag>(FC_RED, orig_red);
    m_blue_flag = std::make_shared<CTFFlag>(FC_BLUE, orig_blue);
    if (NetworkConfig::get()->isNetworking())
    {
        m_red_flag->rewinderAdd();
        m_blue_flag->rewinderAdd();
    }

#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;

    m_red_flag_node = irr_driver->addAnimatedMesh(m_red_flag_mesh, "red_flag");
    m_blue_flag_node = irr_driver->addAnimatedMesh(m_blue_flag_mesh,
        "blue_flag");
    assert(m_red_flag_node);
    assert(m_blue_flag_node);
    m_red_flag_node->grab();
    m_blue_flag_node->grab();

    std::string red_path =
        file_manager->getAsset(FileManager::GUI_ICON, "red_arrow.png");
    std::string blue_path =
        file_manager->getAsset(FileManager::GUI_ICON, "blue_arrow.png");

    m_red_flag_indicator = irr_driver->addBillboard(
        core::dimension2df(1.5f, 1.5f), red_path, NULL);
    m_red_flag_indicator->setPosition(Vec3(
        orig_red(Vec3(0.0f, 2.5f, 0.0f))).toIrrVector());
    m_blue_flag_indicator = irr_driver->addBillboard(
        core::dimension2df(1.5f, 1.5f), blue_path, NULL);
    m_blue_flag_indicator->setPosition(Vec3(
        orig_blue(Vec3(0.0f, 2.5f, 0.0f))).toIrrVector());

    m_red_flag->initFlagRenderInfo(m_red_flag_node);
    m_blue_flag->initFlagRenderInfo(m_blue_flag_node);
#endif
}   // init

// ----------------------------------------------------------------------------
void CaptureTheFlag::reset(bool restart)
{
    // 5 bits for kart id (with -1 and -2 flag status)
    if (m_karts.size() > 29)
        Log::fatal("CaptureTheFlag", "Too many karts");

    FreeForAll::reset(restart);
    m_red_scores = m_blue_scores = 0;
    m_swatter_reset_kart_ticks.clear();
    m_last_captured_flag_ticks = 0;
    m_red_flag_status = m_blue_flag_status = CTFFlag::IN_BASE;
    m_red_flag->resetToBase();
    m_blue_flag->resetToBase();
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;

    if (m_red_flag_node)
        m_red_flag->updateFlagGraphics(m_red_flag_node);
    if (m_blue_flag_node)
        m_blue_flag->updateFlagGraphics(m_blue_flag_node);
#endif
}   // reset

// ----------------------------------------------------------------------------
void CaptureTheFlag::updateGraphics(float dt)
{
    FreeForAll::updateGraphics(dt);

#ifndef SERVER_ONLY
    if (m_red_flag_node)
        m_red_flag->updateFlagGraphics(m_red_flag_node);
    if (m_blue_flag_node)
        m_blue_flag->updateFlagGraphics(m_blue_flag_node);
    if (m_red_flag_indicator)
        m_red_flag_indicator->setVisible(!m_red_flag->isInBase());
    if (m_blue_flag_indicator)
        m_blue_flag_indicator->setVisible(!m_blue_flag->isInBase());

    core::stringw msg;
    // Don't show flag has been returned message if
    // a point has been scored recently
    const bool scored_recently =
        getTicksSinceStart() > m_last_captured_flag_ticks &&
        getTicksSinceStart() - m_last_captured_flag_ticks < stk_config->time2Ticks(2.0f);
    if (m_red_flag_status != m_red_flag->getStatus())
    {
        if (m_red_flag->getHolder() != -1)
        {
            AbstractKart* kart = getKart(m_red_flag->getHolder());
            const core::stringw& name = kart->getController()->getName();
            // I18N: Show when a player gets the red flag in CTF
            msg = _("%s has the red flag!", name);
            if (kart->getController()->isLocalPlayerController())
                SFXManager::get()->quickSound("wee");
        }
        else if (m_red_flag->isInBase() && !scored_recently)
        {
            // I18N: Show when the red flag is returned to its base in CTF
            msg = _("The red flag has returned!");
        }
        m_red_flag_status = m_red_flag->getStatus();
    }
    else if (m_blue_flag_status != m_blue_flag->getStatus())
    {
        if (m_blue_flag->getHolder() != -1)
        {
            AbstractKart* kart = getKart(m_blue_flag->getHolder());
            const core::stringw& name = kart->getController()->getName();
            // I18N: Show when a player gets the blue flag in CTF
            msg = _("%s has the blue flag!", name);
            if (kart->getController()->isLocalPlayerController())
                SFXManager::get()->quickSound("wee");
        }
        else if (m_blue_flag->isInBase() && !scored_recently)
        {
            // I18N: Show when the blue flag is returned to its base in CTF
            msg = _("The blue flag has returned!");
        }
        m_blue_flag_status = m_blue_flag->getStatus();
    }
    if (m_race_gui && !msg.empty())
        m_race_gui->addMessage(msg, NULL, 1.5f);
#endif
}   // updateGraphics

// ----------------------------------------------------------------------------
void CaptureTheFlag::update(int ticks)
{
    FreeForAll::update(ticks);

    for (auto it = m_swatter_reset_kart_ticks.begin();
         it != m_swatter_reset_kart_ticks.end();)
    {
        if (it->second < getTicksSinceStart() - stk_config->time2Ticks(8.0f))
        {
            it = m_swatter_reset_kart_ticks.erase(it);
        }
        else
        {
            if (it->second == getTicksSinceStart())
            {
                AbstractKart* kart = m_karts[it->first].get();
                if (kart->isEliminated() || !kart->isSquashed())
                {
                    it++;
                    continue;
                }
                unsigned int index = getRescuePositionIndex(kart);
                btTransform t = getRescueTransform(index);
                t.setOrigin(t.getOrigin() + t.getBasis().getColumn(1) * 3.0f);
                kart->getBody()->setLinearVelocity(Vec3(0.0f));
                kart->getBody()->setAngularVelocity(Vec3(0.0f));
                kart->getBody()->proceedToTransform(t);
                kart->setTrans(t);
                kart->getPowerup()->reset();
                static_cast<SmoothNetworkBody*>(kart)->reset();
            }
            it++;
        }
    }

    // Update new flags position
    m_red_flag->update(ticks);
    m_blue_flag->update(ticks);

    if (m_red_flag->getHolder() != -1 && m_blue_flag->isInBase() &&
        (m_blue_flag->getBaseOrigin() - m_red_flag->getOrigin()).length() <
        g_capture_length)
    {
        // Blue team scored
        if (!NetworkConfig::get()->isNetworking() ||
            NetworkConfig::get()->isServer())
        {
            int red_holder = m_red_flag->getHolder();
            int new_kart_scores = m_scores.at(red_holder) + g_captured_score;
            int new_blue_scores = m_blue_scores + 1;
            m_scores.at(red_holder) = new_kart_scores;
            if (NetworkConfig::get()->isServer())
            {
                NetworkString p(PROTOCOL_GAME_EVENTS);
                p.setSynchronous(true);
                p.addUInt8(GameEventsProtocol::GE_CTF_SCORED)
                    .addUInt8((int8_t)red_holder)
                    .addUInt8(0/*red_team_scored*/)
                    .addUInt16((int16_t)new_kart_scores)
                    .addUInt8((uint8_t)m_red_scores)
                    .addUInt8((uint8_t)new_blue_scores);
                STKHost::get()->sendPacketToAllPeers(&p, true);
            }
            ctfScored(red_holder, false/*red_team_scored*/, new_kart_scores,
                m_red_scores, new_blue_scores);
        }
        m_last_captured_flag_ticks = World::getWorld()->getTicksSinceStart();
        m_red_flag->resetToBase(RaceManager::get()->getFlagDeactivatedTicks());
    }
    else if (m_blue_flag->getHolder() != -1 && m_red_flag->isInBase() &&
        (m_red_flag->getBaseOrigin() - m_blue_flag->getOrigin()).length() <
        g_capture_length)
    {
        // Red team scored
        if (!NetworkConfig::get()->isNetworking() ||
            NetworkConfig::get()->isServer())
        {
            int blue_holder = m_blue_flag->getHolder();
            int new_kart_scores = m_scores.at(blue_holder) + g_captured_score;
            int new_red_scores = m_red_scores + 1;
            m_scores.at(blue_holder) = new_kart_scores;
            if (NetworkConfig::get()->isServer())
            {
                NetworkString p(PROTOCOL_GAME_EVENTS);
                p.setSynchronous(true);
                p.addUInt8(GameEventsProtocol::GE_CTF_SCORED)
                    .addUInt8((int8_t)blue_holder)
                    .addUInt8(1/*red_team_scored*/)
                    .addUInt16((int16_t)new_kart_scores)
                    .addUInt8((uint8_t)new_red_scores)
                    .addUInt8((uint8_t)m_blue_scores);
                STKHost::get()->sendPacketToAllPeers(&p, true);
            }
            ctfScored(blue_holder, true/*red_team_scored*/, new_kart_scores,
                new_red_scores, m_blue_scores);
        }
        m_last_captured_flag_ticks = World::getWorld()->getTicksSinceStart();
        m_blue_flag->resetToBase(RaceManager::get()->getFlagDeactivatedTicks());
    }

    // Test if red or blue flag is touched
    for (auto& k : m_karts)
    {
        if (k->isEliminated() || k->getKartAnimation() || k->isSquashed())
            continue;

        if (m_red_flag->canBeCaptured() &&
            (k->getXYZ() - m_red_flag->getOrigin()).length() <
            g_capture_length)
        {
            uint8_t kart_id = (uint8_t)k->getWorldKartId();
            if (getKartTeam(kart_id) == KART_TEAM_RED)
            {
                if (!m_red_flag->isInBase())
                {
                    // Return the flag
                    m_red_flag->resetToBase(
                        RaceManager::get()->getFlagDeactivatedTicks());
                }
            }
            else
            {
                // Get the flag
                m_red_flag->setCapturedByKart(kart_id);
            }
        }
        if (m_blue_flag->canBeCaptured() &&
            (k->getXYZ() - m_blue_flag->getOrigin()).length() <
            g_capture_length)
        {
            uint8_t kart_id = (uint8_t)k->getWorldKartId();
            if (getKartTeam(kart_id) == KART_TEAM_BLUE)
            {
                if (!m_blue_flag->isInBase())
                {
                    // Return the flag
                    m_blue_flag->resetToBase(
                        RaceManager::get()->getFlagDeactivatedTicks());
                }
            }
            else
            {
                // Get the flag
                m_blue_flag->setCapturedByKart(kart_id);
            }
        }
    }
}   // update

// ----------------------------------------------------------------------------
int CaptureTheFlag::getRedHolder() const
{
    return m_red_flag->getHolder();
}   // getRedHolder

// ----------------------------------------------------------------------------
int CaptureTheFlag::getBlueHolder() const
{
    return m_blue_flag->getHolder();
}   // getBlueHolder

// ----------------------------------------------------------------------------
bool CaptureTheFlag::isRedFlagInBase() const
{
    return m_red_flag->isInBase();
}   // isRedFlagInBase

// ----------------------------------------------------------------------------
bool CaptureTheFlag::isBlueFlagInBase() const
{
    return m_blue_flag->isInBase();
}   // isBlueFlagInBase

// ----------------------------------------------------------------------------
const Vec3& CaptureTheFlag::getRedFlag() const
{
    return m_red_flag->getOrigin();
}   // getRedFlag

// ----------------------------------------------------------------------------
const Vec3& CaptureTheFlag::getBlueFlag() const
{
    return m_blue_flag->getOrigin();
}   // getBlueFlag

// ----------------------------------------------------------------------------
void CaptureTheFlag::ctfScored(int kart_id, bool red_team_scored,
                               int new_kart_score, int new_red_score,
                               int new_blue_score)
{
    m_scores.at(kart_id) = new_kart_score;
    AbstractKart* kart = getKart(kart_id);
    core::stringw scored_msg;
    const core::stringw& name = kart->getController()->getName();
    m_red_scores = new_red_score;
    m_blue_scores = new_blue_score;
    if (red_team_scored)
    {
        scored_msg = _("%s captured the blue flag!", name);
    }
    else
    {
        scored_msg = _("%s captured the red flag!", name);
    }
#ifndef SERVER_ONLY
    // Don't set animation and show message if receiving in live join
    if (isStartPhase() || GUIEngine::isNoGraphics())
        return;
    if (m_race_gui)
        m_race_gui->addMessage(scored_msg, NULL, 3.0f);
    kart->getKartModel()
        ->setAnimation(KartModel::AF_WIN_START, true/*play_non_loop*/);
    m_scored_sound->play();
#endif
}   // ctfScored

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

    if (RaceManager::get()->getHitCaptureLimit() == 0)
    {
        // Prevent infinitive game
        if (!RaceManager::get()->hasTimeTarget())
            return true;
        return m_count_down_reached_zero;
    }

    if ((m_count_down_reached_zero && RaceManager::get()->hasTimeTarget()) ||
        (m_red_scores >= RaceManager::get()->getHitCaptureLimit() ||
        m_blue_scores >= RaceManager::get()->getHitCaptureLimit()))
        return true;

    return false;
}   // isRaceOver

// ----------------------------------------------------------------------------
void CaptureTheFlag::loseFlagForKart(int kart_id)
{
    if (!(m_red_flag->getHolder() == kart_id ||
        m_blue_flag->getHolder() == kart_id))
        return;

    bool drop_red_flag = m_red_flag->getHolder() == kart_id;
    btTransform dropped_trans = drop_red_flag ?
        m_red_flag->getBaseTrans() :
        m_blue_flag->getBaseTrans();
    bool succeed = getDroppedFlagTrans(getKart(kart_id)->getTrans(),
        &dropped_trans);
    if (drop_red_flag)
    {
        if (succeed)
            m_red_flag->dropFlagAt(dropped_trans);
        else
        {
            m_red_flag->resetToBase(
                RaceManager::get()->getFlagDeactivatedTicks());
        }
    }
    else
    {
        if (succeed)
            m_blue_flag->dropFlagAt(dropped_trans);
        else
        {
            m_blue_flag->resetToBase(
                RaceManager::get()->getFlagDeactivatedTicks());
        }
    }
}   // loseFlagForKart

// ----------------------------------------------------------------------------
bool CaptureTheFlag::kartHit(int kart_id, int hitter)
{
    if (isRaceOver())
        return false;

    if (!NetworkConfig::get()->isNetworking() ||
        NetworkConfig::get()->isServer())
        handleScoreInServer(kart_id, hitter);

    loseFlagForKart(kart_id);
    return true;
}   // kartHit

//-----------------------------------------------------------------------------
unsigned int CaptureTheFlag::getRescuePositionIndex(AbstractKart *kart)
{
    return m_kart_position_map.at(kart->getWorldKartId());
}   // getRescuePositionIndex

// ----------------------------------------------------------------------------
/** Returns the internal identifier for this race.
 */
const std::string& CaptureTheFlag::getIdent() const
{
    return IDENT_CTF;
}   // getIdent

// ----------------------------------------------------------------------------
void CaptureTheFlag::saveCompleteState(BareNetworkString* bns, STKPeer* peer)
{
    FreeForAll::saveCompleteState(bns, peer);
    bns->addUInt32(m_red_scores).addUInt32(m_blue_scores);
}   // saveCompleteState

// ----------------------------------------------------------------------------
void CaptureTheFlag::restoreCompleteState(const BareNetworkString& b)
{
    FreeForAll::restoreCompleteState(b);
    m_red_scores = b.getUInt32();
    m_blue_scores = b.getUInt32();
}   // restoreCompleteState
