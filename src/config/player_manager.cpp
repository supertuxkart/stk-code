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

#include "config/player.hpp"
#include "io/file_manager.hpp"
#include "io/utf_writer.hpp"
#include "io/xml_node.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

PlayerManager *PlayerManager::m_player_manager = NULL;

/** Constructor.
 */
PlayerManager::PlayerManager()
{
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
    if(!players || players->getName()!="players")
    {
        Log::info("player_manager", "The players.xml file is invalid.");
        return;
    }

    for(unsigned int i=0; i<players->getNumNodes(); i++)
    {
        const XMLNode *player_xml = players->getNode(i);
        PlayerProfile *profile = new PlayerProfile(player_xml);
        m_all_players.push_back(profile);
    }
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
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
