//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2014 Marianne Gagnon
//                2014      Joerg Henrichs
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

#include "states_screens/track_info_screen.hpp"

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "race/highscores.hpp"
#include "race/highscore_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/tracks_screen.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>
#include <IGUIImage.h>
#include <IGUIStaticText.h>

using namespace irr::gui;
using namespace irr::video;
using namespace irr::core;
using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( TrackInfoScreen );

// ------------------------------------------------------------------------------------------------------

//TrackInfoScreen::TrackInfoScreen(const std::string& ribbonItem, const std::string& trackIdent,
//                     const irr::core::stringw& trackName, ITexture* screenshot)
TrackInfoScreen::TrackInfoScreen()
          : Screen("track_info.stkgui")
{
}   // TrackInfoScreen

// ----------------------------------------------------------------------------
void TrackInfoScreen::setTrack(const Track *track)
{
    m_track = track;
}   // setTrack

// ----------------------------------------------------------------------------
/** Initialised the display. The previous screen has to setup m_track before
 *  pushing this screen using TrackInfoScreen::getInstance()->setTrack().
 */
void TrackInfoScreen::init()
{
    RaceManager::MinorRaceModeType minor = race_manager->getMinorMode();

    const bool has_AI         = race_manager->hasAI(minor);
    const bool has_laps       = race_manager->modeHasLaps();
    const bool has_highscores = race_manager->modeHasHighscores();

    getWidget<LabelWidget>("name")->setText(m_track->getName(), false);

    //I18N: when showing who is the author of track '%s'
    //I18N: (place %s where the name of the author should appear)
    getWidget<LabelWidget>("author")->setText( _("Track by %s", m_track->getDesigner()),
                                               false );

    // ---- Track screenshot
    Widget* screenshot_div = getWidget("screenshot_div");
    IconButtonWidget* screenshot_widget = 
        new IconButtonWidget(IconButtonWidget::SCALE_MODE_KEEP_CUSTOM_ASPECT_RATIO,
                             false /* tab stop */, false /* focusable */);
    // images are saved squared, but must be stretched to 4:
    screenshot_widget->setCustomAspectRatio(4.0f / 3.0f);

    screenshot_widget->m_x = screenshot_div->m_x;
    screenshot_widget->m_y = screenshot_div->m_y;
    screenshot_widget->m_w = screenshot_div->m_w;
    screenshot_widget->m_h = screenshot_div->m_h;

    // temporary icon, will replace it just after (but it will be shown if the given icon is not found)
    screenshot_widget->m_properties[PROP_ICON] = "gui/main_help.png";
    //FIXME screenshot_widget->setParent(m_irrlicht_window);
    screenshot_widget->add();

    ITexture* screenshot = irr_driver->getTexture(m_track->getScreenshotFile(),
                                    "While loading screenshot for track '%s':",
                                           m_track->getFilename()            );
    if (screenshot != NULL)
        screenshot_widget->setImage(screenshot);
    m_widgets.push_back(screenshot_widget);

    // Lap count m_lap_spinner
    // -----------------------
    if (has_laps)
    {
        m_lap_spinner = getWidget<SpinnerWidget>("lapcountspinner");
        m_lap_spinner->setVisible(true);
        getWidget<LabelWidget>("lap-text")->setVisible(true);
        if (UserConfigParams::m_artist_debug_mode)
            m_lap_spinner->setMin(0);

        m_lap_spinner->setValue(m_track->getActualNumberOfLap());
        race_manager->setNumLaps(m_lap_spinner->getValue());
    }
    else
    {
        getWidget<SpinnerWidget>("lapcountspinner")->setVisible(false);
        getWidget<LabelWidget>("lap-text")->setVisible(false);
        m_lap_spinner = NULL;
    }

    // Number of AIs
    // -------------
    if (has_AI)
    {
        m_ai_kart_spinner = getWidget<SpinnerWidget>("kartcountspinner");
        m_ai_kart_spinner->setActivated();

        // Avoid negative numbers (which can happen if e.g. the number of karts
        // in a previous race was lower than the number of players now.
        int num_ai = UserConfigParams::m_num_karts - race_manager->getNumLocalPlayers();
        if (num_ai < 0) num_ai = 0;
        m_ai_kart_spinner->setValue(num_ai);
        m_ai_kart_spinner->setMax(stk_config->m_max_karts - race_manager->getNumLocalPlayers());
        race_manager->setNumKarts(num_ai + race_manager->getNumLocalPlayers());
    }
    else
    {
        getWidget<SpinnerWidget>("kartcountspinner")->setVisible(false);
        m_ai_kart_spinner = NULL;
    }

    // Reverse track
    // -------------
    const bool reverse_available = m_track->reverseAvailable() &&
               race_manager->getMinorMode() != RaceManager::MINOR_MODE_EASTER_EGG;
    if (reverse_available)
    {
        m_checkbox = getWidget<CheckBoxWidget>("reverse");
        m_checkbox->setState(race_manager->getReverseTrack());
    }
    else
    {
        getWidget<CheckBoxWidget>("reverse")->setVisible(false);
        getWidget<LabelWidget>("reverse-text")->setVisible(false);
        m_checkbox = NULL;
        race_manager->setReverseTrack(false);
    }

    // ---- High Scores
    if (has_highscores)
    {
        m_kart_icons[0] = getWidget<IconButtonWidget>("iconscore1");
        m_kart_icons[1] = getWidget<IconButtonWidget>("iconscore2");
        m_kart_icons[2] = getWidget<IconButtonWidget>("iconscore3");

        m_highscore_entries[0] = getWidget<LabelWidget>("highscore1");
        m_highscore_entries[1] = getWidget<LabelWidget>("highscore2");
        m_highscore_entries[2] = getWidget<LabelWidget>("highscore3");

        updateHighScores();
    }
    else
    {
        getWidget<IconButtonWidget>("iconscore1")->setVisible(false);
        getWidget<IconButtonWidget>("iconscore2")->setVisible(false);
        getWidget<IconButtonWidget>("iconscore3")->setVisible(false);

        getWidget<LabelWidget>("highscores")->setVisible(false);
        getWidget<LabelWidget>("highscore1")->setVisible(false);
        getWidget<LabelWidget>("highscore2")->setVisible(false);
        getWidget<LabelWidget>("highscore3")->setVisible(false);
    }

    //FIXME getWidget<ButtonWidget>("start")->setFocusForPlayer( PLAYER_ID_GAME_MASTER );

}   // TrackInfoScreen

