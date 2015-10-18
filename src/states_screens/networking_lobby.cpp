//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#include "states_screens/networking_lobby.hpp"

#include <string>
#include <iostream>

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
#include "network/protocol_manager.hpp"
#include "network/protocols/client_lobby_room_protocol.hpp"
#include "modes/demo_world.hpp"
#include "utils/translation.hpp"
#include "online/servers_manager.hpp"

using namespace Online;
using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( NetworkingLobby );

// ----------------------------------------------------------------------------

NetworkingLobby::NetworkingLobby() : Screen("online/networking_lobby.stkgui")
{
    m_server = NULL;
}   // NetworkingLobby

// ----------------------------------------------------------------------------

void NetworkingLobby::loadedFromFile()
{
    m_back_widget = getWidget<IconButtonWidget>("back");
    assert(m_back_widget != NULL);

    m_server_name_widget = getWidget<LabelWidget>("server_name");
    assert(m_server_name_widget != NULL);

    m_online_status_widget = getWidget<LabelWidget>("online_status");
    assert(m_online_status_widget != NULL);

    m_bottom_menu_widget = getWidget<RibbonWidget>("menu_bottomrow");
    assert(m_bottom_menu_widget != NULL);

    m_exit_widget = (IconButtonWidget *) m_bottom_menu_widget->findWidgetNamed("exit");
    assert(m_exit_widget != NULL);

}   // loadedFromFile

// ----------------------------------------------------------------------------
void NetworkingLobby::beforeAddingWidget()
{

} // beforeAddingWidget



// ----------------------------------------------------------------------------
void NetworkingLobby::init()
{
    Screen::init();
    setInitialFocus();
    DemoWorld::resetIdleTime(); //FIXME : what's this?
    m_server = ServersManager::get()->getJoinedServer();
    m_server_name_widget->setText(m_server->getName(),false);

}   // init

// ----------------------------------------------------------------------------
void NetworkingLobby::onUpdate(float delta)
{
}   // onUpdate

// ----------------------------------------------------------------------------

void NetworkingLobby::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == m_back_widget->m_properties[PROP_ID])
    {
        StateManager::get()->escapePressed();
        return;
    }

    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
    if (ribbon == NULL) return;
    const std::string &selection = ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);

    if (selection == m_exit_widget->m_properties[PROP_ID])
    {
        StateManager::get()->escapePressed();
    }
}   // eventCallback

// ----------------------------------------------------------------------------

void NetworkingLobby::tearDown()
{
}   // tearDown

// ----------------------------------------------------------------------------

bool NetworkingLobby::onEscapePressed()
{
    // notify the server that we left
    ClientLobbyRoomProtocol* protocol = static_cast<ClientLobbyRoomProtocol*>(
            ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM));
    if (protocol)
        protocol->leave();
    return true; // close the screen
}

// ----------------------------------------------------------------------------
void NetworkingLobby::onDisabledItemClicked(const std::string& item)
{

}   // onDisabledItemClicked

// ----------------------------------------------------------------------------
void NetworkingLobby::setInitialFocus()
{
}   // setInitialFocus

// ----------------------------------------------------------------------------
void NetworkingLobby::onDialogClose()
{
    setInitialFocus();
}   // onDialogClose()
