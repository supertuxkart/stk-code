//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 SuperTuxKart-Team
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

#include "utils/log.hpp"

//-----------------------------------------------------------------------------

GameSetup::GameSetup()
{
}

//-----------------------------------------------------------------------------

GameSetup::~GameSetup()
{
}

//-----------------------------------------------------------------------------

void GameSetup::addPlayer(NetworkPlayerProfile profile)
{
    m_players.push_back(profile);
}

//-----------------------------------------------------------------------------
 
bool GameSetup::removePlayer(uint32_t id)
{
    for (unsigned int i = 0; i < m_players.size(); i++)
    {
        if (m_players[i].user_profile->getUserID() == id)
        {
            m_players.erase(m_players.begin()+i, m_players.begin()+i+1);
            Log::verbose("GameSetup", "Removed a player from the game setup.");
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
 
bool GameSetup::removePlayer(uint8_t id)
{
    for (unsigned int i = 0; i < m_players.size(); i++)
    {
        if (m_players[i].race_id == id) // check the given id
        {
            m_players.erase(m_players.begin()+i, m_players.begin()+i+1);
            Log::verbose("GameSetup", "Removed a player from the game setup.");
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
        
const NetworkPlayerProfile* GameSetup::getProfile(uint32_t id)
{
    for (unsigned int i = 0; i < m_players.size(); i++)
    {
        if (m_players[i].user_profile->getUserID() == id)
        {
            return &m_players[i];
        }
    }
    return NULL;
}

//-----------------------------------------------------------------------------
 
const NetworkPlayerProfile* GameSetup::getProfile(uint8_t id)
{
    for (unsigned int i = 0; i < m_players.size(); i++)
    {
        if (m_players[i].race_id == id)
        {
            return &m_players[i];
        }
    }
    return NULL;
}

//-----------------------------------------------------------------------------

bool GameSetup::isKartAvailable(std::string kart_name)
{
    for (unsigned int i = 0; i < m_players.size(); i++)
    {
        if (m_players[i].kart_name == kart_name)
            return false;
    }
    return true;
}
