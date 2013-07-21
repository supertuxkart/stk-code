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

#include "audio/sfx_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/scalable_font.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "main_loop.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/dialogs/login_dialog.hpp"
#include "states_screens/dialogs/registration_dialog.hpp"
#include "states_screens/networking_lobby.hpp"
#include "states_screens/networking_lobby_settings.hpp"
#include "states_screens/server_selection.hpp"
#include "modes/demo_world.hpp"
#include "online/servers_manager.hpp"
#include "online/messages.hpp"



using namespace GUIEngine;
using namespace Online;

DEFINE_SCREEN_SINGLETON( OnlineScreen );

// ----------------------------------------------------------------------------

OnlineScreen::OnlineScreen() : Screen("online/main.stkgui")
{
    m_recorded_state = CurrentUser::SIGNED_OUT;
    CurrentUser::acquire()->requestSavedSession();
    CurrentUser::release();
}   // OnlineScreen

// ----------------------------------------------------------------------------

OnlineScreen::~OnlineScreen()
{
}

// ----------------------------------------------------------------------------

void OnlineScreen::loadedFromFile()
{
    //Box ? FIXME
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
    m_sign_in_widget = (IconButtonWidget *) m_bottom_menu_widget->findWidgetNamed("sign_in");
    assert(m_sign_in_widget != NULL);
    m_register_widget = (IconButtonWidget *) m_bottom_menu_widget->findWidgetNamed("register");
    assert(m_register_widget != NULL);
    m_sign_out_widget = (IconButtonWidget *) m_bottom_menu_widget->findWidgetNamed("sign_out");
    assert(m_sign_out_widget != NULL);

}   // loadedFromFile

// ----------------------------------------------------------------------------
bool OnlineScreen::hasStateChanged()
{
    CurrentUser::UserState previous_state = m_recorded_state;
    m_recorded_state = CurrentUser::acquire()->getUserState();
    CurrentUser::release();
    if (previous_state != m_recorded_state)
        return true;
    return false;
}

// ----------------------------------------------------------------------------
void OnlineScreen::beforeAddingWidget()
{
    //Set everything that could be set invisible or deactivated, to active and visible
    m_bottom_menu_widget->setVisible(true);
    m_top_menu_widget->setVisible(true);
    hasStateChanged();
    if (m_recorded_state == CurrentUser::SIGNED_IN)
    {
        m_register_widget->setVisible(false);
        m_sign_in_widget->setVisible(false);
    }
    else if (m_recorded_state == CurrentUser::SIGNED_OUT || m_recorded_state == CurrentUser::SIGNING_IN || m_recorded_state == CurrentUser::SIGNING_OUT)
    {
        m_quick_play_widget->setDeactivated();
        m_find_server_widget->setDeactivated();
        m_create_server_widget->setDeactivated();
        m_sign_out_widget->setVisible(false);
        if(m_recorded_state == CurrentUser::SIGNING_IN || m_recorded_state == CurrentUser::SIGNING_OUT)
        {
            m_register_widget->setDeactivated();
            m_sign_in_widget->setDeactivated();
        }
    }
    else if (m_recorded_state == CurrentUser::GUEST)
    {
        m_find_server_widget->setDeactivated();
        m_create_server_widget->setDeactivated();
        m_sign_in_widget->setVisible(false);
    }

} // beforeAddingWidget



// ----------------------------------------------------------------------------
void OnlineScreen::init()
{
    Screen::init();
    setInitialFocus();
    DemoWorld::resetIdleTime();
    m_online_status_widget->setText(Messages::signedInAs(CurrentUser::acquire()->getUserName()), false);
    CurrentUser::release();

}   // init

// ----------------------------------------------------------------------------
void OnlineScreen::onUpdate(float delta,  irr::video::IVideoDriver* driver)
{
    if (hasStateChanged())
        GUIEngine::reshowCurrentScreen();
    if (m_recorded_state == CurrentUser::SIGNING_IN)
    {
        m_online_status_widget->setText(Messages::signingIn(), false);
    }
    else if (m_recorded_state == CurrentUser::SIGNING_OUT)
    {
        m_online_status_widget->setText(Messages::signingOut(), false);
    }

    XMLRequest * sign_in_request = HTTPManager::get()->getXMLResponse(Request::RT_SIGN_IN);
    if(sign_in_request != NULL)
    {
        if(sign_in_request->isSuccess())
        {
            new MessageDialog(_("Signed in."));
        }
        else
        {
            sfx_manager->quickSound( "anvil" );
            new MessageDialog(sign_in_request->getInfo());
        }
        delete sign_in_request;
    }
    XMLRequest * sign_out_request = HTTPManager::get()->getXMLResponse(Request::RT_SIGN_OUT);
    if(sign_out_request != NULL)
    {
        if(sign_out_request->isSuccess())
        {
            new MessageDialog(_("Signed out successfully."));
        }
        else
        {
            sfx_manager->quickSound( "anvil" );
            new MessageDialog(sign_out_request->getInfo());
        }
        delete sign_out_request;
    }


}   // onUpdate

// ----------------------------------------------------------------------------

void OnlineScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
        return;
    }

    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
    if (ribbon == NULL) return; // what's that event??
    std::string selection = ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);

    if (selection == "sign_in")
    {
        new LoginDialog(LoginDialog::Normal);
    }
    else if (selection == "sign_out")
    {
        CurrentUser::acquire()->requestSignOut();
        CurrentUser::release();
    }
    else if (selection == "register")
    {
        new RegistrationDialog();
    }
    else if (selection == "find_server")
    {
        StateManager::get()->pushScreen(ServerSelection::getInstance());
    }
    else if (selection == "create_server")
    {
        StateManager::get()->pushScreen(NetworkingLobbySettings::getInstance());
    }
    else if (selection == "quick_play")
    {
        //FIXME temporary and the request join + join sequence should be placed in one method somewhere
            /*
        Server * server = ServersManager::get()->getQuickPlay();
        irr::core::stringw info;
        if (Online::CurrentUser::get()->requestJoin( server->getServerId(), info))
        {
            ServersManager::get()->setJoinedServer(server);
            StateManager::get()->pushScreen(NetworkingLobby::getInstance());
        }
        else
        {
            sfx_manager->quickSound( "anvil" );
        }*/
    }

}   // eventCallback

// ----------------------------------------------------------------------------
void OnlineScreen::tearDown()
{
}

// ----------------------------------------------------------------------------
void OnlineScreen::onDisabledItemClicked(const std::string& item)
{
    if (item == "find_server")
    {
        new LoginDialog(LoginDialog::Registration_Required);
    }
    else if (item =="create_server")
    {
        new LoginDialog(LoginDialog::Registration_Required);
    }
    else if (item == "quick_play")
    {
        new LoginDialog(LoginDialog::Signing_In_Required);
    }
}   // onDisabledItemClicked

// ----------------------------------------------------------------------------
void OnlineScreen::setInitialFocus()
{
    if(m_recorded_state == CurrentUser::SIGNED_IN)
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

