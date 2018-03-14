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
#include "network/race_config.hpp"
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
    m_players.clear();
    delete m_race_config;
}   // ~GameSetup

//-----------------------------------------------------------------------------
void GameSetup::addPlayer(std::shared_ptr<NetworkPlayerProfile> profile)
{
    m_players.push_back(profile);
    Log::info("GameSetup", "New player in the game setup. Player name : %s.",
        StringUtils::wideToUtf8(profile->getName()).c_str());
}   // addPlayer

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

void GameSetup::bindKartsToProfiles()
{
    World::KartList karts = World::getWorld()->getKarts();

    /*for (unsigned int i = 0; i < m_players.size(); i++)
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
    }*/
}   // bindKartsToProfiles
