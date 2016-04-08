//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015  Supertuxkart-Team
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


#ifndef CONTROLLER_EVENTS_PROTOCOL_HPP
#define CONTROLLER_EVENTS_PROTOCOL_HPP

#include "network/protocol.hpp"

#include "input/input.hpp"
#include "utils/cpp2011.hpp"

class Controller;
class STKPeer;

class ControllerEventsProtocol : public Protocol
{

public:
             ControllerEventsProtocol();
    virtual ~ControllerEventsProtocol();

    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual void update(float dt) OVERRIDE {};
    virtual void setup() OVERRIDE {};
    virtual void asynchronousUpdate() OVERRIDE {}

    void controllerAction(Controller* controller, PlayerAction action,
                          int value);

};   // class ControllerEventsProtocol

#endif // CONTROLLER_EVENTS_PROTOCOL_HPP
