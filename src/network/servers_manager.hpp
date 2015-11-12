//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#ifndef HEADER_SERVERS_MANAGER_HPP
#define HEADER_SERVERS_MANAGER_HPP

#include "online/request_manager.hpp"
#include "network/server.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/synchronised.hpp"
#include "utils/types.hpp"

namespace Online { class XMLRequest; }

/**
 * \brief
 * \ingroup online
 */
class ServersManager
{
private:
     ServersManager();
    ~ServersManager();

    /** Sorted vector of servers */
    Synchronised<PtrVector<Server> >                m_sorted_servers;

    /** Maps server id's to the same servers*/
    Synchronised<std::map<uint32_t, Server*> >      m_mapped_servers;

    /** This is a pointer to a copy of the server, the moment it got joined */
    Synchronised<Server *>   m_joined_server;

    Synchronised<float>      m_last_load_time;
    void                     refresh(bool success, const XMLNode * input);
    void                     cleanUpServers();
    Online::XMLRequest *     getWANRefreshRequest() const;
    Online::XMLRequest *     getLANRefreshRequest() const;

public:
    // Singleton
    static ServersManager*   get();
    static void              deallocate();

    Online::XMLRequest *     getRefreshRequest(bool request_now = true);
    void                     setJoinedServer(uint32_t server_id);
    void                     unsetJoinedServer();
    void                     addServer(Server * server);
    int                      getNumServers() const;
    const Server *           getServerByID(uint32_t server_id) const;
    const Server *           getServerBySort(int index) const;
    void                     sort(bool sort_desc);
    Server *                 getJoinedServer() const;

    // Returns the best server to join
    const Server *           getQuickPlay() const;

};   // class ServersManager
#endif // HEADER_SERVERS_MANAGER_HPP
