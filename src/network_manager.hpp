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

#ifdef HAVE_ENET
#  include "enet/enet.h"
#endif

class NetworkManager
{
public:
    // The mode the network manager is operating in
    enum NetworkMode {NW_SERVER, NW_CLIENT, NW_NONE};

    // The current state.
    enum NetworkState {NS_SYNCHRONISING, NS_RACING};
private:
    NetworkMode  m_mode;
    NetworkState m_state;
    int          m_port;
    std::string  m_server_address;
    int          m_num_clients;
#ifdef HAVE_ENET
    ENetHost    *m_host;
#endif

    bool         initServer();
    bool         initClient();
    void         handleNewConnection(ENetEvent *event);
    void         handleNewMessage   (ENetEvent *event);
    void         handleDisconnection(ENetEvent *event);


public:
                 NetworkManager();
                ~NetworkManager();
    void         setMode(NetworkMode m)            {m_mode = m;          }
    NetworkMode  getMode() const                   {return m_mode;       }
    void         setState(NetworkState s)          {m_state = s;         }
    NetworkState getState() const                  {return m_state;      }
    void         setNumClients(int n)              {m_num_clients = n;   }
    int          getNumClients() const             {return m_num_clients;}
    void         setPort(int p)                    {m_port=p;            }
    void         setServerIP(const std::string &s) {m_server_address=s;  }
    bool         initialiseConnections();
    void         update(float dt);
    void         sendKartsInformationToServer();
    void         waitForKartsInformation();
    void         sendRaceInformationToClients();
    void         waitForRaceInformation();
};

extern NetworkManager *network_manager;

#endif

