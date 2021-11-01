//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
//            (C) 2014-2015 Joerg Henrichs
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
#include "graphics/material.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "guiengine/CGUISpriteBank.hpp"
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

// ----------------------------------------------------------------------------
/** Constructor, which loads the corresponding track_info.stkgui file. */
TrackInfoScreen::TrackInfoScreen()
          : Screen("track_info.stkgui")
{
}   // TrackInfoScreen

// ----------------------------------------------------------------------------
/* Saves some often used pointers. */
void TrackInfoScreen::loadedFromFile()
{
    m_target_type_spinner   = getWidget<SpinnerWidget>("target-type-spinner");
    m_target_type_label     = getWidget <LabelWidget>("target-type-text");
    m_ai_blue_spinner       = getWidget<SpinnerWidget>("ai-blue-spinner");
    m_ai_blue_label         = getWidget <LabelWidget>("ai-blue-text");
    m_ai_blue_div           = getWidget<Widget>("ai-blue-div");
    m_target_type_div       = getWidget<Widget>("target-type-div");
    m_target_value_spinner  = getWidget<SpinnerWidget>("target-value-spinner");
    m_target_value_label    = getWidget<LabelWidget>("target-value-text");
    m_ai_kart_spinner       = getWidget<SpinnerWidget>("ai-spinner");
    m_ai_kart_label         = getWidget<LabelWidget>("ai-text");
    m_option                = getWidget<CheckBoxWidget>("option");
    m_record_race           = getWidget<CheckBoxWidget>("record");
    m_option->setState(false);
    m_record_race->setState(false);

    m_icon_bank = new irr::gui::STKModifiedSpriteBank( GUIEngine::getGUIEnv());

    for (unsigned int i=0; i < kart_properties_manager->getNumberOfKarts(); i++)
    {
        const KartProperties* prop = kart_properties_manager->getKartById(i);
        m_icon_bank->addTextureAsSprite(prop->getIconMaterial()->getTexture());
    }

    video::ITexture* kart_not_found = irr_driver->getTexture(
              file_manager->getAsset(FileManager::GUI_ICON, "random_kart.png"));

    m_icon_unknown_kart = m_icon_bank->addTextureAsSprite(kart_not_found);

    m_highscore_label = getWidget<LabelWidget>("highscores");

    for (unsigned int i=0;i<HIGHSCORE_COUNT;i++)
    {
        m_highscore_entries = getWidget<ListWidget>("highscore_entries");
    }
    
    GUIEngine::IconButtonWidget* screenshot = getWidget<IconButtonWidget>("screenshot");
    screenshot->setFocusable(false);
    screenshot->m_tab_stop = false;

    m_is_soccer = false;
    m_show_ffa_spinner = false;

}   // loadedFromFile



