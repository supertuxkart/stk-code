//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Joerg Henrichs
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

namespace Online
{
    class CurrentUser;
    class HTTPRequest;
    class OnlineProfile;
    class XMLRequest;
}
class PlayerProfile;

/** A special class that manages all local player accounts. It reads all player
 *  accounts from the players.xml file in the user config directory. For each
 *  player an instance of PlayerProfile is created, which keeps track of
 *  story mode progress, achievements and other data. It also keeps track of
 *  the currently logged in player.
 *  It includes several handy static functions which avoid long call
 *  sequences, e.g.:
 *    PlayerManager::getCurrentOnlineId()
 *  which is just:
 *    PlayerManager::get()->getCurrentUser()->getID();
 */
class PlayerManager : public NoCopy
{
private:
    static PlayerManager* m_player_manager;

    PtrVector<PlayerProfile> m_all_players;

    /** A pointer to the current player. */
    PlayerProfile* m_current_player;

    /** Saves the XML tree from players.xml for use in the 2nd
     * loading stage (initRemainingData). */
    const XMLNode *m_player_data;

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

    void save();
    void initRemainingData();
    unsigned int getUniqueId() const;
    void addDefaultPlayer();
    PlayerProfile* addNewPlayer(const irr::core::stringw& name);
    void createGuestPlayers(int n);
    void deletePlayer(PlayerProfile *player);
    void setCurrentPlayer(PlayerProfile *player);
    const PlayerProfile *getPlayerById(unsigned int id);
    void enforceCurrentPlayer();
    unsigned int getNumNonGuestPlayers() const;
    static void setUserDetails(Online::HTTPRequest *request,
                               const std::string &action,
                               const std::string &php_name = "");
    static unsigned int getCurrentOnlineId();
    static bool isCurrentLoggedIn();
    static Online::OnlineProfile* getCurrentOnlineProfile();

    static PlayerProfile::OnlineState getCurrentOnlineState();
    static const irr::core::stringw& getCurrentOnlineUserName();
    static void requestOnlinePoll();
    static void resumeSavedSession();
    static void onSTKQuit();
    static void requestSignOut();
    static Online::XMLRequest *requestSignIn(const irr::core::stringw &username,
                                             const irr::core::stringw &password);

    // ------------------------------------------------------------------------
    /** Returns the current player. */
    static PlayerProfile* getCurrentPlayer()
    {
        return get()->m_current_player;
    }   // getCurrentPlayer

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
        return PlayerManager::getCurrentPlayer()->getAchievementsStatus();
    }   // getCurrentAchievementsStatus
    // ------------------------------------------------------------------------
    /** A handy shortcut to increase points for an achievement key of the
     *  current player.
     *  \param achievement_id The achievement id.
     *  \param key The key of the current value to increase.
     *  \param increase How much to increase the current value.
     *  \param goal_key Optional: The goal key to compare the current value
     *         with. If not set, defaults to key.
     */
    static void increaseAchievement(unsigned int achievement_id,
                                    const std::string &key,
                                    int increase = 1, 
                                    const std::string &goal_key="")
    {
        Achievement *a = getCurrentAchievementsStatus()
                       ->getAchievement(achievement_id);
        if (!a)
        {
            Log::fatal("PlayerManager", "Achievement '%d' not found.",
                        achievement_id);
        }
        a->increase(key, goal_key.empty() ? key : goal_key, increase);

    }   // increaseAchievement
    // ------------------------------------------------------------------------
};   // PlayerManager
#endif
