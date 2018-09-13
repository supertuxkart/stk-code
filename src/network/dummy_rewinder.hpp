//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#ifndef HEADER_DUMMY_REWINDER_HPP
#define HEADER_DUMMY_REWINDER_HPP

#include "network/event_rewinder.hpp"
#include "network/network_string.hpp"
#include "network/rewinder.hpp"

/** A dummy Rewinder and EventRewinder class for unit testing and handle undo
 *  destruction of projectiles. */
class DummyRewinder : public Rewinder, public EventRewinder
{
public:
    // -------------------------------------------------------------------------
    BareNetworkString* saveState(std::vector<std::string>* ru)  { return NULL; }
    // -------------------------------------------------------------------------
    virtual void undoEvent(BareNetworkString* s)                              {}
    // -------------------------------------------------------------------------
    virtual void rewindToEvent(BareNetworkString* s)                          {}
    // -------------------------------------------------------------------------
    virtual void restoreState(BareNetworkString* s, int count)
                                                             { s->skip(count); }
    // -------------------------------------------------------------------------
    virtual void undoState(BareNetworkString *s)                              {}
    // -------------------------------------------------------------------------
    virtual void undo(BareNetworkString *s)                                   {}
    // -------------------------------------------------------------------------
    virtual void rewind(BareNetworkString *s)                                 {}
    // -------------------------------------------------------------------------
    virtual void saveTransform()                                              {}
    // -------------------------------------------------------------------------
    virtual void computeError()                                               {}
};

#endif

