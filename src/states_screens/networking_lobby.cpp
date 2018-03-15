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

#include "states_screens/networking_lobby.hpp"

#include <string>

#include "config/user_config.hpp"
#include "config/player_manager.hpp"
#include "guiengine/CGUISpriteBank.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/CGUIEditBox.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "network/network_config.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/server.hpp"
#include "network/stk_host.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/network_user_dialog.hpp"
#include "utils/translation.hpp"

using namespace Online;
using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( NetworkingLobby );

/** This is the lobby screen that is shown on all clients, but not on the
 *  server. It shows currently connected clients, and allows the 'master'
 *  client (i.e. the stk instance that created the server) to control the
 *  server. This especially means that it can initialise the actual start
 *  of the racing.
 *  This class is responsible for creating the ActivePlayers data structure
 *  for all local players (the ActivePlayer maps device to player, i.e.
 *  controls which device is used by which player). Note that a server
 *  does not create an instance of this class and will create the ActivePlayer
 *  data structure in LobbyProtocol::loadWorld().
 */
// ----------------------------------------------------------------------------
NetworkingLobby::NetworkingLobby() : Screen("online/networking_lobby.stkgui")
{
    m_player_list = NULL;
}   // NetworkingLobby

// ----------------------------------------------------------------------------

void NetworkingLobby::loadedFromFile()
{
    m_back_widget = getWidget<IconButtonWidget>("back");
    assert(m_back_widget != NULL);

    m_start_button = getWidget<IconButtonWidget>("start");
    assert(m_start_button!= NULL);

    m_text_bubble = getWidget<LabelWidget>("text");
    assert(m_text_bubble != NULL);

    m_chat_box = getWidget<TextBoxWidget>("chat");
    assert(m_chat_box != NULL);

    m_send_button = getWidget<ButtonWidget>("send");
    assert(m_send_button != NULL);

    m_player_list = getWidget<ListWidget>("players");
    assert(m_player_list!= NULL);

    m_exit_widget = getWidget<IconButtonWidget>("exit");
    assert(m_exit_widget != NULL);

    m_icon_bank = new irr::gui::STKModifiedSpriteBank(GUIEngine::getGUIEnv());
    video::ITexture* icon_1 = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI, "cup_gold.png"));
    video::ITexture* icon_2 = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI, "difficulty_medium.png"));
    video::ITexture* icon_3 = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI, "main_help.png"));
    m_icon_bank->addTextureAsSprite(icon_1);
    m_icon_bank->addTextureAsSprite(icon_2);
    m_icon_bank->addTextureAsSprite(icon_3);
    const int screen_width = irr_driver->getFrameSize().Width;
    m_icon_bank->setScale(screen_width > 1280 ? 0.4f : 0.25f);
}   // loadedFromFile

// ---------------------------------------------------------------------------
void NetworkingLobby::beforeAddingWidget()
{
} // beforeAddingWidget

// ----------------------------------------------------------------------------
/** This function is a callback from the parent class, and is called each time
 *  this screen is shown to initialise the screen. This class is responsible
 *  for creating the active players and binding them to the right device.
 */
void NetworkingLobby::init()
{
    Screen::init();
    m_server_info_height = GUIEngine::getFont()->getDimension(L"X").Height;
    m_start_button->setVisible(false);

    // For now create the active player and bind it to the right
    // input device.
    InputDevice* device =
        input_manager->getDeviceManager()->getLatestUsedDevice();
    PlayerProfile* profile = PlayerManager::getCurrentPlayer();
    StateManager::get()->createActivePlayer(profile, device);

    if (!UserConfigParams::m_lobby_chat)
    {
        getWidget("chat")->setVisible(false);
        getWidget("chat")->setActive(false);
        getWidget("send")->setVisible(false);
        getWidget("send")->setActive(false);
    }
    else
    {
        m_chat_box->addListener(this);
        getWidget("chat")->setVisible(true);
        getWidget("chat")->setActive(true);
        getWidget("send")->setVisible(true);
        getWidget("send")->setActive(true);
    }
}   // init

// ----------------------------------------------------------------------------
void NetworkingLobby::setJoinedServer(std::shared_ptr<Server> server)
{
    if (server == m_joined_server)
        return;

    m_joined_server = server;
    m_server_info.clear();

    if (!m_joined_server)
        return;
    core::stringw each_line;

    //I18N: In the networking lobby
    each_line = _("Server name: %s", m_joined_server->getName());
    m_server_info.push_back(each_line);

    const core::stringw& difficulty_name =
        race_manager->getDifficultyName(m_joined_server->getDifficulty());
    //I18N: In the networking lobby
    each_line = _("Difficulty: %s", difficulty_name);
    m_server_info.push_back(each_line);

    //I18N: In the networking lobby
    each_line = _("Max players: %d", m_joined_server->getMaxPlayers());
    m_server_info.push_back(each_line);

    //I18N: In the networking lobby
    core::stringw mode = RaceManager::getNameOf(m_joined_server->getRaceMinorMode());
    each_line = _("Game mode: %s", mode);

    race_manager->setMinorMode(m_joined_server->getRaceMinorMode());
    race_manager->setMajorMode(m_joined_server->getRaceMajorMode());
    race_manager->setDifficulty(m_joined_server->getDifficulty());

    m_server_info.push_back(each_line);
}   // setJoinedServer

