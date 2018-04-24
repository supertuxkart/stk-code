//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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


#include "states_screens/main_menu_screen.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "guiengine/message_queue.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "input/device_manager.hpp"
#include "network/protocols/connect_to_server.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/network_config.hpp"
#include "network/server.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/request_manager.hpp"
#include "states_screens/networking_lobby.hpp"
#include "states_screens/online_lan.hpp"
#include "states_screens/online_profile_achievements.hpp"
#include "states_screens/online_profile_servers.hpp"
#include "states_screens/online_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/user_screen.hpp"
#include "states_screens/dialogs/general_text_field_dialog.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "utils/string_utils.hpp"

#include <string>


using namespace GUIEngine;
using namespace Online;

// ----------------------------------------------------------------------------

OnlineScreen::OnlineScreen() : Screen("online/online.stkgui")
{
    m_online_string = _("Your profile");
    //I18N: Used as a verb, appears on the main networking menu (login button)
    m_login_string = _("Login");
}   // OnlineScreen

// ----------------------------------------------------------------------------

void OnlineScreen::loadedFromFile()
{
    m_enable_splitscreen = getWidget<CheckBoxWidget>("enable-splitscreen");
    assert(m_enable_splitscreen);
    m_enable_splitscreen->setState(false);
}   // loadedFromFile

// ----------------------------------------------------------------------------

void OnlineScreen::beforeAddingWidget()
{
    bool is_logged_in = false;
    if (PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_GUEST ||
        PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_SIGNED_IN)
    {
        is_logged_in = true;
    }

    IconButtonWidget* wan = getWidget<IconButtonWidget>("wan");
    if (wan)
    {
        wan->setActive(is_logged_in);
        wan->setVisible(is_logged_in);
    }
} // beforeAddingWidget

// ----------------------------------------------------------------------------
//
void OnlineScreen::init()
{
    Screen::init();

    m_online = getWidget<IconButtonWidget>("online");

    if (!MainMenuScreen::m_enable_online)
        m_online->setActive(false);

    m_user_id = getWidget<ButtonWidget>("user-id");
    assert(m_user_id);

    RibbonWidget* r = getWidget<RibbonWidget>("menu_toprow");
    r->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    // Pre-add a default single player profile in network
    if (!m_enable_splitscreen->getState() &&
        NetworkConfig::get()->getNetworkPlayers().empty())
    {
        NetworkConfig::get()->addNetworkPlayer(
            input_manager->getDeviceManager()->getLatestUsedDevice(),
            PlayerManager::getCurrentPlayer(), false/*handicap*/);
        NetworkConfig::get()->doneAddingNetworkPlayers();
    }
}   // init

// ----------------------------------------------------------------------------

void OnlineScreen::onUpdate(float delta)
{
    PlayerProfile *player = PlayerManager::getCurrentPlayer();
    if (PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_GUEST  ||
        PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_SIGNED_IN)
    {
        m_online->setActive(true);
        m_online->setLabel(m_online_string);
        m_user_id->setText(player->getLastOnlineName() + "@stk");
    }
    else if (PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_SIGNED_OUT)
    {
        m_online->setActive(true);
        m_online->setLabel(m_login_string);
        m_user_id->setText(player->getName());
    }
    else
    {
        // now must be either logging in or logging out
        m_online->setActive(false);
        m_user_id->setText(player->getName());
    }

    m_online->setLabel(PlayerManager::getCurrentOnlineId() ? m_online_string
                                                           : m_login_string);
    // In case for entering server address finished
    if (auto lb = LobbyProtocol::get<LobbyProtocol>())
    {
        NetworkingLobby::getInstance()->setJoinedServer(nullptr);
        StateManager::get()->resetAndSetStack(
            NetworkConfig::get()->getResetScreens(true/*lobby*/).data());
    }
}   // onUpdate

// ----------------------------------------------------------------------------

