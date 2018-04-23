//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

#include "network/game_setup.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/stk_host.hpp"
#include "race/race_manager.hpp"
#include "utils/log.hpp"

#include <algorithm>

//-----------------------------------------------------------------------------
/** Update and see if any player disconnects.
 *  \param remove_disconnected_players remove the disconnected players,
 *  otherwise eliminate the kart in world, so this function must be called
 *  in main thread.
 */
void GameSetup::update(bool remove_disconnected_players)
{
    std::unique_lock<std::mutex> lock(m_players_mutex);
    if (remove_disconnected_players)
    {
        m_players.erase(std::remove_if(m_players.begin(), m_players.end(), []
            (const std::weak_ptr<NetworkPlayerProfile> npp)->bool
            {
                return npp.expired();
            }), m_players.end());
        return;
    }
    lock.unlock();
    if (!World::getWorld() ||
        World::getWorld()->getPhase() < WorldStatus::MUSIC_PHASE)
        return;
    for (uint8_t i = 0; i < (uint8_t)m_players.size(); i++)
    {
        if (!m_players[i].expired())
            continue;
        AbstractKart* k = World::getWorld()->getKart(i);
        if (!k->isEliminated())
        {
            World::getWorld()->eliminateKart(i,
                false/*notify_of_elimination*/);
            k->setPosition(
                World::getWorld()->getCurrentNumKarts() + 1);
            k->finishedRace(World::getWorld()->getTime());
            NetworkString p(PROTOCOL_GAME_EVENTS);
            p.setSynchronous(true);
            p.addUInt8(GameEventsProtocol::GE_PLAYER_DISCONNECT).addUInt8(i);
            STKHost::get()->sendPacketToAllPeers(&p, true);
        }
    }
}   // removePlayer

//-----------------------------------------------------------------------------
void GameSetup::loadWorld()
{
    assert(!m_tracks.empty());
    // Disable accidentally unlocking of a challenge
    if (PlayerManager::getCurrentPlayer())
        PlayerManager::getCurrentPlayer()->setCurrentChallenge("");
    race_manager->setTimeTarget(0.0f);
    race_manager->setReverseTrack(m_reverse);
    if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_SOCCER)
    {
        if (isSoccerGoalTarget())
            race_manager->setMaxGoal(m_laps);
        else
            race_manager->setTimeTarget((float)m_laps * 60.0f);
    }
    else
    {
        race_manager->startSingleRace(m_tracks.back(), m_laps,
            false/*from_overworld*/);
    }
}   // loadWorld

//-----------------------------------------------------------------------------
bool GameSetup::isGrandPrix() const
{
    return m_extra_server_info != -1 &&
        NetworkConfig::get()->getLocalGameMode().second ==
        RaceManager::MAJOR_MODE_GRAND_PRIX;
}   // isGrandPrix

//-----------------------------------------------------------------------------
void GameSetup::addServerInfo(NetworkString* ns)
{
    assert(NetworkConfig::get()->isServer());
    ns->encodeString(NetworkConfig::get()->getServerName());
    ns->addUInt8(race_manager->getDifficulty())
        .addUInt8((uint8_t)NetworkConfig::get()->getMaxPlayers())
        .addUInt8((uint8_t)NetworkConfig::get()->getServerMode());
    if (hasExtraSeverInfo())
    {
        if (isGrandPrix())
        {
            uint8_t cur_track = (uint8_t)m_tracks.size();
            if (!isGrandPrixStarted())
                cur_track = 0;
            ns->addUInt8((uint8_t)2).addUInt8(cur_track)
                .addUInt8(getExtraServerInfo());
        }
        else
        {
            // Soccer mode
            ns->addUInt8((uint8_t)1).addUInt8(getExtraServerInfo());
        }
    }
    else
    {
        // No extra server info
        ns->addUInt8((uint8_t)0);
    }
    ns->encodeString(NetworkConfig::get()->getMOTD());
}   // addServerInfo

//-----------------------------------------------------------------------------
void GameSetup::sortPlayersForGrandPrix()
{
    if (!isGrandPrix() || m_tracks.size() == 1)
        return;
    std::lock_guard<std::mutex> lock(m_players_mutex);
    std::sort(m_players.begin(), m_players.end(),
        [](const std::weak_ptr<NetworkPlayerProfile>& a,
        const std::weak_ptr<NetworkPlayerProfile>& b)
        {
            // They should never expired
            auto c = a.lock();
            assert(c);
            auto d = b.lock();
            assert(d);
            return (c->getScore() < d->getScore()) ||
                (c->getScore() == d->getScore() &&
                c->getOverallTime() > d->getOverallTime());
        });
    if (UserConfigParams::m_gp_most_points_first)
    {
        std::reverse(m_players.begin(), m_players.end());
    }
}   // sortPlayersForGrandPrix