// ----------------------------------------------------------------------------
void NetworkingLobby::addMoreServerInfo(core::stringw info)
{
    assert(m_text_bubble->getDimension().Width > 10);
    while ((int)GUIEngine::getFont()->getDimension(info.c_str()).Width >
        m_text_bubble->getDimension().Width - 10)
    {
        int size = (m_text_bubble->getDimension().Width - 10)
            / m_server_info_height;
        assert(size > 0);
        core::stringw new_info = info.subString(0, size);
        m_server_info.push_back(new_info);
        info = info.subString(new_info.size(), 80);
    }
    if (info.size() > 0)
    {
        m_server_info.push_back(info);
    }
    while ((int)m_server_info.size() * m_server_info_height >
        m_text_bubble->getDimension().Height)
    {
        m_server_info.erase(m_server_info.begin());
    }
}   // addMoreServerInfo

// ----------------------------------------------------------------------------
void NetworkingLobby::onUpdate(float delta)
{
    auto lp = LobbyProtocol::get<LobbyProtocol>();
    if (!lp)
    {
        core::stringw connect_msg;
        if (m_joined_server)
        {
            connect_msg = StringUtils::loadingDots(
                _("Connecting to server %s", m_joined_server->getName()));
        }
        else if (NetworkConfig::get()->isClient())
        {
            connect_msg =
                StringUtils::loadingDots(_("Finding a quick play server"));
        }
        m_text_bubble->setText(connect_msg, true);
        m_start_button->setVisible(false);
    }
    else
    {
        core::stringw total_msg;
        for (auto& string : m_server_info)
        {
            total_msg += string;
            total_msg += L"\n";
        }
        m_text_bubble->setText(total_msg, true);
    }
    if (NetworkConfig::get()->isClient() &&
        STKHost::get()->isAuthorisedToControl())
    {
        m_start_button->setVisible(true);
    }

}   // onUpdate

// ----------------------------------------------------------------------------
void NetworkingLobby::sendChat(irr::core::stringw text)
{
    text = text.trim().removeChars(L"\n\r");
    if (text.size() > 0)
    {
        NetworkString chat(PROTOCOL_LOBBY_ROOM);
        chat.addUInt8(LobbyProtocol::LE_CHAT);

        core::stringw name;
        PlayerProfile* player = PlayerManager::getCurrentPlayer();
        if (PlayerManager::getCurrentOnlineState() ==
            PlayerProfile::OS_SIGNED_IN)
            name = PlayerManager::getCurrentOnlineUserName();
        else
            name = player->getName();
        // Max 80 words
        chat.encodeString((name + L": " + text).subString(0, 80));

        STKHost::get()->sendToServer(&chat, true);
    }
}   // sendChat

// ----------------------------------------------------------------------------
void NetworkingLobby::eventCallback(Widget* widget, const std::string& name,
                                    const int playerID)
{
    if (name == m_back_widget->m_properties[PROP_ID])
    {
        StateManager::get()->escapePressed();
        return;
    }
    else if (name == m_player_list->m_properties[GUIEngine::PROP_ID])
    {
        auto host_online_ids = StringUtils::splitToUInt
            (m_player_list->getSelectionInternalName(), '_');
        if (host_online_ids.size() != 2 ||
            STKHost::get()->getMyHostId() == host_online_ids[0])
        {
            return;
        }
        new NetworkUserDialog(host_online_ids[0], host_online_ids[1],
            m_player_list->getSelectionLabel());
    }   // click on a user
    else if (name == m_send_button->m_properties[PROP_ID])
    {
        sendChat(m_chat_box->getText());
        m_chat_box->setText("");
    }   // send chat message

    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
    if (ribbon == NULL) return;
    const std::string &selection =
                     ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);

    if (selection == m_exit_widget->m_properties[PROP_ID])
    {
        StateManager::get()->escapePressed();
    }
    else if (selection == m_start_button->m_properties[PROP_ID])
    {
        if (NetworkConfig::get()->isServer())
        {
            auto slrp = LobbyProtocol::get<ServerLobby>();
            slrp->startSelection();
        }
        else
        {
            // Send a message to the server to start
            NetworkString start(PROTOCOL_LOBBY_ROOM);
            start.setSynchronous(true);
            start.addUInt8(LobbyProtocol::LE_REQUEST_BEGIN);
            STKHost::get()->sendToServer(&start, true);
        }
    }
}   // eventCallback

// ----------------------------------------------------------------------------
void NetworkingLobby::unloaded()
{
    delete m_icon_bank;
    m_icon_bank = NULL;
}   // unloaded

// ----------------------------------------------------------------------------

void NetworkingLobby::tearDown()
{
}   // tearDown

// ----------------------------------------------------------------------------
bool NetworkingLobby::onEscapePressed()
{
    STKHost::get()->shutdown();
    return true; // close the screen
}   // onEscapePressed

// ----------------------------------------------------------------------------
void NetworkingLobby::updatePlayers(const std::vector<std::tuple<uint32_t,
                                    uint32_t, core::stringw, int> >& p)
{
    // In GUI-less server this function will be called without proper
    // initialisation
    if (!m_player_list)
        return;
    m_player_list->clear();

    if (p.empty())
        return;

    irr::gui::STKModifiedSpriteBank* icon_bank = m_icon_bank;
    for (auto& q : p)
    {
        if (icon_bank)
        {
            m_player_list->setIcons(icon_bank);
            icon_bank = NULL;
        }
        const std::string internal_name =
            StringUtils::toString(std::get<0>(q)) + "_" +
            StringUtils::toString(std::get<1>(q));
        m_player_list->addItem(internal_name, std::get<2>(q), std::get<3>(q));
    }
}  // updatePlayers
