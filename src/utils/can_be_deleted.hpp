//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Joerg Henrichs
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

#ifndef HEADER_CAN_BE_DELETED
#define HEADER_CAN_BE_DELETED

#include "utils/log.hpp"
#include "utils/synchronised.hpp"
#include "utils/time.hpp"

/** A simple class that a adds a function to wait with a timeout for a
 *  class to be ready to be deleted. It is used for objects with their
 *  own threads (e.g. RequestManager) to make sure they can be deleted.
 *  For example, the RequestManager might be executing a download request.
 *  So we have to signal libcurl to abort the download request, then
 *  potentially handle a high priority sign-out request before the thread
 *  can be deleted. With this object the main thread can wait for a given
 *  amount of time (in case that of a bad internet hickup) before deleting
 *  the RequestManager.
 */
class CanBeDeleted
{
private:
    Synchronised<bool> m_can_be_deleted;
public:
    /** Set this instance to be not ready to be deleted. */
    CanBeDeleted() { m_can_be_deleted.setAtomic(false); }
    // ------------------------------------------------------------------------
    /** Sets this instance to be ready to be deleted. */
    void setCanBeDeleted() {m_can_be_deleted.setAtomic(true); }
    // ------------------------------------------------------------------------
    /** Waits at most t seconds for this class to be ready to be deleted.
     *  \return true if the class is ready, false in case of a time out.
     */
    bool waitForReadyToDeleted(float waiting_time)
    {
        if (m_can_be_deleted.getAtomic()) return true;
        double start = StkTime::getRealTime();
        Log::verbose("Thread", "Start waiting %lf", start);
        while(1)
        {
            if(m_can_be_deleted.getAtomic())
            {
                Log::verbose("Thread",
                         "Waited %lf seconds for thread to become deleteable.",
                         StkTime::getRealTime()-start);
                Log::verbose("Thread", "Stop waiting %lf", StkTime::getRealTime());
                return true;
            }
            StkTime::sleep(10);
            if(StkTime::getRealTime() - start > waiting_time)
            {
                Log::verbose("Thread", "Stop waiting %lf", StkTime::getRealTime());
                Log::verbose("Thread", "Waited for more than %f seconds for "
                                       "thread to become deleteable",
                                       waiting_time);
                return false;
            }
        }   // while 1
    }   // waitForReadyToDeleted

    // ------------------------------------------------------------------------
};   // CanBeDeleted
#endif
