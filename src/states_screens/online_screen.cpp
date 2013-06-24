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

#include "addons/inetwork_http.hpp"
#include "challenges/game_slot.hpp"
#include "challenges/unlock_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "main_loop.hpp"
#include "modes/cutscene_world.hpp"
#include "modes/overworld.hpp"
#include "modes/demo_world.hpp"
#include "network/network_manager.hpp"
#include "states_screens/online_screen.hpp"
#include "states_screens/addons_screen.hpp"
#include "states_screens/credits.hpp"
#include "states_screens/help_screen_1.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
#include "modes/demo_world.hpp"

#include "states_screens/dialogs/login_dialog.hpp"

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( OnlineScreen );

// ----------------------------------------------------------------------------

OnlineScreen::OnlineScreen() : Screen("online.stkgui")
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

    // Avoid incorrect behaviour in certain race circumstances:
    // If a multi-player game is played with two keyboards, the 2nd
    // player selects his kart last, and only the keyboard is used
    // to select all other settings - then if the next time the kart
    // selection screen comes up, the default device will still be
    // the 2nd player. So if the first player presses 'select', it
    // will instead add a second player (so basically the key
    // binding for the second player become the default, so pressing
    // select will add a new player). See bug 3090931
    // To avoid this, we will clean the last used device, making
    // the key bindings for the first player the default again.
    input_manager->getDeviceList()->clearLatestUsedDevice();

    RibbonWidget* r = getWidget<RibbonWidget>("menu_toprow");
    r->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    DemoWorld::resetIdleTime();

#if _IRR_MATERIAL_MAX_TEXTURES_ < 8
    getWidget<IconButtonWidget>("logo")->setImage("gui/logo_broken.png",
        IconButtonWidget::ICON_PATH_TYPE_RELATIVE);
#endif

}   // init

// ----------------------------------------------------------------------------
void OnlineScreen::onUpdate(float delta,  irr::video::IVideoDriver* driver)
{

}   // onUpdate

// ----------------------------------------------------------------------------

void OnlineScreen::eventCallback(Widget* widget, const std::string& name,
                                   const int playerID)
{
    // most interesting stuff is in the ribbons, so start there
    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);

    if (ribbon == NULL) return; // what's that event??

    // ---- A ribbon icon was clicked
    std::string selection =
        ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);

    if (selection == "login")
    {
        new LoginDialog(0.8f, 0.7f);
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
