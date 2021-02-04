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

#include "states_screens/high_score_selection.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/material.hpp"
#include "guiengine/CGUISpriteBank.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "race/highscores.hpp"
#include "race/highscore_manager.hpp"
#include "states_screens/dialogs/high_score_info_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/online/tracks_screen.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;

// ----------------------------------------------------------------------------
/** Constructor, which loads the stkgui file.
 */
HighScoreSelection::HighScoreSelection() : Screen("high_score_selection.stkgui")
{
    //m_is_comparing = false;
    //m_high_scores_to_compare_uid = 0;
}   // HighScoreSelection

// ----------------------------------------------------------------------------
/** Destructor.
 */
HighScoreSelection::~HighScoreSelection()
{
}   // HighScoreSelection

// ----------------------------------------------------------------------------
void HighScoreSelection::tearDown()
{
    m_high_scores_list_widget->setIcons(NULL);
}

// ----------------------------------------------------------------------------
void HighScoreSelection::unloaded()
{
    delete m_icon_bank;
    m_icon_bank = NULL;
}   // unloaded


// ----------------------------------------------------------------------------
/** Triggers a refresh of the high score entries list.
 */
void HighScoreSelection::refresh(bool forced_update, bool update_columns)
{
    if (highscore_manager->highscoresEmpty() || forced_update)
    {
        highscore_manager->clearHighscores();
        highscore_manager->loadHighscores();
    }
    //defaultSort();

    loadList();

    // Allow to disable a comparison, but not to start one
    //m_compare_toggle_widget->setVisible(m_is_comparing);
    //m_compare_toggle_widget->setState(m_is_comparing);
    //getWidget<LabelWidget>("compare-toggle-text")->setVisible(m_is_comparing);

    if (update_columns)
    {
        m_high_scores_list_widget->clearColumns();
        beforeAddingWidget();//Reload the columns used
    }
}   // refresh

// ----------------------------------------------------------------------------
/** Set pointers to the various widgets.
 */
void HighScoreSelection::loadedFromFile()
{
    m_high_scores_list_widget = getWidget<GUIEngine::ListWidget>("high_scores_list");
    assert(m_high_scores_list_widget != NULL);
    m_high_scores_list_widget->setColumnListener(this);
/*
    m_high_scores_difficulty_toggle_widget =
        getWidget<GUIEngine::CheckBoxWidget>("high_scores_difficulty_toggle");
    m_high_scores_difficulty_toggle_widget->setState( default value  true);
    //m_same_difficulty = m_high_scores_difficulty_toggle_widget->getState();

    m_high_scores_multiplayer_toggle_widget =
        getWidget<GUIEngine::CheckBoxWidget>("replay_multiplayer_toggle");
    m_high_scores_multiplayer_toggle_widget->setState( default value true  hide );
    m_multiplayer = !m_high_scores_multiplayer_toggle_widget->getState();

    m_high_scores_version_toggle_widget =
        getWidget<GUIEngine::CheckBoxWidget>("replay_version_toggle");
    m_high_scores_version_toggle_widget->setState( default value true);
    m_same_version = m_high_scores_version_toggle_widget->getState();

    m_best_times_toggle_widget =
        getWidget<GUIEngine::CheckBoxWidget>("best_times_toggle");
    m_best_times_toggle_widget->setState( default value false);
    m_best_times = m_best_times_toggle_widget->getState();

    m_compare_toggle_widget =
        getWidget<GUIEngine::CheckBoxWidget>("compare_toggle");
    m_compare_toggle_widget->setState( default value false);
    m_is_comparing = false;
    m_compare_toggle_widget->setVisible(false);
    getWidget<LabelWidget>("compare-toggle-text")->setVisible(false);
*/
    m_mode_tabs = getWidget<GUIEngine::RibbonWidget>("race_mode");
    m_active_mode = RaceManager::MINOR_MODE_NORMAL_RACE;
    m_active_mode_is_linear = true;

    m_icon_bank = new irr::gui::STKModifiedSpriteBank( GUIEngine::getGUIEnv());

    for(unsigned int i=0; i<kart_properties_manager->getNumberOfKarts(); i++)
    {
        const KartProperties* prop = kart_properties_manager->getKartById(i);
        m_icon_bank->addTextureAsSprite(prop->getIconMaterial()->getTexture());
    }

    video::ITexture* kart_not_found = irr_driver->getTexture(
                file_manager->getAsset(FileManager::GUI_ICON, "random_kart.png"));

    m_icon_unknown_kart = m_icon_bank->addTextureAsSprite(kart_not_found);

    video::ITexture* lock = irr_driver->getTexture( file_manager->getAsset(
                                        FileManager::GUI_ICON, "gui_lock.png"));

    m_icon_lock = m_icon_bank->addTextureAsSprite(lock);
}   // loadedFromFile

