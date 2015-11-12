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

#include "states_screens/create_server_screen.hpp"

#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "modes/demo_world.hpp"
#include "network/network_config.hpp"
#include "network/servers_manager.hpp"
#include "network/stk_host.hpp"
#include "states_screens/online_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/networking_lobby.hpp"
#include "states_screens/dialogs/server_info_dialog.hpp"
#include "utils/translation.hpp"

#include <irrString.h>

#include <string>
#include <iostream>


using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( CreateServerScreen );

// ----------------------------------------------------------------------------

CreateServerScreen::CreateServerScreen() : Screen("online/create_server.stkgui")
{
}   // CreateServerScreen

// ----------------------------------------------------------------------------

void CreateServerScreen::loadedFromFile()
{
    m_name_widget = getWidget<TextBoxWidget>("name");
    assert(m_name_widget != NULL);
 
    m_max_players_widget = getWidget<SpinnerWidget>("max_players");
    assert(m_max_players_widget != NULL);
    m_max_players_widget->setValue(8);

    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget != NULL);

    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_create_widget = getWidget<IconButtonWidget>("create");
    assert(m_create_widget != NULL);
    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);

}   // loadedFromFile

// ----------------------------------------------------------------------------
void CreateServerScreen::init()
{
    Screen::init();
    DemoWorld::resetIdleTime();
    m_info_widget->setText("", false);
    LabelWidget *title = getWidget<LabelWidget>("title");

    title->setText(NetworkConfig::get()->isLAN() ? _("Create LAN Server")
                                                 : _("Create Server")    ,
                   false);

    // I18n: Name of the server. %s is either the online or local user name
    m_name_widget->setText(_("%s's server",
                             NetworkConfig::get()->isLAN() 
                             ? PlayerManager::getCurrentPlayer()->getName()
                             : PlayerManager::getCurrentOnlineUserName()
                             )
                          );
}   // init

// ----------------------------------------------------------------------------
/** Event callback which starts the server creation process.
 */
void CreateServerScreen::eventCallback(Widget* widget, const std::string& name,
                                       const int playerID)
{
    if (name == m_options_widget->m_properties[PROP_ID])
    {
        const std::string& selection =
            m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == m_cancel_widget->m_properties[PROP_ID])
        {
            StateManager::get()->escapePressed();
        }
        else if (selection == m_create_widget->m_properties[PROP_ID])
        {
            createServer();
        }   // is create_widget
    }
}   // eventCallback

// ----------------------------------------------------------------------------
/** Called once per framce to check if the server creation request has
 *  finished. If so, if pushes the server creation sceen.
 */
void CreateServerScreen::onUpdate(float delta)
{
    // If no host has been created, keep on waiting.
    if(!STKHost::existHost())
        return;

    // First check if an error happened while registering the server:
    // --------------------------------------------------------------
    const irr::core::stringw &error = STKHost::get()->getErrorMessage();
    if(error!="")
    {
        SFXManager::get()->quickSound("anvil");
        m_info_widget->setErrorColor();
        m_info_widget->setText(error, false);
        return;
    }

    // Otherwise wait till we get an answer from the server:
    // -----------------------------------------------------
    if(!STKHost::get()->isRegistered())
    {
        m_info_widget->setDefaultColor();
        m_info_widget->setText(StringUtils::loadingDots(_("Creating server")),
                               false);
        return;
    }

    //FIXME If we really want a gui, we need to decide what else to do here
    // For now start the (wrong i.e. client) lobby, to prevent to create
    // a server more than once.
    NetworkingLobby::getInstance()->push();
}   // onUpdate

// ----------------------------------------------------------------------------
/** In case of WAN it adds the server to the list of servers. In case of LAN
 *  networking, it registers this game server with the stk server.
 */
void CreateServerScreen::createServer()
{
    const irr::core::stringw name = m_name_widget->getText().trim();
    const int max_players = m_max_players_widget->getValue();
    m_info_widget->setErrorColor();
    if (name.size() < 4 || name.size() > 30)
    {
        m_info_widget->setText(
            _("Name has to be between 4 and 30 characters long!"), false);
        SFXManager::get()->quickSound("anvil");
        return;
    }
    else if (max_players < 2 || max_players > 12)
    {
        m_info_widget->setText(
            _("The maxinum number of players has to be between 2 and 12."),
            false);
        SFXManager::get()->quickSound("anvil");
        return;
    }

    // In case of a LAN game, we can create the new server object now
    if (NetworkConfig::get()->isLAN())
    {
        // FIXME Is this actually necessary?? Only in case of WAN, or LAN and WAN?
        TransportAddress address(0x7f000001,0);  // 127.0.0.1
        Server *server = new Server(name, /*lan*/true, max_players,
                                    /*current_player*/1, address);
        ServersManager::get()->addServer(server);
    }

    // In case of a WAN game, we register this server with the
    // stk server, and will get the server's id when this 
    // request is finished.
    NetworkConfig::get()->setMaxPlayers(max_players);
    NetworkConfig::get()->setServerName(name);
    STKHost::create();

}   // createServer

// ----------------------------------------------------------------------------

void CreateServerScreen::tearDown()
{
}   // tearDown

