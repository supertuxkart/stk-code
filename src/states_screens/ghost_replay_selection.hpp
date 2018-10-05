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

#ifndef HEADER_GHOST_REPLAY_SELECTION_HPP
#define HEADER_GHOST_REPLAY_SELECTION_HPP

#include "guiengine/screen.hpp"
#include "guiengine/widgets.hpp"
#include "race/race_manager.hpp"
#include "states_screens/dialogs/message_dialog.hpp"

namespace GUIEngine { class Widget; }

/**
  * \brief GhostReplaySelection
  * \ingroup states_screens
  */
class GhostReplaySelection : public GUIEngine::Screen,
                             public GUIEngine::ScreenSingleton<GhostReplaySelection>,
                             public GUIEngine::IListWidgetHeaderListener,
                             public MessageDialog::IConfirmDialogListener

{
    friend class GUIEngine::ScreenSingleton<GhostReplaySelection>;

private:
    GhostReplaySelection();
    ~GhostReplaySelection();

    GUIEngine::ListWidget*     m_replay_list_widget;
    GUIEngine::CheckBoxWidget* m_replay_difficulty_toggle_widget;
    GUIEngine::CheckBoxWidget* m_replay_multiplayer_toggle_widget;
    GUIEngine::CheckBoxWidget* m_replay_version_toggle_widget;
    GUIEngine::CheckBoxWidget* m_best_times_toggle_widget;
    GUIEngine::CheckBoxWidget* m_compare_toggle_widget;
    GUIEngine::RibbonWidget*   m_mode_tabs;
    RaceManager::Difficulty    m_cur_difficulty;
    std::string                m_file_to_be_deleted;
    std::vector<unsigned int>  m_best_times_index;
    bool                       m_same_difficulty;
    bool                       m_same_version;
    bool                       m_multiplayer;
    bool                       m_best_times;
    bool                       m_is_comparing;
    bool                       m_active_mode_is_linear;
    RaceManager::MinorRaceModeType m_active_mode;
    // The index id of a replay file can change with sorting, etc.
    // Using the UID guarantees exact matchess
    uint64_t                   m_replay_to_compare_uid;

    irr::gui::STKModifiedSpriteBank *m_icon_bank;

    /** Icon for unknown karts */
    int                        m_icon_unknown_kart;
    /** Icon for locked replays */
    int                        m_icon_lock;

    void defaultSort();

public:
    irr::gui::STKModifiedSpriteBank* getIconBank() { return m_icon_bank; }

    int  getUnknownKartIcon() { return m_icon_unknown_kart; }

    void setCompareReplayUid(uint64_t uid) { m_replay_to_compare_uid = uid; }
    void setCompare(bool compare) { m_is_comparing = compare; }

    void refresh(bool forced_update = true, bool update_columns = false);

    /** Load the addons into the main list.*/
    void loadList();

    void onDeleteReplay(std::string& filename);

    const RaceManager::MinorRaceModeType getActiveMode() { return m_active_mode; }

    const bool isActiveModeLinear() { return m_active_mode_is_linear; }

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    virtual void onColumnClicked(int column_id, bool sort_desc, bool sort_default) OVERRIDE;

    virtual void init() OVERRIDE;

    virtual void tearDown() OVERRIDE;
    
    virtual void unloaded() OVERRIDE;

    virtual bool onEscapePressed() OVERRIDE;

    /** \brief Implement IConfirmDialogListener callback */
    virtual void onConfirm() OVERRIDE;

};   // GhostReplaySelection

#endif
