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

#include "network/event_rewinder.hpp"
#include "network/protocol.hpp"

#include "input/input.hpp"                // for PlayerAction
#include "utils/cpp2011.hpp"
#include "utils/singleton.hpp"

#include <vector>

class BareNetworkString;
class NetworkString;
class STKPeer;

class GameProtocol : public Protocol
                   , public EventRewinder
{
private:

    /** The type of game events to be forwarded to the server. */
    enum { GP_CONTROLLER_ACTION,
           GP_STATE,
           GP_ADJUST_TIME
    };

    /** A network string that collects all information from the server to be sent
     *  next. */
    NetworkString *m_data_to_send;

    /** The server might request that the world clock of a client is adjusted
     *  to reduce number of rollbacks. */
    std::vector<int8_t> m_adjust_time;

    // Dummy data structure to save all kart actions.
    struct Action
    {
        int          m_ticks;
        int          m_kart_id;
        PlayerAction m_action;
        int          m_value;
        int          m_value_l;
        int          m_value_r;
    };   // struct Action

    // List of all kart actions to send to the server
    std::vector<Action> m_all_actions;

    void handleControllerAction(Event *event);
    void handleState(Event *event);
    void handleAdjustTime(Event *event);
    static std::weak_ptr<GameProtocol> m_game_protocol;
public:
             GameProtocol();
    virtual ~GameProtocol();

    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual void update(int ticks) OVERRIDE;

    void controllerAction(int kart_id, PlayerAction action,
                          int value, int val_l, int val_r);
    void startNewState(bool local_save);
    void addState(BareNetworkString *buffer);
    void sendState();
    void adjustTimeForClient(STKPeer *peer, int ticks);

    virtual void undo(BareNetworkString *buffer) OVERRIDE;
    virtual void rewind(BareNetworkString *buffer) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void setup() OVERRIDE {};
    // ------------------------------------------------------------------------
    virtual void asynchronousUpdate() OVERRIDE {}
    // ------------------------------------------------------------------------
    static std::shared_ptr<GameProtocol> createInstance();
    // ------------------------------------------------------------------------
    static bool emptyInstance()
    {
        return m_game_protocol.expired();
    }   // emptyInstance
    // ------------------------------------------------------------------------
    static std::shared_ptr<GameProtocol> lock()
    {
        return m_game_protocol.lock();
    }   // lock
    // ------------------------------------------------------------------------
    /** Returns the NetworkString in which a state was saved. */
    NetworkString* getState() const { return m_data_to_send;  }

};   // class GameProtocol

#endif // GAME_PROTOCOL_HPP
