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

#include "physics/physics.hpp"

/** Constructor for a state: it only takes the size, and allocates a buffer
 *  for all state info.
 *  \param size Necessary buffer size for a state.
 */
RewindInfo::RewindInfo(float time, bool is_confirmed)
{
    m_time         = time;
    m_is_confirmed = is_confirmed;
}   // RewindInfo

// ============================================================================
RewindInfoTime::RewindInfoTime(float time)
              : RewindInfo(time, /*is_confirmed*/true)
{
}   // RewindInfoTime

// ============================================================================
RewindInfoState::RewindInfoState(float time, Rewinder *rewinder, 
                                 BareNetworkString *buffer, bool is_confirmed)
    : RewindInfoRewinder(time, rewinder, buffer, is_confirmed)
{
    m_local_physics_time = Physics::getInstance()->getPhysicsWorld()
                                                 ->getLocalTime();
}   // RewindInfoState

// ============================================================================
RewindInfoEvent::RewindInfoEvent(float time, EventRewinder *event_rewinder,
                                 BareNetworkString *buffer, bool is_confirmed)
    : RewindInfo(time, is_confirmed)
{
    m_event_rewinder = event_rewinder;
    m_buffer         = buffer;
}   // RewindInfoEvent

