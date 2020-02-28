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

#ifndef NETWORK_WORLD_HPP
#define NETWORK_WORLD_HPP

#include <memory>

class Controller;
class GameEventsProtocol;
class AbstractKart;
class Item;

/** \brief This is the interface between the main game and the online
 *  implementation. The main game informs this object about important
 *  events (e.g. item collected), which need to be forwarded from the
 *  server to all clients. This object then triggers the right message
 *  from the various running protocols.
*/
class RaceEventManager
{
private:
    bool m_running;

    std::weak_ptr<GameEventsProtocol> m_game_events_protocol;

    RaceEventManager();
    ~RaceEventManager();

public:
    // ----------------------------------------------------------------------------------------
    static RaceEventManager* get();
    // ----------------------------------------------------------------------------------------
    static void create();
    // ----------------------------------------------------------------------------------------
    static void destroy();
    // ----------------------------------------------------------------------------------------
    static void clear();
    // ------------------------------------------------------------------------
    void update(int ticks, bool fast_forward);
    // ------------------------------------------------------------------------
    void start(std::shared_ptr<GameEventsProtocol> gep)
    {
        m_game_events_protocol = gep;
        m_running = true;
    }
    // ------------------------------------------------------------------------
    void stop()                                          { m_running = false; }
    // ------------------------------------------------------------------------
    /** Returns if this instance is in running state or not. */
    bool isRunning()                                      { return m_running; }
    // ------------------------------------------------------------------------
    std::shared_ptr<GameEventsProtocol> getProtocol() const
                                      { return m_game_events_protocol.lock(); }
    // ------------------------------------------------------------------------
    bool protocolStopped() const   { return m_game_events_protocol.expired(); }
    // ------------------------------------------------------------------------
    bool isRaceOver();
    // ------------------------------------------------------------------------
    void kartFinishedRace(AbstractKart *kart, float time);

};

#endif // NETWORK_WORLD_HPP
