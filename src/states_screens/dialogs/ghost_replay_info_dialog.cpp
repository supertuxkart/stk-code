//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#include "states_screens/dialogs/ghost_replay_info_dialog.hpp"

#include "config/player_manager.hpp"
#include "guiengine/CGUISpriteBank.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "race/race_manager.hpp"
#include "replay/replay_play.hpp"
#include "states_screens/ghost_replay_selection.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::core;

// -----------------------------------------------------------------------------
GhostReplayInfoDialog::GhostReplayInfoDialog(unsigned int replay_id,
                     uint64_t compare_replay_uid, bool compare_ghost)
                      : ModalDialog(0.95f,0.75f), m_replay_id(replay_id)
{
    m_self_destroy         = false;
    m_record_race          = false;
    m_watch_only           = false;

    m_compare_ghost        = compare_ghost;
    m_compare_replay_uid   = compare_replay_uid;

    m_rd = ReplayPlay::get()->getReplayData(m_replay_id);

    loadFromFile("ghost_replay_info_dialog.stkgui");

    Track* track = track_manager->getTrack(m_rd.m_track_name);

    m_track_screenshot_widget = getWidget<IconButtonWidget>("track_screenshot");
    m_track_screenshot_widget->setFocusable(false);
    m_track_screenshot_widget->m_tab_stop = false;

    // temporary icon, will replace it just after (but it will be shown if the given icon is not found)
    m_track_screenshot_widget->m_properties[PROP_ICON] = "gui/icons/main_help.png";

    irr::video::ITexture* image = STKTexManager::getInstance()
        ->getTexture(track->getScreenshotFile(),
        "While loading screenshot for track '%s':", track->getFilename());
    if(!image)
    {
        image = STKTexManager::getInstance()->getTexture("main_help.png",
            "While loading screenshot for track '%s':", track->getFilename());
    }
    if (image != NULL)
        m_track_screenshot_widget->setImage(image);

    // TODO : small refinement, add the possibility to tab stops for lists
    //        to make this unselectable by keyboard/mouse
    m_replay_info_widget = getWidget<GUIEngine::ListWidget>("current_replay_info");
    assert(m_replay_info_widget != NULL);

    /* Used to display kart icons for the selected replay(s) */
    irr::gui::STKModifiedSpriteBank *icon_bank = GhostReplaySelection::getInstance()->getIconBank();
    int icon_height = GUIEngine::getFontHeight() * 3 / 2;
    m_replay_info_widget->setIcons(icon_bank, (int)icon_height);

    updateReplayDisplayedInfo();

    LabelWidget *name = getWidget<LabelWidget>("name");
    assert(name);
    name->setText(stringw((m_rd.m_custom_replay_file ? StringUtils::getBasename
        (m_rd.m_filename) : m_rd.m_filename).c_str()), false);

    m_back_widget = getWidget<IconButtonWidget>("back");

    // Non-deletable for custom (standard) replay file
    getWidget<IconButtonWidget>("remove")->setActive(!m_rd.m_custom_replay_file);

    m_action_widget = getWidget<RibbonWidget>("actions");
    m_record_widget = getWidget<CheckBoxWidget>("record-race");
    m_watch_widget = getWidget<CheckBoxWidget>("watch-only");
    m_compare_widget = getWidget<CheckBoxWidget>("compare-ghost");

    if (RaceManager::get()->getNumLocalPlayers() > 1)
    {
        // No watching replay when split-screen
        m_watch_widget->setVisible(false);
        getWidget<LabelWidget>("watch-only-text")->setVisible(false);
    }

    m_record_widget->setState(false);
    m_watch_widget->setState(m_compare_ghost);
    m_compare_widget->setState(m_compare_ghost);

    if (m_compare_ghost)
    {
        m_watch_only = true;
        m_record_race = false;
        m_record_widget->setState(false);
        m_record_widget->setVisible(!m_watch_only);
        getWidget<LabelWidget>("record-race-text")->setVisible(!m_watch_only);
    }

    // Display this checkbox only if there is another replay file to compare with
    getWidget<LabelWidget>("compare-ghost-text")->setVisible(m_compare_ghost);
    m_compare_widget->setVisible(m_compare_ghost);


    m_action_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    m_action_widget->select("start", PLAYER_ID_GAME_MASTER);
}   // GhostReplayInfoDialog
// -----------------------------------------------------------------------------
GhostReplayInfoDialog::~GhostReplayInfoDialog()
{
}   // ~GhostReplayInfoDialog

