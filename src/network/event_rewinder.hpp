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

#ifndef HEADER_EVENT_REWINDER_HPP
#define HEADER_EVENT_REWINDER_HPP

class BareNetworkString;

/** A simple class that defines an interface to event rewinding: an undo()
 *  function when going back in time, and a replay() function when going
 *  forward, replaying the event.
 */
class EventRewinder
{
public:
 	         EventRewinder();
    virtual ~EventRewinder();

    /** Called when an event needs to be undone. This is called while going
     *  backwards for rewinding - all stored events will get an 'undo' call.
     */
    virtual void undo(BareNetworkString *buffer) = 0;

    /** Called when an event needs to be replayed. This is called during 
     *  rewind, i.e. when going forward in time again.
     */
    virtual void rewind(BareNetworkString *buffer) = 0;
};   // EventRewinder
#endif

