//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Joerg Henrichs
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

#ifndef HEADER_REWINDER_HPP
#define HEADER_REWINDER_HPP

class BareNetworkString;

class Rewinder
{
private:
    bool m_can_be_destroyed;
public:
 	        Rewinder(bool can_be_destroyed);
    virtual ~Rewinder();

    /** Provides a copy of the state of the object in one memory buffer.
     *  The memory is managed by the RewindManager.
     *  \param[out] buffer The address of the memory buffer with the state.
     *  \return Size of the buffer, or -1 in case of an error.
     */
    virtual BareNetworkString* saveState() const  = 0;

    /** Called when an event needs to be undone. This is called while going
     *  backwards for rewinding - all stored events will get an 'undo' call.
     */
    virtual void undoEvent(BareNetworkString *buffer) = 0;

    /** Called when an event needs to be replayed. This is called during 
     *  rewind, i.e. when going forward in time again.
     */
    virtual void rewindToEvent(BareNetworkString *buffer) = 0;

    /** Called when a state needs to be replayed. This is called during 
     *  rewind, i.e. when going forward in time again, and only for confirmed
     *  states.
     */
    virtual void rewindToState(BareNetworkString *buffer) = 0;

   /** Undo the effects of the given state, but do not rewind to that 
    *  state (which is done by rewindTo). This is called while going
    *  backwards for rewinding - all stored events will get an 'undo' call.
    */
   virtual void undoState(BareNetworkString *buffer) = 0;

   // -------------------------------------------------------------------------
   /** Nothing to do here. */
   virtual void reset() {};
   // -------------------------------------------------------------------------
   /** True if this rewinder can be destroyed. Karts can not be destroyed,
    *  cakes can. This is used by the RewindManager in reset. */
   bool canBeDestroyed() const { return m_can_be_destroyed; }

};   // Rewinder
#endif

