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
#include "network/network_player_profile.hpp"
#include "online/online_profile.hpp"
#include "race/race_manager.hpp"
#include "utils/log.hpp"

//-----------------------------------------------------------------------------
GameSetup::GameSetup()
{
    m_num_local_players = 0;
    m_local_master      = 0;
    m_laps              = 0;
    m_reverse           = false;
}   // GameSetup

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
    assert(!m_track.empty());
    // Disable accidentally unlocking of a challenge
    PlayerManager::getCurrentPlayer()->setCurrentChallenge("");
    race_manager->setReverseTrack(m_reverse);
    race_manager->startSingleRace(m_track, m_laps, false/*from_overworld*/);
}   // loadWorld
