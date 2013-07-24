//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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

#include "utils/ptr_vector.hpp"
#include "utils/types.hpp"
#include "online/server.hpp"
#include "http_manager.hpp"



namespace Online {

    /**
      * \brief
      * \ingroup online
      */
    class ServersManager
    {
    public:
        enum RequestType
        {
            RT_REFRESH = 1
        };

        class RefreshRequest : public XMLRequest
        {
            virtual void callback ();
        public:
            RefreshRequest() : XMLRequest(RT_REFRESH) {}
        };

    private:
        ServersManager();
        ~ServersManager();
        /** Sorted vector of servers */
        PtrVector<Server>               m_sorted_servers;
        /** Maps server id's to the same servers*/
        std::map<uint32_t, Server*>     m_mapped_servers;
        /** This is a pointer to a copy of the server, the moment it got joined */
        Server *                        m_joined_server;

        bool                            m_not_fetched;
        irr::core::stringw              m_info_message;
        float                           m_last_load_time;
        void                            refresh(RefreshRequest * input);
        void                            cleanUpServers();

    public:
        // Singleton
        static ServersManager*          acquire();
        static void                     release();
        static void                     deallocate();

        RefreshRequest *                refreshRequest();
        void                            setJoinedServer(uint32_t server_id);
        void                            unsetJoinedServer();
        void                            addServer(Server * server);
        int                             getNumServers ();
        Server *                        getServerByID (uint32_t server_id);
        Server *                        getServerBySort (int index);
        void                            sort(bool sort_desc)            { m_sorted_servers.insertionSort(0, sort_desc); }
        Server *                        getJoinedServer()               { return m_joined_server;                       }
        //Returns the best server to join
        Server *                        getQuickPlay();
    };   // class ServersManager


} // namespace Online


#endif

/*EOF*/
