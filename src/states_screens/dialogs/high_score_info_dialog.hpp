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

#ifndef HEADER_HIGH_SCORE_INFO_DIALOG_HPP
#define HEADER_HIGH_SCORE_INFO_DIALOG_HPP

#include "guiengine/modaldialog.hpp"
#include "guiengine/widgets.hpp"
#include "race/grand_prix_data.hpp"
#include "race/highscores.hpp"

/** \brief Dialog that allows a user to manage a high score
 * \ingroup states_screens
 */
class HighScoreInfoDialog : public GUIEngine::ModalDialog
{

private:
    Highscores* m_hs;

    GUIEngine::RibbonWidget*      m_action_widget;
    GUIEngine::IconButtonWidget*  m_start_widget;

    GUIEngine::LabelWidget*       m_high_score_label;
    GUIEngine::LabelWidget*       m_track_name_label;
    GUIEngine::LabelWidget*       m_num_karts_label;
    GUIEngine::LabelWidget*       m_difficulty_label;
    GUIEngine::LabelWidget*       m_reverse_label;
    GUIEngine::LabelWidget*       m_num_laps_label;

    GUIEngine::ListWidget*        m_high_score_list;
    GUIEngine::IconButtonWidget*  m_track_screenshot_widget;

    void updateHighscoreEntries();

    RaceManager::MajorRaceModeType m_major_mode;
    RaceManager::MinorRaceModeType m_minor_mode;

    float m_curr_time;

    GrandPrixData m_gp;

public:
    HighScoreInfoDialog(Highscores* highscore, bool is_linear, RaceManager::MajorRaceModeType major_mode);
    ~HighScoreInfoDialog();

    GUIEngine::EventPropagation processEvent(const std::string& eventSource);

    virtual void onUpdate(float dt);
};   // class HighScoreInfoDialog

#endif
