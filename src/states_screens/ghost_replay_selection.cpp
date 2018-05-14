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

#include "states_screens/ghost_replay_selection.hpp"

#include "states_screens/dialogs/ghost_replay_info_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/tracks_screen.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;

// ----------------------------------------------------------------------------
/** Constructor, which loads the stkgui file.
 */
GhostReplaySelection::GhostReplaySelection() : Screen("ghost_replay_selection.stkgui")
{
    m_sort_desc = true;
    m_is_comparing = false;
    m_replay_to_compare_uid = 0;
}   // GhostReplaySelection

// ----------------------------------------------------------------------------
/** Destructor.
 */
GhostReplaySelection::~GhostReplaySelection()
{
}   // GhostReplaySelection

// ----------------------------------------------------------------------------
/** Triggers a refresh of the replay file list.
 */
void GhostReplaySelection::refresh(bool forced_update, bool update_columns)
{
    if (ReplayPlay::get()->getNumReplayFile() == 0 || forced_update)
        ReplayPlay::get()->loadAllReplayFile();
    loadList();

    // Allow to disable a comparison, but not to start one
    m_compare_toggle_widget->setVisible(m_is_comparing);
    m_compare_toggle_widget->setState(m_is_comparing);
    getWidget<LabelWidget>("compare-toggle-text")->setVisible(m_is_comparing);

    if (update_columns)
    {
        m_replay_list_widget->clearColumns();
        beforeAddingWidget();//Reload the columns used
    }
}   // refresh

// ----------------------------------------------------------------------------
/** Set pointers to the various widgets.
 */
void GhostReplaySelection::loadedFromFile()
{
    m_replay_list_widget = getWidget<GUIEngine::ListWidget>("replay_list");
    assert(m_replay_list_widget != NULL);
    m_replay_list_widget->setColumnListener(this);

    m_replay_difficulty_toggle_widget =
        getWidget<GUIEngine::CheckBoxWidget>("replay_difficulty_toggle");
    m_replay_difficulty_toggle_widget->setState(/* default value */ true);
    m_same_difficulty = m_replay_difficulty_toggle_widget->getState();

    m_replay_version_toggle_widget =
        getWidget<GUIEngine::CheckBoxWidget>("replay_version_toggle");
    m_replay_version_toggle_widget->setState(/* default value */ true);
    m_same_version = m_replay_version_toggle_widget->getState();

    m_best_times_toggle_widget =
        getWidget<GUIEngine::CheckBoxWidget>("best_times_toggle");
    m_best_times_toggle_widget->setState(/* default value */ false);
    m_best_times = m_best_times_toggle_widget->getState();

    m_compare_toggle_widget =
        getWidget<GUIEngine::CheckBoxWidget>("compare_toggle");
    m_compare_toggle_widget->setState(/* default value */ false);
    m_is_comparing = false;
    m_compare_toggle_widget->setVisible(false);
    getWidget<LabelWidget>("compare-toggle-text")->setVisible(false);

    m_mode_tabs = getWidget<GUIEngine::RibbonWidget>("race_mode");
    m_active_mode = RaceManager::MINOR_MODE_TIME_TRIAL;
    m_active_mode_is_linear = true;
}   // loadedFromFile

// ----------------------------------------------------------------------------
/** Clear the replay file list, which will be reloaded.
 */
void GhostReplaySelection::beforeAddingWidget()
{
    m_replay_list_widget->addColumn( _("Track"), 9 );
    m_replay_list_widget->addColumn( _("Players"), 3);
    if (m_active_mode_is_linear)
        m_replay_list_widget->addColumn( _("Reverse"), 3);
    if (!m_same_difficulty)
        m_replay_list_widget->addColumn( _("Difficulty"), 4);
    if (m_active_mode_is_linear)
        m_replay_list_widget->addColumn( _("Laps"), 3);
    m_replay_list_widget->addColumn( _("Time"), 4);
    m_replay_list_widget->addColumn( _("User"), 5);
    if (!m_same_version)
        m_replay_list_widget->addColumn( _("Version"), 3);

    m_replay_list_widget->createHeader();
}   // beforeAddingWidget

// ----------------------------------------------------------------------------
void GhostReplaySelection::init()
{
    Screen::init();
    m_cur_difficulty = race_manager->getDifficulty();
    refresh(/*reload replay files*/ false, /* update columns */ true);
}   // init

// ----------------------------------------------------------------------------
/** Loads the list of all replay files. The gui element will be
 *  updated.
 */
