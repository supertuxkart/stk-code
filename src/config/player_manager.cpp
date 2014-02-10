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
#include "config/player.hpp"
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

    if(!m_current_player)
    {
        PlayerProfile *player;
        for_in(player, m_all_players)
        {
            if(!player->isGuestAccount())
            {
                m_current_player = player;
                player->setDefault(true);
                break;
            }
        }
    }
    if(!m_current_player)
        Log::fatal("PlayerManager", "Can't find a default player.");

}   // load

// ----------------------------------------------------------------------------
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
void PlayerManager::deletePlayer(PlayerProfile *player)
{
    m_all_players.erase(player);
}   // deletePlayer

// ----------------------------------------------------------------------------
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
    m_all_players.push_back( new PlayerProfile(_LTR("Guest")) );


}   // addDefaultPlayer

// ----------------------------------------------------------------------------
/** This returns a unique id. This is 1 + larger id so far used.
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
void PlayerManager::setCurrentPlayer(PlayerProfile *player)
{
    // Reset current default player
    if(m_current_player)
        m_current_player->setDefault(false);
    m_current_player = player;
    m_current_player->setDefault(true);
    AchievementsManager::get()->updateCurrentPlayer();
}   // setCurrentPlayer

// ----------------------------------------------------------------------------
