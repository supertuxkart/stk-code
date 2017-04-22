//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Lucas Baudin
//            (C) 2014-2015 Joerg Henrichs
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

#include "online/request_manager.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/vs.hpp"

#include <iostream>
#include <stdio.h>
#include <memory.h>
#include <errno.h>

#if defined(WIN32) && !defined(__CYGWIN__)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <sys/time.h>
#  include <math.h>
#endif

using namespace Online;

namespace Online
{
    RequestManager * RequestManager::m_request_manager = NULL;

    // ------------------------------------------------------------------------
    /** Deletes the http manager.
     */
    void RequestManager::deallocate()
    {
        if (m_request_manager!= NULL)
        {
            delete m_request_manager;
            m_request_manager = NULL;
        }
    }   // deallocate

    // ------------------------------------------------------------------------
    /** Checks if the http manager is running.
     */
    bool RequestManager::isRunning()
    {
        return m_request_manager != NULL;
    }   // isRunning
    // ------------------------------------------------------------------------
    /** Constructor. It only initialised values, it does not start the actual
     *  thread.
     */
    RequestManager::RequestManager()
    {
        m_menu_polling_interval = 60;  // Default polling: every 60 seconds.
        m_game_polling_interval = 60;  // same for game polling
        m_time_since_poll       = m_menu_polling_interval;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        pthread_cond_init(&m_cond_request, NULL);
        m_abort.setAtomic(false);
    }   // RequestManager

    // ------------------------------------------------------------------------
    RequestManager::~RequestManager()
    {
        m_thread_id.lock();
        pthread_join(*m_thread_id.getData(), NULL);
        delete m_thread_id.getData();
        m_thread_id.unlock();
        pthread_cond_destroy(&m_cond_request);
        curl_global_cleanup();
    }   // ~RequestManager

    // ------------------------------------------------------------------------
    /** Start the actual network thread. This can not be done as part of
     *  the constructor, since the assignment to the global network_http
     *  variable has not been assigned at that stage, and the thread might
     *  use network_http - a very subtle race condition. So the thread can
     *  only be started after the assignment (in main) has been done.
     *  \pre PlayerManager was created and has read the main data for each
     *                     player so that all data for automatic login is
     *                     availale.
     */
    void RequestManager::startNetworkThread()
    {
        pthread_attr_t  attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        m_thread_id.setAtomic(new pthread_t());
        int error = pthread_create(m_thread_id.getData(), &attr,
                                   &RequestManager::mainLoop, this);
        if (error)
        {
            m_thread_id.lock();
            delete m_thread_id.getData();
            m_thread_id.unlock();
            m_thread_id.setAtomic(0);
            Log::error("HTTP Manager", "Could not create thread, error=%d.",
                       errno);
        }
        pthread_attr_destroy(&attr);

        // In case that login id was not saved (or first start of stk),
        // current player would not be defined at this stage.
        PlayerProfile *player = PlayerManager::getCurrentPlayer();
        if (player && player->wasOnlineLastTime() &&
            !UserConfigParams::m_always_show_login_screen)
        {
            PlayerManager::resumeSavedSession();
        }
    }   // startNetworkThread

    // ------------------------------------------------------------------------
    /** This function inserts a high priority request to quit into the request
     *  queue of the network thead, and also aborts any ongoing download.
     *  Separating this allows more time for the thread to finish cleanly,
     *  before it gets cancelled in the destructor.
     */
    void RequestManager::stopNetworkThread()
    {
        // This will queue a sign-out or client-quit request
        PlayerManager::onSTKQuit();

        // Put in a high priortity quit request in. It has the same priority
        // as a sign-out request (so the sign-out will be executed before the
        // quit request).
        Request *quit = new Request(true, HTTP_MAX_PRIORITY, Request::RT_QUIT);
        quit->setAbortable(false);
        addRequest(quit);

        // It is possible that downloads are still ongoing (either an addon
        // download that the user aborted, or the addon icons etc are still
        // queued). In order to allow a quick exit of stk we set a flag that
        // will cause libcurl to abort downloading asap, and then allow the
        // other requests (sign-out and quit) to be executed asap. Note that
        // the sign-out request is set to be not abortable, so it still will
        // be executed (before the quit request is executed, which causes this
        // thread to exit).
        m_abort.setAtomic(true);
    }   // stopNetworkThread

