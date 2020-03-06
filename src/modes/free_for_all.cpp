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

#include "modes/free_for_all.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/stk_host.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"

#include <algorithm>
#include <utility>

// ----------------------------------------------------------------------------
/** Constructor. Sets up the clock mode etc.
 */
FreeForAll::FreeForAll() : WorldWithRank()
{
    if (RaceManager::get()->hasTimeTarget())
    {
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN,
            RaceManager::get()->getTimeTarget());
    }
    else
    {
        WorldStatus::setClockMode(CLOCK_CHRONO);
    }
}   // FreeForAll

// ----------------------------------------------------------------------------
FreeForAll::~FreeForAll()
{
}   // ~FreeForAll

// ----------------------------------------------------------------------------
void FreeForAll::init()
{
    WorldWithRank::init();
    m_display_rank = false;
    m_count_down_reached_zero = false;
    m_use_highscores = false;
}   // init

// ----------------------------------------------------------------------------
/** Called when a battle is restarted.
 */
void FreeForAll::reset(bool restart)
{
    WorldWithRank::reset(restart);
    m_count_down_reached_zero = false;
    if (RaceManager::get()->hasTimeTarget())
    {
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN,
            RaceManager::get()->getTimeTarget());
    }
    else
    {
        WorldStatus::setClockMode(CLOCK_CHRONO);
    }
    m_scores.clear();
    m_scores.resize(m_karts.size(), 0);
}   // reset

// ----------------------------------------------------------------------------
/** Called when the match time ends.
 */
void FreeForAll::countdownReachedZero()
{
    // Prevent negative time in network soccer when finishing
    m_time_ticks = 0;
    m_time = 0.0f;
    m_count_down_reached_zero = true;
}   // countdownReachedZero

// ----------------------------------------------------------------------------
/** Called when a kart is hit.
 *  \param kart_id The world kart id of the kart that was hit.
 *  \param hitter The world kart id of the kart who hit(-1 if none).
 */
bool FreeForAll::kartHit(int kart_id, int hitter)
{
    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient())
        return false;

    if (isRaceOver())
        return false;

    handleScoreInServer(kart_id, hitter);
    return true;
}   // kartHit

// ----------------------------------------------------------------------------
/** Called when the score of kart needs updated.
 *  \param kart_id The world kart id of the kart that was hit.
 *  \param hitter The world kart id of the kart who hit(-1 if none).
 */
void FreeForAll::handleScoreInServer(int kart_id, int hitter)
{
    int new_score = 0;
    if (kart_id == hitter || hitter == -1)
        new_score = --m_scores[kart_id];
    else
        new_score = ++m_scores[hitter];

    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isServer())
    {
        NetworkString p(PROTOCOL_GAME_EVENTS);
        p.setSynchronous(true);
        p.addUInt8(GameEventsProtocol::GE_BATTLE_KART_SCORE);
        if (kart_id == hitter || hitter == -1)
            p.addUInt8((uint8_t)kart_id).addUInt16((int16_t)new_score);
        else
            p.addUInt8((uint8_t)hitter).addUInt16((int16_t)new_score);
        STKHost::get()->sendPacketToAllPeers(&p, true);
    }
}   // handleScoreInServer

// ----------------------------------------------------------------------------
void FreeForAll::setKartScoreFromServer(NetworkString& ns)
{
    int kart_id = ns.getUInt8();
    int16_t score = ns.getUInt16();
    m_scores.at(kart_id) = score;
}   // setKartScoreFromServer

// ----------------------------------------------------------------------------
/** Returns the internal identifier for this race.
 */
const std::string& FreeForAll::getIdent() const
{
    return IDENT_FFA;
}   // getIdent

// ------------------------------------------------------------------------
void FreeForAll::update(int ticks)
{
    WorldWithRank::update(ticks);
    WorldWithRank::updateTrack(ticks);
    if (Track::getCurrentTrack()->hasNavMesh())
        updateSectorForKarts();

    std::vector<std::pair<int, int> > ranks;
    for (unsigned i = 0; i < m_scores.size(); i++)
    {
        // For eliminated (disconnected or reserved player) make his score
        // int min so always last in rank
        int cur_score = getKart(i)->isEliminated() ?
            std::numeric_limits<int>::min() : m_scores[i];
        ranks.emplace_back(i, cur_score);
    }
    std::sort(ranks.begin(), ranks.end(),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b)
        {
            return a.second > b.second;
        });
    beginSetKartPositions();
    for (unsigned i = 0; i < ranks.size(); i++)
        setKartPosition(ranks[i].first, i + 1);
    endSetKartPositions();
}   // update