// -----------------------------------------------------------------------------
void GhostReplayInfoDialog::updateReplayDisplayedInfo()
{
    m_replay_info_widget->clear();

    bool is_linear = GhostReplaySelection::getInstance()->isActiveModeLinear();
    std::vector<GUIEngine::ListWidget::ListCell> row;

    // Display the header in normal cells
    // as the header doesn't work with modal dialogs
    if (is_linear)
        row.push_back(GUIEngine::ListWidget::ListCell
            (_C("column_name", "Reverse"), -1, 3, true));
    row.push_back(GUIEngine::ListWidget::ListCell
        (_C("column_name", "Difficulty"), -1, 4, true));
    if (is_linear)
        row.push_back(GUIEngine::ListWidget::ListCell
            (_C("column_name", "Laps"), -1, 3, true));
    row.push_back(GUIEngine::ListWidget::ListCell
        (_C("column_name", "Time"), -1, 3, true));
    row.push_back(GUIEngine::ListWidget::ListCell
        (_C("column_name", "Kart"), -1, 1, true));
    row.push_back(GUIEngine::ListWidget::ListCell
        (_C("column_name", "User"), -1, 5, true));
    row.push_back(GUIEngine::ListWidget::ListCell
        (_C("column_name", "Version"), -1, 2, true));

    m_replay_info_widget->addItem(StringUtils::toString(0), row);

    // Now display the data from the selected replay,
    // and if applicable from the replay to compare with
    int num_replays_to_list = (m_compare_ghost) ? 2 : 1;

    for (int i=1; i<=num_replays_to_list; i++)
    {
        row.clear();

        int id;

        if (i==1)
            id = m_replay_id;
        else
            id = ReplayPlay::get()->getReplayIdByUID(m_compare_replay_uid);

        const ReplayPlay::ReplayData& rd = ReplayPlay::get()->getReplayData(id);

        int icon = -1;

        for(unsigned int i=0; i<kart_properties_manager->getNumberOfKarts(); i++)
        {
            const KartProperties* prop = kart_properties_manager->getKartById(i);
            if (rd.m_kart_list[0] == prop->getIdent())
            {
                icon = i;
                break;
            }
        }

        if (icon == -1)
        {
            icon = GhostReplaySelection::getInstance()->getUnknownKartIcon();
            Log::warn("GhostReplayInfoDialog", "Kart not found, using default icon.");
        }

        if (is_linear)
            row.push_back(GUIEngine::ListWidget::ListCell
                (rd.m_reverse ? _("Yes") : _("No"), -1, 3, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (RaceManager::get()->
                getDifficultyName((RaceManager::Difficulty) rd.m_difficulty),
                                                              -1, 4, true));
        if (is_linear)
            row.push_back(GUIEngine::ListWidget::ListCell
                (StringUtils::toWString(rd.m_laps), -1, 3, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (StringUtils::toWString(StringUtils::timeToString(rd.m_min_time)), -1, 3, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            ("", icon, 1, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (rd.m_user_name.empty() ? " " : rd.m_user_name, -1, 5, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (rd.m_stk_version.empty() ? " " : rd.m_stk_version, -1, 2, true));

        m_replay_info_widget->addItem(StringUtils::toString(i), row);
    } // for num_replays_to_list
} // updateReplayDisplayedInfo

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation
    GhostReplayInfoDialog::processEvent(const std::string& event_source)
{

    if (event_source == "actions")
    {
        const std::string& selection =
                m_action_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if(selection == "start")
        {
            // Make sure to enable the correct race mode
            RaceManager::get()->setMinorMode(GhostReplaySelection::getInstance()->getActiveMode());

            bool reverse = m_rd.m_reverse;
            std::string track_name = m_rd.m_track_name;
            int laps = m_rd.m_laps;
            int replay_id = m_replay_id;

            RaceManager::get()->setRecordRace(m_record_race);
            RaceManager::get()->setWatchingReplay(m_watch_only);
            if (m_watch_only)
                RaceManager::get()->setDifficulty((RaceManager::Difficulty)m_rd.m_difficulty);
            ReplayPlay::get()->setReplayFile(replay_id);
            if (m_compare_ghost)
            {
                int second_replay_id = ReplayPlay::get()->getReplayIdByUID(m_compare_replay_uid);
                ReplayPlay::get()->setSecondReplayFile(second_replay_id, /* use a second replay*/ true);
                m_compare_ghost = false;
            }
            else
                ReplayPlay::get()->setSecondReplayFile(0, /* use a second replay*/ false);

            RaceManager::get()->setRaceGhostKarts(true);

            // The race manager automatically adds karts for the ghosts
            // so only set it to the number of human players
            RaceManager::get()->setNumKarts(RaceManager::get()->getNumLocalPlayers());

            // Disable accidentally unlocking of a challenge
            PlayerManager::getCurrentPlayer()->setCurrentChallenge("");

            RaceManager::get()->setReverseTrack(reverse);

            //Reset comparison if active
            GhostReplaySelection::getInstance()->setCompare(false);

            ModalDialog::dismiss();
          
            if (RaceManager::get()->isWatchingReplay())
                RaceManager::get()->startWatchingReplay(track_name, laps);
            else
                RaceManager::get()->startSingleRace(track_name, laps, false);

            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == "add-ghost-to-compare")
        {
            // First set values for comparison
            m_compare_replay_uid = m_rd.m_replay_uid;

            m_compare_ghost = true;

            refreshMainScreen();

            // Now quit the dialog
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == "remove")
        {
            std::string fn = m_rd.m_filename;
            ModalDialog::dismiss();

            dynamic_cast<GhostReplaySelection*>(GUIEngine::getCurrentScreen())
                ->onDeleteReplay(fn);
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "back")
        {
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
    }

    if (event_source == "record-race")
    {
        m_record_race = m_record_widget->getState();
    }
    else if (event_source == "watch-only")
    {
        m_watch_only = m_watch_widget->getState();
        m_record_race = false;
        m_record_widget->setState(false);
        m_record_widget->setVisible(!m_watch_only);
        getWidget<LabelWidget>("record-race-text")->setVisible(!m_watch_only);
        if (!m_watch_only && m_compare_ghost)
        {
            m_compare_ghost = false;
            m_compare_widget->setState(false);
            refreshMainScreen();

            m_replay_id = ReplayPlay::get()->getReplayIdByUID(m_rd.m_replay_uid);
        }
    }

    else if (event_source == "compare-ghost")
    {
        m_compare_ghost = m_compare_widget->getState();
        m_record_race = false;
        m_record_widget->setState(false);
        if (m_compare_ghost)
        {
            m_watch_only = true;
            m_watch_widget->setState(true);
        }
        m_record_widget->setVisible(!m_watch_only);
        getWidget<LabelWidget>("record-race-text")->setVisible(!m_watch_only);

        refreshMainScreen();

        m_replay_id = ReplayPlay::get()->getReplayIdByUID(m_rd.m_replay_uid);
    }

    return GUIEngine::EVENT_LET;
}   // processEvent

// -----------------------------------------------------------------------------
bool GhostReplayInfoDialog::onEscapePressed()
{
    if (m_back_widget->isActivated())
        m_self_destroy = true;
    return false;
}   // onEscapePressed

// -----------------------------------------------------------------------------
void GhostReplayInfoDialog::onUpdate(float dt)
{
    if (m_self_destroy)
    {
        ModalDialog::clearWindow();
        ModalDialog::dismiss();
        return;
    }
}   // onUpdate

void GhostReplayInfoDialog::refreshMainScreen()
{
    GhostReplaySelection::getInstance()->setCompare(m_compare_ghost);
    GhostReplaySelection::getInstance()->setCompareReplayUid(m_compare_replay_uid);

    // Refresh the list to have only compatible replays
    dynamic_cast<GhostReplaySelection*>(GUIEngine::getCurrentScreen())
        ->refresh();
}
