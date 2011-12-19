//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 SuperTuxKart-Team
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

#include "modes/linear_world.hpp"
#include "utils/aligned_array.hpp"


/*
 * The overworld map where challenges are played.
 * \note This mode derives from LinearWorld to get support for drivelines,
 *       minimap and rescue, even though this world is not technically
 *       linear.
 * \ingroup modes
 */
class OverWorld : public LinearWorld
{

public:
                  OverWorld();
   /** call just after instanciating. can't be moved to the contructor as child
       classes must be instanciated, otherwise polymorphism will fail and the
       results will be incorrect */
    virtual void  init();
    virtual      ~OverWorld();

    virtual void  update(float delta);
    
    // ------------------------------------------------------------------------    
    /** Returns if this race mode has laps. */
    virtual bool  raceHasLaps(){ return false; }
    // ------------------------------------------------------------------------    
    virtual void checkForWrongDirection(unsigned int i);
    // ------------------------------------------------------------------------    
    /** The overworld is not a race per se so it's never over */
    virtual bool    isRaceOver() { return false; }
    // ------------------------------------------------------------------------    
    virtual const std::string& 
                    getIdent() const { return IDENT_OVERWORLD; }
};

#endif