void TrackInfoScreen::beforeAddingWidget()
{
    m_is_soccer = RaceManager::get()->isSoccerMode();
    m_is_lap_trial = RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_LAP_TRIAL;
    m_show_ffa_spinner = RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_3_STRIKES
                        || RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_FREE_FOR_ALL;

    m_target_type_div->setCollapsed(!m_is_soccer && !m_show_ffa_spinner, this);

    // show 'Number of blue team AI karts' if soccer
    m_ai_blue_div->setCollapsed(!(RaceManager::get()->isSoccerMode()), this);
} // beforeAddingWidget

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
    m_record_this_race = false;

    const int max_arena_players = m_track->getMaxArenaPlayers();
    const int local_players     = RaceManager::get()->getNumLocalPlayers();
    const bool has_laps         = RaceManager::get()->modeHasLaps();
    const bool has_highscores   = RaceManager::get()->modeHasHighscores();

    getWidget<LabelWidget>("name")->setText(m_track->getName(), false);

    //I18N: when showing who is the author of track '%s'
    //I18N: (place %s where the name of the author should appear)
    getWidget<LabelWidget>("author")->setText( _("Track by %s", m_track->getDesigner()),
                                               false );

    LabelWidget* max_players = getWidget<LabelWidget>("max-arena-players");
    max_players->setVisible(m_track->isArena());
    if (m_track->isArena())
    {
        //I18N: the max players supported by an arena.
        max_players->setText( _("Max players supported: %d", max_arena_players), false );
    }

    // ---- Track screenshot
    GUIEngine::IconButtonWidget* screenshot = getWidget<IconButtonWidget>("screenshot");

    ITexture* image = STKTexManager::getInstance()
        ->getTexture(m_track->getScreenshotFile(),
        "While loading screenshot for track '%s':", m_track->getFilename());
    if(!image)
    {
        image = STKTexManager::getInstance()->getTexture(GUIEngine::getSkin()->getThemedIcon("gui/icons/track_unknown.png"),
            "While loading screenshot for track '%s':", m_track->getFilename());
    }
    if (image != NULL)
        screenshot->setImage(image);

    m_target_value_spinner->setVisible(false);
    m_target_value_label->setVisible(false);

    m_ai_blue_spinner->setVisible(false);
    m_ai_blue_label->setVisible(false);
    m_ai_blue_label->setActive(false);

    // Number of AIs
    // -------------
    const bool has_AI =
        (   RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_3_STRIKES
         || RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_FREE_FOR_ALL
         || m_is_soccer ?
         m_track->hasNavMesh() && (max_arena_players - local_players) > 0 :
         RaceManager::get()->hasAI());

    m_ai_kart_spinner->setVisible(has_AI);
    m_ai_kart_label->setVisible(has_AI);

    if (has_AI)
    {
        m_ai_kart_label->setText(_("Number of AI karts"), false); // change to label back for other modes

        // Set the default number of AIs
        int num_ai = int(UserConfigParams::m_num_karts_per_gamemode
            [RaceManager::get()->getMinorMode()]) - local_players;

        // Avoid negative numbers (which can happen if e.g. the number of karts
        // in a previous race was lower than the number of players now.

        if (num_ai < 0) num_ai = 0;
        m_ai_kart_spinner->setValue(num_ai);

        RaceManager::get()->setNumKarts(num_ai + local_players);

        // Set the max karts supported based on the selected battle arena
        if( RaceManager::get()->isBattleMode() || m_is_soccer)
        {
            m_ai_kart_spinner->setMax(max_arena_players - local_players);
            m_ai_blue_spinner->setMax(max_arena_players - local_players);
        }
        else
            m_ai_kart_spinner->setMax(stk_config->m_max_karts - local_players);

        // A ftl race needs at least three karts to make any sense
        if(RaceManager::get()->isFollowMode())
        {
            m_ai_kart_spinner->setMin(std::max(0, 3 - local_players));
        }
        // In single player battle, make sure there is at least one
        // AI against the player. Soccer is handled in setSoccerWidgets
        else if(   local_players == 1
                && !UserConfigParams::m_artist_debug_mode
                && RaceManager::get()->isBattleMode())
        {
            m_ai_kart_spinner->setMin(1);
        }
        else
        {
            m_ai_kart_spinner->setMin(0);
            m_ai_blue_spinner->setMin(0);
        }
    }   // has_AI
    else
        RaceManager::get()->setNumKarts(local_players);

    // Soccer options
    // -------------
    if (m_is_soccer)
        setSoccerWidgets(has_AI);

    // options for free-for-all and three strikes battle
    // -------------
    if (m_show_ffa_spinner)
    {
        m_target_type_label->setText(_("Game mode"), false);
        m_target_type_spinner->clearLabels();
        m_target_type_spinner->addLabel(_("3 Strikes Battle"));
        m_target_type_spinner->addLabel(_("Free-For-All"));
        m_target_type_spinner->setValue(UserConfigParams::m_use_ffa_mode ? 1 : 0);

        m_target_value_label->setText(_("Maximum time (min.)"), false);
        m_target_value_spinner->setValue(UserConfigParams::m_ffa_time_limit);

        m_target_value_label->setVisible(UserConfigParams::m_use_ffa_mode);
        m_target_value_spinner->setVisible(UserConfigParams::m_use_ffa_mode);
    }

    // Lap count m_lap_spinner
    // -----------------------
    if (has_laps)
    {
        m_target_value_spinner->setVisible(true);
        m_target_value_label->setVisible(true);

        if (UserConfigParams::m_artist_debug_mode)
            m_target_value_spinner->setMin(0);
        else
            m_target_value_spinner->setMin(1);
        m_target_value_spinner->setValue(m_track->getActualNumberOfLap());
        RaceManager::get()->setNumLaps(m_target_value_spinner->getValue());

        m_target_value_label->setText(_("Number of laps"), false);
    }

    if (m_is_lap_trial)
    {
        m_target_value_spinner->setVisible(true);
        m_target_value_label->setVisible(true);

        m_target_value_label->setText(_("Maximum time (min.)"), false);
        m_target_value_spinner->setValue(UserConfigParams::m_lap_trial_time_limit);
    }
    // Reverse track or random item in arena
    // -------------
    const bool reverse_available =     m_track->reverseAvailable()
                                   && !(RaceManager::get()->isEggHuntMode());
    const bool random_item = m_track->hasNavMesh();

    m_option->setVisible(reverse_available || random_item);
    getWidget<LabelWidget>("option-text")->setVisible(reverse_available || random_item);
    if (reverse_available)
    {
        //I18N: In the track info screen
        getWidget<LabelWidget>("option-text")->setText(_("Drive in reverse"), false);
    }
    else if (random_item)
    {
        //I18N: In the track info screen
        getWidget<LabelWidget>("option-text")->setText(_("Random item location"), false);
    }

    if (reverse_available)
        m_option->setState(RaceManager::get()->getReverseTrack());
    else if (random_item)
        m_option->setState(UserConfigParams::m_random_arena_item);
    else
        m_option->setState(false);

    // Record race or not
    // -------------
    const bool record_available = (RaceManager::get()->isTimeTrialMode() || RaceManager::get()->isEggHuntMode());
    m_record_race->setVisible(record_available);
    getWidget<LabelWidget>("record-race-text")->setVisible(record_available);
    if (RaceManager::get()->isRecordingRace())
    {
        // isRecordingRace() is true when it's pre-set by ghost replay selection
        // which force record this race
        m_record_this_race = true;
        m_record_race->setState(true);
        m_record_race->setActive(false);
        m_ai_kart_spinner->setValue(0);
        m_ai_kart_spinner->setActive(false);
        RaceManager::get()->setNumKarts(RaceManager::get()->getNumLocalPlayers());
        
        UserConfigParams::m_num_karts_per_gamemode[RaceManager::get()->getMinorMode()] = RaceManager::get()->getNumLocalPlayers();
    }
    else if (record_available)
    {
        m_record_race->setActive(true);
        m_record_race->setState(false);
        m_ai_kart_spinner->setActive(true);
    }
    else
        m_ai_kart_spinner->setActive(true);

    // ---- High Scores
    m_highscore_label->setText(_("High Scores"), false);
    m_highscore_label->setVisible(has_highscores);

    if (has_highscores)
    {
        int icon_height = GUIEngine::getFontHeight();
        int row_height = GUIEngine::getFontHeight() * 1.2f;
                                                    
        m_icon_bank->setScale(icon_height/128.0f);
        m_icon_bank->setTargetIconSize(128, 128);
        m_highscore_entries->setIcons(m_icon_bank, (int)row_height);
        m_highscore_entries->setVisible(has_highscores);

        updateHighScores();
    } //has_highscores

    RibbonWidget* bt_start = getWidget<GUIEngine::RibbonWidget>("buttons");
    bt_start->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
}   // init

