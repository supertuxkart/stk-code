//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Lucas Baudin
//            (C) 2011-2015 Joerg Henrichs
//            (C) 2013-2015 Glenn De Jonghe
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
#include "utils/can_be_deleted.hpp"
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
    /** A class to execute requests in a separate thread. Typically the
     *  requests involve a http(s) requests to be sent to the stk server, and
     *  receive an answer (e.g. to sign in; or to download an addon). The
     *  requests are sorted by priority (e.g. sign in and out have higher
     *  priority than downloading addon icons).
     *  A request is created and initialised from the main thread. When it
     *  is moved into the request queue, it must not be handled by the main
     *  thread anymore, only the RequestManager thread can handle it.
     *  Once the request is finished, it is put in a separate ready queue.
     *  The main thread regularly checks the ready queue for any ready
     *  request, and executes a callback. So there is no need to protect
     *  any functions or data members in requests, since they will either
     *  be handled by the main thread, or RequestManager thread, never by
     *  both.
     *  On exit, if necessary a high priority sign-out or client-quit request
     *  is put into the queue, and a flag is set which causes libcurl to
     *  abort any ongoing download. Then an additional 'quit' event with
     *  same priority as the sign-out is added to the queue (since it will
     *  be added later, the sign-out will be executed first, making sure that
     *  a logged in user is logged out (or its session saved). Once this is
     *  done, most of stk is deleted (except a few objects like the file
     *  manager which might be accessed if a download just finished before the
     *  abort). On executing the quit request, the request manager will set
     *  a flag that it is ready to be deleted (using the CanBeDeleted class).
     *  The main thread will wait for a certain amount of time for the
     *  RequestManager to be ready to be deleted (i.e. the sign-out and quit
     *  request have been processes), before deleting the RequestManager.
     *  Typically the RequestManager will finish while the rest of stk is
     *  shutting down, so the user will not experience any waiting time. Only
     *  on first start of stk (which will trigger downloading of all addon
     *  icons) is it possible that actually a download request is running,
     *  which might take a bit before it can be deleted.
     * \ingroup online
     */
    class RequestManager : public CanBeDeleted
    {
    public:
        /** If stk has permission to access the internet (for news
        *  server etc).
        *  IPERM_NOT_ASKED: The user needs to be asked if he wants to
        *                   grant permission
        *  IPERM_ALLOWED:   STK is allowed to access server.
        *  IPERM_NOT_ALLOWED: STK must not access external servers. */
        enum InternetPermission
        {
            IPERM_NOT_ASKED   = 0,
            IPERM_ALLOWED     = 1,
            IPERM_NOT_ALLOWED = 2
        };
    private:
            /** Time passed since the last poll request. */
            float                     m_time_since_poll;

            /** The current requested being worked on. */
            Online::Request *         m_current_request;

            /** A conditional variable to wake up the main loop. */
            pthread_cond_t            m_cond_request;

            /** Signal an abort in case that a download is still happening. */
            Synchronised<bool>        m_abort;

            /** The polling interval while a game is running. */
            float m_game_polling_interval;

            /** The polling interval while the menu is shown. */
            float m_menu_polling_interval;

            /** Thread id of the thread running in this object. */
            Synchronised<pthread_t *> m_thread_id;

            /** The list of pointers to all requests that still need to be
             *  handled. */
            Synchronised< std::priority_queue <
                                                Online::Request*,
                                                std::vector<Online::Request*>,
                                                Online::Request::Compare
                                               >
                        >  m_request_queue;

            /** The list of pointers to all requests that are already executed
             *  by the networking thread, but still need to be processed by the
             *  main thread. */
            Synchronised< std::queue<Online::Request*> >    m_result_queue;

            void addResult(Online::Request *request);
            void handleResultQueue();

            static void *mainLoop(void *obj);

            RequestManager(); //const std::string &url
            ~RequestManager();

            static RequestManager * m_request_manager;

        public:
            static const int HTTP_MAX_PRIORITY = 9999;

            // ----------------------------------------------------------------
            /** Singleton access function. Creates the RequestManager if
             * necessary. */
            static RequestManager* get()
            {
                    if (m_request_manager == NULL)
                    {
                        m_request_manager = new RequestManager();
                    }
                    return m_request_manager;
            }   // get
            // ----------------------------------------------------------------

            static void deallocate();
            static bool isRunning();

            void addRequest(Online::Request *request);
            void startNetworkThread();
            void stopNetworkThread();

            bool getAbort() { return m_abort.getAtomic(); }
            void update(float dt);

            // ----------------------------------------------------------------
            /** Sets the interval with which poll requests are send to the
             *  server. This can happen from the news manager (i.e. info
             *  contained in the news.xml file), or a poll request. */
            void setMenuPollingInterval(float polling_interval)
            {
                m_menu_polling_interval = polling_interval;
            }   // setPollingInterval
            // ----------------------------------------------------------------
            /** Sets the interval with which poll requests are send to the
             *  server. This can happen from the news manager (i.e. info
             *  contained in the news.xml file), or a poll request. */
            void setGamePollingInterval(float polling_interval)
            {
                m_game_polling_interval = polling_interval;
            }   // setPollingInterval

    }; //class RequestManager
} // namespace Online
#endif // request_manager_HPP
