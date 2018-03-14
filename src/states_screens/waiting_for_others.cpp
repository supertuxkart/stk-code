//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#include "states_screens/waiting_for_others.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/label_widget.hpp"

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( WaitingForOthersScreen );

// -----------------------------------------------------------------------------
WaitingForOthersScreen::WaitingForOthersScreen()
                      : Screen("online/waiting_for_others.stkgui")
{
}   // WaitingForOthersScreen

// -----------------------------------------------------------------------------
void WaitingForOthersScreen::loadedFromFile()
{
}   // loadedFromFile

// -----------------------------------------------------------------------------
void WaitingForOthersScreen::eventCallback(Widget* widget,
                                           const std::string& name,
                                           const int player_id)
{
}   // eventCallback

// -----------------------------------------------------------------------------
void WaitingForOthersScreen::init()
{
    Screen::init();
}   //init

// -----------------------------------------------------------------------------
void WaitingForOthersScreen::onUpdate(float dt)
{
}
