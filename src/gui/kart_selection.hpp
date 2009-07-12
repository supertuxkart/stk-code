//  $Id: font.cpp 3625 2009-06-21 01:10:43Z auria $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 
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

#include <string>

namespace GUIEngine
{
    class Widget;
}
class InputDevice;
class ActivePlayer;

namespace StateManager
{
    void setPlayer0Device(InputDevice* device);
    void firePressedOnNewDevice(InputDevice* device);
    void playerPressedRescue(ActivePlayer* player);
    
    void kartSelectionUpdate(float delta);
    
    void menuEventKarts(GUIEngine::Widget* widget, std::string& name);
}
