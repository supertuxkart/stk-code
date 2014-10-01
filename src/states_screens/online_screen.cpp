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

#include "audio/sfx_manager.hpp"
#include "config/player_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/scalable_font.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "main_loop.hpp"
#include "modes/demo_world.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/connect_to_server.hpp"
#include "network/protocols/request_connection.hpp"
#include "online/profile_manager.hpp"
#include "online/request.hpp"
#include "online/servers_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/networking_lobby.hpp"
#include "states_screens/server_selection.hpp"
#include "states_screens/create_server_screen.hpp"
#include "states_screens/online_profile_achievements.hpp"

#include <string>
#include <iostream>

using namespace GUIEngine;
using namespace Online;


DEFINE_SCREEN_SINGLETON( OnlineScreen );

// ----------------------------------------------------------------------------

OnlineScreen::OnlineScreen() : Screen("online/main.stkgui")
{
    m_recorded_state = PlayerProfile::OS_SIGNED_OUT;
}   // OnlineScreen

// ----------------------------------------------------------------------------

OnlineScreen::~OnlineScreen()
{
}

// ----------------------------------------------------------------------------

void OnlineScreen::loadedFromFile()
{
    m_back_widget = getWidget<IconButtonWidget>("back");
    assert(m_back_widget != NULL);

    m_top_menu_widget = getWidget<RibbonWidget>("menu_toprow");
    assert(m_top_menu_widget != NULL);
    m_quick_play_widget = (IconButtonWidget *) m_top_menu_widget->findWidgetNamed("quick_play");
    assert(m_quick_play_widget != NULL);
    m_find_server_widget = (IconButtonWidget *) m_top_menu_widget->findWidgetNamed("find_server");
    assert(m_find_server_widget != NULL);
    m_create_server_widget = (IconButtonWidget *) m_top_menu_widget->findWidgetNamed("create_server");
    assert(m_create_server_widget != NULL);

    m_online_status_widget = getWidget<LabelWidget>("online_status");
    assert(m_online_status_widget != NULL);

    m_bottom_menu_widget = getWidget<RibbonWidget>("menu_bottomrow");
    assert(m_bottom_menu_widget != NULL);
    m_profile_widget = (IconButtonWidget *) m_bottom_menu_widget->findWidgetNamed("profile");
    assert(m_profile_widget != NULL);
    m_sign_out_widget = (IconButtonWidget *) m_bottom_menu_widget->findWidgetNamed("sign_out");
    assert(m_sign_out_widget != NULL);

}   // loadedFromFile

// ----------------------------------------------------------------------------
/** Checks if the recorded state differs from the actual state and sets it.
 */
bool OnlineScreen::hasStateChanged()
{
    PlayerProfile::OnlineState previous_state = m_recorded_state;
    m_recorded_state = PlayerManager::getCurrentOnlineState();
    if (previous_state != m_recorded_state)
        return true;
    return false;
}   // hasStateChanged

// ----------------------------------------------------------------------------
void OnlineScreen::beforeAddingWidget()
{
    //Set everything that could be set invisible or deactivated, to active and visible
    m_bottom_menu_widget->setVisible(true);
    m_top_menu_widget->setVisible(true);
    hasStateChanged();
    if (m_recorded_state == PlayerProfile::OS_SIGNED_OUT ||
        m_recorded_state == PlayerProfile::OS_SIGNING_IN ||
        m_recorded_state == PlayerProfile::OS_SIGNING_OUT)
    {
        m_quick_play_widget->setDeactivated();
        m_find_server_widget->setDeactivated();
        m_create_server_widget->setDeactivated();
        m_sign_out_widget->setVisible(false);
        m_profile_widget->setVisible(false);
    }
    else if (m_recorded_state == PlayerProfile::OS_GUEST)
    {
        m_find_server_widget->setDeactivated();
        m_create_server_widget->setDeactivated();
        m_profile_widget->setVisible(false);
    }

} // beforeAddingWidget

