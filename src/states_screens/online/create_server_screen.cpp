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

#include "states_screens/online/create_server_screen.hpp"

#include "audio/sfx_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "karts/controller/network_ai_controller.hpp"
#include "network/network_config.hpp"
#include "network/server.hpp"
#include "network/server_config.hpp"
#include "network/child_loop.hpp"
#include "network/socket_address.hpp"
#include "network/stk_host.hpp"
#include "online/online_profile.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "utils/stk_process.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <irrString.h>

#include <string>
#include <iostream>


using namespace GUIEngine;

// ----------------------------------------------------------------------------

CreateServerScreen::CreateServerScreen() : Screen("online/create_server.stkgui")
{
}   // CreateServerScreen

// ----------------------------------------------------------------------------

void CreateServerScreen::loadedFromFile()
{
    m_prev_mode = 0;
    m_prev_value = 0;
    m_widgets.bind(this);

    int max = UserConfigParams::m_max_players.getDefaultValue();
    m_widgets.max_players->setMax(max);

    if (UserConfigParams::m_max_players > max)
        UserConfigParams::m_max_players = max;

    m_widgets.max_players->setValue(UserConfigParams::m_max_players);
}   // loadedFromFile

// ----------------------------------------------------------------------------
void CreateServerScreen::init()
{
    Screen::init();
    if (NetworkConfig::get()->isLAN())
        m_supports_ai = !UserConfigParams::m_lan_server_gp;
    else
        m_supports_ai = !UserConfigParams::m_wan_server_gp;

    m_widgets.info->setText("", false);

    m_widgets.title->setText(NetworkConfig::get()->isLAN() ? _("Create LAN Server")
                                                 : _("Create Server")    ,
                   false);

    // I18n: Name of the server. %s is either the online or local user name
    m_widgets.name->setText(_("%s's server",
                             NetworkConfig::get()->isLAN()
                             ? PlayerManager::getCurrentPlayer()->getName()
                             : PlayerManager::getCurrentOnlineProfile()->getUserName()
                             )
                          );


    // -- Difficulty
    m_widgets.difficulty->setSelection(UserConfigParams::m_difficulty, PLAYER_ID_GAME_MASTER);

    // -- Game modes
    m_widgets.gamemode->setSelection(m_prev_mode, PLAYER_ID_GAME_MASTER);
    updateMoreOption(m_prev_mode);
#ifdef MOBILE_STK
    m_widgets.name->setFocusable(true);
#endif
}   // init

// ----------------------------------------------------------------------------
void CreateServerScreen::beforeAddingWidget()
{
#ifdef MOBILE_STK
    // This will prevent name text box being focused first which make screen
    // keyboard always open
    m_widgets.name->setFocusable(false);
#endif
}   // beforeAddingWidget

// ----------------------------------------------------------------------------
/** Event callback which starts the server creation process.
 */
void CreateServerScreen::eventCallback(Widget* widget, const std::string& name,
                                       const int playerID)
{
    if (widget == m_widgets.options)
    {
        const std::string& selection =
            m_widgets.options->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == m_widgets.cancel->m_properties[PROP_ID])
        {
            NetworkConfig::get()->unsetNetworking();
            StateManager::get()->escapePressed();
        }
        else if (selection == m_widgets.create->m_properties[PROP_ID])
        {
            createServer();
        }   // is create_widget
    }
    else if (widget == m_widgets.gamemode)
    {
        const int selection =
            m_widgets.gamemode->getSelection(PLAYER_ID_GAME_MASTER);
        m_prev_value = 0;
        updateMoreOption(selection);
        m_prev_mode = selection;
    }
    else if (widget == m_widgets.max_players && m_supports_ai)
    {
        m_prev_value = m_widgets.more_options_spinner->getValue();
        const int selection =
            m_widgets.gamemode->getSelection(PLAYER_ID_GAME_MASTER);
        updateMoreOption(selection);
    }
    else if (widget == m_widgets.back)
    {
        NetworkConfig::get()->unsetNetworking();
        StateManager::get()->escapePressed();
    }
}   // eventCallback