// ----------------------------------------------------------------------------

TrackInfoScreen::~TrackInfoScreen()
{
}   // ~TrackInfoScreen

// ----------------------------------------------------------------------------

void TrackInfoScreen::updateHighScores()
{
    std::string game_mode_ident = RaceManager::getIdentOf( race_manager->getMinorMode() );
    const Highscores::HighscoreType type = "HST_" + game_mode_ident;

    Highscores* highscores =
        highscore_manager->getHighscores(type,
                                         race_manager->getNumberOfKarts(),
                                         race_manager->getDifficulty(),
                                         m_track->getIdent(),
                                         race_manager->getNumLaps(),
                                         race_manager->getReverseTrack()  );
    const int amount = highscores->getNumberEntries();

    std::string kart_name;
    core::stringw name;
    float time;

    // fill highscore entries
    for (int n=0; n<HIGHSCORE_COUNT; n++)
    {
        irr::core::stringw line;

        // Check if this entry is filled or still empty
        if (n < amount)
        {
            highscores->getEntry(n, kart_name, name, &time);

            std::string time_string = StringUtils::timeToString(time);

            const KartProperties* prop = kart_properties_manager->getKart(kart_name);
            if (prop != NULL)
            {
                const std::string &icon_path = prop->getAbsoluteIconFile();
                ITexture* kart_icon_texture = irr_driver->getTexture( icon_path );
                m_kart_icons[n]->setImage(kart_icon_texture);
            }
            line = name + "\t" + core::stringw(time_string.c_str());
        }
        else
        {
            //I18N: for empty highscores entries
            line = _("(Empty)");

            ITexture* no_kart_texture = irr_driver->getTexture(
                                 file_manager->getAsset(FileManager::GUI,
                                                        "random_kart.png") );
            m_kart_icons[n]->setImage(no_kart_texture);

        }

        m_highscore_entries[n]->setText( line.c_str(), false );

    }
}   // updateHighScores

// ----------------------------------------------------------------------------

void TrackInfoScreen::onEnterPressedInternal()
{

    // Create a copy of member variables we still need, since they will
    // not be accessible after dismiss:
    const int num_laps = (m_lap_spinner == NULL ? -1 : m_lap_spinner->getValue());
    const bool reverse_track = m_checkbox == NULL ? false
                                                  : m_checkbox->getState();
   //FIXME m_track->setActualNumberOfLaps(num_laps);
    race_manager->setReverseTrack(reverse_track);

    // Disable accidentally unlocking of a challenge
    PlayerManager::getCurrentPlayer()->setCurrentChallenge("");

    race_manager->startSingleRace(m_track->getIdent(), num_laps, false);
}   // onEnterPressedInternal

// ----------------------------------------------------------------------------
void TrackInfoScreen::eventCallback(Widget* widget, const std::string& name,
                                   const int playerID)
{
    if (name == "buttons")
    {
        const std::string &button = getWidget<GUIEngine::RibbonWidget>("buttons")
                                  ->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if(button=="start")
            onEnterPressedInternal();
        else if(button=="back")
            StateManager::get()->escapePressed();
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "reverse")
    {
        race_manager->setReverseTrack(m_checkbox->getState());
        // Makes sure the highscores get swapped when clicking the 'reverse'
        // checkbox.
        if (race_manager->modeHasHighscores())
        {
            updateHighScores();
        }
    }
    else if (name == "lapcountspinner")
    {
        assert(m_lap_spinner != NULL);
        const int num_laps = m_lap_spinner->getValue();
        race_manager->setNumLaps(num_laps);
        UserConfigParams::m_num_laps = num_laps;
        updateHighScores();
    }
    else if (name=="kartcountspinner")
    {
        SpinnerWidget* w = dynamic_cast<SpinnerWidget*>(widget);
        race_manager->setNumKarts( race_manager->getNumLocalPlayers() + w->getValue() );
        UserConfigParams::m_num_karts = race_manager->getNumLocalPlayers() + w->getValue();

    }
}   // eventCallback

// ----------------------------------------------------------------------------
