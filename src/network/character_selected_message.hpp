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

#ifndef HEADER_CHARACTER_SELECTED_MESSAGE_HPP
#define HEADER_CHARACTER_SELECTED_MESSAGE_HPP

#include "network/message.hpp"
#include "network/remote_kart_info.hpp"
#include "race/race_manager.hpp"

/** This message is send contains information about selected karts. It is send
 *  from the client to the server to indicate a selected kart, and from the
 *  server to the clients to indicate that a kart was selected. In the latter
 *  case it contains the hostid of the successful selecter. This way a client
 *  selecting a kart can check if its selection was successful or not, and
 *  other clients are informed that a certain kart is not available anymore.
 */
class CharacterSelectedMessage : public Message
{
private:
    /** Number of local players on a host. If the message is send from the
     *  server to the clients, this field instead contains the host id of 
     *  the host which selected the kart
     */
    int            m_num_local_players;
    /** Stores information about the selected kart. */
    RemoteKartInfo m_kart_info;

public:
    /** Contains information about a selected kart. When send from the client
     *  to the server, it contains the number of local players (which 
     *  technically needs only to be sent once); when send from from the server
     *  to the clients this field instead contains the host id of the host
     *  selected the character. This allows the client to detect if a selected
     *  kart was not confirmed by the server (i.e. another client or the server
     *  has selected the kart first
     *  \param player_id The local player id.
     *  \param host_id If this value is specified (>-1), then this value is
     *                 used in the message instead of the number of local
     *                 players.
     */
    CharacterSelectedMessage(int player_id, int host_id=-1)  
        : Message(Message::MT_CHARACTER_INFO) 
    {
        m_kart_info         = race_manager->getLocalKartInfo(player_id);
        m_num_local_players = race_manager->getNumLocalPlayers();

        allocate(getCharLength()       // m_kart_info.getLocalPlayerId())
                +getStringLength(m_kart_info.getKartName())
                +m_kart_info.getPlayerName().size() + 1 // FIXME: encoding issues
                +getCharLength());     // m_num_local_players)
        addChar(m_kart_info.getLocalPlayerId());
        addString(m_kart_info.getKartName());
        addString(core::stringc(m_kart_info.getPlayerName().c_str()).c_str()); // FIXME: encoding issues
        // Piggy backing this information saves sending it as a separate 
        // message. It is actually only required in the first message
        if(host_id>-1)
            addChar(host_id);
        else
            addChar(race_manager->getNumLocalPlayers());
    }   // CharacterSelectedMessage

    // ------------------------------------------------------------------------
    /** Unpacks a character selected message. The additional field is either
     *  the number of local players (when send from client to server), or the
     *  hostid of the host selected the character.
     *  \param pkt Received enet packet.
     */
    CharacterSelectedMessage(ENetPacket* pkt):Message(pkt, MT_CHARACTER_INFO)
    {
        m_kart_info.setLocalPlayerId(getChar());
        m_kart_info.setKartName(getString());
        m_kart_info.setPlayerName(core::stringw(getString().c_str())); // FIXME: encoding issues
        m_num_local_players = getChar();
    }   // CharacterSelectedMessage(EnetPacket)

    // ------------------------------------------------------------------------
    /** Returns the remote kart info structure of the selected kart. */
    const RemoteKartInfo& getKartInfo  () const { return m_kart_info;         }

    /** Returns the number of local players. */
    int                   getNumPlayers() const { return m_num_local_players; }

    /** Returns the host id of the host who selected the kart successfully.
     *  This information is actually stored in m_num_local_players field, which
     *  is used when a client receives this message.
     */
    int                   getHostId    () const { return m_num_local_players; }
};   // CharacterSelectedMessage
#endif
