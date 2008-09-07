//  $Id: network_manager.hpp 2128 2008-06-13 00:53:52Z cosmosninja $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs, Stephen Leak
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

#ifndef HEADER_NETWORK_MANAGER_H
#define HEADER_NETWORK_MANAGER_H

#include <string>
#include <vector>

#include "network/remote_kart_info.hpp"
#include "network/kart_packet.hpp"

#ifdef HAVE_ENET
#  include "enet/enet.h"
#endif

class Message;

class NetworkManager
{
public:
    // The mode the network manager is operating in
    enum NetworkMode {NW_SERVER, NW_CLIENT, NW_NONE};

    // States for the finite state machine. First for server:
    enum NetworkState {NS_NONE, 
                       NS_ACCEPT_CONNECTIONS,              // server: accept connections
                       NS_WAIT_FOR_AVAILABLE_CHARACTERS,   // client: wait for list
                       NS_ALL_REMOTE_CHARACTERS_DONE,      // server: all client data received
                       NS_WAIT_FOR_RACE_DATA,              // client: wait for race info
                       NS_READY_SET_GO_BARRIER,            // c&s: barrier before r.s.g.
                       NS_CHARACTER_SELECT,                // c&s: character select in progress
                       NS_LOADING_WORLD,                   // client: loading world
                       NS_RACING};
private:

    NetworkMode                 m_mode;
    NetworkState                m_state;
    int                         m_num_clients;
    std::vector<RemoteKartInfo> m_kart_info;
    int                         m_host_id;
    std::vector<std::string>    m_client_names;
    std::vector<int>            m_num_local_players;
    int                         m_num_all_players;
    int                         m_barrier_count;

#ifdef HAVE_ENET
    ENetHost                   *m_host;    // me
    ENetPeer                   *m_server;  // (clients only)
    std::vector<ENetPeer*>      m_clients; // (server only) pos in vector is client host_id 
#endif

    bool         initServer();
    bool         initClient();
    void         handleNewConnection(ENetEvent *event);
    void         handleMessageAtServer(ENetEvent *event);
    void         handleMessageAtClient(ENetEvent *event);
    void         handleDisconnection(ENetEvent *event);
    unsigned int getHostId(ENetPeer *p) const {return (int)p->data; }

    void         sendToServer(Message *m);
    void         broadcastToClients(Message &m);
    void         sendToClient(int id, const Message *m);
public:
                 NetworkManager();
                ~NetworkManager();
    void         setMode(NetworkMode m)            {m_mode = m;             }
    NetworkMode  getMode() const                   {return m_mode;          }
    void         setState(NetworkState s)          {m_state = s;            }
    NetworkState getState() const                  {return m_state;         }
    int          getHostId() const                 {return m_host_id;       }
    unsigned int getNumClients() const             {return m_num_clients;   }
    const std::string& 
                 getClientName(int i) const        {return m_client_names[i];}
    void         setKartInfo(int player_id, const std::string& kart, 
                             const std::string& user="", int hostid=-1);
    bool         initialiseConnections();
    void         update(float dt);

    void         disableNetworking();
    void         sendConnectMessage();  // client send initial info to server
    void         switchToCharacterSelection();
    void         sendCharacterSelected(int player_id);
    void         waitForRaceInformation();
    void         worldLoaded();

// which one is actually necessary:
    void         setupPlayerKartInfo();
    void         sendRaceInformationToClients();

    void         switchToRaceDataSynchronisation();
    void         switchToReadySetGoBarrier();
};

extern NetworkManager *network_manager;

#endif