// ----------------------------------------------------------------------------
void CreateServerScreen::updateMoreOption(int game_mode)
{
    switch (game_mode)
    {
        case 0:
        case 1:
        {
            m_widgets.more_options->setVisible(true);
            m_widgets.more_options_spinner->setVisible(true);
            m_widgets.more_options_spinner->clearLabels();
            if (m_supports_ai)
            {
                m_widgets.more_options->setText(_("Number of AI karts"),
                    false);
                for (int i = 0; i <= m_widgets.max_players->getValue() - 2; i++)
                {
                    m_widgets.more_options_spinner->addLabel(
                        StringUtils::toWString(i));
                }
                if (m_prev_value > m_widgets.max_players->getValue() - 2)
                {
                    m_widgets.more_options_spinner->setValue(
                        m_widgets.max_players->getValue() - 2);
                }
                else
                    m_widgets.more_options_spinner->setValue(m_prev_value);

            }
            else
            {
                //I18N: In the create server screen
                m_widgets.more_options->setText(_("No. of grand prix track(s)"),
                    false);
                m_widgets.more_options_spinner->addLabel(_("Disabled"));
                for (int i = 1; i <= 20; i++)
                {
                    m_widgets.more_options_spinner->addLabel(
                        StringUtils::toWString(i));
                }
                m_widgets.more_options_spinner->setValue(m_prev_value);
            }
            break;
        }
        case 2:
        {
            m_widgets.more_options->setVisible(true);
            m_widgets.more_options_spinner->setVisible(true);
            m_widgets.more_options_spinner->clearLabels();
            //I18N: In the create server screen, show various battle mode available
            m_widgets.more_options->setText(_("Battle mode"), false);
            m_widgets.more_options_spinner->setVisible(true);
            m_widgets.more_options_spinner->clearLabels();
            //I18N: In the create server screen for battle server
            m_widgets.more_options_spinner->addLabel(_("Free-For-All"));
            //I18N: In the create server screen for battle server
            m_widgets.more_options_spinner->addLabel(_("Capture The Flag"));
            m_widgets.more_options_spinner->setValue(m_prev_value);
            break;
        }
        case 3:
        {
            m_widgets.more_options->setVisible(true);
            m_widgets.more_options_spinner->setVisible(true);
            m_widgets.more_options_spinner->clearLabels();
            //I18N: In the create server screen
            m_widgets.more_options->setText(_("Soccer game type"), false);
            m_widgets.more_options_spinner->setVisible(true);
            m_widgets.more_options_spinner->clearLabels();
            //I18N: In the create server screen for soccer server
            m_widgets.more_options_spinner->addLabel(_("Time limit"));
            //I18N: In the create server screen for soccer server
            m_widgets.more_options_spinner->addLabel(_("Goals limit"));
            m_widgets.more_options_spinner->setValue(m_prev_value);
            break;
        }
        default:
        {
            m_widgets.more_options->setVisible(false);
            m_widgets.more_options_spinner->setVisible(false);
            break;
        }
    }
}   // updateMoreOption

// ----------------------------------------------------------------------------
/** Called once per framce to check if the server creation request has
 *  finished. If so, if pushes the server creation sceen.
 */
void CreateServerScreen::onUpdate(float delta)
{
    // If no host has been created, keep on waiting.
    if(!STKHost::existHost())
        return;

    NetworkingLobby::getInstance()->push();
}   // onUpdate

// ----------------------------------------------------------------------------
/** In case of WAN it adds the server to the list of servers. In case of LAN
 *  networking, it registers this game server with the stk server.
 */
void CreateServerScreen::createServer()
{
    const irr::core::stringw name = m_widgets.name->getText().trim();
    const int max_players = m_widgets.max_players->getValue();
    m_widgets.info->setErrorColor();

    if (name.size() < 4 || name.size() > 30)
    {
        //I18N: In the create server screen
        m_widgets.info->setText(
            _("Name has to be between 4 and 30 characters long!"), false);
        SFXManager::get()->quickSound("anvil");
        return;
    }
    assert(max_players > 1 && max_players <=
        UserConfigParams::m_max_players.getDefaultValue());

    UserConfigParams::m_max_players = max_players;
    std::string password = StringUtils::wideToUtf8(m_widgets.password->getText());
    if ((!password.empty() != 0 &&
        password.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOP"
        "QRSTUVWXYZ01234567890_") != std::string::npos) ||
        password.size() > 255)
    {
        //I18N: In the create server screen
        m_widgets.info->setText(
            _("Incorrect characters in password!"), false);
        SFXManager::get()->quickSound("anvil");
        return;
    }

    const bool private_server = !password.empty();
    ServerConfig::m_private_server_password = password;
    SocketAddress server_address(0x7f000001, 0);

    auto server = std::make_shared<Server>(0/*server_id*/, name,
        max_players, /*current_player*/0, (RaceManager::Difficulty)
        m_widgets.difficulty->getSelection(PLAYER_ID_GAME_MASTER),
        0, server_address, private_server, false);

