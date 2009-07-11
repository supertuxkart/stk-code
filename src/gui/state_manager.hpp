//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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


#ifndef STATE_MANAGER_HPP
#define STATE_MANAGER_HPP

#include <string>
#include "utils/ptr_vector.hpp"

struct Input;
class ActivePlayer;

namespace StateManager
{
    void initGUI();
    
    void pushMenu(std::string name);
    void replaceTopMostMenu(std::string name);
    void popMenu();
    void resetAndGoToMenu(std::string name);
    void enterGameState();
    bool isGameState();
    void reshowTopMostMenu();
    
    const ptr_vector<ActivePlayer, HOLD>& getActivePlayers();
    
    /**
      * Adds a new player to the list of active players. StateManager takes ownership of the object
      * so no need to delete it yourself.
      */
    void addActivePlayer(ActivePlayer* p);
    
    void resetActivePlayers();
    
    void escapePressed();
}

#endif
