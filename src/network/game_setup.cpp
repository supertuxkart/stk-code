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
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "online/online_profile.hpp"
#include "race/race_manager.hpp"
#include "utils/log.hpp"

//-----------------------------------------------------------------------------
/** Update and see if any player disconnects.
 *  \param remove_disconnected_players remove the disconnected players,
 *  otherwise replace with AI (when racing), so this function must be called
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
    if (!World::getWorld())
        return;
}   // removePlayer

//-----------------------------------------------------------------------------
void GameSetup::loadWorld()
{
    assert(!m_tracks.empty());
    // Disable accidentally unlocking of a challenge
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
void GameSetup::configClientAcceptConnection(NetworkString* ns)
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
            ns->addUInt8((uint8_t)2).addUInt8((uint8_t)m_tracks.size())
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
}   // configClientAcceptConnection