void GhostReplaySelection::loadList()
{
    ReplayPlay::get()->sortReplay(m_sort_desc);
    m_replay_list_widget->clear();

    if (ReplayPlay::get()->getNumReplayFile() == 0)
        return;

    if (m_best_times)
    {
        //First of all, clear the best time index
        m_best_times_index.clear();

        // This is in O(N*M) ; N being the number of replay files
        // and M the number of different configuration (which is
        // the final size of the best times list).
        // Each time has to be compared against the previous best times
        // up until all are checked or a replay with the same configuration
        // is found.
        for (unsigned int i = 0; i < ReplayPlay::get()->getNumReplayFile() ; i++)
        {
            const ReplayPlay::ReplayData& rd = ReplayPlay::get()->getReplayData(i);

            if (m_same_difficulty && m_cur_difficulty !=
                (RaceManager::Difficulty)rd.m_difficulty)
                continue;

            core::stringw current_version = STK_VERSION;
            if (m_same_version && current_version != rd.m_stk_version)
                continue;

            Track* track = track_manager->getTrack(rd.m_track_name);
        
            if (track == NULL)
                continue;

            // If no other replay with the same configuration is found in the index
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
                const ReplayPlay::ReplayData& bt = ReplayPlay::get()->getReplayData(m_best_times_index[j]);

                // If it's not the same track, check further in the index
                if (rd.m_track_name != bt.m_track_name)
                    continue;

                // If it's not the same difficulty, check further in the index
                if (rd.m_difficulty != bt.m_difficulty)
                    continue;

                // If it's not the same direction, check further in the index
                if (rd.m_reverse != bt.m_reverse)
                    continue;

                // If it's not the same lap numbers, check further in the index
                if (rd.m_laps != bt.m_laps)
                    continue;

                // The replay data have the same properties, compare the times
                if (rd.m_min_time < bt.m_min_time)
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
    unsigned int compare_index = ReplayPlay::get()->getReplayIdByUID(m_replay_to_compare_uid);
    const ReplayPlay::ReplayData& rd_compare = ReplayPlay::get()->getReplayData(compare_index);

    for (unsigned int i = 0; i < ReplayPlay::get()->getNumReplayFile() ; i++)
    {
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

        const ReplayPlay::ReplayData& rd = ReplayPlay::get()->getReplayData(i);

        if (m_same_difficulty && m_cur_difficulty !=
            (RaceManager::Difficulty)rd.m_difficulty)
            continue;

        core::stringw current_version = STK_VERSION;
        if (m_same_version && current_version != rd.m_stk_version)
            continue;

        // Only display replays comparable with the replay selected for comparison
        if (m_is_comparing)
        {
                // If it's not the same track, check further in the index
                if (rd.m_track_name != rd_compare.m_track_name)
                    continue;

                // If it's not the same direction, check further in the index
                if (rd.m_reverse != rd_compare.m_reverse)
                    continue;

                // If it's not the same lap numbers, check further in the index
                if (rd.m_laps != rd_compare.m_laps)
                    continue;

                // Don't compare a replay with itself
                if (compare_index == i)
                    continue;
        }
        // Only display replays matching the current mode
        if (m_active_mode == RaceManager::MINOR_MODE_TIME_TRIAL &&
            rd.m_minor_mode != "time-trial")
            continue;
        else if (m_active_mode == RaceManager::MINOR_MODE_EASTER_EGG &&
            rd.m_minor_mode != "egg-hunt")
            continue;

        Track* track = track_manager->getTrack(rd.m_track_name);
        
        if (track == NULL)
            continue;

        std::vector<GUIEngine::ListWidget::ListCell> row;
        //The third argument should match the numbers used in beforeAddingWidget
        row.push_back(GUIEngine::ListWidget::ListCell
            (translations->fribidize(track->getName()) , -1, 9));
        row.push_back(GUIEngine::ListWidget::ListCell
            (StringUtils::toWString(rd.m_kart_list.size()), -1, 3, true));
        if (m_active_mode_is_linear)
            row.push_back(GUIEngine::ListWidget::ListCell
                (rd.m_reverse ? _("Yes") : _("No"), -1, 3, true));
        if (!m_same_difficulty)
            row.push_back(GUIEngine::ListWidget::ListCell
                (race_manager->
                    getDifficultyName((RaceManager::Difficulty) rd.m_difficulty),
                                                                   -1, 4, true));
        if (m_active_mode_is_linear)
            row.push_back(GUIEngine::ListWidget::ListCell
                (StringUtils::toWString(rd.m_laps), -1, 3, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (StringUtils::toWString(rd.m_min_time) + L"s", -1, 4, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (rd.m_user_name.empty() ? " " : rd.m_user_name, -1, 5, true));
        if (!m_same_version)
            row.push_back(GUIEngine::ListWidget::ListCell
                (rd.m_stk_version.empty() ? " " : rd.m_stk_version, -1, 3, true));
        m_replay_list_widget->addItem(StringUtils::toString(i), row);
    }
}   // loadList

// ----------------------------------------------------------------------------
void GhostReplaySelection::eventCallback(GUIEngine::Widget* widget,
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
    else if (name == m_replay_list_widget->m_properties[GUIEngine::PROP_ID])
    {
        int selected_index = -1;
        const bool success = StringUtils::fromString(m_replay_list_widget
            ->getSelectionInternalName(), selected_index);
        // This can happen e.g. when the list is empty and the user
        // clicks somewhere.
        if (selected_index >= (signed)ReplayPlay::get()->getNumReplayFile() ||
            selected_index < 0 || !success)
        {
            return;
        }

        new GhostReplayInfoDialog(selected_index, m_replay_to_compare_uid, m_is_comparing);
    }   // click on replay file
    else if (name == "race_mode")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "tab_time_trial")
            m_active_mode = RaceManager::MINOR_MODE_TIME_TRIAL;
        else if (selection == "tab_egg_hunt")
            m_active_mode = RaceManager::MINOR_MODE_EASTER_EGG;

        m_active_mode_is_linear = race_manager->isLinearRaceMode(m_active_mode);
        m_is_comparing = false;
        m_compare_toggle_widget->setState(false);
        refresh(/*reload replay files*/ false, /* update columns */ true);
    }
    else if (name == "record-ghost")
    {
        race_manager->setRecordRace(true);
        race_manager->setMinorMode(m_active_mode);
        TracksScreen::getInstance()->push();
    }
    else if (name == "replay_difficulty_toggle")
    {
        m_same_difficulty = m_replay_difficulty_toggle_widget->getState();
        refresh(/*reload replay files*/ false, /* update columns */ true);
    }
    else if (name == "replay_version_toggle")
    {
        m_same_version = m_replay_version_toggle_widget->getState();
        refresh(/*reload replay files*/ false, /* update columns */ true);
    }
    else if (name == "best_times_toggle")
    {
        m_best_times = m_best_times_toggle_widget->getState();
        refresh(/*reload replay files*/ false);
    }
    else if (name == "compare_toggle")
    {
        m_is_comparing = m_compare_toggle_widget->getState();
        refresh(/*reload replay files*/ false);
    }

}   // eventCallback

// ----------------------------------------------------------------------------
void GhostReplaySelection::onDeleteReplay(std::string& filename)
{
    m_file_to_be_deleted = filename;
    new MessageDialog( _("Are you sure you want to remove '%s'?",
        m_file_to_be_deleted.c_str()), MessageDialog::MESSAGE_DIALOG_CONFIRM,
        this, false);
}   // onDeleteReplay

// ----------------------------------------------------------------------------
void GhostReplaySelection::onConfirm()
{
    if (!file_manager
        ->removeFile(file_manager->getReplayDir() + m_file_to_be_deleted))
        Log::warn("GhostReplayInfoDialog", "Failed to delete file.");

    ModalDialog::dismiss();
    GhostReplaySelection::getInstance()->refresh();
}   // onConfirm

// ----------------------------------------------------------------------------
/** Change the sort order if a column was clicked.
 *  \param column_id ID of the column that was clicked.
 */
void GhostReplaySelection::onColumnClicked(int column_id)
{
    int diff_difficulty = m_same_difficulty ? 1 : 0;
    int diff_linear = m_active_mode_is_linear ? 0 : 1;

    if (column_id >= 2)
        column_id += diff_linear;

    if (column_id >= 3)
        column_id += diff_difficulty;

    if (column_id >= 4)
        column_id += diff_linear;

    if (column_id == 0)
        ReplayPlay::setSortOrder(ReplayPlay::SO_TRACK);
    else if (column_id == 1)
        ReplayPlay::setSortOrder(ReplayPlay::SO_KART_NUM);
    else if (column_id == 2)
        ReplayPlay::setSortOrder(ReplayPlay::SO_REV);
    else if (column_id == 3)
        ReplayPlay::setSortOrder(ReplayPlay::SO_DIFF);
    else if (column_id == 4)
        ReplayPlay::setSortOrder(ReplayPlay::SO_LAPS);
    else if (column_id == 5)
        ReplayPlay::setSortOrder(ReplayPlay::SO_TIME);
    else if (column_id == 6)
        ReplayPlay::setSortOrder(ReplayPlay::SO_USER);
    else if (column_id == 7)    
        ReplayPlay::setSortOrder(ReplayPlay::SO_VERSION);
    else
        assert(0);

    printf("column_id is %d\n", column_id);

    /** \brief Toggle the sort order after column click **/
    m_sort_desc = !m_sort_desc;
    loadList();
}   // onColumnClicked

// ----------------------------------------------------------------------------
bool GhostReplaySelection::onEscapePressed()
{
    // Reset it when leave this screen
    race_manager->setRecordRace(false);
    return true;
}   // onEscapePressed

// ----------------------------------------------------------------------------
