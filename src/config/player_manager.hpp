//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2014 Joerg Henrichs
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

#ifndef HEADER_PLAYER_CONFIG_HPP
#define HEADER_PLAYER_CONFIG_HPP

#include "utils/no_copy.hpp"
#include "utils/ptr_vector.hpp"

#include <irrString.h>

#include <cstddef>  // NULL

class PlayerProfile;

/** A special class that manages all local player accounts.
 */
class PlayerManager : public NoCopy
{
private:
    static PlayerManager* m_player_manager;

    PtrVector<PlayerProfile> m_all_players;


     PlayerManager();
    ~PlayerManager();


public:
    /** Static singleton get function. */
    static PlayerManager* get()
    {
        if(!m_player_manager)
            m_player_manager = new PlayerManager();
        return m_player_manager;
    }   // get
    // ------------------------------------------------------------------------
    static void destroy()
    {
        delete m_player_manager;
        m_player_manager = NULL;
    }   // destroy
    // ------------------------------------------------------------------------
    
    void load();
    void save();
    unsigned int getUniqueId() const;
    void addDefaultPlayer();
    void addNewPlayer(const irr::core::stringw& name);
    void deletePlayer(PlayerProfile *player);
    const PlayerProfile *getPlayerById(unsigned int id);
    // ------------------------------------------------------------------------
    PlayerProfile *getPlayer(const irr::core::stringw &name);
    // ------------------------------------------------------------------------
    /** Returns the number of players in the config file.*/
    unsigned int getNumPlayers() const { return m_all_players.size(); }
    // ------------------------------------------------------------------------
    /** Returns a player with a given unique id. */
    const PlayerProfile *getPlayer(unsigned int n) const
    {
        return &m_all_players[n];
    }   // getPlayer
    // ------------------------------------------------------------------------
    /** Returns a player with a given unique id. */
    PlayerProfile *getPlayer(unsigned int n)  { return &m_all_players[n];}
    // ------------------------------------------------------------------------
};   // PlayerManager


#endif

/*EOF*/
