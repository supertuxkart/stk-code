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

class Controller;
class STKPeer;

class ControllerEventsProtocol : public Protocol
{
protected:
    std::vector<std::pair<Controller*, STKPeer*> > m_controllers;
    uint32_t m_self_controller_index;

public:
    ControllerEventsProtocol();
    virtual ~ControllerEventsProtocol();

    virtual bool notifyEventAsynchronous(Event* event);
    virtual void setup();
    virtual void update();
    virtual void asynchronousUpdate() {}

    void controllerAction(Controller* controller, PlayerAction action,
                          int value);

};   // class ControllerEventsProtocol

#endif // CONTROLLER_EVENTS_PROTOCOL_HPP
