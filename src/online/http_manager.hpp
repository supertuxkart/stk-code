//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Lucas Baudin
//                2011 Joerg Henrichs
//                2013 Glenn De Jonghe
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

#ifndef HTTP_MANAGER_HPP
#define HTTP_MANAGER_HPP

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


namespace Online{

    /**
      * \brief Class to connect with a server over HTTP
      * \ingroup online
      */
    class HTTPManager
    {
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

            Synchronised< std::queue<Online::Request*> >    m_result_queue;

            void addResult(Online::Request *request);
            void handleResultQueue();

            static void  *mainLoop(void *obj);

            HTTPManager(); //const std::string &url
            ~HTTPManager();

        public:
            static const int MAX_PRIORITY = 9999;

            // singleton
            static HTTPManager* get();
            static void deallocate();
            static bool isRunning();

            //Execute
            std::string getPage(Online::Request * request);
            XMLNode * getXMLFromPage(Online::Request * request);

            void synchronousRequest(Online::Request *request);
            void addRequest(Online::Request *request);
            void cancelAllDownloads();
            void startNetworkThread();
            void stopNetworkThread();

            bool getAbort(){ return m_abort.getAtomic(); };
            void update(float dt);

    }; //class HTTPManager
} // namespace Online

#endif // HTTP_MANAGER_HPP

/*EOF*/
