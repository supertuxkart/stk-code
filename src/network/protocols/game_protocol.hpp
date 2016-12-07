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


#ifndef GAME_PROTOCOL_HPP
#define GAME_PROTOCOL_HPP

#include "network/protocol.hpp"

#include "input/input.hpp"                // for PlayerAction
#include "utils/cpp2011.hpp"
#include "utils/singleton.hpp"

#include <vector>

class NetworkString;


class GameProtocol : public Protocol
                   , public Singleton<GameProtocol>
{
private:

    /** The type of game events to be forwarded to the server. */
    enum { GP_CONTROLLER_ACTION  = 0x01};

    /** A network string that collects all information from the server to be sent
     *  next. */
    NetworkString *m_data_to_send;

    // Dummy data structure to save all kart actions.
    struct Action
    {
        float        m_time;
        int          m_kart_id;
        PlayerAction m_action;
        int          m_value;
    };   // struct Action

    // List of all kart actions to send to the server
    std::vector<Action> m_all_actions;
public:
             GameProtocol();
    virtual ~GameProtocol();

    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual void update(float dt) OVERRIDE;
    void controllerAction(int kart_id, PlayerAction action,
                          int value);
    // ------------------------------------------------------------------------
    virtual void setup() OVERRIDE {};
    // ------------------------------------------------------------------------
    virtual void asynchronousUpdate() OVERRIDE {}
    
};   // class GameProtocol

#endif // GAME_PROTOCOL_HPP
