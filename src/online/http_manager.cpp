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

#include "online/http_manager.hpp"

#include <iostream>
#include <stdio.h>
#include <memory.h>
#include <errno.h>

#if defined(WIN32) && !defined(__CYGWIN__)
#  include <windows.h>
#  define isnan _isnan
#else
#  include <sys/time.h>
#  include <math.h>
#endif

#if defined(WIN32) && !defined(__CYGWIN__)
// Use Sleep, which takes time in msecs. It must be defined after the
// includes, since otherwise irrlicht's sleep function is changed.
#  define sleep(s) Sleep(1000*(s))
#else
#  include <unistd.h>
#endif

namespace Online{

    static HTTPManager * http_singleton = NULL;

    HTTPManager* HTTPManager::get()
    {
        if (http_singleton == NULL)
        {
            http_singleton = new HTTPManager();
            http_singleton->startNetworkThread();
        }
        return http_singleton;
    }   // get

    void HTTPManager::deallocate()
    {
        delete http_singleton;
        http_singleton = NULL;
    }   // deallocate


    HTTPManager::HTTPManager(){
        curl_global_init(CURL_GLOBAL_DEFAULT);
        pthread_cond_init(&m_cond_request, NULL);
    }

    // ============================================================================
    HTTPManager::~HTTPManager(){
        curl_global_cleanup();
    }


    // ---------------------------------------------------------------------------
    /** Start the actual network thread. This can not be done as part of
     *  the constructor, since the assignment to the global network_http
     *  variable has not been assigned at that stage, and the thread might
     *  use network_http - a very subtle race condition. So the thread can
     *  only be started after the assignment (in main) has been done.
     */
    void HTTPManager::startNetworkThread()
    {
        pthread_attr_t  attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        // Should be the default, but just in case:
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

        m_thread_id.setAtomic(new pthread_t());
        int error = pthread_create(m_thread_id.getData(), &attr,
                                   &HTTPManager::mainLoop, this);
        if(error)
        {
            m_thread_id.lock();
            delete m_thread_id.getData();
            m_thread_id.unlock();
            m_thread_id.setAtomic(0);
            Log::error("HTTP Manager", "Could not create thread, error=%d.\n", errno);
        }
        pthread_attr_destroy(&attr);
    }   // startNetworkThread



    // ---------------------------------------------------------------------------
    /** This function inserts a high priority request to quit into the request
     *  queue of the network thead, and also aborts any ongoing download.
     *  Separating this allows more time for the thread to finish cleanly,
     *  before it gets cancelled in the destructor.
     */
    void HTTPManager::stopNetworkThread()
    {
        // If a download should be active (which means it was cancelled by the
        // user, in which case it will still be ongoing in the background)
        // we can't get the mutex, and would have to wait for a timeout,
        // and we couldn't finish STK. This way we request an abort of
        // a download, which mean we can get the mutex and ask the service
        // thread here to cancel properly.
        cancelAllDownloads();

        QuitRequest * request = new QuitRequest();
        addRequest(request);
    }   // stopNetworkThread


    // ----------------------------------------------------------------------------
    /** Signals to the progress function to request any ongoing download to be
     *  cancelled. This function can also be called if there is actually no
     *  download atm. The function progressDownload checks m_abort and will
     *  return a non-zero value which causes libcurl to abort. */
    void HTTPManager::cancelAllDownloads()
    {
        m_abort.setAtomic(true);
    }   // cancelAllDownloads


    // ----------------------------------------------------------------------------
    /** Inserts a request into the queue of all requests. The request will be
     *  sorted by priority.
     *  \param request The pointer to the new request to insert.
     */
    bool HTTPManager::addRequest(Online::Request *request)
    {
        if (request->isAllowedToAdd())
        {
            m_request_queue.lock();

            m_request_queue.getData().push(request);
            // Wake up the network http thread
            pthread_cond_signal(&m_cond_request);

            m_request_queue.unlock();
            return true;
        }
        else
        {
            Log::info("HTTPManager::addrequest", "Did not add request.");
            return false;
        }
    }   // insertRequest

    // ---------------------------------------------------------------------------
    /** The actual main loop, which is started as a separate thread from the
     *  constructor. After testing for a new server, fetching news, the list
     *  of packages to download, it will wait for commands to be issued.
     *  \param obj: A pointer to this object, passed on by pthread_create
     */
    void *HTTPManager::mainLoop(void *obj)
    {
        HTTPManager *me = (HTTPManager*) obj;

        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,      NULL);

        me->m_current_request = NULL;
        me->m_request_queue.lock();
        while(  me->m_request_queue.getData().empty() || !dynamic_cast<Online::QuitRequest*>(me->m_request_queue.getData().top()) )
        {
            bool empty = me->m_request_queue.getData().empty();
            // Wait in cond_wait for a request to arrive. The 'while' is necessary
            // since "spurious wakeups from the pthread_cond_wait ... may occur"
            // (pthread_cond_wait man page)!
            while(empty)
            {
                pthread_cond_wait(&me->m_cond_request, me->m_request_queue.getMutex());
                empty = me->m_request_queue.getData().empty();
            }
            me->m_current_request = me->m_request_queue.getData().top();
            me->m_request_queue.getData().pop();
            me->m_request_queue.unlock();
            me->m_current_request->execute();

            if(me->m_current_request->manageMemory())
            {
                delete me->m_current_request;
                me->m_current_request = NULL;
            }
            me->m_request_queue.lock();
        }   // while

        // At this stage we have the lock for m_request_queue
        while(!me->m_request_queue.getData().empty())
        {
            Online::Request * request = me->m_request_queue.getData().top();
            me->m_request_queue.getData().pop();
            // Manage memory can be ignored here, all requests
            // need to be freed.
            delete request;
        }
        me->m_request_queue.unlock();

        pthread_exit(NULL);
        return 0;
    }   // mainLoop
} // namespace Online