    // ------------------------------------------------------------------------
    /** Inserts a request into the queue of all requests. The request will be
     *  sorted by priority.
     *  \param request The pointer to the new request to insert.
     */
    void RequestManager::addRequest(Request *request)
    {
        assert(request->isPreparing());
        request->setBusy();
        m_request_queue.lock();
        m_request_queue.getData().push(request);

        // Wake up the network http thread
        pthread_cond_signal(&m_cond_request);
        m_request_queue.unlock();
    }   // addRequest

    // ------------------------------------------------------------------------
    /** The actual main loop, which is started as a separate thread from the
     *  constructor. After testing for a new server, fetching news, the list
     *  of packages to download, it will wait for commands to be issued.
     *  \param obj: A pointer to this object, passed on by pthread_create
     */
    void *RequestManager::mainLoop(void *obj)
    {
        VS::setThreadName("RequestManager");
        RequestManager *me = (RequestManager*) obj;

        me->m_current_request = NULL;
        me->m_request_queue.lock();
        while (me->m_request_queue.getData().empty() ||
               me->m_request_queue.getData().top()->getType() != Request::RT_QUIT)
        {
            bool empty = me->m_request_queue.getData().empty();

            // Wait in cond_wait for a request to arrive. The 'while' is necessary
            // since "spurious wakeups from the pthread_cond_wait ... may occur"
            // (pthread_cond_wait man page)!
            while (empty)
            {
                pthread_cond_wait(&me->m_cond_request, me->m_request_queue.getMutex());
                empty = me->m_request_queue.getData().empty();
            }
            me->m_current_request = me->m_request_queue.getData().top();
            me->m_request_queue.getData().pop();

            if (me->m_current_request->getType() == Request::RT_QUIT)
            {
                delete me->m_current_request;
                break;
            }

            me->m_request_queue.unlock();
            me->m_current_request->execute();
            // This test is necessary in case that execute() was aborted
            // (otherwise the assert in addResult will be triggered).
            if (!me->getAbort()) me->addResult(me->m_current_request);
            me->m_request_queue.lock();
        } // while handle all requests

        // Signal that the request manager can now be deleted.
        // We signal this even before cleaning up memory, since there's no
        // need to keep the user waiting for STK to exit.
        me->setCanBeDeleted();

        // At this stage we have the lock for m_request_queue
        while (!me->m_request_queue.getData().empty())
        {
            Online::Request *request = me->m_request_queue.getData().top();
            me->m_request_queue.getData().pop();

            // Manage memory can be ignored here, all requests
            // need to be freed.
            delete request;
        }
        me->m_request_queue.unlock();
        pthread_exit(NULL);

        return 0;
    }   // mainLoop

    // ------------------------------------------------------------------------
    /** Inserts a request into the queue of results.
     *  \param request The pointer to the request to insert.
     */
    void RequestManager::addResult(Online::Request *request)
    {
        assert(request->hasBeenExecuted());
        m_result_queue.lock();
        m_result_queue.getData().push(request);
        m_result_queue.unlock();
    }   // addResult

    // ------------------------------------------------------------------------
    /** Takes a request out of the result queue, if any is present.
     *  Calls the callback method of the request and takes care of memory
     *  management if necessary.
     */
    void RequestManager::handleResultQueue()
    {
        Request * request = NULL;
        m_result_queue.lock();
        if (!m_result_queue.getData().empty())
        {
            request = m_result_queue.getData().front();
            m_result_queue.getData().pop();
        }
        m_result_queue.unlock();
        if (request != NULL)
        {
            request->callback();
            if(request->manageMemory())
            {
                delete request;
                request = NULL;
            }
            else
                request->setDone();
        }
    }   // handleResultQueue

    // ------------------------------------------------------------------------
    /** Should be called every frame and takes care of processing the result
     *  queue and polling the database server if a user is signed in.
     */
    void RequestManager::update(float dt)
    {
        handleResultQueue();

        // Database polling starts here, only needed for registered users. If
        // there is no player data yet (i.e. either because first time start
        // of stk, and loging screen hasn't finished yet, or no default player
        // was saved), don't do anything
        if (!PlayerManager::getCurrentPlayer() ||
            !PlayerManager::isCurrentLoggedIn())
            return;

        m_time_since_poll += dt;
        float interval = m_game_polling_interval;
        if (StateManager::get()->getGameState() == GUIEngine::MENU)
                interval = m_menu_polling_interval;

        if (m_time_since_poll > interval)
        {
            m_time_since_poll = 0;
            PlayerManager::requestOnlinePoll();
        }

    }   // update
} // namespace Online
