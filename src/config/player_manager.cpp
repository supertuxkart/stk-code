//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2014 Joerg Henrichs
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

#include "config/player_manager.hpp"

#include "achievements/achievements_manager.hpp"
#include "config/player_profile.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "io/utf_writer.hpp"
#include "io/xml_node.hpp"
#include "online/online_player_profile.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

PlayerManager *PlayerManager::m_player_manager = NULL;


/** Create the instance of the player manager.
 *  Also make sure that at least one player is defined (and if not,
 *  create a default player and a guest player).
 */
void PlayerManager::create()
{
    assert(!m_player_manager);
    m_player_manager = new PlayerManager();
}   // create

// ----------------------------------------------------------------------------
/** Adds the login credential to a http request. It sets the name of
 *  the script to invokce, token, and user id.
 *  \param request The http request.
 *  \param action If not empty, the action to be set.
 */
void PlayerManager::setUserDetails(Online::HTTPRequest *request,
    const std::string &action,
    const std::string &php_name)
{
    get()->getCurrentPlayer()->setUserDetails(request, action, php_name);
}   // setUserDetails

// ----------------------------------------------------------------------------
/** Returns whether a user is signed in or not. */
bool PlayerManager::isCurrentLoggedIn()
{
    return getCurrentPlayer()->isLoggedIn();
}   // isCurrentLoggedIn

// ----------------------------------------------------------------------------
/** Returns the online id of the current player.
*  \pre User logged in (which is asserted in getID()).
*/
unsigned int PlayerManager::getCurrentOnlineId()
{
    return getCurrentPlayer()->getOnlineId();
}   // getCurrentOnlineId

// ----------------------------------------------------------------------------
/** Returns the online state of the current player. It can be logged out,
 *  logging in, logged in, logging out, logged out, or guest.
 */
PlayerProfile::OnlineState PlayerManager::getCurrentOnlineState()
{
    return getCurrentPlayer()->getOnlineState();
}   // getCurrentOnlineState

// ----------------------------------------------------------------------------
/** Returns the online name of this player.
 */
const irr::core::stringw& PlayerManager::getCurrentOnlineUserName()
{
    if (getCurrentOnlineState() == PlayerProfile::OS_SIGNED_IN ||
        getCurrentOnlineState() == PlayerProfile::OS_GUEST         )
        return getCurrentOnlineProfile()->getUserName();

    static core::stringw not_signed_in = _("Currently not signed in");
    return not_signed_in;
}   // getCurrentOnlineUserName

// ----------------------------------------------------------------------------
/** Sends a request to the server to see if any new information is
 *  available. (online friends, notifications, etc.).
 */
void PlayerManager::requestOnlinePoll()
{
    getCurrentPlayer()->requestPoll();
}   // requestOnlinePoll

// ----------------------------------------------------------------------------
/** Reconnect to the server using the saved session data.
 */
void PlayerManager::resumeSavedSession()
{
    getCurrentPlayer()->requestSavedSession();
}   // resumeSavedSession

// ----------------------------------------------------------------------------
/** Sends a message to the server that the client has been closed, if a
 *  user is signed in.
 */
void PlayerManager::onSTKQuit()
{
    if (getCurrentPlayer() && getCurrentPlayer()->isLoggedIn())
        getCurrentPlayer()->requestSignOut();
}   // onSTKQuit

// ----------------------------------------------------------------------------
/** Create a signin request.
 *  \param username Name of user.
 *  \param password Password.
 *  \param save_session If true, the login credential will be saved to
 *         allow a password-less login.
 *  \param request_now Immediately submit this request to the
 *         RequestManager.
 */

Online::XMLRequest *PlayerManager::requestSignIn(const irr::core::stringw &username,
                                                 const irr::core::stringw &password)
{
    return getCurrentPlayer()->requestSignIn(username, password);
}   // requestSignIn

// ----------------------------------------------------------------------------
/** Signs the current user out.
 */
void PlayerManager::requestSignOut()
{
    getCurrentPlayer()->requestSignOut();
}   // requestSignOut

// ----------------------------------------------------------------------------
/** Returns the current online profile (which is the list of all achievements
 *  and friends).
 */
Online::OnlineProfile* PlayerManager::getCurrentOnlineProfile()
{
    return getCurrentPlayer()->getProfile();
}   // getCurrentOnlineProfile

// ============================================================================
/** Constructor.
 */
PlayerManager::PlayerManager()
{
    m_current_player = NULL;
    load();
}   // PlayerManager

// ----------------------------------------------------------------------------
/** Destructor.
 */
