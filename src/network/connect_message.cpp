//  $Id: connect_message.cpp 2128 2008-06-13 00:53:52Z cosmosninja $
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

#include "user_config.hpp"
#include "track_manager.hpp"

// ----------------------------------------------------------------------------
/** Creates the connect message. It includes the id of the client (currently
 *  player name @ hostname), and the list of available tracks.
 */
ConnectMessage::ConnectMessage() : Message(MT_CONNECT)
{
    setId();
    const std::vector<std::string> &all_tracks = 
                                    track_manager->getAllTrackIdentifiers();
    allocate(getStringLength(m_id) + getStringVectorLength(all_tracks));
    addString(m_id);
    addStringVector(all_tracks);
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
    track_manager->setUnavailableTracks(all_tracks);
}   // ConnectMessage

// ----------------------------------------------------------------------------
/** Sets the id, i.e. player name @ hostname, of this client.
 */
void ConnectMessage::setId()
{
    char hostname[256];
    gethostname(hostname, 255);
    const std::string& id=user_config->m_player[0].getName();
    std::ostringstream o;
    o << id << '@' << hostname;
    m_id = o.str();
}   // ConnectMessage
