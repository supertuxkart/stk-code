//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 SuperTuxKart-Team
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

#ifndef HEADER_OVERWORLD_HPP
#define HEADER_OVERWORLD_HPP

#include <vector>

#include "modes/world.hpp"
#include "utils/aligned_array.hpp"

#include "LinearMath/btTransform.h"

/*
 * The overworld map where challenges are played.
 * \note Extends world to make a simple world where karts can drive around,
 *       it adds challenges and starting of races.
 * \ingroup modes
 */
class OverWorld : public World
{
protected:

    /** Override from base class */
    virtual void  createRaceGUI() OVERRIDE;

    bool m_return_to_garage;

public:
                  OverWorld();
    virtual      ~OverWorld();

    static void enterOverWorld();

    virtual void  update(int ticks) OVERRIDE;
    unsigned int  getRescuePositionIndex(AbstractKart *kart) OVERRIDE;
    virtual void  getKartsDisplayInfo(
                 std::vector<RaceGUIBase::KartIconDisplayInfo> *info) OVERRIDE;
    // ------------------------------------------------------------------------
    /** Returns if this race mode has laps. */
    virtual bool  raceHasLaps() OVERRIDE { return false; }
    // ------------------------------------------------------------------------
    /** The overworld is not a race per se so it's never over */
    virtual bool    isRaceOver() OVERRIDE { return false; }
    // ------------------------------------------------------------------------
    /** Implement base class method */
    virtual const std::string&
                    getIdent() const OVERRIDE { return IDENT_OVERWORLD; }
    // ------------------------------------------------------------------------
    /** Override base class method */
    virtual bool shouldDrawTimer() const OVERRIDE { return false; }
    // ------------------------------------------------------------------------
    /** Override base class method */
    virtual void onFirePressed(Controller* who) OVERRIDE;
    // ------------------------------------------------------------------------
    /** Override settings from base class */
    virtual bool useChecklineRequirements() const OVERRIDE { return false; }
    // ------------------------------------------------------------------------
    void scheduleSelectKart() { m_return_to_garage = true; }
    // ------------------------------------------------------------------------
    virtual void onMouseClick(int x, int y) OVERRIDE;
};

#endif
