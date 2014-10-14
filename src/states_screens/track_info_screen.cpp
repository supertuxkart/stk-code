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

// ----------------------------------------------------------------------------
/** Constructor, which loads the corresponding track_info.stkgui file. */
TrackInfoScreen::TrackInfoScreen()
          : Screen("track_info.stkgui")
{
    m_screenshot = NULL;
}   // TrackInfoScreen

// ----------------------------------------------------------------------------
/* Saves some often used pointers. */
void TrackInfoScreen::loadedFromFile()
{
    m_lap_spinner     = getWidget<SpinnerWidget>("lap-spinner");
    m_ai_kart_spinner = getWidget<SpinnerWidget>("ai-spinner");
    m_reverse         = getWidget<CheckBoxWidget>("reverse");
    m_reverse->setState(false);

    m_highscore_label = getWidget<LabelWidget>("highscores");

    m_kart_icons[0] = getWidget<IconButtonWidget>("iconscore1");
    m_kart_icons[1] = getWidget<IconButtonWidget>("iconscore2");
    m_kart_icons[2] = getWidget<IconButtonWidget>("iconscore3");

    m_highscore_entries[0] = getWidget<LabelWidget>("highscore1");
    m_highscore_entries[1] = getWidget<LabelWidget>("highscore2");
    m_highscore_entries[2] = getWidget<LabelWidget>("highscore3");
}   // loadedFromFile

// ----------------------------------------------------------------------------
void TrackInfoScreen::setTrack(Track *track)
{
    m_track = track;
}   // setTrack

// ----------------------------------------------------------------------------
/** Initialised the display. The previous screen has to setup m_track before
 *  pushing this screen using TrackInfoScreen::getInstance()->setTrack().
 */
void TrackInfoScreen::init()
{
    const bool has_laps       = race_manager->modeHasLaps();
    const bool has_highscores = race_manager->modeHasHighscores();

    getWidget<LabelWidget>("name")->setText(m_track->getName(), false);

    //I18N: when showing who is the author of track '%s'
    //I18N: (place %s where the name of the author should appear)
    getWidget<LabelWidget>("author")->setText( _("Track by %s", m_track->getDesigner()),
                                               false );

    // ---- Track screenshot
    Widget* screenshot_div = getWidget("screenshot_div");
    if(!m_screenshot)
    {
        m_screenshot =
            new IconButtonWidget(IconButtonWidget::SCALE_MODE_KEEP_CUSTOM_ASPECT_RATIO,
                                 false /* tab stop */, false /* focusable */);
        m_screenshot->setCustomAspectRatio(4.0f / 3.0f);

        m_screenshot->m_x = screenshot_div->m_x;
        m_screenshot->m_y = screenshot_div->m_y;
        m_screenshot->m_w = screenshot_div->m_w;
        m_screenshot->m_h = screenshot_div->m_h;
        m_screenshot->add();
        m_widgets.push_back(m_screenshot);
    }
    // images are saved squared, but must be stretched to 4:

    // temporary icon, will replace it just after (but it will be shown if the given icon is not found)
    m_screenshot->m_properties[PROP_ICON] = "gui/main_help.png";

    ITexture* screenshot = irr_driver->getTexture(m_track->getScreenshotFile(),
                                    "While loading screenshot for track '%s':",
                                           m_track->getFilename()            );
    if(!screenshot)
    {
        screenshot = irr_driver->getTexture("main_help.png",
                                    "While loading screenshot for track '%s':",
                                    m_track->getFilename());
    }
    if (screenshot != NULL)
        m_screenshot->setImage(screenshot);

    // Lap count m_lap_spinner
    // -----------------------
    m_lap_spinner->setVisible(has_laps);
    getWidget<LabelWidget>("lap-text")->setVisible(has_laps);
    if (has_laps)
    {
        if (UserConfigParams::m_artist_debug_mode)
            m_lap_spinner->setMin(0);
        else
            m_lap_spinner->setMin(1);
        m_lap_spinner->setValue(m_track->getActualNumberOfLap());
        race_manager->setNumLaps(m_lap_spinner->getValue());
    }

    // Number of AIs
    // -------------
    const bool has_AI = race_manager->hasAI();
    m_ai_kart_spinner->setVisible(has_AI);
    getWidget<LabelWidget>("ai-text")->setVisible(has_AI);
    if (has_AI)
    {
        m_ai_kart_spinner->setActivated();

        // Avoid negative numbers (which can happen if e.g. the number of karts
        // in a previous race was lower than the number of players now.
        int num_ai = UserConfigParams::m_num_karts - race_manager->getNumLocalPlayers();
        if (num_ai < 0) num_ai = 0;
        m_ai_kart_spinner->setValue(num_ai);
        race_manager->setNumKarts(num_ai + race_manager->getNumLocalPlayers());
        m_ai_kart_spinner->setMax(stk_config->m_max_karts - race_manager->getNumLocalPlayers());
        // A ftl reace needs at least three karts to make any sense
        if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER)
        {
            m_ai_kart_spinner->setMin(3-race_manager->getNumLocalPlayers());
        }
        else
            m_ai_kart_spinner->setMin(0);

    }   // has_AI

    // Reverse track
    // -------------
    const bool reverse_available = m_track->reverseAvailable() &&
               race_manager->getMinorMode() != RaceManager::MINOR_MODE_EASTER_EGG;
    m_reverse->setVisible(reverse_available);
    getWidget<LabelWidget>("reverse-text")->setVisible(reverse_available);
    if (reverse_available)
    {
        m_reverse->setState(race_manager->getReverseTrack());
    }
    else
        m_reverse->setState(false);

    // ---- High Scores
    m_highscore_label->setVisible(has_highscores);

    m_kart_icons[0]->setVisible(has_highscores);
    m_kart_icons[1]->setVisible(has_highscores);
    m_kart_icons[2]->setVisible(has_highscores);

    m_highscore_entries[0]->setVisible(has_highscores);
    m_highscore_entries[1]->setVisible(has_highscores);
    m_highscore_entries[2]->setVisible(has_highscores);

    RibbonWidget* bt_start = getWidget<GUIEngine::RibbonWidget>("buttons");
    bt_start->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

}   // init

// ----------------------------------------------------------------------------

TrackInfoScreen::~TrackInfoScreen()
{
}   // ~TrackInfoScreen

// ----------------------------------------------------------------------------

void TrackInfoScreen::updateHighScores()
{
    if (!race_manager->modeHasHighscores())
        return;

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
    const int num_laps = race_manager->modeHasLaps() ? m_lap_spinner->getValue()
                                                     : -1;
    const bool reverse_track = m_reverse == NULL ? false
                                                 : m_reverse->getState();
    // Avoid negative lap numbers (after e.g. easter egg mode).
    if(num_laps>=0)
        m_track->setActualNumberOfLaps(num_laps);
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
        race_manager->setReverseTrack(m_reverse->getState());
        // Makes sure the highscores get swapped when clicking the 'reverse'
        // checkbox.
        updateHighScores();
    }
    else if (name == "lap-spinner")
    {
        assert(race_manager->modeHasLaps());
        const int num_laps = m_lap_spinner->getValue();
        race_manager->setNumLaps(num_laps);
        UserConfigParams::m_num_laps = num_laps;
        updateHighScores();
    }
    else if (name=="ai-spinner")
    {
        const int num_ai = m_ai_kart_spinner->getValue();
        race_manager->setNumKarts( race_manager->getNumLocalPlayers() + num_ai );
        UserConfigParams::m_num_karts = race_manager->getNumLocalPlayers() + num_ai;
    }
}   // eventCallback

// ----------------------------------------------------------------------------
