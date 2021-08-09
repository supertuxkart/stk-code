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

#ifndef HEADER_HIGH_SCORE_SELECTION_HPP
#define HEADER_HIGH_SCORE_SELECTION_HPP

#include "guiengine/screen.hpp"
#include "guiengine/widgets.hpp"
#include "race/race_manager.hpp"
#include "states_screens/dialogs/message_dialog.hpp"

namespace GUIEngine { class Widget; }

/**
  * \brief HighScoreSelection
  * \ingroup states_screens
  */
class HighScoreSelection : public GUIEngine::Screen,
                             public GUIEngine::ScreenSingleton<HighScoreSelection>,
                             public GUIEngine::IListWidgetHeaderListener,
                             public MessageDialog::IConfirmDialogListener

{
    friend class GUIEngine::ScreenSingleton<HighScoreSelection>;

private:
    HighScoreSelection();
    ~HighScoreSelection();

    GUIEngine::ListWidget*     m_high_scores_list_widget;
    GUIEngine::RibbonWidget*   m_mode_tabs;
    bool                       m_active_mode_is_linear;
    bool                       m_reverse_sort;
    RaceManager::MajorRaceModeType m_major_mode;
    RaceManager::MinorRaceModeType m_active_mode;
    int                         m_selected_index;

    irr::gui::STKModifiedSpriteBank *m_icon_bank;

    /** Icon for unknown karts */
    int                        m_icon_unknown_kart;
    /** Icon for locked replays */
    int                        m_icon_lock;

    void defaultSort();

public:
    irr::gui::STKModifiedSpriteBank* getIconBank() { return m_icon_bank; }

    int  getUnknownKartIcon() { return m_icon_unknown_kart; }

    void refresh(bool forced_update = true, bool update_columns = false);

    /** Load the addons into the main list.*/
    void loadList();

    void onDeleteHighscores();

    void onClearHighscores();

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

    /** \brief Implement IConfirmDialogListener callback */
    virtual void onConfirm() OVERRIDE;

};   // HighScoreSelection

#endif
