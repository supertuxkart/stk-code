//  $Id: actionmap.hpp 1259 2007-09-24 12:28:19Z thebohemian $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Robert Schuster
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

#ifndef HEADER_ACTIONMAP_H
#define HEADER_ACTIONMAP_H

#include <map>
#include "input.hpp"

/**
  * This thing is named after what is put in as values.
  */
class ActionMap
{
  typedef std::pair<int, int> Key;

  public:

    void clear();

    void putEntry(Input, GameAction);

    GameAction getEntry(Input);
    GameAction getEntry(InputType, int, int, int);

  private:
    inline Key key(Input);
    Key key(InputType, int, int, int);

    std::map<Key, GameAction> inputMap;

};

#endif
