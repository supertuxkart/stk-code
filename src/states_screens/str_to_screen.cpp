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

#include "challenges/story_mode_timer.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "guiengine/screen.hpp"
#include "modes/cutscene_world.hpp"
#include "modes/overworld.hpp"
#include "network/network_config.hpp"
#include "states_screens/credits.hpp"
#include "states_screens/grand_prix_editor_screen.hpp"
#include "states_screens/help_screen_1.hpp"
#include "states_screens/help_screen_2.hpp"
#include "states_screens/help_screen_3.hpp"
#include "states_screens/help_screen_4.hpp"
#include "states_screens/help_screen_5.hpp"
#include "states_screens/help_screen_6.hpp"
#include "states_screens/help_screen_7.hpp"
#include "states_screens/offline_kart_selection.hpp"
#include "states_screens/online/online_profile_achievements.hpp"
#include "states_screens/online/online_screen.hpp"
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
#include "utils/log.hpp"

#include <vector>

StrToScreen::StrToScreen(std::string screen)
{
    m_screen = screen;
} // StrToScreen

void StrToScreen::runScreen()
{
    if(m_screen == "achievements")
        OnlineProfileAchievements::getInstance()->push();
    else if(m_screen == "credits")
        CreditsScreen::getInstance()->push();
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
    else if(m_screen == "register")
        RegisterScreen::getInstance()->push();
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
    else if(m_screen == "start_story")
        runStory();
    else if(m_screen == "start_singleplayer")
        runOfflineGame(false);
    else if(m_screen == "start_multiplayer")
        runOfflineGame(true);
    else
        Log::warn("StrToScreen",
                  "Unsupported screen \"%s\"", m_screen.c_str());
}

void StrToScreen::runStory()
{
    NetworkConfig::get()->unsetNetworking();
    PlayerProfile *player = PlayerManager::getCurrentPlayer();

    // Start the story mode (and speedrun) timer
    story_mode_timer->startTimer();

    if (player->isFirstTime())
    {
        CutsceneWorld::setUseDuration(true);
        StateManager::get()->enterGameState();
        race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        race_manager->setNumKarts(0);
        race_manager->setNumPlayers(0);
        race_manager->startSingleRace("introcutscene", 999, false);

        std::vector<std::string> parts;
        parts.push_back("introcutscene");
        parts.push_back("introcutscene2");
        ((CutsceneWorld*)World::getWorld())->setParts(parts);
        //race_manager->startSingleRace("introcutscene2", 999, false);
        return;
    }
    else
    {
        // Unpause the story mode timer when entering back the story mode
        story_mode_timer->unpauseTimer(/* exit loading pause */ false);

        const std::string default_kart = UserConfigParams::m_default_kart;
        if (player->isLocked(default_kart))
        {
            KartSelectionScreen *next = OfflineKartSelectionScreen::getInstance();
            next->setGoToOverworldNext();
            next->setMultiplayer(false);
            next->push();
            return;
        }
        OverWorld::enterOverWorld();
    }
}

void StrToScreen::runOfflineGame(bool multiplayer)
{
    NetworkConfig::get()->unsetNetworking();
    KartSelectionScreen* s = OfflineKartSelectionScreen::getInstance();
    s->setMultiplayer(multiplayer);
    s->setFromOverworld(false);
    s->push();
}