void TrackInfoScreen::setSoccerWidgets(bool has_AI)
{
    // Set up the spinner for the choice between time or goal limit
    m_target_type_label->setText(_("Soccer game type"), false);

    m_target_value_spinner->setVisible(true);
    m_target_value_label->setVisible(true);

    if (UserConfigParams::m_num_goals <= 0)
        UserConfigParams::m_num_goals = UserConfigParams::m_num_goals.getDefaultValue();

    if (UserConfigParams::m_soccer_time_limit <= 0)
        UserConfigParams::m_soccer_time_limit = UserConfigParams::m_soccer_time_limit.getDefaultValue();

    m_target_type_spinner->clearLabels();
    m_target_type_spinner->addLabel(_("Time limit"));
    m_target_type_spinner->addLabel(_("Goals limit"));
    m_target_type_spinner->setValue(UserConfigParams::m_soccer_use_time_limit ? 0 : 1);

    setSoccerTarget(UserConfigParams::m_soccer_use_time_limit /* true if time limit */);

    if (has_AI)
    {
        const int max_arena_players = m_track->getMaxArenaPlayers();
        const int local_players     = RaceManager::get()->getNumLocalPlayers();
        // Set up the spinners for the number of red and blue AIs
        m_ai_blue_spinner->setVisible(true);
        m_ai_blue_label->setVisible(true);
        m_ai_blue_spinner->setActive(true);
        m_ai_kart_label->setText(_("Number of red team AI karts"), false);
        m_ai_kart_spinner->setValue(UserConfigParams::m_soccer_red_ai_num);
        m_ai_blue_spinner->setValue(UserConfigParams::m_soccer_blue_ai_num);

        // Check if there's any local players in both team
        int num_blue_players = 0, num_red_players = 0;
    
        for (int i = 0; i < local_players; i++)
        {
            KartTeam team = RaceManager::get()->getKartInfo(i).getKartTeam();
            // Happens in profiling mode
            if (team == KART_TEAM_NONE)
                num_blue_players++; // No team will be set to blue
            else
                team == KART_TEAM_BLUE ? num_blue_players++ : num_red_players++;
        }

        const int max_num_ai = max_arena_players - local_players;
        // Make sure each team has at least 1 (ai + player)
        bool reuse_ai = ((num_blue_players + UserConfigParams::m_soccer_blue_ai_num) > 0) &&
                        ((num_red_players  + UserConfigParams::m_soccer_red_ai_num ) > 0) &&
                        ((UserConfigParams::m_soccer_red_ai_num + UserConfigParams::m_soccer_blue_ai_num) <= max_num_ai);

        // Try the saved values.
        // If they can't be used, use default balanced values
        if (!reuse_ai)
        {
            const int additional_blue = num_red_players - num_blue_players;
            int num_blue_ai = (max_num_ai - additional_blue) / 2 + additional_blue;
            int num_red_ai  = (max_num_ai - additional_blue) / 2;

            if ((max_num_ai + additional_blue)%2 == 1)
                additional_blue < 0 ? num_red_ai++ : num_blue_ai++;

            UserConfigParams::m_soccer_red_ai_num  = num_red_ai;
            UserConfigParams::m_soccer_blue_ai_num = num_blue_ai;
            m_ai_kart_spinner->setValue(UserConfigParams::m_soccer_red_ai_num);
            m_ai_blue_spinner->setValue(UserConfigParams::m_soccer_blue_ai_num);
        }

        if(  local_players == 1
           && !UserConfigParams::m_artist_debug_mode)
        {
            if(num_blue_players == 0)
                m_ai_blue_spinner->setMin(1);
            if(num_red_players == 0)
                m_ai_kart_spinner->setMin(1);
        }

        if(num_blue_players == 0 && !UserConfigParams::m_artist_debug_mode)
            m_ai_kart_spinner->setMax(max_arena_players - local_players - 1);
        if(num_red_players == 0 && !UserConfigParams::m_artist_debug_mode)
            m_ai_blue_spinner->setMax(max_arena_players - local_players - 1);
    }
} // setSoccerWidgets

