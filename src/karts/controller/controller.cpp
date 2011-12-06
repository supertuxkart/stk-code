//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010      Joerg Henrichs
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


//The AI debugging works best with just 1 AI kart, so set the number of karts
//to 2 in main.cpp with quickstart and run supertuxkart with the arg -N.

#include "karts/controller/controller.hpp"

#include "karts/kart.hpp"

/** Constructor, saves the kart pointer and a pointer to the KartControl
 *  of the kart.
 */
Controller::Controller(Kart *kart, StateManager::ActivePlayer *player)
{
    m_controls = &(kart->getControls());
    m_kart     = kart;
    m_player   = player;
}   // Controller

// ----------------------------------------------------------------------------
const irr::core::stringw& Controller::getNamePostfix() const
{
    // Static to avoid returning the address of a temporary stringq
    static irr::core::stringw name("");
    return name;
}   // getNamePostfix

// ----------------------------------------------------------------------------
