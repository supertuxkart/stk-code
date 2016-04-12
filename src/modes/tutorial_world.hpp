//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2012-2015 Marianne Gagnon
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

#ifndef HEADER_TUTORIAL_MODE_HPP
#define HEADER_TUTORIAL_MODE_HPP

#include "modes/standard_race.hpp"
#include "LinearMath/btTransform.h"

class TutorialWorld : public StandardRace
{
public:

    TutorialWorld();
    virtual unsigned int getNumberOfRescuePositions() const OVERRIDE
    {
        // Don't use LinearWorld's function, but WorldWithRank, since the
        // latter is based on rescuing to start positions
        return WorldWithRank::getNumberOfRescuePositions();
    }
    // ------------------------------------------------------------------------
    /** Determines the rescue position index of the specified kart. */
    virtual unsigned int getRescuePositionIndex(AbstractKart *kart) OVERRIDE;
    // ------------------------------------------------------------------------
    /** Returns the bullet transformation for the specified rescue index. */
    virtual btTransform getRescueTransform(unsigned int index) const OVERRIDE
    {
        // Don't use LinearWorld's function, but WorldWithRank, since the
        // latter is based on rescuing to start positions
        return WorldWithRank::getRescueTransform(index);
    }



};   // class TutorialWorld

#endif
