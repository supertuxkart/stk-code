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

#ifndef SERVER_ONLY // No GUI files in server builds

// Manages includes common to all options screens
#include "states_screens/options/options_common.hpp"

#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"

using namespace GUIEngine;

// -----------------------------------------------------------------------------

OptionsScreenAudio::OptionsScreenAudio() : Screen("options/options_audio.stkgui")
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
    OptionsCommon::setTabStatus();

    // Bind typed widget pointers (one-time lookup)
    m_widgets.bind(this);

    m_widgets.options_choice->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    m_widgets.options_choice->select("tab_audio", PLAYER_ID_GAME_MASTER);

    // ---- sfx volume
    m_widgets.sfx_volume->setMax(UserConfigParams::m_volume_denominator);
    m_widgets.sfx_volume->setValue(UserConfigParams::m_sfx_numerator);

    // ---- music volume
    m_widgets.music_volume->setMax(UserConfigParams::m_volume_denominator);
    m_widgets.music_volume->setValue(UserConfigParams::m_music_numerator);

    // ---- audio enables/disables
    m_widgets.sfx_enabled->setState(UserConfigParams::m_sfx);
    m_widgets.music_enabled->setState(UserConfigParams::m_music);

    if (!UserConfigParams::m_sfx)
        m_widgets.sfx_volume->setActive(false);
    if (!UserConfigParams::m_music)
        m_widgets.music_volume->setActive(false);
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
    if (widget == m_widgets.options_choice)
    {
        std::string selection = m_widgets.options_choice->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection != "tab_audio")
            OptionsCommon::switchTab(selection);
    }
    else if (widget == m_widgets.back)
    {
        StateManager::get()->escapePressed();
    }
    else if (widget == m_widgets.music_volume)
    {
        float new_volume = computeVolume(m_widgets.music_volume->getValue(),
                                         UserConfigParams::m_volume_denominator);

        UserConfigParams::m_music_numerator = m_widgets.music_volume->getValue();
        music_manager->setMasterMusicVolume(new_volume);
    }
    else if (widget == m_widgets.sfx_volume)
    {
        static SFXBase* sample_sound = NULL;

        if (sample_sound == NULL)
            sample_sound = SFXManager::get()->createSoundSource("pre_start_race");
        sample_sound->setVolume(1);

        float new_volume = computeVolume(m_widgets.sfx_volume->getValue(),
                                         UserConfigParams::m_volume_denominator);
        SFXManager::get()->setMasterSFXVolume(new_volume);
        UserConfigParams::m_sfx_numerator = m_widgets.sfx_volume->getValue();
        UserConfigParams::m_sfx_volume = new_volume;

        // play a sample sound to show the user what this volume is like
        sample_sound->play();
    }
    else if (widget == m_widgets.music_enabled)
    {
        UserConfigParams::m_music = m_widgets.music_enabled->getState();
        Log::info("OptionsScreenAudio", "Music is now %s",
                  ((bool)UserConfigParams::m_music) ? "on" : "off");

        if (!m_widgets.music_enabled->getState())
        {
            music_manager->stopMusic();
            m_widgets.music_volume->setActive(false);
        }
        else
        {
            music_manager->startMusic();
            m_widgets.music_volume->setActive(true);
        }
    }
    else if (widget == m_widgets.sfx_enabled)
    {
        UserConfigParams::m_sfx = m_widgets.sfx_enabled->getState();
        SFXManager::get()->toggleSound(UserConfigParams::m_sfx);

        if (UserConfigParams::m_sfx)
        {
            SFXManager::get()->quickSound("horn");
            m_widgets.sfx_volume->setActive(true);
        }
        else
        {
            m_widgets.sfx_volume->setActive(false);
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

#endif // ifndef SERVER_ONLY