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

DEFINE_SCREEN_SINGLETON( GhostReplaySelection );

// ----------------------------------------------------------------------------
/** Constructor, which loads the stkgui file.
 */
GhostReplaySelection::GhostReplaySelection() : Screen("ghost_replay_selection.stkgui")
{
    m_sort_desc = true;
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
void GhostReplaySelection::refresh(bool forced_update)
{
    if (ReplayPlay::get()->getNumReplayFile() == 0 || forced_update)
        ReplayPlay::get()->loadAllReplayFile();
    loadList();
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
    m_replay_difficulty_toggle_widget->setState(true);
    m_same_difficulty = m_replay_difficulty_toggle_widget->getState();
}   // loadedFromFile

// ----------------------------------------------------------------------------
/** Clear the replay file list, which will be reloaded.
 */
void GhostReplaySelection::beforeAddingWidget()
{
    m_replay_list_widget->clearColumns();
    m_replay_list_widget->addColumn( _("Track"), 3 );
    m_replay_list_widget->addColumn( _("Players"), 1);
    m_replay_list_widget->addColumn( _("Reverse"), 1);
    m_replay_list_widget->addColumn( _("Difficulty"), 1);
    m_replay_list_widget->addColumn( _("Laps"), 1);
    m_replay_list_widget->addColumn( _("Finish Time"), 1);
    m_replay_list_widget->addColumn( _("User"), 1);
}   // beforeAddingWidget

// ----------------------------------------------------------------------------
void GhostReplaySelection::init()
{
    Screen::init();
    m_cur_difficulty = race_manager->getDifficulty();
    refresh(/*forced_update*/false);
}   // init

// ----------------------------------------------------------------------------
/** Loads the list of all replay files. The gui element will be
 *  updated.
 */
void GhostReplaySelection::loadList()
{
    ReplayPlay::get()->sortReplay(m_sort_desc);
    m_replay_list_widget->clear();
    for (unsigned int i = 0; i < ReplayPlay::get()->getNumReplayFile() ; i++)
    {
        const ReplayPlay::ReplayData& rd = ReplayPlay::get()->getReplayData(i);

        if (m_same_difficulty && m_cur_difficulty !=
            (RaceManager::Difficulty)rd.m_difficulty)
            continue;

        Track* track = track_manager->getTrack(rd.m_track_name);
        
        if (track == NULL)
            continue;

        std::vector<GUIEngine::ListWidget::ListCell> row;
        row.push_back(GUIEngine::ListWidget::ListCell
            (translations->fribidize(track->getName()) , -1, 3));
        row.push_back(GUIEngine::ListWidget::ListCell
            (StringUtils::toWString(rd.m_kart_list.size()), -1, 1, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (rd.m_reverse ? _("Yes") : _("No"), -1, 1, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (rd.m_difficulty == 3 ? _("SuperTux") : rd.m_difficulty == 2 ?
            _("Expert") : rd.m_difficulty == 1 ?
            _("Intermediate") : _("Novice") , -1, 1, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (StringUtils::toWString(rd.m_laps), -1, 1, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (StringUtils::toWString(rd.m_min_time) + L"s", -1, 1, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (rd.m_user_name, -1, 1, true));
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
        new GhostReplayInfoDialog(selected_index);
    }   // click on replay file
    else if (name == "record-ghost")
    {
        race_manager->setRecordRace(true);
        TracksScreen::getInstance()->setOfficalTrack(false);
        TracksScreen::getInstance()->push();
    }
    else if (name == "replay_difficulty_toggle")
    {
        m_same_difficulty = m_replay_difficulty_toggle_widget->getState();
        refresh(/*forced_update*/false);
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
    switch (column_id)
    {
        case 0:
            ReplayPlay::setSortOrder(ReplayPlay::SO_TRACK);
            break;
        case 1:
            ReplayPlay::setSortOrder(ReplayPlay::SO_KART_NUM);
            break;
        case 2:
            ReplayPlay::setSortOrder(ReplayPlay::SO_REV);
            break;
        case 3:
            ReplayPlay::setSortOrder(ReplayPlay::SO_DIFF);
            break;
        case 4:
            ReplayPlay::setSortOrder(ReplayPlay::SO_LAPS);
            break;
        case 5:
            ReplayPlay::setSortOrder(ReplayPlay::SO_TIME);
            break;
        case 6:
            ReplayPlay::setSortOrder(ReplayPlay::SO_USER);
            break;
        default:
            assert(0);
            break;
    }   // switch
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
