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

#include "states_screens/dialogs/track_info_dialog.hpp"

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
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

// ------------------------------------------------------------------------------------------------------

TrackInfoDialog::TrackInfoDialog(const std::string& ribbonItem, const std::string& trackIdent,
                                 const irr::core::stringw& trackName, ITexture* screenshot,
                                 const float w, const float h) : ModalDialog(w, h)
{
    loadFromFile("track_info_dialog.stkgui");

    const bool has_laps       = race_manager->modeHasLaps();
    const bool has_highscores = race_manager->modeHasHighscores();

    m_track_ident = trackIdent;
    m_ribbon_item = ribbonItem;

    getWidget<LabelWidget>("name")->setText(trackName.c_str(), false);

    Track* track = track_manager->getTrack(trackIdent);
    //I18N: when showing who is the author of track '%s' (place %s where the name of the author should appear)
    getWidget<LabelWidget>("author")->setText( _("Track by %s", track->getDesigner().c_str()), false );


    // ---- Track screenshot
    Widget* screenshot_div = getWidget("screenshot_div");
    IconButtonWidget* screenshotWidget = new IconButtonWidget(IconButtonWidget::SCALE_MODE_KEEP_CUSTOM_ASPECT_RATIO,
                                                              false /* tab stop */, false /* focusable */);
    // images are saved squared, but must be stretched to 4:
    screenshotWidget->setCustomAspectRatio(4.0f / 3.0f);

    screenshotWidget->m_x = screenshot_div->m_x;
    screenshotWidget->m_y = screenshot_div->m_y;
    screenshotWidget->m_w = screenshot_div->m_w;
    screenshotWidget->m_h = screenshot_div->m_h;

    // temporary icon, will replace it just after (but it will be shown if the given icon is not found)
    screenshotWidget->m_properties[PROP_ICON] = "gui/main_help.png";
    screenshotWidget->setParent(m_irrlicht_window);
    screenshotWidget->add();

    if (screenshot != NULL)
    {
        screenshotWidget->setImage(screenshot);
    }
    m_widgets.push_back(screenshotWidget);


    // ---- Lap count m_spinner
    if (has_laps)
    {
        m_spinner = getWidget<SpinnerWidget>("lapcountspinner");

        m_spinner->m_properties[PROP_ID] = "lapcountspinner";
        if (UserConfigParams::m_artist_debug_mode)
        {
            m_spinner->setMin(0);
        }

        //I18N: In the track setup screen (number of laps choice, where %i is the number)
        //m_spinner->setText( _("%i laps") );
        m_spinner->setValue( UserConfigParams::m_num_laps );
        //m_spinner->getIrrlichtElement()->setTabStop(true);
        //m_spinner->getIrrlichtElement()->setTabGroup(false);

        const int num_laps = m_spinner->getValue();
        race_manager->setNumLaps(num_laps);
    }
    else
    {
        getWidget<SpinnerWidget>("lapcountspinner")->setVisible(false);
        m_spinner = NULL;
    }


    // Reverse track
    const bool reverse_available = track->reverseAvailable() &&
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

    getWidget<ButtonWidget>("start")->setFocusForPlayer( PLAYER_ID_GAME_MASTER );

}   // TrackInfoDialog

// ------------------------------------------------------------------------------------------------------

TrackInfoDialog::~TrackInfoDialog()
{
    // Place focus back on selected track, in case the dialog was cancelled and we're back to
    // the track selection screen after
    GUIEngine::Screen* curr_screen = GUIEngine::getCurrentScreen();
    if (curr_screen->getName() == "tracks.stkgui")
    {
        ((TracksScreen*)curr_screen)->setFocusOnTrack(m_ribbon_item);
    }

}   // ~TrackInfoDialog

// ------------------------------------------------------------------------------------------------------

void TrackInfoDialog::updateHighScores()
{
    std::string game_mode_ident = RaceManager::getIdentOf( race_manager->getMinorMode() );
    const Highscores::HighscoreType type = "HST_" + game_mode_ident;

    Highscores* highscores =
        highscore_manager->getHighscores(type,
                                         race_manager->getNumberOfKarts(),
                                         race_manager->getDifficulty(),
                                         m_track_ident,
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

// ------------------------------------------------------------------------------------------------------

void TrackInfoDialog::onEnterPressedInternal()
{

    // Create a copy of member variables we still need, since they will
    // not be accessible after dismiss:
    const int num_laps = (m_spinner == NULL ? -1 : m_spinner->getValue());
    const bool reverse_track = m_checkbox == NULL ? false
                                                  : m_checkbox->getState();
    race_manager->setReverseTrack(reverse_track);
    std::string track_ident = m_track_ident;
    // Disable accidentally unlocking of a challenge
    PlayerManager::getCurrentPlayer()->setCurrentChallenge("");

    ModalDialog::dismiss();
    race_manager->startSingleRace(track_ident, num_laps, false);
}   // onEnterPressedInternal

// ------------------------------------------------------------------------------------------------------

GUIEngine::EventPropagation TrackInfoDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "start" )
    {
        onEnterPressedInternal();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "closePopup")
    {
        ModalDialog::dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "reverse")
    {
        race_manager->setReverseTrack(m_checkbox->getState());
        // Makes sure the highscores get swapped when clicking the 'reverse'
        // checkbox.
        if (race_manager->modeHasHighscores())
        {
            updateHighScores();
        }
    }
    else if (eventSource == "lapcountspinner")
    {
        assert(m_spinner != NULL);
        const int num_laps = m_spinner->getValue();
        race_manager->setNumLaps(num_laps);
        UserConfigParams::m_num_laps = num_laps;
        updateHighScores();
    }

    return GUIEngine::EVENT_LET;
}   // processEvent

// ------------------------------------------------------------------------------------------------------
