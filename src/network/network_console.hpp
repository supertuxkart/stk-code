//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

#ifndef HEADER_NETWORK_CONSOLE_HPP
#define HEADER_NETWORK_CONSOLE_HPP

#include "utils/types.hpp"

#include "pthread.h"

class NetworkString;
class STKHost;

class NetworkConsole
{
protected:

    STKHost *m_localhost;

    pthread_t* m_thread_keyboard;

    uint8_t m_max_players;

    static void* mainLoop(void* data);

public:
             NetworkConsole();
    virtual ~NetworkConsole();

    virtual void run();
    void kickAllPlayers();
    // ------------------------------------------------------------------------
    void setMaxPlayers(uint8_t count) { m_max_players = count; }
    // ------------------------------------------------------------------------
    uint8_t getMaxPlayers() { return m_max_players; }
    // ------------------------------------------------------------------------
    virtual bool isServer() { return true; }

};   // class NetworkConsole

#endif // SERVER_CONSOLE_HPP
