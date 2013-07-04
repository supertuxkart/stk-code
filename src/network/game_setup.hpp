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

/*! \file game_setup.hpp
 */

#ifndef GAME_SETUP_HPP
#define GAME_SETUP_HPP

#include "online/online_user.hpp"

#include <vector>
#include <string>

/*! \class PlayerProfile
 *  \brief Contains the profile of a player. 
 */
class PlayerProfile 
{
    public:
        PlayerProfile() : user_profile("") {}
        ~PlayerProfile() {}
        
        uint8_t race_id; //!< The id of the player for the race
        std::string kart_name; //!< The selected kart.
        OnlineUser user_profile; //!< Pointer to the lobby profile
};

/*! \class GameSetup
 *  \brief Used to store the needed data about the players that join a game.
 *  This class stores all the possible information about players in a lobby.
 */
class GameSetup
{
    public:
        GameSetup();
        virtual ~GameSetup();
        
        void addPlayer(PlayerProfile profile); //!< Add a player.
        void removePlayer(uint32_t id); //!< Remove a player by id.
        void removePlayer(uint8_t id); //!< Remove a player by local id.
        
        const PlayerProfile* getProfile(uint32_t id); //!< Get a profile by database id
        const PlayerProfile* getProfile(uint8_t id); //!< Get the profile by the lobby id
        
    protected:
        std::vector<PlayerProfile> m_players; //!< Information about players
        PlayerProfile m_self_profile; //!< Information about self
};

#endif // GAME_SETUP_HPP
