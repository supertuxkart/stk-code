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
class TransportAddress;
class XMLNode;

/**
 * \brief
 * \ingroup online
 */
class ServersManager
{
private:
    /** List of servers */
    std::vector<std::shared_ptr<Server> > m_servers;
    
    /** List of broadcast addresses to use. */
    std::vector<TransportAddress> m_broadcast_address;

    std::atomic<int64_t> m_last_load_time;

    std::atomic_bool m_list_updated;
    // ------------------------------------------------------------------------
     ServersManager();
    // ------------------------------------------------------------------------
    ~ServersManager();
    // ------------------------------------------------------------------------
    void setWanServers(bool success, const XMLNode* input);
    // ------------------------------------------------------------------------
    std::shared_ptr<Online::XMLRequest> getWANRefreshRequest() const;
    // ------------------------------------------------------------------------
    std::shared_ptr<Online::XMLRequest> getLANRefreshRequest() const;
    // ------------------------------------------------------------------------
    void setLanServers(const std::map<irr::core::stringw, 
                                      std::shared_ptr<Server> >& servers);
                                      
    void setDefaultBroadcastAddresses();
    void addAllBroadcastAddresses(const TransportAddress &a, int len);
    void updateBroadcastAddresses();
public:
    // ------------------------------------------------------------------------
    // Singleton
    static ServersManager* get();
    // ------------------------------------------------------------------------
    static void deallocate();
    // ------------------------------------------------------------------------
    void cleanUpServers()                                { m_servers.clear(); }
    // ------------------------------------------------------------------------
    bool refresh(bool full_refresh);
    // ------------------------------------------------------------------------
    std::vector<std::shared_ptr<Server> >& getServers()   { return m_servers; }
    // ------------------------------------------------------------------------
    bool listUpdated() const                         { return m_list_updated; }
    // ------------------------------------------------------------------------
    const std::vector<TransportAddress>& getBroadcastAddresses();
    // ------------------------------------------------------------------------
    void reset()
    {
        m_last_load_time.store(-5000);
        m_list_updated = false;
    }
};   // class ServersManager
#endif // HEADER_SERVERS_MANAGER_HPP
