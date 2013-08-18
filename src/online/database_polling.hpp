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

#ifndef HEADER_DATABASE_POLLING_HPP
#define HEADER_DATABASE_POLLING_HPP

#include "utils/types.hpp"
#include "utils/synchronised.hpp"
#include "http_manager.hpp"
#include <queue>



namespace Online {

    /**
      * \brief
      * \ingroup online
      */
    class DatabasePolling
    {
        class PollRequest : public XMLRequest
        {
            void callback ()
            {
                DatabasePolling::get()->addResult(this);
            }
        public:
            PollRequest() : XMLRequest() {}
            void onPollFetch() = 0;
        };

    private:
        DatabasePolling();
        ~DatabasePolling();

        float                                           m_time_since_poll;

        /** The list of pointers to all requests that are handled and needed to be put in the queue */
        Synchronised< std::queue<Online::Request*> >    m_result_queue;

        void addResult(Online::Request *request);
        PollRequest * popResult();
        void DatabasePolling::generateNewPollRequest();

    public:
        // Singleton
        static DatabasePolling*                         get();
        static void                                     deallocate();

        void                                            update(float dt);



    };   // class DatabasePolling


} // namespace Online


#endif

/*EOF*/
