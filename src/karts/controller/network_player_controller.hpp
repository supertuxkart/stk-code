//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 Joerg Henrichs
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

#ifndef HEADER_NETWORK_PLAYER_CONTROLLER_HPP
#define HEADER_NETWORK_PLAYER_CONTROLLER_HPP

#include "karts/controller/player_controller.hpp"

class AbstractKart;
class Player;

class NetworkPlayerController : public PlayerController
{
public:
    NetworkPlayerController(AbstractKart *kart) : PlayerController(kart)
    {
        Log::info("NetworkPlayerController",
                  "New network player controller.");
    }   // NetworkPlayerController
    // ------------------------------------------------------------------------
    virtual ~NetworkPlayerController()
    {
    }   // ~NetworkPlayerController
    // ------------------------------------------------------------------------
    /** This player is not a local player. This affect e.g. special sfx and
     *  camera effects to be triggered. */
    virtual bool isLocalPlayerController() const OVERRIDE
    {
        return false; 
    }   // isLocal
    // ------------------------------------------------------------------------
};   // NetworkPlayerController

#endif // NETWORK_PLAYER_CONTROLLER_HPP