// ----------------------------------------------------------------------------
/** Clear the high score entry list, which will be reloaded.
 */
void HighScoreSelection::beforeAddingWidget()
{
    m_high_scores_list_widget->addColumn(_C("column_name", "Track"), 9 );
    if (m_active_mode_is_linear)
        m_high_scores_list_widget->addColumn(_C("column_name", "# of Karts"), 3);
    //if (!m_same_difficulty)
    m_high_scores_list_widget->addColumn(_C("column_name", "Difficulty"), 4);
    if (m_active_mode_is_linear)
    {
        m_high_scores_list_widget->addColumn(_C("column_name", "Laps"), 3);
        m_high_scores_list_widget->addColumn(_C("column_name", "Reverse"), 3);
    }
    //m_high_scores_list_widget->addColumn(_C("column_name", "Time"), 4);
    //m_high_scores_list_widget->addColumn(_C("column_name", "Kart"), 1);
    //m_high_scores_list_widget->addColumn(_C("column_name", "User"), 5);

    m_high_scores_list_widget->createHeader();
}   // beforeAddingWidget

// ----------------------------------------------------------------------------
void HighScoreSelection::init()
{
    Screen::init();
    //m_cur_difficulty = RaceManager::get()->getDifficulty();

    int icon_height = GUIEngine::getFontHeight();
    int row_height = GUIEngine::getFontHeight() * 5 / 4;

    // 128 is the height of the image file
    m_icon_bank->setScale(icon_height/128.0f);
    m_icon_bank->setTargetIconSize(128, 128);
    m_high_scores_list_widget->setIcons(m_icon_bank, (int)row_height);

    refresh(/*reload replay files*/ false, /* update columns */ true);
}   // init

// ----------------------------------------------------------------------------
/** Loads the list of all high score entries. The gui element will be
 *  updated.
 */