void TrackInfoScreen::setSoccerTarget(bool time_limit)
{
    if (time_limit)
    {
        m_target_value_label->setText(_("Maximum time (min.)"), false);
        m_target_value_spinner->setValue(UserConfigParams::m_soccer_time_limit);
    }
    else
    {
        m_target_value_label->setText(_("Number of goals to win"), false);
        m_target_value_spinner->setValue(UserConfigParams::m_num_goals);
    }
} // setSoccerTarget

// ----------------------------------------------------------------------------
TrackInfoScreen::~TrackInfoScreen()
{
}   // ~TrackInfoScreen


// ----------------------------------------------------------------------------
void TrackInfoScreen::tearDown()
{
    // This will save number of ai karts per game mode
    user_config->saveConfig();
    m_highscore_entries->setIcons(NULL);
}

// ----------------------------------------------------------------------------
void TrackInfoScreen::unloaded()
{
    delete m_icon_bank;
    m_icon_bank = NULL;
}   // unloaded

// ----------------------------------------------------------------------------
void TrackInfoScreen::updateHighScores()
{
    if (!RaceManager::get()->modeHasHighscores())
        return;

    std::string game_mode_ident = RaceManager::getIdentOf( RaceManager::get()->getMinorMode() );
    const Highscores::HighscoreType type = "HST_" + game_mode_ident;

    Highscores* highscores =
        highscore_manager->getHighscores(type,
                                         RaceManager::get()->getNumberOfKarts(),
                                         RaceManager::get()->getDifficulty(),
                                         m_track->getIdent(),
                                         RaceManager::get()->isLapTrialMode() ? m_target_value_spinner->getValue() * 60 : RaceManager::get()->getNumLaps(),
                                         RaceManager::get()->getReverseTrack()  );
    const int amount = highscores->getNumberEntries();

    std::string kart_name;
    core::stringw name;
    float time;

    int time_precision = RaceManager::get()->currentModeTimePrecision();

    m_highscore_entries->clear();
    
    // fill highscore entries
    for (int n=0; n<HIGHSCORE_COUNT; n++)
    {
        irr::core::stringw line;
        int icon = -1;

        // Check if this entry is filled or still empty
        if (n < amount)
        {
            highscores->getEntry(n, kart_name, name, &time);

            std::string highscore_string;
            if (RaceManager::get()->isLapTrialMode())
                highscore_string = std::to_string(static_cast<int>(time));
            else
                highscore_string = StringUtils::timeToString(time, time_precision);

            for(unsigned int i=0; i<kart_properties_manager->getNumberOfKarts(); i++)
            {
                const KartProperties* prop = kart_properties_manager->getKartById(i);
                if (kart_name == prop->getIdent())
                {
                    icon = i;
                    break;
                }
            }
        
            line = name + "    " + core::stringw(highscore_string.c_str());
        }
        else
        {
            //I18N: for empty highscores entries
            line = _("(Empty)");
        }

        if (icon == -1)
        {
            icon = m_icon_unknown_kart;
        }

        std::vector<GUIEngine::ListWidget::ListCell> row;
        
        row.push_back(GUIEngine::ListWidget::ListCell(line.c_str(), icon, 5, false));
        m_highscore_entries->addItem(StringUtils::toString(n), row);
    }
}   // updateHighScores

