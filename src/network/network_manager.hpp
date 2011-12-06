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

#ifndef HEADER_NETWORK_MANAGER_HPP
#define HEADER_NETWORK_MANAGER_HPP

#include <string>
#include <vector>

#include "enet/enet.h"

#include "network/remote_kart_info.hpp"


class Message;

class NetworkManager
{
public:
    // The mode the network manager is operating in
    enum NetworkMode {NW_SERVER, NW_CLIENT, NW_NONE};

    // States for the finite state machine. First for server:
    enum NetworkState {NS_MAIN_MENU,                       // before char sel gui
                       NS_ACCEPT_CONNECTIONS,              // server: accept connections
                       NS_WAIT_FOR_AVAILABLE_CHARACTERS,   // client: wait for list
                       NS_ALL_REMOTE_CHARACTERS_DONE,      // server: all client data received
                       NS_WAIT_FOR_KART_CONFIRMATION,      // client: wait for confirmation
                                                           // if character selection was ok
                       NS_KART_CONFIRMED,                  // Character was confirmed
                       NS_WAIT_FOR_RACE_DATA,              // client: wait for race info
                       NS_READY_SET_GO_BARRIER,            // c&s: barrier before r.s.g.
                       NS_CHARACTER_SELECT,                // c&s: character select in progress
                       NS_LOADING_WORLD,                   // client: loading world
                       NS_RACING,
                       NS_WAIT_FOR_RACE_RESULT,            // clients: waiting for race results
                       NS_RACE_RESULT_BARRIER ,            // Wait till all ack results
                       NS_RACE_RESULT_BARRIER_OVER         // Barrier is over, goto next state
    };
private:

    NetworkMode                 m_mode;
    NetworkState                m_state;
    unsigned int                m_num_clients;
    std::vector<RemoteKartInfo> m_kart_info;
    int                         m_host_id;
    std::vector<std::string>    m_client_names;
    std::vector<int>            m_num_local_players;
    std::vector<int>            m_kart_id_offset;    // kart id of first kart on host i
    int                         m_num_all_players;
    int                         m_barrier_count;

    ENetHost                   *m_host;    // me
    ENetPeer                   *m_server;  // (clients only)
    std::vector<ENetPeer*>      m_clients; // (server only) pos in vector is client host_id 
    /** Name of the kart that a client is waiting for confirmation for. */
    std::string                 m_kart_to_confirm;

    bool         initServer();
    bool         initClient();
    void         handleNewConnection(ENetEvent *event);
    void         handleMessageAtServer(ENetEvent *event);
    void         handleMessageAtClient(ENetEvent *event);
    void         handleDisconnection(ENetEvent *event);
    // the first cast to long avoid compiler errors on 64 bit systems
    // about lost precision, then cast long to int to get the right type
    unsigned int getHostId(ENetPeer *p) const {return (int)(long)p->data; }

    void         sendToServer(Message &m);
    void         broadcastToClients(Message &m);
public:
                 NetworkManager();
                ~NetworkManager();
    void         setMode(NetworkMode m)            {m_mode = m;              }
    NetworkMode  getMode() const                   {return m_mode;           }
    void         becomeServer();
    void         becomeClient();
    void         setState(NetworkState s)          {m_state = s;             }
    NetworkState getState() const                  {return m_state;          }
    int          getMyHostId() const               {return m_host_id;        }
    void         setHostId(int host_id)            {m_host_id = host_id;     }
    unsigned int getNumClients() const             {return m_num_clients;    }
    const std::string& 
                 getClientName(int i) const        {return m_client_names[i];}
    bool         initialiseConnections();
    void         update(float dt);

    void         disableNetworking();
    void         sendConnectMessage();  // client send initial info to server
    void         initCharacterDataStructures();
    void         sendCharacterSelected(int player_id, const std::string &kartid);
    void         waitForRaceInformation();
    void         worldLoaded();
    void         setupPlayerKartInfo();
    void         beginReadySetGoBarrier();
    void         sendRaceInformationToClients();
    void         sendUpdates();
    void         receiveUpdates();
    void         waitForClientData();
    void         sendRaceResults();
    void         beginRaceResultBarrier();
    void         sendRaceResultAck(char menu_selection=-1);
};

extern NetworkManager *network_manager;

#endif