void HighScoreSelection::loadList()
{
    m_high_scores_list_widget->clear();

    if (highscore_manager->highscoresEmpty())
        return;
/*
    if (m_best_times)
    {
        //First of all, clear the best time index
        m_best_times_index.clear();

        // This is in O(N*M) ; N being the number of high score entries
        // and M the number of different configuration (which is
        // the final size of the best times list).
        // Each time has to be compared against the previous best times
        // up until all are checked or a high score entry with the same configuration
        // is found.
        for (unsigned int i = 0; i < highscore_manager->highscoresSize() ; i++)
        {
            const Highscores::Highscores* hs = highscore_manager->getHighscoresAt(i);

            if (m_same_difficulty && m_cur_difficulty !=
                (RaceManager::Difficulty)hs->m_difficulty)
                continue;

            Track* track = track_manager->getTrack(hs->m_track);

            if (track == NULL)
                continue;

            // If no other high score entry with the same configuration is found in the index
            // this is the best time
            bool is_best_time = true;
            bool replace_old_best = false;
            // This is the position inside the best_times_index itself,
            // not inside the full list of replay files
            unsigned int index_old_best = 0;

            for (unsigned int j = 0; j < m_best_times_index.size() ; j++)
            {
                // The replay in the best times index conform to the version and difficulty settings,
                // no need to check this again.
                const Highscores::Highscores* bt = highscore_manager->getHighscoresAt(m_best_times_index[j]);

                // If it's not the same track, check further in the index
                if (hs->m_track_name != bt.m_track_name)
                    continue;

                // If it's not the same difficulty, check further in the index
                if (hs->m_difficulty != bt.m_difficulty)
                    continue;

                // If it's not the same direction, check further in the index
                if (hs->m_reverse != bt.m_reverse)
                    continue;

                // If it's not the same lap numbers, check further in the index
                if (hs->m_number_of_laps != bt.m_number_of_laps)
                    continue;

                // The replay data have the same properties, compare the times
                if (hs->m_min_time < bt.m_min_time)
                {
                    replace_old_best = true;
                    index_old_best = j;
                }
                else
                    is_best_time = false;

                //No need to compare against other best times
                break;
            }

            if (is_best_time)
            {
                // Update the value
                if (replace_old_best)
                {
                    m_best_times_index[index_old_best] = i;
                }
                else
                {
                    m_best_times_index.push_back(i);
                }
            }
        }
    }

    // Sort the best times index for faster lookup
    if (m_best_times && m_best_times_index.size() > 1)
    {
        // std::sort sorts by default in ascending order (0,1,2...)
        std::sort (m_best_times_index.begin(), m_best_times_index.end());
    }

    unsigned int best_index = 0;

    // getReplayIdByUID will send 0 if the UID is incorrect,
    // and m_is_comparing will be false if it is incorrect,
    // so it always work
    //unsigned int compare_index = ReplayPlay::get()->getReplayIdByUID(m_high_scores_to_compare_uid);
    //const ReplayPlay::ReplayData& rd_compare = ReplayPlay::get()->getReplayData(compare_index);
*/
    for (int i = 0; i < highscore_manager->highscoresSize(); i++)
    {
        /*
        if (m_best_times)
        {
            // All best times have already been added
            if (best_index + 1 > m_best_times_index.size())
                break;
            // This works because the best times index is already sorted
            else if (m_best_times_index[best_index] == i)
                best_index++;
            // There are still best times to display
            // The current i don't correspond to a best time
            else
                continue;
        }
        */
        const Highscores* hs = highscore_manager->getHighscoresAt(i);

        //if (m_same_difficulty && m_cur_difficulty !=
        //    (RaceManager::Difficulty)hs->m_difficulty)
        //    continue;
/*
        // Only display replays comparable with the replay selected for comparison
        if (m_is_comparing)
        {
                // If it's not the same track, check further in the index
                if (hs->m_track_name != hs_compare.m_track_name)
                    continue;

                // If it's not the same direction, check further in the index
                if (hs->m_reverse != hs_compare.m_reverse)
                    continue;

                // If it's not the same lap numbers, check further in the index
                if (hs->m_number_of_laps != hs_compare.m_number_of_laps)
                    continue;

                // Don't compare a high score entry with itself
                if (compare_index == i)
                    continue;
        }
*/
        // Only display high scores matching the current mode

        if (m_active_mode == RaceManager::MINOR_MODE_NORMAL_RACE &&
            hs->m_highscore_type != "HST_STANDARD")
            continue;
        if (m_active_mode == RaceManager::MINOR_MODE_TIME_TRIAL &&
            hs->m_highscore_type != "HST_STD_TIMETRIAL")
            continue;
        else if (m_active_mode == RaceManager::MINOR_MODE_EASTER_EGG &&
            hs->m_highscore_type != "HST_EASTER_EGG_HUNT")
            continue;

        Track* track = track_manager->getTrack(hs->m_track);

        if (track == NULL)
            continue;
/*
        int icon = -1;

        for (unsigned int i = 0; i < kart_properties_manager->getNumberOfKarts(); i++)
        {
            const KartProperties* prop = kart_properties_manager->getKartById(i);
            if (hs->m_kart_name[hi] == prop->getIdent())
            {
                icon = i;
                break;
            }
        }

        if (icon == -1)
        {
            icon = m_icon_unknown_kart;
            Log::warn("HighScoreSelection", "Kart not found, using default icon.");
        }
*/
        std::vector<GUIEngine::ListWidget::ListCell> row;
        //The third argument should match the numbers used in beforeAddingWidget
        row.push_back(GUIEngine::ListWidget::ListCell(track->getName() , -1, 9));

        if (m_active_mode_is_linear)
            row.push_back(GUIEngine::ListWidget::ListCell
                (StringUtils::toWString(hs->m_number_of_karts), -1, 3, true));
/*
        if (!m_same_difficulty)
        {*/
        bool display_lock = false;
        if ((RaceManager::Difficulty)hs->m_difficulty == RaceManager::DIFFICULTY_BEST &&
            PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
            display_lock = true;

        row.push_back(GUIEngine::ListWidget::ListCell
            (RaceManager::get()->
                getDifficultyName((RaceManager::Difficulty) hs->m_difficulty),
                                   display_lock ? m_icon_lock : -1, 4, true));
        //}
        if (m_active_mode_is_linear)
        {
            row.push_back(GUIEngine::ListWidget::ListCell
                (StringUtils::toWString(hs->m_number_of_laps), -1, 3, true));
            row.push_back(GUIEngine::ListWidget::ListCell
                (hs->m_reverse ? _("Yes") : _("No"), -1, 3, true));
        }
/*
        row.push_back(GUIEngine::ListWidget::ListCell
            (StringUtils::toWString(StringUtils::timeToString(hs->m_time[hi])), -1, 4, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            ("", icon, 1, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (hs->m_name.empty() ? " " : hs->m_name[hi], -1, 5, true));
*/
        m_high_scores_list_widget->addItem(StringUtils::toString(i), row);
    }
}   // loadList

// ----------------------------------------------------------------------------
void HighScoreSelection::eventCallback(GUIEngine::Widget* widget,
                                         const std::string& name,
                                         const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "reload")
    {
        refresh();
    }
    else if (name == m_high_scores_list_widget->m_properties[GUIEngine::PROP_ID])
    {
        int selected_index = -1;
        const bool success = StringUtils::fromString(m_high_scores_list_widget
            ->getSelectionInternalName(), selected_index);
        // This can happen e.g. when the list is empty and the user
        // clicks somewhere.
        if (selected_index >= (signed)highscore_manager->highscoresSize() ||
            selected_index < 0 || !success)
        {
            return;
        }
        if (PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
        {
            const Highscores* hs = highscore_manager->getHighscoresAt(selected_index);
            if((RaceManager::Difficulty)hs->m_difficulty == RaceManager::DIFFICULTY_BEST)
                return;
        }

        new HighScoreInfoDialog(highscore_manager->getHighscoresAt(selected_index));
    }   // click on high score entry
    else if (name == "race_mode")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "tab_normal_race")
            m_active_mode = RaceManager::MINOR_MODE_NORMAL_RACE;
        else if (selection == "tab_time_trial")
            m_active_mode = RaceManager::MINOR_MODE_TIME_TRIAL;
        else if (selection == "tab_egg_hunt")
            m_active_mode = RaceManager::MINOR_MODE_EASTER_EGG;

        m_active_mode_is_linear = RaceManager::get()->isLinearRaceMode(m_active_mode);
        //m_is_comparing = false;
        //m_compare_toggle_widget->setState(false);
        refresh(/*reload replay files*/ false, /* update columns */ true);
    }
    //else if (name == "high_scores_difficulty_toggle")
    //{
    //    m_same_difficulty = m_high_scores_difficulty_toggle_widget->getState();
    //    refresh(/*reload replay files*/ false, /* update columns */ true);
    //}
    //else if (name == "replay_multiplayer_toggle")
    //{
    //    m_multiplayer = !m_high_scores_multiplayer_toggle_widget->getState();
    //    refresh(/*reload replay files*/ false, /* update columns */ true);
    //}
    //}
    //else if (name == "best_times_toggle")
    //{
    //    m_best_times = m_best_times_toggle_widget->getState();
    //    refresh(/*reload replay files*/ false);
    //}
    //else if (name == "compare_toggle")
    //{
    //    m_is_comparing = m_compare_toggle_widget->getState();
    //    refresh(/*reload replay files*/ false);
    //}

}   // eventCallback
/*
// ----------------------------------------------------------------------------
void HighScoreSelection::onDeleteReplay(std::string& filename)
{
    m_file_to_be_deleted = filename;
    new MessageDialog( _("Are you sure you want to remove '%s'?",
        m_file_to_be_deleted.c_str()), MessageDialog::MESSAGE_DIALOG_CONFIRM,
        this, false);
}   // onDeleteReplay

// ----------------------------------------------------------------------------
void HighScoreSelection::onConfirm()
{
    if (!file_manager
        ->removeFile(file_manager->getReplayDir() + m_file_to_be_deleted))
        Log::warn("HighScoreInfoDialog", "Failed to delete file.");

    ModalDialog::dismiss();
    HighScoreSelection::getInstance()->refresh();
}   // onConfirm
*/
// ----------------------------------------------------------------------------
/** Change the sort order if a column was clicked.
 *  \param column_id ID of the column that was clicked.
 */

