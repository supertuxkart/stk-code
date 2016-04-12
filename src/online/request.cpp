//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#include "online/request.hpp"

#include "config/user_config.hpp"
#include "online/request_manager.hpp"

#ifdef WIN32
#  include <winsock2.h>
#endif

#include <assert.h>

namespace Online
{
    // ========================================================================
    /**
     *  Creates a request that can be handled by the RequestManager
     *  \param manage_memory whether or not the RequestManager should take care of
     *         deleting the object after all callbacks have been done
     *  \param priority by what priority should the RequestManager take care of
     *         this request
     *  \param type indicates whether the request has a special task for the
     *         RequestManager
     */
    Request::Request(bool manage_memory, int priority, int type)
        : m_type(type), m_manage_memory(manage_memory), m_priority(priority)
    {
        m_cancel.setAtomic(false);
        m_state.setAtomic(S_PREPARING);
        m_is_abortable.setAtomic(true);
    }   // Request

    // ------------------------------------------------------------------------
    /** Inserts this request into the RequestManager's queue for executing.
     */
    void Request::queue()
    {
        RequestManager::get()->addRequest(this);
    }   // queue

    // ------------------------------------------------------------------------
    /** Executes the request. This calles prepareOperation, operation, and
     *  afterOperation.
     */
    void Request::execute()
    {
        assert(isBusy());
        // Abort as early as possible if abort is requested
        if (RequestManager::get()->getAbort() && isAbortable()) return;
        prepareOperation();
        if (RequestManager::get()->getAbort() && isAbortable()) return;
        operation();
        if (RequestManager::get()->getAbort() && isAbortable()) return;
        setExecuted();
        if (RequestManager::get()->getAbort() && isAbortable()) return;
        afterOperation();
    }   // execute

    // ------------------------------------------------------------------------
    /** Executes the request now, i.e. in the main thread and without involving
     *  the manager thread.. This calles prepareOperation, operation, and
     *  afterOperation.
     */
    void Request::executeNow()
    {
        assert(isPreparing());
        setBusy();
        execute();
        callback();
        setDone();
    }   // executeNow

} // namespace Online
