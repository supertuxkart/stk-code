//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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

#define DEBUG_MENU_ITEM 0

#include "states_screens/networking_lobby_settings.hpp"

#include <string>
#include <iostream>

#include "challenges/game_slot.hpp"
#include "challenges/unlock_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/scalable_font.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "main_loop.hpp"
#include "states_screens/online_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "modes/demo_world.hpp"
#include "utils/translation.hpp"

#include "online/current_online_user.hpp"


using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( NetworkingLobbySettings );

// ----------------------------------------------------------------------------

NetworkingLobbySettings::NetworkingLobbySettings() : Screen("online/lobby_settings.stkgui")
{

}   // NetworkingLobbySettings

// ----------------------------------------------------------------------------

void NetworkingLobbySettings::loadedFromFile()
{

}   // loadedFromFile

// ----------------------------------------------------------------------------
bool NetworkingLobbySettings::hasLostConnection()
{
    return !CurrentOnlineUser::get()->isSignedIn();
}

// ----------------------------------------------------------------------------
void NetworkingLobbySettings::beforeAddingWidget()
{

} // beforeAddingWidget



// ----------------------------------------------------------------------------
void NetworkingLobbySettings::init()
{
    Screen::init();
    setInitialFocus();
    DemoWorld::resetIdleTime(); //FIXME : what's this?}   // init
}
// ----------------------------------------------------------------------------
void NetworkingLobbySettings::onUpdate(float delta,  irr::video::IVideoDriver* driver)
{
}   // onUpdate

// ----------------------------------------------------------------------------

void NetworkingLobbySettings::eventCallback(Widget* widget, const std::string& name, const int playerID)
{

}   // eventCallback

// ----------------------------------------------------------------------------

void NetworkingLobbySettings::tearDown()
{
}   // tearDown

// ----------------------------------------------------------------------------
void NetworkingLobbySettings::onDisabledItemClicked(const std::string& item)
{

}   // onDisabledItemClicked

// ----------------------------------------------------------------------------
void NetworkingLobbySettings::setInitialFocus()
{
}   // setInitialFocus

// ----------------------------------------------------------------------------
void NetworkingLobbySettings::onDialogClose()
{
    setInitialFocus();
}   // onDialogClose()
