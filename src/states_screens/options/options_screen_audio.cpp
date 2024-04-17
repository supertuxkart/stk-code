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

#include "states_screens/options/options_screen_audio.hpp"

#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "config/user_config.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "states_screens/options/options_screen_general.hpp"
#include "states_screens/options/options_screen_input.hpp"
#include "states_screens/options/options_screen_language.hpp"
#include "states_screens/options/options_screen_ui.hpp"
#include "states_screens/options/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/options/user_screen.hpp"
#include "utils/translation.hpp"

#include <iostream>
#include <sstream>

using namespace GUIEngine;

// -----------------------------------------------------------------------------

OptionsScreenAudio::OptionsScreenAudio() : Screen("options_audio.stkgui")
{
}   // OptionsScreenAudio

// -----------------------------------------------------------------------------

void OptionsScreenAudio::loadedFromFile()
{
}   // loadedFromFile

// -----------------------------------------------------------------------------

void OptionsScreenAudio::init()
{
    Screen::init();
    RibbonWidget* ribbon = this->getWidget<RibbonWidget>("options_choice");
    assert(ribbon != NULL);
    ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    ribbon->select( "tab_audio", PLAYER_ID_GAME_MASTER );

    // ---- sfx volume
    SpinnerWidget* gauge = this->getWidget<SpinnerWidget>("sfx_volume");
    assert(gauge != NULL);
    gauge->setMax(UserConfigParams::m_volume_denominator);
    gauge->setValue(UserConfigParams::m_sfx_numerator);

    gauge = this->getWidget<SpinnerWidget>("music_volume");
    assert(gauge != NULL);
    gauge->setMax(UserConfigParams::m_volume_denominator);
    gauge->setValue(UserConfigParams::m_music_numerator);

    // ---- music volume
    CheckBoxWidget* sfx = this->getWidget<CheckBoxWidget>("sfx_enabled");

    CheckBoxWidget* music = this->getWidget<CheckBoxWidget>("music_enabled");

    // ---- audio enables/disables
    sfx->setState( UserConfigParams::m_sfx );
    music->setState( UserConfigParams::m_music );

    if(!UserConfigParams::m_sfx)
        getWidget<SpinnerWidget>("sfx_volume")->setActive(false);
    if(!UserConfigParams::m_music)
        getWidget<SpinnerWidget>("music_volume")->setActive(false);

}   // init

// -----------------------------------------------------------------------------

void OptionsScreenAudio::tearDown()
{
    Screen::tearDown();
    // save changes when leaving screen
    user_config->saveConfig();
}   // tearDown

// -----------------------------------------------------------------------------

void OptionsScreenAudio::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        Screen *screen = NULL;
        //if (selection == "tab_audio")
        //    screen = OptionsScreenAudio::getInstance();
        if (selection == "tab_video")
            screen = OptionsScreenVideo::getInstance();
        else if (selection == "tab_players")
            screen = TabbedUserScreen::getInstance();
        else if (selection == "tab_controls")
            screen = OptionsScreenInput::getInstance();
        else if (selection == "tab_ui")
            screen = OptionsScreenUI::getInstance();
        else if (selection == "tab_general")
            screen = OptionsScreenGeneral::getInstance();
        else if (selection == "tab_language")
            screen = OptionsScreenLanguage::getInstance();
        if(screen)
            StateManager::get()->replaceTopMostScreen(screen);
    }
    else if(name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if(name == "music_volume")
    {
        SpinnerWidget* w = dynamic_cast<SpinnerWidget*>(widget);
        assert(w != NULL);

        float new_volume = computeVolume(w->getValue(), UserConfigParams::m_volume_denominator);

        UserConfigParams::m_music_numerator = w->getValue(); 
        music_manager->setMasterMusicVolume(new_volume);
    }
    else if(name == "sfx_volume")
    {
        static SFXBase* sample_sound = NULL;

        SpinnerWidget* w = dynamic_cast<SpinnerWidget*>(widget);
        assert(w != NULL);

        if (sample_sound == NULL) sample_sound = SFXManager::get()->createSoundSource( "pre_start_race" );
        sample_sound->setVolume(1);

        float new_volume = computeVolume(w->getValue(), UserConfigParams::m_volume_denominator);
        SFXManager::get()->setMasterSFXVolume(new_volume);
        UserConfigParams::m_sfx_numerator = w->getValue(); 
        UserConfigParams::m_sfx_volume = new_volume;

        // play a sample sound to show the user what this volume is like
        sample_sound->play();
    }
    else if(name == "music_enabled")
    {
        CheckBoxWidget* w = dynamic_cast<CheckBoxWidget*>(widget);

        UserConfigParams::m_music = w->getState();
        Log::info("OptionsScreenAudio", "Music is now %s", ((bool) UserConfigParams::m_music) ? "on" : "off");

        if(w->getState() == false)
        {
            music_manager->stopMusic();
            getWidget<SpinnerWidget>("music_volume")->setActive(false);
        }
        else
        {
            music_manager->startMusic();
            getWidget<SpinnerWidget>("music_volume")->setActive(true);
        }
    }
    else if(name == "sfx_enabled")
    {
        CheckBoxWidget* w = dynamic_cast<CheckBoxWidget*>(widget);

        UserConfigParams::m_sfx = w->getState();
        SFXManager::get()->toggleSound(UserConfigParams::m_sfx);

        if (UserConfigParams::m_sfx)
        {
            SFXManager::get()->quickSound("horn");
            getWidget<SpinnerWidget>("sfx_volume")->setActive(true);
        }
        else
        {
            getWidget<SpinnerWidget>("sfx_volume")->setActive(false);
        }
    }
}   // eventCallback

float OptionsScreenAudio::computeVolume(int numerator, int denominator)
{
    if (numerator <= 1)
    {
        return 0.025f;
    }
    else if (numerator == denominator)
    {
        return 1.0f;
    }
    else
    {
        float num_root = pow(40.0f, 1.0f / (float)(denominator - 1));
        return 0.025f * pow(num_root, (float)(numerator - 1));
    }
}

// -----------------------------------------------------------------------------

void OptionsScreenAudio::unloaded()
{
}   // unloaded

// -----------------------------------------------------------------------------

