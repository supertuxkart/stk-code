//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2013 SuperTuxKart-Team
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

#include "config/player.hpp"

#include "config/player_manager.hpp"
#include "io/xml_node.hpp"
#include "io/utf_writer.hpp"
#include "utils/string_utils.hpp"

#include <sstream>
#include <stdlib.h>

//------------------------------------------------------------------------------
/** Constructor to create a new player that didn't exist before.
 *  \param name Name of the player.
 *  \param is_guest True if this is a guest account.
*/
PlayerProfile::PlayerProfile(const core::stringw& name, bool is_guest)
{
#ifdef DEBUG
    m_magic_number = 0xABCD1234;
#endif
    m_name             =  name;
    m_is_guest_account = is_guest;
    m_use_frequency    = 0;
    m_unique_id        = PlayerManager::get()->getUniqueId();

}   // PlayerProfile

//------------------------------------------------------------------------------
/** Constructor to deserialize a player that was saved to a XML file.
 *  \param node The XML node representing this player.
*/
PlayerProfile::PlayerProfile(const XMLNode* node)
{
    node->get("name",          &m_name            );
    node->get("guest",         &m_is_guest_account);
    node->get("use-frequency", &m_use_frequency   );
    node->get("unique-id",     &m_unique_id       );
    #ifdef DEBUG
    m_magic_number = 0xABCD1234;
    #endif
}   // PlayerProfile

//------------------------------------------------------------------------------
/** Writes the data for this player to the specified UTFWriter.
 *  \param out The utf writer to write the data to.
 */
void PlayerProfile::save(UTFWriter &out)
{
    out << L"    <player name=\""         << m_name             << L"\"\n";
    out << L"            guest=\""        << m_is_guest_account << L"\"\n";
    out << L"            use-frequency=\""<< m_use_frequency    << L"\"\n";
    out << L"            unique-id=\""    << m_unique_id        << L"\"\n";
    out << L"    />\n";
}   // save

//------------------------------------------------------------------------------
/** Increments how often that account was used. Guest accounts are not counted.
 */
void PlayerProfile::incrementUseFrequency()
{
    if (m_is_guest_account) m_use_frequency = -1;
    else m_use_frequency++;
}   // incrementUseFrequency