void HighScoreSelection::onColumnClicked(int column_id, bool sort_desc, bool sort_default)
{
    /*
    // Begin by resorting the list to default
    defaultSort();

    if (sort_default)
    {
        loadList();
        return;
    }

    int diff_difficulty = m_same_difficulty ? 1 : 0;
    int diff_linear = m_active_mode_is_linear ? 0 : 1;
    //int diff_multiplayer = m_multiplayer ? 0 : 1;

    if (column_id >= 1)
        column_id += diff_linear;

    if (column_id >= 2)
        column_id += diff_difficulty;

    if (column_id >= 3)
        column_id += diff_linear;

    //if (column_id >= 7)
    //    column_id += diff_multiplayer;

    if (column_id == 0)
        ReplayPlay::setSortOrder(ReplayPlay::SO_TRACK);
    else if (column_id == 1)
        ReplayPlay::setSortOrder(ReplayPlay::SO_REV);
    else if (column_id == 2)
        ReplayPlay::setSortOrder(ReplayPlay::SO_DIFF);
    else if (column_id == 3)
        ReplayPlay::setSortOrder(ReplayPlay::SO_LAPS);
    else if (column_id == 4)
        ReplayPlay::setSortOrder(ReplayPlay::SO_TIME);
    else if (column_id == 5)
        return; // no sorting by kart icon (yet ?)
    else if (column_id == 6)
        ReplayPlay::setSortOrder(ReplayPlay::SO_USER);
    else if (column_id == 7)
        ReplayPlay::setSortOrder(ReplayPlay::SO_KART_NUM);
    else
        assert(0);

    ReplayPlay::get()->sortReplay(sort_desc);
    */
    loadList();
}   // onColumnClicked

// ----------------------------------------------------------------------------
/** Apply the default sorting to the high score list
 */
/*
void HighScoreSelection::defaultSort()
{
    ReplayPlay::setSortOrder(ReplayPlay::SO_TIME);
    //ReplayPlay::get()->sortReplay(false);
    //ReplayPlay::setSortOrder(ReplayPlay::SO_LAPS);
    //ReplayPlay::get()->sortReplay(false);
    //ReplayPlay::setSortOrder(ReplayPlay::SO_REV);
    //ReplayPlay::get()->sortReplay(false);
    //ReplayPlay::setSortOrder(ReplayPlay::SO_TRACK);
    //ReplayPlay::get()->sortReplay(false);
//}   // defaultSort

// ----------------------------------------------------------------------------
bool HighScoreSelection::onEscapePressed()
{
    // Reset it when leave this screen
    //RaceManager::get()->setRecordRace(false);
    return true;
}   // onEscapePressed
*/
// ----------------------------------------------------------------------------
