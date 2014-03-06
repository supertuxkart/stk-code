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

#ifndef HEADER_PLAYER_MANAGER_HPP
#define HEADER_PLAYER_MANAGER_HPP

#include "achievements/achievement.hpp"
#include "achievements/achievements_status.hpp"
#include "config/player_profile.hpp"
#include "utils/no_copy.hpp"
#include "utils/ptr_vector.hpp"

#include <irrString.h>

#include <cstddef>  // NULL

class AchievementsStatus;
class PlayerProfile;

/** A special class that manages all local player accounts. It reads all player
 *  accounts from the players.xml file in the user config directory. It also
 *  keeps track of the currently logged in player. For each player an instance
 *  of PlayerProfile is created, which keeps track of story mode progress,
 *  achievements and other data.
 */
class PlayerManager : public NoCopy
{
private:
    static PlayerManager* m_player_manager;

    PtrVector<PlayerProfile> m_all_players;

    /** A pointer to the current player. */
    PlayerProfile* m_current_player;

    void load();
     PlayerManager();
    ~PlayerManager();


public:
    static void create();
    // ------------------------------------------------------------------------
    /** Static singleton get function. */
    static PlayerManager* get()
    {
        assert(m_player_manager);
        return m_player_manager;
    }   // get
    // ------------------------------------------------------------------------
    static void destroy()
    {
        assert(m_player_manager);
        delete m_player_manager;
        m_player_manager = NULL;
    }   // destroy
    // ------------------------------------------------------------------------
    
    void save();
    unsigned int getUniqueId() const;
    void addDefaultPlayer();
    void addNewPlayer(const irr::core::stringw& name);
    void deletePlayer(PlayerProfile *player);
    void setCurrentPlayer(PlayerProfile *player, bool remember_me);
    const PlayerProfile *getPlayerById(unsigned int id);
    void enforceCurrentPlayer();
    // ------------------------------------------------------------------------
    /** Returns the current player. */
    PlayerProfile* getCurrentPlayer() { return m_current_player; }
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
    /** A handy shortcut funtion. */
    static AchievementsStatus* getCurrentAchievementsStatus()
    {
        return get()->getCurrentPlayer()->getAchievementsStatus();
    }   // getCurrentAchievementsStatus
    // ------------------------------------------------------------------------
    /** A handy shortcut to increase points for an achievement key of the
     *  current player. */
    static void increaseAchievement(unsigned int index, const std::string &key,
                                    int increase = 1)
    {
        Achievement *a = getCurrentAchievementsStatus()->getAchievement(index);
        if (!a)
        {
            Log::fatal("PlayerManager", "Achievement '%d' not found.", index);
        }
        a->increase(key, increase);
    }   // increaseAchievement
    // ------------------------------------------------------------------------
};   // PlayerManager


#endif

/*EOF*/
