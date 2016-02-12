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
#include "replay/replay_play.hpp"
#include "states_screens/ghost_replay_selection.hpp"
#include "states_screens/state_manager.hpp"

using namespace GUIEngine;
using namespace irr::core;

// -----------------------------------------------------------------------------
GhostReplayInfoDialog::GhostReplayInfoDialog(unsigned int replay_id)
                : ModalDialog(0.5f,0.5f), m_replay_id(replay_id)
{
    m_self_destroy = false;
    m_rd = ReplayPlay::get()->getReplayData(m_replay_id);
    loadFromFile("ghost_replay_info_dialog.stkgui");

    LabelWidget *name = getWidget<LabelWidget>("name");
    assert(name);
    name->setText(stringw((m_rd.m_filename).c_str()), false);

    m_back_widget = getWidget<IconButtonWidget>("back");
    m_back_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    m_action_widget = getWidget<RibbonWidget>("actions");
}   // GhostReplayInfoDialog

// -----------------------------------------------------------------------------
GhostReplayInfoDialog::~GhostReplayInfoDialog()
{
}   // ~GhostReplayInfoDialog

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
            ModalDialog::dismiss();
            ReplayPlay::get()->setReplayFile(m_replay_id);
            race_manager->setRaceGhostKarts(true);
            race_manager->setNumKarts(race_manager->getNumLocalPlayers());

            // Disable accidentally unlocking of a challenge
            PlayerManager::getCurrentPlayer()->setCurrentChallenge("");

            race_manager->setReverseTrack(m_rd.m_reverse);
            race_manager->startSingleRace(m_rd.m_track_name, m_rd.m_laps, false);
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == "remove")
        {
            m_self_destroy = true;
            if (!file_manager
                ->removeFile(file_manager->getReplayDir() + m_rd.m_filename))
                Log::warn("GhostReplayInfoDialog", "Failed to delete file.");
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "back")
        {
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
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
        ModalDialog::dismiss();
        GhostReplaySelection::getInstance()->refresh();
        return;
    }
}   // onUpdate
