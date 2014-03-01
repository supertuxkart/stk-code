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
#include "io/file_manager.hpp"
#include "io/utf_writer.hpp"
#include "io/xml_node.hpp"
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
    if(m_player_manager->getNumPlayers() == 0)
    {
        m_player_manager->addDefaultPlayer();
        m_player_manager->save();
    }

}   // create

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

    const XMLNode *players = file_manager->createXMLTree(filename);
    if(!players)
    {
        Log::info("player_manager", "A new players.xml file will be created.");
        return;
    }
    else if(players->getName()!="players")
    {
        Log::info("player_manager", "The players.xml file is invalid.");
        return;
    }

    m_current_player = NULL;
    for(unsigned int i=0; i<players->getNumNodes(); i++)
    {
        const XMLNode *player_xml = players->getNode(i);
        PlayerProfile *player = new PlayerProfile(player_xml);
        m_all_players.push_back(player);
        if(player->isDefault())
            m_current_player = player;
    }
    m_all_players.insertionSort(/*start*/0, /*desc*/true);

    delete players;
}   // load

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

        PlayerProfile *player;
        for_in(player, m_all_players)
        {
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
void PlayerManager::addNewPlayer(const core::stringw& name)
{
    m_all_players.push_back( new PlayerProfile(name) );
}   // addNewPlayer

// ----------------------------------------------------------------------------
/** Deletes a player profile from the list of all profiles. 
 */
void PlayerManager::deletePlayer(PlayerProfile *player)
{
    m_all_players.erase(player);
}   // deletePlayer

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

    // Set the name as the default name for all players.
    m_all_players.push_back(new PlayerProfile(username.c_str()) );

    // add default guest player
    m_all_players.push_back( new PlayerProfile(_LTR("Guest"), /*guest*/true) );


}   // addDefaultPlayer

// ----------------------------------------------------------------------------
/** This returns a unique id. This is 1 + largest id used so far.
 */
unsigned int PlayerManager::getUniqueId() const
{
    unsigned int max_id=0;
    const PlayerProfile *player;
    for_in(player, m_all_players)
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
    const PlayerProfile *player;
    for_in(player, m_all_players)
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
    PlayerProfile *player;
    for_in(player, m_all_players)
    {
        if(player->getName()==name)
            return player;
    }
    return NULL;
}   // getPlayer
// ----------------------------------------------------------------------------
/** Sets the current player. This is the player that is used for story mode
 *  and achievements. If 'remember_me' is set, this information will be
 *  stored in the players.xml file, and automatically loaded next time
 *  STK is started.
 *  \param Player profile to be the current player.
 *  \param remember_me If this player should be marked as default
 *         player in players.xml
 */
void PlayerManager::setCurrentPlayer(PlayerProfile *player, bool remember_me)
{
    // Reset current default player
    if(m_current_player)
        m_current_player->setDefault(false);
    m_current_player = player;
    if(m_current_player)
    {
        m_current_player->setDefault(remember_me);
        m_current_player->computeActive();
    }
}   // setCurrentPlayer

// ----------------------------------------------------------------------------
