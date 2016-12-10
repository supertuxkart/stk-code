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

#include "input/input.hpp"
#include "utils/singleton.hpp"
#include <map>

class Controller;
class KartUpdateProtocol;
class AbstractKart;
class Item;

/** \brief This is the interface between the main game and the online
 *  implementation. The main game informs this object about important
 *  events (e.g. item collected), which need to be forwarded from the
 *  server to all clients. This object then triggers the right message
 *  from the various running protocols.
*/
class RaceEventManager : public AbstractSingleton<RaceEventManager>
{
private:
    bool m_running;
    float m_race_time;

    friend class AbstractSingleton<RaceEventManager>;

             RaceEventManager();
    virtual ~RaceEventManager();

public:
    void update(float dt);

    void start();
    void stop();
    bool isRaceOver();

    void collectedItem(Item *item, AbstractKart *kart);
    void controllerAction(Controller* controller, PlayerAction action, 
                          int value);
    void kartFinishedRace(AbstractKart *kart, float time);
    // ------------------------------------------------------------------------
    /** Returns if this instance is in running state or not. */
    bool isRunning() { return m_running; }

};

#endif // NETWORK_WORLD_HPP
