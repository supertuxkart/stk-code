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
                      : ModalDialog(0.8f,0.5f), m_replay_id(replay_id)
{
    m_self_destroy = false;
    m_record_race = false;
    m_watch_only = false;

    m_rd = ReplayPlay::get()->getReplayData(m_replay_id);
    loadFromFile("ghost_replay_info_dialog.stkgui");

    LabelWidget *name = getWidget<LabelWidget>("name");
    assert(name);
    name->setText(stringw((m_rd.m_custom_replay_file ? StringUtils::getBasename
        (m_rd.m_filename) : m_rd.m_filename).c_str()), false);

    m_back_widget = getWidget<IconButtonWidget>("back");
    m_back_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    // Non-deletable for custom (standard) replay file
    getWidget<IconButtonWidget>("remove")->setActive(!m_rd.m_custom_replay_file);

    m_action_widget = getWidget<RibbonWidget>("actions");
    m_record_widget = getWidget<CheckBoxWidget>("record-race");
    m_watch_widget = getWidget<CheckBoxWidget>("watch-only");

    if (race_manager->getNumLocalPlayers() > 1)
    {
        // No watching replay when split-screen
        m_watch_widget->setVisible(false);
        getWidget<LabelWidget>("watch-only-text")->setVisible(false);
    }

    m_record_widget->setState(false);
    m_watch_widget->setState(false);

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
            bool reverse = m_rd.m_reverse;
            std::string track_name = m_rd.m_track_name;
            int laps = m_rd.m_laps;
            int replay_id = m_replay_id;

            race_manager->setRecordRace(m_record_race);
            race_manager->setWatchingReplay(m_watch_only);

            ModalDialog::dismiss();
            ReplayPlay::get()->setReplayFile(replay_id);
            race_manager->setRaceGhostKarts(true);

            race_manager->setNumKarts(race_manager->getNumLocalPlayers());

            // Disable accidentally unlocking of a challenge
            PlayerManager::getCurrentPlayer()->setCurrentChallenge("");

            race_manager->setReverseTrack(reverse);

            if (race_manager->isWatchingReplay())
                race_manager->startWatchingReplay(track_name, laps);
            else
                race_manager->startSingleRace(track_name, laps, false);

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
        return;
    }
}   // onUpdate
