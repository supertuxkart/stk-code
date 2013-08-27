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

#include "online/profile.hpp"

#include <vector>
#include <string>

/*! \class PlayerProfile
 *  \brief Contains the profile of a player.
 */
class NetworkPlayerProfile
{
    public:
        NetworkPlayerProfile() { race_id = 0; user_profile = NULL; }
        ~NetworkPlayerProfile() {}

        uint8_t race_id; //!< The id of the player for the race
        std::string kart_name; //!< The selected kart.
        Online::Profile* user_profile; //!< Pointer to the lobby profile
        uint8_t world_kart_id; //!< the kart id in the World class (pointer to AbstractKart)
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

        void addPlayer(NetworkPlayerProfile* profile); //!< Add a player.
        bool removePlayer(uint32_t id); //!< Remove a player by id.
        bool removePlayer(uint8_t id); //!< Remove a player by local id.
        void setPlayerKart(uint8_t id, std::string kart_name); //!< Set the kart of a player
        void bindKartsToProfiles();

        std::vector<NetworkPlayerProfile*> getPlayers() { return m_players; }
        int getPlayerCount() { return m_players.size(); }
        const NetworkPlayerProfile* getProfile(uint32_t id); //!< Get a profile by database id
        const NetworkPlayerProfile* getProfile(uint8_t id); //!< Get the profile by the lobby id
        const NetworkPlayerProfile* getProfile(std::string kart_name);

        bool isKartAvailable(std::string kart_name);
        bool isKartAllowed(std::string kart_name) {return true; }

    protected:
        std::vector<NetworkPlayerProfile*> m_players; //!< Information about players
        NetworkPlayerProfile m_self_profile; //!< Information about self (client only)
};

#endif // GAME_SETUP_HPP
