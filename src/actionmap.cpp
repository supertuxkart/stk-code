//  $Id: actionmap.cpp 1259 2007-09-24 12:28:19Z thebohemian $
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

#include <map>

#include "input.hpp"
#include "actionmap.hpp"

using namespace std;

void
ActionMap::putEntry(Input input, GameAction ga)
{
	inputMap[key(input)] = ga;
}

GameAction
ActionMap::getEntry(Input input)
{
	return inputMap[key(input)];
}

GameAction
ActionMap::getEntry(InputType type, int id0, int id1, int id2)
{
	return inputMap[key(type, id0, id1, id2)];
}

void
ActionMap::clear()
{
    inputMap.clear();
}

ActionMap::Key
ActionMap::key(Input input)
{
	return key(input.type, input.id0, input.id1, input.id2);
}

ActionMap::Key
ActionMap::key(InputType it, int id0, int id1, int id2)
{
    /*
     * A short reminder on the bit distribution and their usage:
     * it gets 8 bits (InputType)
     * id1 gets 16 bits (button, hat or axis number)
     * id2 gets 8 bits (direction bit)
     * id0 gets 32 bits (That is because id0 can be the keyboard key ids and
     * those are unicode and unicode 4.0 is 32 bit)
     *
     * Assumption: int is (at least) 32 bit
     */
    return Key(it << 24 | id1 << 8 | id2, id0);
}

