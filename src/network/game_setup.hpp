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

#include "network/race_config.hpp"

#include <vector>
#include <string>

namespace Online { class OnlineProfile; }


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
        Online::OnlineProfile* user_profile; //!< Pointer to the lobby profile
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
        void bindKartsToProfiles(); //!< Sets the right world_kart_id in profiles

        /** \brief Get the players that are in the game
         *  \return A vector containing pointers on the players profiles.
         */
        std::vector<NetworkPlayerProfile*> getPlayers() { return m_players; }
        int getPlayerCount() { return (int)m_players.size(); }
        /*! \brief Get a network player profile matching a universal id.
         *  \param id : Global id of the player (the one in the SQL database)
         *  \return The profile of the player matching the id.
         */
        const NetworkPlayerProfile* getProfile(uint32_t id);
        /*! \brief Get a network player profile matching a kart name.
         *  \param kart_name : Name of the kart used by the player.
         *  \return The profile of the player having the kart kart_name
         */
        const NetworkPlayerProfile* getProfile(uint8_t id);
        /*! \brief Get a network player profile matching a kart name.
         *  \param kart_name : Name of the kart used by the player.
         *  \return The profile of the player having the kart kart_name.
         */
        const NetworkPlayerProfile* getProfile(std::string kart_name);

        /*! \brief Used to know if a kart is available.
         *  \param kart_name : Name of the kart to check.
         *  \return True if the kart hasn't been selected yet, false elseway.
         */
        bool isKartAvailable(std::string kart_name);
        /*! \brief Used to know if a kart is playable.
         *  \param kart_name : Name of the kart to check.
         *  \return True if the kart is playable (standard kart).
         *  Currently this is always true as the kart selection screen shows
         *  only the standard karts.
         */
        bool isKartAllowed(std::string kart_name) {return true; }

        RaceConfig* getRaceConfig() { return m_race_config; }

    protected:
        std::vector<NetworkPlayerProfile*> m_players; //!< Information about players
        NetworkPlayerProfile m_self_profile; //!< Information about self (client only)
        RaceConfig* m_race_config;
};

#endif // GAME_SETUP_HPP
