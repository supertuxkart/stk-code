//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#include "network/connect_message.hpp"

#include <string>
#include <vector>
#include <sstream>
#ifndef WIN32
#  include <unistd.h>
#endif

#include "config/user_config.hpp"
#include "config/player.hpp"
#include "karts/kart_properties_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track_manager.hpp"

// ----------------------------------------------------------------------------
/** Creates the connect message. It includes the id of the client (currently
 *  player name @ hostname), and the list of available tracks.
 */
ConnectMessage::ConnectMessage() : Message(MT_CONNECT)
{
    setId();
    const std::vector<std::string> &all_tracks = 
                               track_manager->getAllTrackIdentifiers();
    std::vector<std::string> all_karts = 
                               kart_properties_manager->getAllAvailableKarts();
    allocate(getStringLength(m_id) + getStringVectorLength(all_tracks)
             + getStringVectorLength(all_karts));
    addString(m_id);
    addStringVector(all_tracks);
    addStringVector(all_karts);
}   // ConnectMessage

// ----------------------------------------------------------------------------
/** Unpacks a connect message. The id of the client is stored in this object,
 *  and the list of tracks is used to set tracks that are not available on
 *  the client to be 'unavailable' on the server.
 *  \param pkt Enet packet.
 */
ConnectMessage::ConnectMessage(ENetPacket* pkt):Message(pkt, MT_CONNECT)
{
    m_id       = getString();
    std::vector<std::string> all_tracks = getStringVector();
    std::vector<std::string> all_karts  = getStringVector();
    track_manager->setUnavailableTracks(all_tracks);
    kart_properties_manager->setUnavailableKarts(all_karts);
}   // ConnectMessage

// ----------------------------------------------------------------------------
/** Sets the id, i.e. player name @ hostname, of this client.
 */
void ConnectMessage::setId()
{
    char hostname[256];
    gethostname(hostname, 255);
    const std::string& id = core::stringc(StateManager::get()->getActivePlayerProfile(0)->getName()).c_str();
    std::ostringstream o;
    o << id << '@' << hostname;
    m_id = o.str();
}   // ConnectMessage