#undef USE_GRAPHICS_SERVER
#ifdef USE_GRAPHICS_SERVER
    NetworkConfig::get()->setIsServer(true);
    // In case of a WAN game, we register this server with the
    // stk server, and will get the server's id when this
    // request is finished.
    ServerConfig::m_server_max_players = max_players;
    ServerConfig::m_server_name = StringUtils::xmlEncode(name);

    // FIXME: Add the following fields to the create server screen
    // FIXME: Long term we might add a 'vote' option (e.g. GP vs single race,
    // and normal vs FTL vs time trial could be voted about).
    std::string difficulty = m_widgets.difficulty->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    RaceManager::get()->setDifficulty(RaceManager::convertDifficulty(difficulty));
    RaceManager::get()->setMajorMode(RaceManager::MAJOR_MODE_SINGLE);

    std::string game_mode = m_widgets.gamemode->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    if (game_mode == "timetrial")
        RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_TIME_TRIAL);
    else
        RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_NORMAL_RACE);

    RaceManager::get()->setReverseTrack(false);
    auto sl = STKHost::create();
    assert(sl);
    sl->requestStart();
#else

    NetworkConfig::get()->setIsServer(false);

    ServerConfig::m_server_name = StringUtils::xmlEncode(name);
    // Server always configurable if created with this screen
    ServerConfig::m_server_configurable = true;
    struct ChildLoopConfig clc;
    clc.m_lan_server = NetworkConfig::get()->isLAN();
    clc.m_login_id = NetworkConfig::get()->getCurrentUserId();
    clc.m_token = NetworkConfig::get()->getCurrentUserToken();
    clc.m_server_ai = 0;

    switch (m_widgets.gamemode->getSelection(PLAYER_ID_GAME_MASTER))
    {
    case 0:
        ServerConfig::m_server_mode = 3;
        break;
    case 1:
        ServerConfig::m_server_mode = 4;
        break;
    case 3:
        ServerConfig::m_server_mode = 6;
        break;
    }

    ServerConfig::m_server_difficulty =
        m_widgets.difficulty->getSelection(PLAYER_ID_GAME_MASTER);
    ServerConfig::m_server_max_players = max_players;

    if (m_widgets.more_options_spinner->isVisible())
    {
        int esi = m_widgets.more_options_spinner->getValue();
        if (m_widgets.gamemode->getSelection(PLAYER_ID_GAME_MASTER) ==
            3/*is soccer*/)
        {
            if (esi == 0)
                ServerConfig::m_soccer_goal_target = false;
            else
                ServerConfig::m_soccer_goal_target = true;
        }
        else if (m_widgets.gamemode->getSelection(PLAYER_ID_GAME_MASTER) ==
            2/*is battle*/)
        {
            if (esi == 0)
                ServerConfig::m_server_mode = 7;
            else
                ServerConfig::m_server_mode = 8;
        }
        else
        {
            if (m_supports_ai)
            {
                if (esi > 0)
                {
                    clc.m_server_ai = esi;
                    NetworkAIController::setAIFrequency(10);
                }
            }
            else
            {
                // Grand prix track count
                if (esi > 0)
                {
                    ServerConfig::m_gp_track_count = esi;
                    if (ServerConfig::m_server_mode == 3)
                        ServerConfig::m_server_mode = 0;
                    else
                        ServerConfig::m_server_mode = 1;
                }
            }
        }
        m_prev_mode = m_widgets.gamemode->getSelection(PLAYER_ID_GAME_MASTER);
        m_prev_value = esi;
    }
    else
    {
        m_prev_mode = m_prev_value = 0;
    }

    ChildLoop* cl = new ChildLoop(clc);
    STKHost::create(cl);
    NetworkingLobby::getInstance()->setJoinedServer(server);
#endif
}   // createServer

// ----------------------------------------------------------------------------

void CreateServerScreen::tearDown()
{
}   // tearDown