// ----------------------------------------------------------------------------

void TrackInfoScreen::onEnterPressedInternal()
{
    RaceManager::get()->setRecordRace(m_record_this_race);
    // Create a copy of member variables we still need, since they will
    // not be accessible after dismiss:
    const int num_laps = RaceManager::get()->modeHasLaps() ? m_target_value_spinner->getValue()
                                                     : -1;
    const bool option_state = m_option == NULL ? false
                                               : m_option->getState();
    // Avoid negative lap numbers (after e.g. easter egg mode).
    if(num_laps>=0)
        m_track->setActualNumberOfLaps(num_laps);

    if(m_track->hasNavMesh())
        UserConfigParams::m_random_arena_item = option_state;
    else
        RaceManager::get()->setReverseTrack(option_state);

    // Avoid invaild Ai karts number during switching game modes
    const int max_arena_players = m_track->getMaxArenaPlayers();
    const int local_players = RaceManager::get()->getNumLocalPlayers();
    const bool has_AI =
        (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_3_STRIKES ||
         RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_FREE_FOR_ALL ||
         RaceManager::get()->isSoccerMode() ?
         m_track->hasNavMesh() && (max_arena_players - local_players) > 0 :
         RaceManager::get()->hasAI());

    int num_ai = 0;
    if (has_AI)
    {
        num_ai = m_ai_kart_spinner->getValue();
        
        if (m_is_soccer) // Soccer mode
            num_ai += m_ai_blue_spinner->getValue();
    }

    const int selected_target_type = m_target_type_spinner->getValue();
    const int selected_target_value = m_target_value_spinner->getValue();

    const bool enable_ffa = m_show_ffa_spinner && selected_target_type != 0;

    if (enable_ffa)
    {
        RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_FREE_FOR_ALL);
        RaceManager::get()->setHitCaptureTime(0, static_cast<float>(selected_target_value) * 60);
    }

    if (m_is_soccer)
    {
        if (selected_target_type == 0)
            RaceManager::get()->setTimeTarget(static_cast<float>(selected_target_value) * 60);
        else
            RaceManager::get()->setMaxGoal(selected_target_value);
    }

    if (m_is_lap_trial)
    {
        RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_LAP_TRIAL);
        RaceManager::get()->setTimeTarget(static_cast<float>(selected_target_value) * 60);
    }

    if (UserConfigParams::m_num_karts_per_gamemode
        [RaceManager::get()->getMinorMode()] != unsigned(local_players + num_ai))
    {
        UserConfigParams::m_num_karts_per_gamemode[RaceManager::get()->getMinorMode()] = local_players + num_ai;
    }

    // Disable accidentally unlocking of a challenge
    PlayerManager::getCurrentPlayer()->setCurrentChallenge("");

    RaceManager::get()->setNumKarts(num_ai + local_players);
    if (m_is_soccer)
    {
        RaceManager::get()->setNumRedAI(m_ai_kart_spinner->getValue());
        RaceManager::get()->setNumBlueAI(m_ai_blue_spinner->getValue());
    }
    RaceManager::get()->startSingleRace(m_track->getIdent(), num_laps, false);
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
    else if (name == "target-type-spinner")
    {
        const bool target_value = m_target_type_spinner->getValue();
        if (m_is_soccer)
        {
            const bool timed = target_value == 0;
            UserConfigParams::m_soccer_use_time_limit = timed;

            setSoccerTarget(timed /* true if time limit */);
        }
        else if (m_show_ffa_spinner)
        {
            const bool enable_ffa = target_value != 0;
            UserConfigParams::m_use_ffa_mode = enable_ffa;

            m_target_value_label->setVisible(enable_ffa);
            m_target_value_spinner->setVisible(enable_ffa);
        }
    }
    else if (name == "target-value-spinner")
    {
        if (m_is_soccer)
        {
            const bool timed = m_target_type_spinner->getValue() == 0;
            if (timed)
                UserConfigParams::m_soccer_time_limit = m_target_value_spinner->getValue();
            else
                UserConfigParams::m_num_goals = m_target_value_spinner->getValue();
        }
        else if (m_show_ffa_spinner)
        {
            const bool enable_ffa = m_target_type_spinner->getValue() != 0;

            if (enable_ffa)
                UserConfigParams::m_ffa_time_limit = m_target_value_spinner->getValue();
        }
        else if (m_is_lap_trial)
        {
            UserConfigParams::m_lap_trial_time_limit = m_target_value_spinner->getValue();
            updateHighScores();
        }
        else
        {
            assert(RaceManager::get()->modeHasLaps());
            const int num_laps = m_target_value_spinner->getValue();
            RaceManager::get()->setNumLaps(num_laps);
            UserConfigParams::m_num_laps = num_laps;
            updateHighScores();
        }
    }
    else if (name == "option")
    {
        if (m_track->hasNavMesh())
        {
            UserConfigParams::m_random_arena_item = m_option->getState();
        }
        else
        {
            RaceManager::get()->setReverseTrack(m_option->getState());
            // Makes sure the highscores get swapped when clicking the 'reverse'
            // checkbox.
            updateHighScores();
        }
    }
    else if (name == "record")
    {
        const bool record = m_record_race->getState();
        m_record_this_race = record;
        // Disable the AI when recording ghost race
        m_ai_kart_spinner->setValue(0);
        m_ai_kart_spinner->setActive(!record);
        if (record)
        {
            RaceManager::get()->setNumKarts(RaceManager::get()->getNumLocalPlayers());
            UserConfigParams::m_num_karts_per_gamemode[RaceManager::get()->getMinorMode()] = RaceManager::get()->getNumLocalPlayers();
        }
    }
    else if (name=="ai-spinner")
    {
        if (m_is_soccer) // Soccer mode
        {            
            soccerSpinnerUpdate(false /* blue spinner */);            
        }
        else // Other modes
        {
            const int num_ai = m_ai_kart_spinner->getValue();
            RaceManager::get()->setNumKarts( RaceManager::get()->getNumLocalPlayers() + num_ai );
            UserConfigParams::m_num_karts_per_gamemode[RaceManager::get()->getMinorMode()] = RaceManager::get()->getNumLocalPlayers() + num_ai;
            updateHighScores();
        }
    }
    else if (name == "ai-blue-spinner" && m_is_soccer)
    {
        soccerSpinnerUpdate(true /* blue spinner */);  
    }
}   // eventCallback

