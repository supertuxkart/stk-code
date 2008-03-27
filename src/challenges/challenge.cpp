//  $Id: challenge.cpp 1259 2007-09-24 12:28:19Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include "challenges/challenge.hpp"
#include "unlock_manager.hpp"

Challenge::Challenge(std::string id, std::string name) : 
    m_state(CH_INACTIVE), m_Id(id), m_Name(name)
{
}   // Challenge

//-----------------------------------------------------------------------------
/** Loads the state for a challenge object (esp. m_state), and calls the 
 *  virtual function loadState for additional information
 */
void Challenge::load(const lisp::Lisp* config)
{
    const lisp::Lisp* subnode= config->getLisp(getId());
    if(!subnode) return;
    
    // See if the challenge is solved (it's activated later from the
    // unlock_manager).
    bool finished=false;
    subnode->get("solved", finished);
    m_state = finished ? CH_SOLVED : CH_INACTIVE;
    if(!finished) loadState(subnode);
}   // load

//-----------------------------------------------------------------------------
void Challenge::save(lisp::Writer* writer)
{
    writer->beginList(getId());
    writer->write("solved", isSolved());
    if(!isSolved()) saveState(writer);
    writer->endList(getId());
}   // save

//-----------------------------------------------------------------------------