void OnlineScreen::eventCallback(Widget* widget, const std::string& name,
                                   const int playerID)
{
    if (name == "user-id")
    {
        NetworkConfig::get()->cleanNetworkPlayers();
        UserScreen::getInstance()->push();
        return;
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
        return;
    }
    else if (name == "enable-splitscreen")
    {
        CheckBoxWidget* splitscreen = dynamic_cast<CheckBoxWidget*>(widget);
        assert(splitscreen);
        if (!splitscreen->getState())
        {
            // Default single player
            NetworkConfig::get()->cleanNetworkPlayers();
            NetworkConfig::get()->addNetworkPlayer(
                input_manager->getDeviceManager()->getLatestUsedDevice(),
                PlayerManager::getCurrentPlayer(), false/*handicap*/);
            NetworkConfig::get()->doneAddingNetworkPlayers();
        }
        else
        {
            // Let lobby add the players
            NetworkConfig::get()->cleanNetworkPlayers();
        }
        return;
    }

    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
    if (ribbon == NULL) return; // what's that event??

    // ---- A ribbon icon was clicked
    std::string selection =
        ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);

    if (selection == "lan")
    {
        OnlineLanScreen::getInstance()->push();
    }
    else if (selection == "wan")
    {
        OnlineProfileServers::getInstance()->push();
    }
    else if (selection == "online")
    {
        if (UserConfigParams::m_internet_status != RequestManager::IPERM_ALLOWED)
        {
            new MessageDialog(_("You can not play online without internet access. "
                                "If you want to play online, go to options, select "
                                " tab 'User Interface', and edit "
                                "\"Connect to the Internet\"."));
            return;
        }

        if (PlayerManager::getCurrentOnlineId())
        {
            ProfileManager::get()->setVisiting(PlayerManager::getCurrentOnlineId());
            TabOnlineProfileAchievements::getInstance()->push();
        }
        else
        {
            UserScreen::getInstance()->push();
        }
    }
    else if (selection == "enter-address")
    {
        if (NetworkConfig::get()->isAddingNetworkPlayers())
        {
            core::stringw msg =
                _("No player available for connecting to server.");
            MessageQueue::add(MessageQueue::MT_ERROR, msg);
            return;
        }
        core::stringw instruction =
            _("Enter the server address with IP (optional) followed by : and"
            " then port.");
        auto gtfd = new GeneralTextFieldDialog(instruction.c_str(),
            [] (const irr::core::stringw& text) {},
            [this] (GUIEngine::LabelWidget* lw,
                   GUIEngine::TextBoxWidget* tb)->bool
            {
                TransportAddress server_addr(
                    StringUtils::wideToUtf8(tb->getText()));
                if (server_addr.getIP() == 0)
                {
                    core::stringw err = _("Invalid server address: %s.",
                        tb->getText());
                    lw->setText(err, true);
                    return false;
                }
                NetworkConfig::get()->setIsWAN();
                NetworkConfig::get()->setIsServer(false);
                NetworkConfig::get()->setPassword("");
                auto server = std::make_shared<Server>(0, L"", 0, 0, 0, 0,
                    server_addr, false);
                STKHost::create();
                auto cts = std::make_shared<ConnectToServer>(server);
                cts->setup();
                Log::info("OnlineScreen", "Trying to connect to server '%s'.",
                    server_addr.toString().c_str());
                if (!cts->handleDirectConnect(10000))
                {
                    core::stringw err = _("Cannot connect to server %s.",
                        server_addr.toString().c_str());
                    STKHost::get()->shutdown();
                    NetworkConfig::get()->unsetNetworking();
                    lw->setText(err, true);
                    return false;
                }

                m_entered_server_address = 
                    STKHost::get()->getServerPeerForClient()->getAddress();
                auto cl = LobbyProtocol::create<ClientLobby>();
                cl->setAddress(m_entered_server_address);
                cl->requestStart();
                return true;
            });
        if (!m_entered_server_address.isUnset())
        {
            gtfd->getTextField()->setText(StringUtils::utf8ToWide(
                m_entered_server_address.toString()));
        }
    }
}   // eventCallback

// ----------------------------------------------------------------------------
/** Also called when pressing the back button. It resets the flags to indicate
 *  a networked game.
 */
bool OnlineScreen::onEscapePressed()
{
    NetworkConfig::get()->cleanNetworkPlayers();
    NetworkConfig::get()->unsetNetworking();
    return true;
}   // onEscapePressed
