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
#include "network/stk_host.hpp"
#include "online/servers_manager.hpp"
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
using namespace Online;

DEFINE_SCREEN_SINGLETON( CreateServerScreen );

// ----------------------------------------------------------------------------

CreateServerScreen::CreateServerScreen() : Screen("online/create_server.stkgui")
{
    m_server_creation_request = NULL;
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

    title->setText(STKHost::isLAN() ? _("Create LAN Server")
                                    : _("Create Server")    , false);

    // I18n: Name of the server. %s is either the online or local user name
    m_name_widget->setText(_("%s's server",
                STKHost::isLAN() ? PlayerManager::getCurrentPlayer()->getName()
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
            serverCreationRequest();
        }   // is create_widget
    }
}   // eventCallback

// ----------------------------------------------------------------------------
/** Called once per framce to check if the server creation request has
 *  finished. If so, if pushes the server creation sceen.
 */
void CreateServerScreen::onUpdate(float delta)
{
    if (!m_server_creation_request) return;

    // If the request is not ready, wait till it is done.
    if (!m_server_creation_request->isDone())
    {
        m_info_widget->setDefaultColor();
        m_info_widget->setText(StringUtils::loadingDots(_("Creating server")),
                               false);
        return;
    }

    // Now the request has been executed.
    // ----------------------------------
    if (m_server_creation_request->isSuccess())
    {
        new ServerInfoDialog(m_server_creation_request->getCreatedServerID(),
                             true);
    }
    else
    {
        SFXManager::get()->quickSound("anvil");
        m_info_widget->setErrorColor();
        m_info_widget->setText(m_server_creation_request->getInfo(), false);
    }
    delete m_server_creation_request;
    m_server_creation_request = NULL;

}   // onUpdate

// ----------------------------------------------------------------------------
/** In case of WAN it adds the server to the list of servers. In case of LAN
 *  networking, it registers this game server with the stk server.
 */
void CreateServerScreen::serverCreationRequest()
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

    if (STKHost::isLAN())
    {
        Server *server = new Server(name, /*lan*/true, max_players,
                                    /*current_player*/1);
        ServersManager::get()->addServer(server);
        new ServerInfoDialog(server->getServerId(), true);
        return;
    }

    STKHost::setMaxPlayers(max_players);
    STKHost::create(/*is_server*/true);
    STKHost::get()->setServerName(name);

    // Now must be WAN: forward request to the stk server
    m_server_creation_request = new ServerCreationRequest();
    PlayerManager::setUserDetails(m_server_creation_request, "create",
                                  Online::API::SERVER_PATH);
    m_server_creation_request->addParameter("name", name);
    m_server_creation_request->addParameter("max_players", max_players);
    m_server_creation_request->queue();

}   // serverCreationRequest

// ----------------------------------------------------------------------------
/** Callbacks from the online create server request.
 */
void CreateServerScreen::ServerCreationRequest::callback()
{
    if (isSuccess())
    {
        // Must be a WAN server
        Server *server = new Server(*getXMLData()->getNode("server"),
                                    /*is lan*/false);
        ServersManager::get()->addServer(server);
        m_created_server_id = server->getServerId();
    }   // isSuccess
}   // callback

// ----------------------------------------------------------------------------

void CreateServerScreen::tearDown()
{
    delete m_server_creation_request;
    m_server_creation_request = NULL;
}   // tearDown

