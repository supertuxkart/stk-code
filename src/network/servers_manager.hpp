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

#include <irrString.h>

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Online { class XMLRequest; }
class Server;
class SocketAddress;
class XMLNode;

/**
 * \brief
 * \ingroup online
 */
struct ServerList
{
    /** List of servers */
    std::vector<std::shared_ptr<Server> > m_servers;
    std::atomic_bool m_list_updated;
    ServerList() { m_list_updated.store(false); }
};

class ServersManager
{
private:
    /** List of broadcast addresses to use. */
    std::vector<SocketAddress> m_broadcast_address;

    // ------------------------------------------------------------------------
     ServersManager();
    // ------------------------------------------------------------------------
    ~ServersManager();
    // ------------------------------------------------------------------------
    void setWanServers(bool success, const XMLNode* input);
    // ------------------------------------------------------------------------
    void setLanServers(const std::map<irr::core::stringw, 
                                      std::shared_ptr<Server> >& servers);
                                      
    std::vector<SocketAddress> getDefaultBroadcastAddresses();
    void addAllBroadcastAddresses(const SocketAddress &a, int len,
                                  std::vector<SocketAddress>* result);
public:
    // ------------------------------------------------------------------------
    // Singleton
    static ServersManager* get();
    // ------------------------------------------------------------------------
    static void deallocate();
    // ------------------------------------------------------------------------
    std::vector<SocketAddress> getBroadcastAddresses(bool ipv6);
    // ------------------------------------------------------------------------
    std::shared_ptr<ServerList> getWANRefreshRequest() const;
    // ------------------------------------------------------------------------
    std::shared_ptr<ServerList> getLANRefreshRequest() const;

};   // class ServersManager
#endif // HEADER_SERVERS_MANAGER_HPP
