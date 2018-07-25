//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 Joerg Henrichs
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

#include "network/rewind_info.hpp"

#include "network/network_config.hpp"
#include "network/rewinder.hpp"
#include "network/rewind_manager.hpp"
#include "items/projectile_manager.hpp"

/** Constructor for a state: it only takes the size, and allocates a buffer
 *  for all state info.
 *  \param size Necessary buffer size for a state.
 */
RewindInfo::RewindInfo(int ticks, bool is_confirmed)
{
    m_ticks        = ticks;
    m_is_confirmed = is_confirmed;
}   // RewindInfo

// ----------------------------------------------------------------------------
/** Adjusts the time of this RewindInfo. This is only called on the server
 *  in case that an event is received in the past - in this case the server
 *  needs to avoid a Rewind by moving this event forward to the current time.
 */
void RewindInfo::setTicks(int ticks)
{
    assert(NetworkConfig::get()->isServer());
    assert(m_ticks < ticks);
    m_ticks = ticks;
}   // setTicks

// ============================================================================
RewindInfoState::RewindInfoState(int ticks, BareNetworkString *buffer,
                                 bool is_confirmed)
    : RewindInfo(ticks, is_confirmed)
{
    m_rewinder_using = RewindManager::get()->getRewinderUsing();
    m_buffer = buffer;
}   // RewindInfoState

// ------------------------------------------------------------------------
/** Rewinds to this state. This is called while going forwards in time
 *  again to reach current time. It will call rewindToState().
 *  if the state is a confirmed state.
 */
void RewindInfoState::restore()
{
    m_buffer->reset();
    for (const std::string& name : m_rewinder_using)
    {
        uint16_t count = m_buffer->getUInt16();
        std::shared_ptr<Rewinder> r =
            RewindManager::get()->getRewinder(name);
        if (!r)
        {
            // For now we only need to get missing rewinder from
            // projectile_manager
            r = projectile_manager->addRewinderFromNetworkState(name);
        }
        if (!r)
        {
            Log::error("RewindInfoState", "Missing rewinder %s",
                name.c_str());
            m_buffer->skip(count);
            continue;
        }
        r->restoreState(m_buffer, count);
    }   // for all rewinder
}   // restore

// ============================================================================
RewindInfoEvent::RewindInfoEvent(int ticks, EventRewinder *event_rewinder,
                                 BareNetworkString *buffer, bool is_confirmed)
               : RewindInfo(ticks, is_confirmed)
{
    m_event_rewinder = event_rewinder;
    m_buffer         = buffer;
}   // RewindInfoEvent

