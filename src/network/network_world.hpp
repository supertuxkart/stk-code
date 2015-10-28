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

/*! \file network_world.hpp
 */

#ifndef NETWORK_WORLD_HPP
#define NETWORK_WORLD_HPP

#include "input/input.hpp"
#include "utils/singleton.hpp"
#include <map>

class Controller;
class KartUpdateProtocol;
class AbstractKart;
class Item;

/*! \brief Manages the world updates during an online game
 *  This function's update is to be called instead of the normal World update
*/
class NetworkWorld : public AbstractSingleton<NetworkWorld>
{
    friend class AbstractSingleton<NetworkWorld>;
    public:
        void update(float dt);

        void start();
        void stop();
        bool isRunning() { return m_running; }
        bool isRaceOver();

        void collectedItem(Item *item, AbstractKart *kart);
        void controllerAction(Controller* controller, PlayerAction action, int value);

        std::string m_self_kart;
    protected:
        bool m_running;
        float m_race_time;
        bool m_has_run;

    private:
        NetworkWorld();
        virtual ~NetworkWorld();
};

#endif // NETWORK_WORLD_HPP
