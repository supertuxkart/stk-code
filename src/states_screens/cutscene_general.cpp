//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 SuperTuxKart-Team
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

#include "states_screens/cutscene_general.hpp"
#include "modes/cutscene_world.hpp"

CutSceneGeneral::CutSceneGeneral() : CutsceneScreen("cutscene.stkgui")
{
}  // CutSceneGeneral

// ----------------------------------------------------------------------------

void CutSceneGeneral::loadedFromFile()
{
}   // loadedFromFile

// ----------------------------------------------------------------------------

void CutSceneGeneral::init()
{
}   // init

// ----------------------------------------------------------------------------

void CutSceneGeneral::tearDown()
{
    Screen::tearDown();
}   // tearDown

// ----------------------------------------------------------------------------

bool CutSceneGeneral::onEscapePressed()
{
    ((CutsceneWorld*)World::getWorld())->abortCutscene();
    return false;
}   // onEscapePressed

// ----------------------------------------------------------------------------

void CutSceneGeneral::eventCallback(GUIEngine::Widget* widget,
                                            const std::string& name,
                                            const int playerID)
{
    if (name == "continue")
    {
        onEscapePressed();
    }
}   // eventCallback
