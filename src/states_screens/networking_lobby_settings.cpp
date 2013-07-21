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
#include "audio/sfx_manager.hpp"
#include "states_screens/online_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "modes/demo_world.hpp"
#include "utils/translation.hpp"
#include "states_screens/networking_lobby.hpp"

#include "online/current_user.hpp"


using namespace GUIEngine;
using namespace Online;

DEFINE_SCREEN_SINGLETON( NetworkingLobbySettings );

// ----------------------------------------------------------------------------

NetworkingLobbySettings::NetworkingLobbySettings() : Screen("online/lobby_settings.stkgui")
{

}   // NetworkingLobbySettings

// ----------------------------------------------------------------------------

void NetworkingLobbySettings::loadedFromFile()
{


    m_name_widget = getWidget<TextBoxWidget>("name");
    assert(m_name_widget != NULL);
    m_max_players_widget = getWidget<SpinnerWidget>("max_players");
    assert(m_max_players_widget != NULL);

    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget != NULL);

    m_create_widget = getWidget<ButtonWidget>("create");
    assert(m_create_widget != NULL);
    m_cancel_widget = getWidget<ButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);

}   // loadedFromFile

// ----------------------------------------------------------------------------
void NetworkingLobbySettings::beforeAddingWidget()
{

} // beforeAddingWidget



// ----------------------------------------------------------------------------
void NetworkingLobbySettings::init()
{
    Screen::init();
    setInitialFocus();
    DemoWorld::resetIdleTime();
}
// ----------------------------------------------------------------------------
void NetworkingLobbySettings::onUpdate(float delta,  irr::video::IVideoDriver* driver)
{
}   // onUpdate


// ----------------------------------------------------------------------------
void NetworkingLobbySettings::createServer()
{
    const stringw name = m_name_widget->getText().trim();
    int max_players = m_max_players_widget->getValue();
    stringw info = "";
    /*if(online::CurrentUser::get()->createServer(name, max_players, info))
    {
        StateManager::get()->escapePressed();
        StateManager::get()->pushScreen(NetworkingLobby::getInstance());
    }
    else
    {
        sfx_manager->quickSound( "anvil" );
        m_info_widget->setColor(irr::video::SColor(255, 255, 0, 0));
        m_info_widget->setText(info, false);
    }*/
}

// ----------------------------------------------------------------------------
void NetworkingLobbySettings::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == m_cancel_widget->m_properties[PROP_ID])
    {
        StateManager::get()->escapePressed();
    }
    else if (name == m_create_widget->m_properties[PROP_ID])
    {
        createServer();
    }
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
