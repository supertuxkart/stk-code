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

#include "online/database_polling.hpp"

#include <string>
#include <irrString.h>
#include <assert.h>
#include "utils/translation.hpp"
#include "utils/time.hpp"
#include "online/current_user.hpp"

#define MENU_POLLING_INTERVAL 5.0f
#define RACE_POLLING_INTERVAL 10.0f

namespace Online{

    static DatabasePolling * database_polling_singleton(NULL);

    DatabasePolling* DatabasePolling::get()
    {
        if (database_polling_singleton == NULL)
            database_polling_singleton = new DatabasePolling();
        return database_polling_singleton;
    }

    void DatabasePolling::deallocate()
    {
        delete database_polling_singleton;
        database_polling_singleton = NULL;
    }   // deallocate

    // ============================================================================
    DatabasePolling::DatabasePolling(){
        m_time_since_poll = 0.0f;
    }

    DatabasePolling::~DatabasePolling(){

    }

    void DatabasePolling::generateNewPollRequest(){

    }

    void DatabasePolling::addResult(Online::Request *request)
    {
        assert(request->isDone());
        m_result_queue.lock();
        m_result_queue.getData().push(request);
        m_result_queue.unlock();
    }

    DatabasePolling::PollRequest * DatabasePolling::popResult()
    {
        PollRequest * request = NULL;
        m_result_queue.lock();
        if(!m_result_queue.getData().empty())
        {
            request = (PollRequest*) m_result_queue.getData().front();
            m_result_queue.getData().pop();
        }
        m_result_queue.unlock();
        return request;
    }

    void DatabasePolling::update(float dt){
        if(!CurrentUser::get()->isRegisteredUser())
            return;
        PollRequest * request = popResult();
        if(request != NULL)
            request->onPollFetch();

        m_time_since_poll += dt;
        float interval = MENU_POLLING_INTERVAL;
        if(m_time_since_poll > interval)
        {
            m_time_since_poll = 0;
            generateNewPollRequest();
        }

    }

} // namespace Online