void TrackInfoScreen::soccerSpinnerUpdate(bool blue_spinner)
{
    const int max_arena_players = m_track->getMaxArenaPlayers();
    const int local_players = RaceManager::get()->getNumLocalPlayers();
    const int num_ai = max_arena_players - local_players;

    // Reduce the value of the other spinner if going over the max total num of AI
    if(m_ai_kart_spinner->getValue() + m_ai_blue_spinner->getValue() > num_ai)
    {
        if (blue_spinner)
            m_ai_kart_spinner->setValue(num_ai - m_ai_blue_spinner->getValue());
        else
            m_ai_blue_spinner->setValue(num_ai - m_ai_kart_spinner->getValue());
    }

    KartTeam all_team = KART_TEAM_NONE;
    unsigned num_red = 0;
    unsigned num_blue = 0;
    for (unsigned i = 0; i < RaceManager::get()->getNumLocalPlayers(); i++)
    {
        RaceManager::get()->getKartInfo(i).getKartTeam() == KART_TEAM_RED ?
            num_red++ : num_blue++;
    }
    if (num_red == 0)
        all_team = KART_TEAM_BLUE;
    else if (num_blue == 0)
        all_team = KART_TEAM_RED;

    // Need at least 1 kart at each side to avoid soccer ai crash,
    // if there is any ai
    if (blue_spinner)
    {
        if (all_team == KART_TEAM_BLUE &&
            (m_ai_blue_spinner->getValue() == 0 || m_ai_kart_spinner->getValue() == 0))
        {
            if (m_ai_kart_spinner->getValue() == 0)
                m_ai_kart_spinner->setValue(1);
        }
        if (all_team == KART_TEAM_RED && m_ai_blue_spinner->getValue() == 0 &&
            m_ai_kart_spinner->getValue() != 0)
            m_ai_blue_spinner->setValue(1);
    }
    else
    {
        if (all_team == KART_TEAM_RED &&
            (m_ai_kart_spinner->getValue() == 0 || m_ai_blue_spinner->getValue() == 0))
        {
            if (m_ai_blue_spinner->getValue() == 0)
                m_ai_blue_spinner->setValue(1);
        }
        if (all_team == KART_TEAM_BLUE && m_ai_kart_spinner->getValue() == 0 &&
            m_ai_blue_spinner->getValue() != 0)
            m_ai_kart_spinner->setValue(1);
    }

    UserConfigParams::m_soccer_red_ai_num  = m_ai_kart_spinner->getValue();
    UserConfigParams::m_soccer_blue_ai_num = m_ai_blue_spinner->getValue();


} // soccerSpinnerUpdate
// ----------------------------------------------------------------------------
