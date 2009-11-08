//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006  Eduardo Hernandez Munoz, Steve Baker
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

#ifndef HEADER_AUTOKART_HPP
#define HEADER_AUTOKART_HPP

#include "irrlicht.h"

#include "karts/kart.hpp"
#include "karts/kart_properties.hpp"

class AutoKart : public Kart
{
public:
    AutoKart(const std::string& kart_name, int position,
        const btTransform& init_pos) :
    Kart(kart_name, position, init_pos) {}

    bool  isPlayerKart() const {return false;}
    virtual const irr::core::stringw& getName() const 
    {
        // Static to avoid returning the address of a temporary
        // string.
        static irr::core::stringw name = Kart::getName()+"(auto)";
        return name;
    }   // getName
};

#endif

/* EOF */
