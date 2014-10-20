//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Marianne Gagnon
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

#include "states_screens/options_screen_audio.hpp"

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
#include "states_screens/options_screen_input.hpp"
#include "states_screens/options_screen_ui.hpp"
#include "states_screens/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/user_screen.hpp"
#include "utils/translation.hpp"

#include <iostream>
#include <sstream>

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( OptionsScreenAudio );

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
    if (ribbon != NULL)  ribbon->select( "tab_audio", PLAYER_ID_GAME_MASTER );

    ribbon->getRibbonChildren()[0].setTooltip( _("Graphics") );
    ribbon->getRibbonChildren()[2].setTooltip( _("User Interface") );
    ribbon->getRibbonChildren()[3].setTooltip( _("Players") );
    ribbon->getRibbonChildren()[4].setTooltip( _("Controls") );

    // ---- sfx volume
    SpinnerWidget* gauge = this->getWidget<SpinnerWidget>("sfx_volume");
    assert(gauge != NULL);

    gauge->setValue( (int)(SFXManager::get()->getMasterSFXVolume()*10.0f) );


    gauge = this->getWidget<SpinnerWidget>("music_volume");
    assert(gauge != NULL);
    gauge->setValue( (int)(music_manager->getMasterMusicVolume()*10.f) );

    // ---- music volume
    CheckBoxWidget* sfx = this->getWidget<CheckBoxWidget>("sfx_enabled");

    CheckBoxWidget* music = this->getWidget<CheckBoxWidget>("music_enabled");

    // ---- audio enables/disables
    sfx->setState( UserConfigParams::m_sfx );
    music->setState( UserConfigParams::m_music );

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

        music_manager->setMasterMusicVolume( w->getValue()/10.0f );
    }
    else if(name == "sfx_volume")
    {
        static SFXBase* sample_sound = NULL;

        SpinnerWidget* w = dynamic_cast<SpinnerWidget*>(widget);
        assert(w != NULL);

        if (sample_sound == NULL) sample_sound = SFXManager::get()->createSoundSource( "pre_start_race" );
        sample_sound->setVolume(1);

        SFXManager::get()->setMasterSFXVolume( w->getValue()/10.0f );
        UserConfigParams::m_sfx_volume = w->getValue()/10.0f;

        // play a sample sound to show the user what this volume is like
        sample_sound->play();
    }
    else if(name == "music_enabled")
    {
        CheckBoxWidget* w = dynamic_cast<CheckBoxWidget*>(widget);

        UserConfigParams::m_music = w->getState();
        Log::info("OptionsScreenAudio", "Music is now %s", ((bool) UserConfigParams::m_music) ? "on" : "off");

        if(w->getState() == false)
            music_manager->stopMusic();
        else
            music_manager->startMusic(music_manager->getCurrentMusic(), 0);
    }
    else if(name == "sfx_enabled")
    {
        CheckBoxWidget* w = dynamic_cast<CheckBoxWidget*>(widget);

        UserConfigParams::m_sfx = w->getState();
        SFXManager::get()->toggleSound(UserConfigParams::m_sfx);

        if (UserConfigParams::m_sfx)
        {
            SFXManager::get()->quickSound("horn");
        }
    }
}   // eventCallback

// -----------------------------------------------------------------------------

void OptionsScreenAudio::unloaded()
{
}   // unloaded

// -----------------------------------------------------------------------------

