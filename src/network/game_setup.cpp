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

#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "network/network_player_profile.hpp"
#include "online/online_profile.hpp"
#include "race/race_manager.hpp"
#include "utils/log.hpp"

//-----------------------------------------------------------------------------

GameSetup::GameSetup()
{
    m_race_config       = new RaceConfig();
    m_num_local_players = 0;
    m_local_master      = 0;
}   // GameSetup

//-----------------------------------------------------------------------------

GameSetup::~GameSetup()
{
    // remove all players
    for (unsigned int i = 0; i < m_players.size(); i++)
    {
        delete m_players[i];
    };
    m_players.clear();
    delete m_race_config;
}   // ~GameSetup

//-----------------------------------------------------------------------------

void GameSetup::addPlayer(NetworkPlayerProfile* profile)
{
    m_players.push_back(profile);
    Log::info("GameSetup", "New player in the game setup. Player id : %d.",
              profile->getGlobalPlayerId());
}   // addPlayer

//-----------------------------------------------------------------------------
/** Removed a player give his NetworkPlayerProfile.
 *  \param profile The NetworkPlayerProfile to remove.
 *  \return True if the player was found and removed, false otherwise.
 */
bool GameSetup::removePlayer(const NetworkPlayerProfile *profile)
{
    for (unsigned int i = 0; i < m_players.size(); i++)
    {
        if (m_players[i] == profile)
        {
            delete m_players[i];
            m_players.erase(m_players.begin()+i, m_players.begin()+i+1);
            Log::verbose("GameSetup",
                         "Removed a player from the game setup. Remains %u.",
                          m_players.size());
            return true;
        }
    }
    return false;
}   // removePlayer

//-----------------------------------------------------------------------------
/** Sets the player id of the local master.
 *  \param player_id The id of the player who is the local master.
 */
void GameSetup::setLocalMaster(uint8_t player_id)
{
    m_local_master = player_id;
}   // setLocalMaster

//-----------------------------------------------------------------------------
/** Returns true if the player id is the local game master (used in the
 *  network game selection.
 *  \param Local player id to test.
 */
bool GameSetup::isLocalMaster(uint8_t player_id)
{
    return m_local_master == player_id;
}   // isLocalMaster

//-----------------------------------------------------------------------------
/** Sets the kart the specified player uses.
 *  \param player_id  ID of this player (in this race).
 *  \param kart_name Name of the kart the player picked.
 */
void GameSetup::setPlayerKart(uint8_t player_id, const std::string &kart_name)
{
    bool found = false;
    for (unsigned int i = 0; i < m_players.size(); i++)
    {
        if (m_players[i]->getGlobalPlayerId() == player_id)
        {
            m_players[i]->setKartName(kart_name);
            Log::info("GameSetup::setPlayerKart", "Player %d took kart %s",
                      player_id, kart_name.c_str());
            found = true;
        }
    }
    if (!found)
    {
        Log::info("GameSetup::setPlayerKart", "The player %d was unknown.",
                  player_id);
    }
}   // setPlayerKart

//-----------------------------------------------------------------------------

void GameSetup::bindKartsToProfiles()
{
    World::KartList karts = World::getWorld()->getKarts();

    for (unsigned int i = 0; i < m_players.size(); i++)
    {
        Log::info("GameSetup", "Player %d has id %d and kart %s", i,
                  m_players[i]->getGlobalPlayerId(),
                  m_players[i]->getKartName().c_str());
    }
    for (unsigned int i = 0; i < karts.size(); i++)
    {
        Log::info("GameSetup", "Kart %d has id %d and kart %s", i,
                   karts[i]->getWorldKartId(), karts[i]->getIdent().c_str());
    }
    for (unsigned int j = 0; j < m_players.size(); j++)
    {
        bool found = false;
        for (unsigned int i = 0 ; i < karts.size(); i++)
        {
            if (karts[i]->getIdent() == m_players[j]->getKartName())
            {
                m_players[j]->setWorldKartID(karts[i]->getWorldKartId());
                found = true;
                break;
            }
        }
        if (!found)
        {
            Log::error("GameSetup", "Error while binding world kart ids to players profiles.");
        }
    }
}   // bindKartsToProfiles

//-----------------------------------------------------------------------------
/** \brief Get a network player profile with the specified player id.
 *  \param player_id : Player id in this race.
 *  \return The profile of the player having the specified player id, or
 *          NULL if no such player exists.
 */
const NetworkPlayerProfile* GameSetup::getProfile(uint8_t player_id)
{
    for (unsigned int i = 0; i < m_players.size(); i++)
    {
        if (m_players[i]->getGlobalPlayerId()== player_id)
        {
            return m_players[i];
        }
    }
    return NULL;
}   // getProfile

//-----------------------------------------------------------------------------
/** \brief Get a network player profile matching a kart name.
 *  \param kart_name : Name of the kart used by the player.
 *  \return The profile of the player having the kart kart_name, or NULL
 *           if no such network profile exists.
 */

const NetworkPlayerProfile* GameSetup::getProfile(const std::string &kart_name)
{
    for (unsigned int i = 0; i < m_players.size(); i++)
    {
        if (m_players[i]->getKartName() == kart_name)
        {
            return m_players[i];
        }
    }
    return NULL;
}   // getProfile(kart_name)

//-----------------------------------------------------------------------------
/** Returns the list of all player profiles from a specified host. Note that
 *  this function is somewhat expensive (it loops over all network profiles
 *  to find the ones with the specified host id).
 *  \param host_id The host id which players should be collected.
 *  \return List of NetworkPlayerProfile pointers/
 */
std::vector<NetworkPlayerProfile*> GameSetup::getAllPlayersOnHost(uint8_t host_id)
{
    std::vector<NetworkPlayerProfile*> result;

    for (unsigned int i = 0; i < m_players.size(); i++)
    {
        if (m_players[i]->getHostId() == host_id)
            result.push_back(m_players[i]);
    }
    return result;
}   // getAllPlayersOnHost

//-----------------------------------------------------------------------------

bool GameSetup::isKartAvailable(std::string kart_name)
{
    for (unsigned int i = 0; i < m_players.size(); i++)
    {
        if (m_players[i]->getKartName() == kart_name)
            return false;
    }
    return true;
}
