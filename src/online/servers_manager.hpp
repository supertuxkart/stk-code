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
#define HEADER_CURRENT_ONLINE_USER_HPP

#include "utils/ptr_vector.hpp"
#include "online/server.hpp"


namespace online {

    /**
      * \brief
      * \ingroup online
      */
    class ServersManager
    {
        private:
            ServersManager();
            PtrVector<Server> * m_servers;
            bool m_not_fetched;
            Server * m_joined_server;

        public:
            // singleton
            static ServersManager* get();
            static void deallocate();

            void refresh();
            PtrVector<Server> * getServers () const { return m_servers; };
            int getNumServers () const { return m_servers->size(); };
            Server * getServer (int index) const { return m_servers->get(index);};
            void sort(bool sort_desc) { m_servers->insertionSort(0, sort_desc); };
            void setJoinedServer(Server * server){ m_joined_server = server;};
            Server * getJoinedServer(){ return m_joined_server;};
            //Returns the best server to join
            Server * getQuickPlay();
    };   // class ServersManager


} // namespace online


#endif

/*EOF*/