PlayerManager::~PlayerManager()
{
    // If the passwords should not be remembered, clear the saved session.
    for_var_in(PlayerProfile*, player, m_all_players)
    {
        if(!player->rememberPassword())
        {
            // Don't let the player trigger a save, since it
            // will be done below anyway.
            player->clearSession(/*save*/false);
        }
    }
    save();

}   // ~PlayerManager

// ----------------------------------------------------------------------------
/** Manages the loading of saved player data from the players.xml file.
 *  This function create the XML tree from the file, and then creates the
 *  instances of PlayerProfile which read each node.
 */
void PlayerManager::load()
{
    std::string filename = file_manager->getUserConfigFile("players.xml");

    m_player_data = file_manager->createXMLTree(filename);
    if(!m_player_data)
    {
        Log::info("player_manager", "A new players.xml file will be created.");
        return;
    }
    else if(m_player_data->getName()!="players")
    {
        Log::info("player_manager", "The players.xml file is invalid.");
        return;
    }

    std::vector<XMLNode*> player_list;
    m_player_data->getNodes("player", player_list);
    for(unsigned int i=0; i<player_list.size(); i++)
    {
        const XMLNode *player_xml = player_list[i];
        PlayerProfile *player = new Online::OnlinePlayerProfile(player_xml);
        m_all_players.push_back(player);
    }
    m_current_player = NULL;
    const XMLNode *current = m_player_data->getNode("current");
    if(current)
    {
        stringw name;
        current->get("player", &name);
        m_current_player = getPlayer(name);
    }

}   // load

// ----------------------------------------------------------------------------
/** The 2nd loading stage. During this stage achievements and story mode
 *  data is initialised for each player. In case of existing player (i.e. not
 *  first time start of stk) the data is read from the players.xml file,
 *  in case of a first time start new/empty data structures for the players
 *  (which were created by default) are created.
 */
void PlayerManager::initRemainingData()
{
    // Filter the player nodes out (there is one additional node 'online-ids'
    // which makes this necessary), otherwise the index of m_all_players
    // is not identical with the index in the xml file.
    std::vector<XMLNode*> player_nodes;
    if(m_player_data)
        m_player_data->getNodes("player", player_nodes);
    for (unsigned int i = 0; i<m_all_players.size(); i++)
    {
        // On the first time STK is run, there is no player data,
        // so just initialise the story and achievement data
        // structures
        if (!m_player_data)
            m_all_players[i].initRemainingData();
        else   // not a first time start, load remaining data
            m_all_players[i].loadRemainingData(player_nodes[i]);
    }

    delete m_player_data;
    m_player_data = NULL;

    // Sort player by frequency
    m_all_players.insertionSort(/*start*/0, /*desc*/true);
}   // initRemainingData

// ----------------------------------------------------------------------------
/** Saves all player profiles to players.xml.
 */
void PlayerManager::save()
{
    std::string filename = file_manager->getUserConfigFile("players.xml");
    try
    {
        UTFWriter players_file(filename.c_str());

        players_file << L"<?xml version=\"1.0\"?>\n";
        players_file << L"<players version=\"1\" >\n";

        if(m_current_player)
        {
            players_file << L"    <current player=\""
                         << m_current_player->getName() << L"\"/>\n";
        }

        // Save all non-guest players
        for_var_in(PlayerProfile*, player, m_all_players)
        {
            if(!player->isGuestAccount())
                player->save(players_file);
        }
        players_file << L"</players>\n";
        players_file.close();
    }
    catch (std::runtime_error& e)
    {
        Log::error("PlayerManager", "Failed to write config to %s.",
                    filename.c_str());
        Log::error("PlayerManager", "Error: %s", e.what());
    }

}   // save

// ----------------------------------------------------------------------------
/** Adds a new player to the list of all players.
 *  \param name Name of the new player.
 */
PlayerProfile* PlayerManager::addNewPlayer(const core::stringw& name)
{
    PlayerProfile *profile = new Online::OnlinePlayerProfile(name);
    m_all_players.push_back(profile);
    return profile;
}   // addNewPlayer

// ----------------------------------------------------------------------------
/** Deletes a player profile from the list of all profiles.
 */
void PlayerManager::deletePlayer(PlayerProfile *player)
{
    m_all_players.erase(player);
    if(player==m_current_player)
        m_current_player = NULL;
}   // deletePlayer

// ----------------------------------------------------------------------------
/** This function makes sure that a current player is defined. This is called
 *  when a screen skipping command line option is given (-N, -R, ...), in
 *  which case there might not be a current player (if no default player is
 *  set in players.xml, i.e. the 'remember be' option was not picked ). Since
 *  a lot of code depends on having a local player, just set the most
 *   frequently used non-guest to be the current player.
 */