// ----------------------------------------------------------------------------
/** The battle is over if only one kart is left, or no player kart.
 */
bool FreeForAll::isRaceOver()
{
    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient())
        return false;

    if (!getKartAtPosition(1))
        return false;

    const int top_id = getKartAtPosition(1)->getWorldKartId();
    const int hit_capture_limit = RaceManager::get()->getHitCaptureLimit();

    return (m_count_down_reached_zero && RaceManager::get()->hasTimeTarget()) ||
        (hit_capture_limit != 0 && m_scores[top_id] >= hit_capture_limit);
}   // isRaceOver

// ----------------------------------------------------------------------------
/** Returns the data to display in the race gui.
 */
void FreeForAll::getKartsDisplayInfo(
                           std::vector<RaceGUIBase::KartIconDisplayInfo> *info)
{
    const unsigned int kart_amount = getNumKarts();
    for (unsigned int i = 0; i < kart_amount ; i++)
    {
        RaceGUIBase::KartIconDisplayInfo& rank_info = (*info)[i];
        rank_info.lap = -1;
        rank_info.m_outlined_font = true;
        rank_info.m_color = getColor(i);
        rank_info.m_text = getKart(i)->getController()->getName();
        if (RaceManager::get()->getKartGlobalPlayerId(i) > -1)
        {
            const core::stringw& flag = StringUtils::getCountryFlag(
                RaceManager::get()->getKartInfo(i).getCountryCode());
            if (!flag.empty())
            {
                rank_info.m_text += L" ";
                rank_info.m_text += flag;
            }
        }
        rank_info.m_text += core::stringw(L" (") +
            StringUtils::toWString(m_scores[i]) + L")";
    }
}   // getKartsDisplayInfo

// ----------------------------------------------------------------------------
void FreeForAll::terminateRace()
{
    const unsigned int kart_amount = getNumKarts();
    for (unsigned int i = 0; i < kart_amount ; i++)
    {
        getKart(i)->finishedRace(0.0f, true/*from_server*/);
    }   // i<kart_amount
    WorldWithRank::terminateRace();
}   // terminateRace

// ----------------------------------------------------------------------------
video::SColor FreeForAll::getColor(unsigned int kart_id) const
{
    return GUIEngine::getSkin()->getColor("font::normal");
}   // getColor

// ----------------------------------------------------------------------------
bool FreeForAll::getKartFFAResult(int kart_id) const
{
    // the kart(s) which has the top score wins
    AbstractKart* k = getKartAtPosition(1);
    if (!k)
        return false;
    int top_score = getKartScore(k->getWorldKartId());
    return getKartScore(kart_id) == top_score;
}   // getKartFFAResult

// ----------------------------------------------------------------------------
void FreeForAll::saveCompleteState(BareNetworkString* bns, STKPeer* peer)
{
    for (unsigned i = 0; i < m_scores.size(); i++)
        bns->addUInt32(m_scores[i]);
}   // saveCompleteState

// ----------------------------------------------------------------------------
void FreeForAll::restoreCompleteState(const BareNetworkString& b)
{
    for (unsigned i = 0; i < m_scores.size(); i++)
        m_scores[i] = b.getUInt32();
}   // restoreCompleteState

// ----------------------------------------------------------------------------
std::pair<uint32_t, uint32_t> FreeForAll::getGameStartedProgress() const
{
    std::pair<uint32_t, uint32_t> progress(
        std::numeric_limits<uint32_t>::max(),
        std::numeric_limits<uint32_t>::max());
    if (RaceManager::get()->hasTimeTarget())
    {
        progress.first = (uint32_t)m_time;
    }
    AbstractKart* k = getKartAtPosition(1);
    float score = -1.0f;
    if (k)
        score = (float)getKartScore(k->getWorldKartId());

    if (score >= 0.0f)
    {
        progress.second = (uint32_t)(score /
            (float)RaceManager::get()->getHitCaptureLimit() * 100.0f);
    }
    return progress;
}   // getGameStartedProgress
