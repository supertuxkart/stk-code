//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010  Joerg Henrichs
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

#ifndef HEADER_CONTROLLER_HPP
#define HEADER_CONTROLLER_HPP

#include "irrlicht.h"
using namespace irr;

#include "karts/controller/kart_control.hpp"

class Kart;
class Item;

/** This is the base class for kart controller - that can be a player 
 *  or a a robot.
 */
class Controller
{
protected:
    /** Pointer to the kart that is controlled by this controller. */
    Kart         *m_kart;

    /** A pointer to the main controller, from which the kart takes
     *  it commands. */
    KartControl  *m_controls;

public:
                  Controller         (Kart *kart);
    virtual      ~Controller         () {};
    virtual void  reset              () {};
    virtual void  update             (float dt) {};
    virtual void  handleZipper       () {};
    virtual void  collectedItem      (const Item &item, int add_info=-1,
                                     float previous_energy=0) {};
    virtual void  crashed            () {};
    virtual void  setPosition        (int p) {};
    virtual void  finishedRace       (float time) {};
    virtual bool  isPlayerController () const {return false;}
    virtual bool  isNetworkController() const {return false;}
    virtual const irr::core::stringw& getNamePostfix() const;
};   // Controller

#endif

/* EOF */
