//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2019 dumaosen
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

#include "str_to_screen.hpp"

#include "guiengine/screen.hpp"
#include "states_screens/addons_screen.hpp"
#include "states_screens/arenas_screen.hpp"
#include "states_screens/credits.hpp"
#include "states_screens/easter_egg_screen.hpp"
#include "states_screens/ghost_replay_selection.hpp"
#include "states_screens/grand_prix_editor_screen.hpp"
#include "states_screens/help_screen_1.hpp"
#include "states_screens/help_screen_2.hpp"
#include "states_screens/help_screen_3.hpp"
#include "states_screens/help_screen_4.hpp"
#include "states_screens/help_screen_5.hpp"
#include "states_screens/help_screen_6.hpp"
#include "states_screens/help_screen_7.hpp"
#include "states_screens/online/create_server_screen.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/online/online_lan.hpp"
#include "states_screens/online/online_profile_achievements.hpp"
#include "states_screens/online/online_profile_friends.hpp"
#include "states_screens/online/online_profile_servers.hpp"
#include "states_screens/online/online_profile_settings.hpp"
#include "states_screens/online/online_screen.hpp"
#include "states_screens/online/online_user_search.hpp"
#include "states_screens/online/register_screen.hpp"
#include "states_screens/online/server_selection.hpp"
#include "states_screens/options/options_screen_audio.hpp"
#include "states_screens/options/options_screen_device.hpp"
#include "states_screens/options/options_screen_general.hpp"
#include "states_screens/options/options_screen_input.hpp"
#include "states_screens/options/options_screen_language.hpp"
#include "states_screens/options/options_screen_ui.hpp"
#include "states_screens/options/options_screen_video.hpp"
#include "states_screens/options/user_screen.hpp"
#include "states_screens/race_setup_screen.hpp"
#include "states_screens/soccer_setup_screen.hpp"
#include "states_screens/tracks_and_gp_screen.hpp"
#include "utils/log.hpp"

StrToScreen::StrToScreen(std::string screen)
{
    m_screen = screen;
} // StrToScreen

void StrToScreen::runScreen()
{
    if(m_screen == "addons")
        AddonsScreen::getInstance()->push();
    else if(m_screen == "arenas")
        ArenasScreen::getInstance()->push();
    else if(m_screen == "credits")
        CreditsScreen::getInstance()->push();
    else if(m_screen == "easter_egg")
        EasterEggScreen::getInstance()->push();
    else if(m_screen == "ghost_replay_selection")
        GhostReplaySelection::getInstance()->push();
    else if(m_screen == "grand_prix_editor")
        GrandPrixEditorScreen::getInstance()->push();
    else if(m_screen == "help_1")
        HelpScreen1::getInstance()->push();
    else if(m_screen == "help_2")
        HelpScreen2::getInstance()->push();
    else if(m_screen == "help_3")
        HelpScreen3::getInstance()->push();
    else if(m_screen == "help_4")
        HelpScreen4::getInstance()->push();
    else if(m_screen == "help_5")
        HelpScreen5::getInstance()->push();
    else if(m_screen == "help_6")
        HelpScreen6::getInstance()->push();
    else if(m_screen == "help_7")
        HelpScreen7::getInstance()->push();
    else if(m_screen == "create_server")
        CreateServerScreen::getInstance()->push();
    else if(m_screen == "networking_lobby")
        NetworkingLobby::getInstance()->push();
    else if(m_screen == "online_lan")
        OnlineLanScreen::getInstance()->push();
    else if(m_screen == "profile_achievements")
        OnlineProfileAchievements::getInstance()->push();
    else if(m_screen == "profile_friends")
        OnlineProfileFriends::getInstance()->push();
    else if(m_screen == "profile_servers")
        OnlineProfileServers::getInstance()->push();
    else if(m_screen == "profile_settings")
        OnlineProfileSettings::getInstance()->push();
    else if(m_screen == "online")
        OnlineScreen::getInstance()->push();
    else if(m_screen == "user_search")
        OnlineUserSearch::getInstance()->push();
    else if(m_screen == "register")
        RegisterScreen::getInstance()->push();
    else if(m_screen == "server_selection")
        ServerSelection::getInstance()->push();
    else if(m_screen == "options_general")
        OptionsScreenGeneral::getInstance()->push();
    else if(m_screen == "options_audio")
        OptionsScreenAudio::getInstance()->push();
    else if(m_screen == "options_device")
        OptionsScreenDevice::getInstance()->push();
    else if(m_screen == "options_input")
        OptionsScreenInput::getInstance()->push();
    else if(m_screen == "options_language")
        OptionsScreenLanguage::getInstance()->push();
    else if(m_screen == "options_ui")
        OptionsScreenUI::getInstance()->push();
    else if(m_screen == "options_video")
        OptionsScreenVideo::getInstance()->push();
    else if(m_screen == "user_screen")
        UserScreen::getInstance()->push();
    else if(m_screen == "race_setup")
        RaceSetupScreen::getInstance()->push();
    else if(m_screen == "soccer_setup")
        SoccerSetupScreen::getInstance()->push();
    else if(m_screen == "tracks_and_gp")
        TracksAndGPScreen::getInstance()->push();
    else
        Log::warn("StrToScreen",
                  "Unsupported screen \"%s\"", m_screen.c_str());
}