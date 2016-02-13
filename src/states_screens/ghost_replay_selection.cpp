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

#include "replay/replay_play.hpp"
#include "states_screens/dialogs/ghost_replay_info_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

DEFINE_SCREEN_SINGLETON( GhostReplaySelection );

// ----------------------------------------------------------------------------
/** Constructor, which loads the stkgui file.
 */
GhostReplaySelection::GhostReplaySelection() : Screen("ghost_replay_selection.stkgui")
{
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
void GhostReplaySelection::refresh()
{
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
}   // beforeAddingWidget

// ----------------------------------------------------------------------------
void GhostReplaySelection::init()
{
    Screen::init();
    refresh();
}   // init

// ----------------------------------------------------------------------------
/** Loads the list of all replay files. The gui element will be
 *  updated.
 */
void GhostReplaySelection::loadList()
{
    m_replay_list_widget->clear();
    for (unsigned int i = 0; i < ReplayPlay::get()->getNumReplayFile() ; i++)
    {
        const ReplayBase::ReplayData& rd = ReplayPlay::get()->getReplayData(i);

        std::vector<GUIEngine::ListWidget::ListCell> row;
        Track* t = track_manager->getTrack(rd.m_track_name);
        row.push_back(GUIEngine::ListWidget::ListCell
            (translations->fribidize(t->getName()) , -1, 3));
        row.push_back(GUIEngine::ListWidget::ListCell
            (StringUtils::toWString(rd.m_kart_list.size()), -1, 1, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (rd.m_reverse ? _("Yes") : _("No"), -1, 1, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (rd.m_difficulty == 3 ? _("Supertux") : rd.m_difficulty == 2 ?
            _("Expert") : rd.m_difficulty == 1 ?
            _("Intermediate") : _("Novice") , -1, 1, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (StringUtils::toWString(rd.m_laps), -1, 1, true));
        row.push_back(GUIEngine::ListWidget::ListCell
            (StringUtils::toWString(rd.m_min_time) + L"s", -1, 1, true));
        m_replay_list_widget->addItem("replay", row);
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
        unsigned int selected_index = m_replay_list_widget->getSelectionID();
        // This can happen e.g. when the list is empty and the user
        // clicks somewhere.
        if(selected_index >= ReplayPlay::get()->getNumReplayFile() ||
           selected_index < 0                                        )
        {
            return;
        }
        new GhostReplayInfoDialog(selected_index);
    }   // click on replay file

}   // eventCallback
