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
#include "config/user_config.hpp"
#include "modes/demo_world.hpp"
#include "network/protocols/lobby_protocol.hpp"
#include "network/network_config.hpp"
#include "network/servers_manager.hpp"
#include "network/stk_host.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/networking_lobby.hpp"
#include "states_screens/dialogs/server_info_dialog.hpp"
#include "utils/separate_process.hpp"
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
    m_max_players_widget
        ->setMax(UserConfigParams::m_server_max_players.getDefaultValue());
    m_max_players_widget->setValue(UserConfigParams::m_server_max_players);

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


    // -- Difficulty
    RibbonWidget* difficulty = getWidget<RibbonWidget>("difficulty");
    assert(difficulty != NULL);
    difficulty->setSelection(UserConfigParams::m_difficulty, PLAYER_ID_GAME_MASTER);

    // -- Game modes
    RibbonWidget* gamemode = getWidget<RibbonWidget>("gamemode");
    assert(gamemode != NULL);
    gamemode->setSelection(0, PLAYER_ID_GAME_MASTER);
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
            NetworkConfig::get()->unsetNetworking();
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

    RibbonWidget* difficulty_widget = getWidget<RibbonWidget>("difficulty");
    RibbonWidget* gamemode_widget = getWidget<RibbonWidget>("gamemode");

    if (name.size() < 4 || name.size() > 30)
    {
        m_info_widget->setText(
            _("Name has to be between 4 and 30 characters long!"), false);
        SFXManager::get()->quickSound("anvil");
        return;
    }
    assert(max_players > 1 &&
        max_players <= UserConfigParams::m_server_max_players.getDefaultValue());

    UserConfigParams::m_server_max_players = max_players;
    core::stringw password_w = getWidget<TextBoxWidget>("password")->getText();
    std::string password = StringUtils::xmlEncode(password_w);
    NetworkConfig::get()->setPassword(StringUtils::wideToUtf8(password_w));
    if (!password.empty())
        password = std::string(" --server-password=") + password;

    ServersManager::get()->cleanUpServers();
    TransportAddress server_address(0x7f000001,
        NetworkConfig::get()->getServerDiscoveryPort());


    TransportAddress address(0x7f000001,
        NetworkConfig::get()->getServerDiscoveryPort());
    Server *server = new Server(0/*server_id*/, name, max_players,
        /*current_player*/0, (RaceManager::Difficulty)
        difficulty_widget->getSelection(PLAYER_ID_GAME_MASTER),
        NetworkConfig::get()->getServerGameMode(race_manager->getMinorMode(),
            race_manager->getMajorMode()), server_address);
    ServersManager::get()->addServer(server);
    ServersManager::get()->setJoinedServer(0);

#undef USE_GRAPHICS_SERVER
#ifdef USE_GRAPHICS_SERVER
    NetworkConfig::get()->setIsServer(true);
    // In case of a WAN game, we register this server with the
    // stk server, and will get the server's id when this 
    // request is finished.
    NetworkConfig::get()->setMaxPlayers(max_players);
    NetworkConfig::get()->setServerName(name);

    // FIXME: Add the following fields to the create server screen
    // FIXME: Long term we might add a 'vote' option (e.g. GP vs single race,
    // and normal vs FTL vs time trial could be voted about).
    std::string difficulty = difficulty_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    race_manager->setDifficulty(RaceManager::convertDifficulty(difficulty));
    race_manager->setMajorMode(RaceManager::MAJOR_MODE_SINGLE);

    std::string game_mode = gamemode_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    if (game_mode == "timetrial")
        race_manager->setMinorMode(RaceManager::MINOR_MODE_TIME_TRIAL);
    else
        race_manager->setMinorMode(RaceManager::MINOR_MODE_NORMAL_RACE);

    race_manager->setReverseTrack(false);
    STKHost::create();
#else

    NetworkConfig::get()->setIsServer(false);
    std::ostringstream server_cfg;
#ifdef WIN32
    server_cfg << " ";
#endif

    const std::string server_name = StringUtils::xmlEncode(name);
    if (NetworkConfig::get()->isWAN())
    {
        server_cfg << "--public-server --wan-server=" <<
            server_name << " --login-id=" <<
            NetworkConfig::get()->getCurrentUserId() << " --token=" <<
            NetworkConfig::get()->getCurrentUserToken();
    }
    else
    {
        server_cfg << "--lan-server=" << server_name;
    }

    std::string server_id_file = "server_id_file_";
    server_id_file += StringUtils::toString(StkTime::getTimeSinceEpoch());
    NetworkConfig::get()->setServerIdFile(
        file_manager->getUserConfigFile(server_id_file));

    server_cfg << " --no-graphics --stdout=server.log --type=" <<
        gamemode_widget->getSelection(PLAYER_ID_GAME_MASTER) <<
        " --difficulty=" <<
        difficulty_widget->getSelection(PLAYER_ID_GAME_MASTER) <<
        " --max-players=" << max_players <<
        " --server-id-file=" << server_id_file <<
        " --log=1 --no-console-log";

    SeparateProcess* sp =
        new SeparateProcess(SeparateProcess::getCurrentExecutableLocation(),
        server_cfg.str() + password);
    STKHost::create(sp);

#endif
}   // createServer

// ----------------------------------------------------------------------------

void CreateServerScreen::tearDown()
{
}   // tearDown

