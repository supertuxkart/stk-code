//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2014 Lucas Baudin
//                2011-2014 Joerg Henrichs
//                2013-2014 Glenn De Jonghe
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

#ifndef HEADER_REQUEST_MANAGER_HPP
#define HEADER_REQUEST_MANAGER_HPP

#include "io/xml_node.hpp"
#include "online/request.hpp"
#include "utils/string_utils.hpp"
#include "utils/synchronised.hpp"

#include <irrString.h>

#include <string>
#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#endif

#include <curl/curl.h>
#include <queue>
#include <pthread.h>


namespace Online
{

    /**
      * \brief Class to connect with a server over HTTP(S)
      * \ingroup online
      */
    class RequestManager
    {
    public:
        /** If stk has permission to access the internet (for news
        *  server etc).
        *  IPERM_NOT_ASKED: The user needs to be asked if he wants to
        *                   grant permission
        *  IPERM_ALLOWED:   STK is allowed to access server.
        *  IPERM_NOT_ALLOWED: STK must not access external servers. */
        enum InternetPermission {IPERM_NOT_ASKED  =0,
                                 IPERM_ALLOWED    =1,
                                 IPERM_NOT_ALLOWED=2 };
    protected:

            float                     m_time_since_poll;

            /** The current requested being worked on. */
            Online::Request *         m_current_request;

            /** A conditional variable to wake up the main loop. */
            pthread_cond_t            m_cond_request;

            /** Signal an abort in case that a download is still happening. */
            Synchronised<bool>        m_abort;

            /** Thread id of the thread running in this object. */
            Synchronised<pthread_t *> m_thread_id;

            /** The list of pointers to all requests that still need to be handled. */
            Synchronised< std::priority_queue <
                                                Online::Request*,
                                                std::vector<Online::Request*>,
                                                Online::Request::Compare
                                               >
                        >  m_request_queue;

            /** The list of pointers to all requests that are already executed by the networking thread, but still need to be processed by the main thread. */
            Synchronised< std::queue<Online::Request*> >    m_result_queue;

            void addResult(Online::Request *request);
            void handleResultQueue();

            static void  *mainLoop(void *obj);

            RequestManager(); //const std::string &url
            ~RequestManager();

        public:
            static const int HTTP_MAX_PRIORITY = 9999;

            // singleton
            static RequestManager* get();
            static void deallocate();
            static bool isRunning();

            void addRequest(Online::Request *request);
            void startNetworkThread();
            void stopNetworkThread();

            bool getAbort(){ return m_abort.getAtomic(); };
            void update(float dt);

    }; //class RequestManager
} // namespace Online

#endif // request_manager_HPP

/*EOF*/