void PlayerManager::enforceCurrentPlayer()
{
    if (m_current_player) return;
    
    PlayerProfile* player;
    for_in(player, m_all_players)
    {
        if (!player->isGuestAccount())
        {
            Log::info("PlayerManager", "Enfocring current player '%ls'.",
                      player->getName().c_str()           );
            m_current_player = player;
            return;
        }
    }   // for player in m_all_players

    // This shouldn't happen - but just in case: add the default players
    // again, and search again for a non-guest player.
    addDefaultPlayer();
    for_in(player, m_all_players)
    {
        if (!player->isGuestAccount())
        {
            Log::info("PlayerManager", "Enfocring current player '%s'.",
                       player->getName().c_str());
            m_current_player = player;
            return;
        }
    }   // for player in m_all_players

    // Now this really really should not happen.
    Log::fatal("PlayerManager", "Failed to find a non-guest player.");
}   // enforceCurrentPlayer

// ----------------------------------------------------------------------------
/** Called when no player profiles exists. It creates two players: one
 *  guest player, and one non-guest player for whic hit tries to guess a
 *  mame based on environment variables.
 */
void PlayerManager::addDefaultPlayer()
{
    std::string username = "unnamed player";

    if(getenv("USERNAME")!=NULL)        // for windows
        username = getenv("USERNAME");
    else if(getenv("USER")!=NULL)       // Linux, Macs
        username = getenv("USER");
    else if(getenv("LOGNAME")!=NULL)    // Linux, Macs
        username = getenv("LOGNAME");

    // Set the name as the default name, but don't mark it as 'default'
    // yet, since not having a default player forces the player selection
    // screen to be shown.
    m_all_players.push_back(new Online::OnlinePlayerProfile(username.c_str()) );

}   // addDefaultPlayer

// ----------------------------------------------------------------------------
/** Makes sure at least n guest players exist. This is used by the multiplayer
 *  KartSelection screen to make sure enough guest players can be picked by
 *  the players.
 *  \param n Minimum number of guest players that must exist.
 */
void PlayerManager::createGuestPlayers(int n)
{
    int num_guests = m_all_players.size() - getNumNonGuestPlayers();
    for(int i=num_guests; i<n; i++)
    {
        core::stringw guest_name;
        if(i==0)
        {
            // I18N: Name of first guest player (without number)
            guest_name = _LTR("Guest");
        }
        else
        {
            // I18N: Name of further guest players, with a 1, 2, ... attached
            guest_name = _LTR("Guest %d", i);
        }
        PlayerProfile *guest = new Online::OnlinePlayerProfile(guest_name,
                                                               /*guest*/ true);
        m_all_players.push_back(guest);
    }
}   // createGuestPlayers

// ----------------------------------------------------------------------------
/** Returns the number of 'real' (non-guest) players.
 */
unsigned int PlayerManager::getNumNonGuestPlayers() const
{
    unsigned int count=0;
    for_var_in(const PlayerProfile*, player, m_all_players)
    {
        if(!player->isGuestAccount()) count ++;
    }
    return count;
}   // getNumNonGuestPlayers

// ----------------------------------------------------------------------------
/** This returns a unique id. This is 1 + largest id used so far.
 */
unsigned int PlayerManager::getUniqueId() const
{
    unsigned int max_id=0;
    for_var_in(const PlayerProfile*, player, m_all_players)
    {
        if(player->getUniqueID()>max_id)
            max_id = player->getUniqueID();
    }
    return max_id+1;
}   // getUniqueId

// ----------------------------------------------------------------------------
/** Returns a PlayerProfile with a given id. It searches linearly through
 *  the list of all players.
 *  \returns The profile, or NULL if no such profile exists.
 *  \param id The id of the player to look for.
 */
const PlayerProfile *PlayerManager::getPlayerById(unsigned int id)
{
    for_var_in(const PlayerProfile*, player, m_all_players)
    {
        if(player->getUniqueID()==id)
            return player;
    }
    return NULL;
}   // getPlayerById

// ----------------------------------------------------------------------------
/** Returns a player with a given name.
 *  \return The player profile or NULL if the name was not found.
 *  \param name The name to search for.
 */
PlayerProfile *PlayerManager::getPlayer(const irr::core::stringw &name)
{
    for_var_in(PlayerProfile*, player, m_all_players)
    {
        if(player->getName()==name)
            return player;
    }
    return NULL;
}   // getPlayer
// ----------------------------------------------------------------------------
/** Sets the current player. This is the player that is used for story mode
 *  and achievements.
 *  \param Player profile to be the current player.
 */
void PlayerManager::setCurrentPlayer(PlayerProfile *player)
{
    m_current_player = player;
    if(m_current_player)
    {
        m_current_player->computeActive();
    }
}   // setCurrentPlayer

