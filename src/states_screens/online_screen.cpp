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

#include "states_screens/online_screen.hpp"

#include <string>
#include <iostream>

#include "challenges/game_slot.hpp"
#include "challenges/unlock_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "main_loop.hpp"
#include "states_screens/online_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "modes/demo_world.hpp"
#include "utils/translation.hpp"

#include "online/current_online_user.hpp"
#include "states_screens/dialogs/login_dialog.hpp"

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( OnlineScreen );

// ----------------------------------------------------------------------------

OnlineScreen::OnlineScreen() : Screen("online/main.stkgui")
{
}   // OnlineScreen

// ----------------------------------------------------------------------------

void OnlineScreen::loadedFromFile()
{

}   // loadedFromFile

// ----------------------------------------------------------------------------
//
void OnlineScreen::init()
{
    Screen::init();
    RibbonWidget* r = getWidget<RibbonWidget>("menu_toprow");
    r->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    DemoWorld::resetIdleTime();

}   // init


void OnlineScreen::beforeAddingWidget()
{
    RibbonWidget* topRow = getWidget<RibbonWidget>("menu_toprow");
    assert(topRow != NULL);
    RibbonWidget* bottomRow = getWidget<RibbonWidget>("menu_bottomrow");
    assert(bottomRow != NULL);
    if(CurrentOnlineUser::get()->isSignedIn())
    {
        if(CurrentOnlineUser::get()->isGuest())
        {

        }
        else
        {
            //Signed in and not guest
            bottomRow->removeChildNamed("sign_in");
        }
    }
    else
    {
        bottomRow->removeChildNamed("sign_out");
    }
}

// ----------------------------------------------------------------------------
void OnlineScreen::onUpdate(float delta,  irr::video::IVideoDriver* driver)
{

}   // onUpdate

// ----------------------------------------------------------------------------

void OnlineScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }

    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
    if (ribbon == NULL) return; // what's that event??
    std::string selection = ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);

    if (selection == "sign_in")
    {
        new LoginDialog(0.6f, 0.6f, _("Not yet an account? Press register beneath!"));
    }
    else if (selection == "sign_out")
    {
        CurrentOnlineUser::get()->signOut();
    }

}   // eventCallback

// ----------------------------------------------------------------------------

void OnlineScreen::tearDown()
{
}   // tearDown

// ----------------------------------------------------------------------------

void OnlineScreen::onDisabledItemClicked(const std::string& item)
{
}   // onDisabledItemClicked

// ----------------------------------------------------------------------------

void OnlineScreen::reload()
{
    this->unload();
    if (!this->isLoaded()) this->loadFromFile();
    this->init();
}
