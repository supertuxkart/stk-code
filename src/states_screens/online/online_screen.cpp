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
#include "network/event.hpp"
#include "network/network.hpp"
#include "network/network_config.hpp"
#include "network/server.hpp"
#include "network/server_config.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/request_manager.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/online/online_lan.hpp"
#include "states_screens/online/online_profile_achievements.hpp"
#include "states_screens/online/online_profile_servers.hpp"
#include "states_screens/online/online_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/options/user_screen.hpp"
#include "states_screens/dialogs/general_text_field_dialog.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

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
} // beforeAddingWidget

// ----------------------------------------------------------------------------
//
void OnlineScreen::init()
{
    Screen::init();

    m_online = getWidget<IconButtonWidget>("online");
    assert(m_online);

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
            PlayerManager::getCurrentPlayer(), HANDICAP_NONE);
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
    if (m_entered_server)
    {
        NetworkConfig::get()->setIsLAN();
        NetworkConfig::get()->setIsServer(false);
        ServerConfig::m_private_server_password = "";
        STKHost::create();
        NetworkingLobby::getInstance()->setJoinedServer(m_entered_server);
        m_entered_server = nullptr;
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
                PlayerManager::getCurrentPlayer(), HANDICAP_NONE);
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
        if (PlayerManager::getCurrentOnlineState() != PlayerProfile::OS_SIGNED_IN)
        {
            //I18N: Shown to players when he is not is not logged in
            new MessageDialog(_("You must be logged in to play Global "
                "networking. Click your username above."));
        }
        else
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
        m_entered_server = nullptr;
        if (NetworkConfig::get()->isAddingNetworkPlayers())
        {
            core::stringw msg =
                _("No player available for connecting to server.");
            MessageQueue::add(MessageQueue::MT_ERROR, msg);
            return;
        }
        core::stringw instruction =
            _("Enter the server address optionally followed by : and"
            " then port.");
        auto gtfd = new GeneralTextFieldDialog(instruction.c_str(),
            [] (const irr::core::stringw& text) {},
            [this] (GUIEngine::LabelWidget* lw,
                   GUIEngine::TextBoxWidget* tb)->bool
            {
                TransportAddress server_addr =
                    TransportAddress::fromDomain(
                    StringUtils::wideToUtf8(tb->getText()));
                if (server_addr.getIP() == 0)
                {
                    core::stringw err = _("Invalid server address: %s.",
                        tb->getText());
                    lw->setText(err, true);
                    return false;
                }
                if (server_addr.getPort() == 0)
                {
                    ENetAddress ea;
                    ea.host = STKHost::HOST_ANY;
                    ea.port = STKHost::PORT_ANY;
                    Network* nw = new Network(/*peer_count*/1,
                        /*channel_limit*/EVENT_CHANNEL_COUNT,
                        /*max_in_bandwidth*/0, /*max_out_bandwidth*/0, &ea,
                        true/*change_port_if_bound*/);
                    BareNetworkString s(std::string("stk-server-port"));
                    TransportAddress address(server_addr.getIP(),
                        stk_config->m_server_discovery_port);
                    nw->sendRawPacket(s, address);
                    TransportAddress sender;
                    const int LEN = 2048;
                    char buffer[LEN];
                    int len = nw->receiveRawPacket(buffer, LEN, &sender, 2000);
                    if (len != 2)
                    {
                        //I18N: In enter server ip address dialog
                        core::stringw err = _("Failed to detect port number.");
                        lw->setText(err, true);
                        delete nw;
                        return false;
                    }
                    BareNetworkString server_port(buffer, len);
                    uint16_t port = server_port.getUInt16();
                    server_addr.setPort(port);
                    Log::info("OnlineScreen",
                        "Detected port %d for server address: %s.",
                        port, server_addr.toString(false).c_str());
                    delete nw;
                }
                auto server = std::make_shared<Server>(0,
                    StringUtils::utf8ToWide(server_addr.toString()), 0, 0, 0, 0,
                    server_addr, false, false);
                m_entered_server = server;
                m_entered_server_address = server_addr;
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