// ----------------------------------------------------------------------------
void OnlineScreen::init()
{
    Screen::init();
    setInitialFocus();
    DemoWorld::resetIdleTime();
    core::stringw m = _("Signed in as: %s.",
                        PlayerManager::getCurrentOnlineUserName());
    m_online_status_widget->setText(m, false);
}   // init

// ----------------------------------------------------------------------------
void OnlineScreen::onUpdate(float delta)
{
    if (hasStateChanged())
    {
        GUIEngine::reshowCurrentScreen();
        return;
    }

    if (m_recorded_state == PlayerProfile::OS_SIGNING_IN)
    {
        m_online_status_widget->setText(StringUtils::loadingDots(_("Signing in")),
                                        false                                   );
    }
    else if (m_recorded_state == PlayerProfile::OS_SIGNING_OUT)
    {
        m_online_status_widget->setText(StringUtils::loadingDots(_("Signing out")),
                                        false                                    );
    }
}   // onUpdate

// ----------------------------------------------------------------------------
/** Executes the quick play selection. Atm this is all blocking.
 */
void OnlineScreen::doQuickPlay()
{
    // Refresh server list.
    HTTPRequest* refresh_request = ServersManager::get()->refreshRequest(false);
    if (refresh_request != NULL) // consider request done
    {
        refresh_request->executeNow();
        delete refresh_request;
    }
    else
    {
        Log::error("OnlineScreen", "Could not get the server list.");
        return;
    }

    // select first one
    const Server *server = ServersManager::get()->getQuickPlay();

    // do a join request
    XMLRequest *join_request = new RequestConnection::ServerJoinRequest();
    if (!join_request)
    {
        SFXManager::get()->quickSound("anvil");
        return;
    }

    PlayerManager::setUserDetails(join_request, "request-connection", Online::API::SERVER_PATH);
    join_request->addParameter("server_id", server->getServerId());

    join_request->executeNow();
    if (join_request->isSuccess())
    {
        delete join_request;
        NetworkingLobby::getInstance()->push();
        ConnectToServer *cts = new ConnectToServer(server->getServerId(),
                                                   server->getHostId());
        ProtocolManager::getInstance()->requestStart(cts);
    }
    else
    {
        SFXManager::get()->quickSound("anvil");
    }

}   // doQuickPlay

// ----------------------------------------------------------------------------

void OnlineScreen::eventCallback(Widget* widget, const std::string& name,
                                 const int playerID)
{
    if (name == m_back_widget->m_properties[PROP_ID])
    {
        StateManager::get()->escapePressed();
        return;
    }

    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
    if (ribbon == NULL) return;
    std::string selection = ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);

    if (selection == m_sign_out_widget->m_properties[PROP_ID])
    {
        PlayerManager::requestSignOut();
        StateManager::get()->popMenu();
    }
    else if (selection == m_profile_widget->m_properties[PROP_ID])
    {
        ProfileManager::get()->setVisiting(PlayerManager::getCurrentOnlineId());
        OnlineProfileAchievements::getInstance()->push();
    }
    else if (selection == m_find_server_widget->m_properties[PROP_ID])
    {
        ServerSelection::getInstance()->push();
    }
    else if (selection == m_create_server_widget->m_properties[PROP_ID])
    {
        CreateServerScreen::getInstance()->push();
    }
    else if (selection == m_quick_play_widget->m_properties[PROP_ID])
    {
        doQuickPlay();
    }

}   // eventCallback

// ----------------------------------------------------------------------------
void OnlineScreen::tearDown()
{
}

// ----------------------------------------------------------------------------
/** Sets which widget has to be focused. Depends on the user state.
 */
void OnlineScreen::setInitialFocus()
{
    if (m_recorded_state == PlayerProfile::OS_SIGNED_IN)
        m_top_menu_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    else
        m_bottom_menu_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

}   // setInitialFocus

// ----------------------------------------------------------------------------
void OnlineScreen::onDialogClose()
{
    if (hasStateChanged())
        GUIEngine::reshowCurrentScreen();
    else
        setInitialFocus();
}   // onDialogClose()

