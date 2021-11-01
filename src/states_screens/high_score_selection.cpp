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
#include "race/grand_prix_data.hpp"
#include "race/grand_prix_manager.hpp"
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
    m_selected_index = -1;
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
/** Triggers a refresh of the high score list.
 */
void HighScoreSelection::refresh(bool forced_update, bool update_columns)
{
    m_selected_index = -1;

    if (highscore_manager->highscoresEmpty() || forced_update)
    {
        if (!highscore_manager->highscoresEmpty())
        {
            highscore_manager->clearHighscores();
        }
        highscore_manager->loadHighscores();
    }
    defaultSort();

    loadList();

    if (update_columns)
    {
        m_high_scores_list_widget->clearColumns();
        beforeAddingWidget(); //Reload the columns used
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

    m_mode_tabs = getWidget<GUIEngine::RibbonWidget>("race_mode");
    m_active_mode = RaceManager::MINOR_MODE_NORMAL_RACE;
    m_major_mode = RaceManager::MAJOR_MODE_SINGLE;
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
    core::stringw track_type_name;

    if (m_major_mode == RaceManager::MAJOR_MODE_GRAND_PRIX)
        track_type_name = _("Grand Prix");
    else
        track_type_name = _("Track");

    m_high_scores_list_widget->addColumn(_C("column_name", track_type_name.c_str()), 7);
    m_high_scores_list_widget->addColumn(_C("column_name", "Difficulty"), 4);
    if (m_active_mode_is_linear)
    {
        m_high_scores_list_widget->addColumn(_C("column_name", "Number of karts"), 4);

        if (m_major_mode == RaceManager::MAJOR_MODE_GRAND_PRIX)
        {
            m_high_scores_list_widget->addColumn(_("Game mode"),3);
        }
        else if (m_active_mode != RaceManager::MINOR_MODE_LAP_TRIAL)
        {
            m_high_scores_list_widget->addColumn(_C("column_name", "Laps"), 3);
        }
            m_high_scores_list_widget->addColumn(_C("column_name", "Reverse"), 3);
    }

    if (m_active_mode == RaceManager::MINOR_MODE_LAP_TRIAL)
        m_high_scores_list_widget->addColumn(_("Time limit"),4);

    m_high_scores_list_widget->createHeader();
}   // beforeAddingWidget

// ----------------------------------------------------------------------------
void HighScoreSelection::init()
{
    Screen::init();

    int icon_height = GUIEngine::getFontHeight();
    int row_height = GUIEngine::getFontHeight() * 5 / 4;

    // 128 is the height of the image file
    m_icon_bank->setScale(icon_height/128.0f);
    m_icon_bank->setTargetIconSize(128, 128);
    m_high_scores_list_widget->setIcons(m_icon_bank, (int)row_height);

    refresh(/*reload high score list*/ false, /* update columns */ true);
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

    for (int i = 0; i < highscore_manager->highscoresSize(); i++)
    {
        Highscores* hs = highscore_manager->getHighscoresAt(i);

        if (m_major_mode == RaceManager::MAJOR_MODE_SINGLE)
        {
            if (m_active_mode == RaceManager::MINOR_MODE_NORMAL_RACE &&
                hs->m_highscore_type != "HST_STANDARD")
                continue;
            else if (m_active_mode == RaceManager::MINOR_MODE_TIME_TRIAL &&
                hs->m_highscore_type != "HST_STD_TIMETRIAL")
                continue;
            else if (m_active_mode == RaceManager::MINOR_MODE_EASTER_EGG &&
                hs->m_highscore_type != "HST_EASTER_EGG_HUNT")
                continue;
            else if (m_active_mode == RaceManager::MINOR_MODE_LAP_TRIAL &&
                hs->m_highscore_type != "HST_LAP_TRIAL")
                continue;
        }
        else if (m_major_mode == RaceManager::MAJOR_MODE_GRAND_PRIX &&
            hs->m_highscore_type != "HST_GRANDPRIX")
            continue;

        std::vector<GUIEngine::ListWidget::ListCell> row;

        if (m_major_mode == RaceManager::MAJOR_MODE_GRAND_PRIX)
        {
            const GrandPrixData* gp = grand_prix_manager->getGrandPrix(hs->m_track);

            if (gp == NULL || hs->getNumberEntries() < 1)
                continue;

            //The third argument should match the numbers used in beforeAddingWidget
            row.push_back(GUIEngine::ListWidget::ListCell(gp->getName() , -1, 7));
        }
        else
        {
            Track* track = track_manager->getTrack(hs->m_track);

            if (track == NULL || hs->getNumberEntries() < 1)
                continue;

            //The third argument should match the numbers used in beforeAddingWidget
            row.push_back(GUIEngine::ListWidget::ListCell(track->getName() , -1, 7));
        }

        bool display_lock = false;
        if ((RaceManager::Difficulty)hs->m_difficulty == RaceManager::DIFFICULTY_BEST &&
            PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
            display_lock = true;

        row.push_back(GUIEngine::ListWidget::ListCell(RaceManager::get()->
                getDifficultyName((RaceManager::Difficulty) hs->m_difficulty),
                                   display_lock ? m_icon_lock : -1, 4, true));

        if (m_active_mode_is_linear || m_active_mode == RaceManager::MINOR_MODE_LAP_TRIAL)
        {
            row.push_back(GUIEngine::ListWidget::ListCell
                (StringUtils::toWString(hs->m_number_of_karts), -1, 4, true));

            if (m_major_mode != RaceManager::MAJOR_MODE_GRAND_PRIX && m_active_mode != RaceManager::MINOR_MODE_LAP_TRIAL)
            {
                row.push_back(GUIEngine::ListWidget::ListCell
                    (StringUtils::toWString(hs->m_number_of_laps), -1, 3, true));
            }
            if (m_major_mode == RaceManager::MAJOR_MODE_GRAND_PRIX)
            {
                row.push_back(GUIEngine::ListWidget::ListCell(
                    RaceManager::getNameOf((RaceManager::MinorRaceModeType)hs->m_gp_minor_mode), -1, 3, true));
                row.push_back(GUIEngine::ListWidget::ListCell(
                    GrandPrixData::reverseTypeToString((GrandPrixData::GPReverseType)hs->m_gp_reverse_type), -1, 3, true));
            }
            else
            {
                row.push_back(GUIEngine::ListWidget::ListCell
                    (hs->m_reverse ? _("Yes") : _("No"), -1, 3, true));
            }
        }
        if (m_active_mode == RaceManager::MINOR_MODE_LAP_TRIAL)
        {
            row.push_back(GUIEngine::ListWidget::ListCell(
                StringUtils::toWString(StringUtils::timeToString(hs->m_number_of_laps)), -1, 4, true));
        }
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
    else if (name == "remove-all")
    {
        onClearHighscores();
    }
    else if (name == m_high_scores_list_widget->m_properties[GUIEngine::PROP_ID])
    {
        m_selected_index = -1;
        const bool success = StringUtils::fromString(m_high_scores_list_widget
            ->getSelectionInternalName(), m_selected_index);
        // This can happen e.g. when the list is empty and the user
        // clicks somewhere.
        if (m_selected_index >= (signed)highscore_manager->highscoresSize() ||
            m_selected_index < 0 || !success)
        {
            return;
        }
        if (PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
        {
            Highscores* hs = highscore_manager->getHighscoresAt(m_selected_index);
            if((RaceManager::Difficulty)hs->m_difficulty == RaceManager::DIFFICULTY_BEST)
                return;
        }

        new HighScoreInfoDialog(highscore_manager->getHighscoresAt(m_selected_index), m_active_mode_is_linear, m_major_mode);
    }   // click on high score entry
    else if (name == "race_mode")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "tab_normal_race")
        {
            m_active_mode = RaceManager::MINOR_MODE_NORMAL_RACE;
            m_major_mode = RaceManager::MAJOR_MODE_SINGLE;
        }
        else if (selection == "tab_time_trial")
        {
            m_active_mode = RaceManager::MINOR_MODE_TIME_TRIAL;
            m_major_mode = RaceManager::MAJOR_MODE_SINGLE;
        }
        else if (selection == "tab_egg_hunt")
        {
            m_active_mode = RaceManager::MINOR_MODE_EASTER_EGG;
            m_major_mode = RaceManager::MAJOR_MODE_SINGLE;
        }
        else if (selection == "tab_lap_trial")
        {
            m_active_mode = RaceManager::MINOR_MODE_LAP_TRIAL;
            m_major_mode = RaceManager::MAJOR_MODE_SINGLE;
        }
        else if (selection == "tab_grand_prix")
        {
            m_active_mode = RaceManager::MINOR_MODE_NORMAL_RACE;
            m_major_mode = RaceManager::MAJOR_MODE_GRAND_PRIX;
        }

        if (m_major_mode == RaceManager::MAJOR_MODE_GRAND_PRIX || m_active_mode == RaceManager::MINOR_MODE_LAP_TRIAL)
            m_active_mode_is_linear = true;
        else
            m_active_mode_is_linear = RaceManager::get()->isLinearRaceMode(m_active_mode);
        refresh(/*keep high score list*/ false, /* update columns */ true);
    }
}   // eventCallback

// ----------------------------------------------------------------------------
void HighScoreSelection::onDeleteHighscores()
{
    new MessageDialog( _("Are you sure you want to remove this high score entry?"),
    MessageDialog::MESSAGE_DIALOG_CONFIRM, this, false);
}   // onDeleteHighscores

// ----------------------------------------------------------------------------
void HighScoreSelection::onClearHighscores()
{
    m_selected_index = -1;
    new MessageDialog( _("Are you sure you want to remove all of your high scores?"),
    MessageDialog::MESSAGE_DIALOG_CONFIRM, this, false);
}   // onClearHighscores

// ----------------------------------------------------------------------------
void HighScoreSelection::onConfirm()
{
    if (m_selected_index < 0)
    {
        highscore_manager->clearHighscores();
    }
    else
    {
        highscore_manager->deleteHighscores(m_selected_index);
    }
    defaultSort();

    highscore_manager->saveHighscores();

    // Restore the previously used sort direction
    highscore_manager->sortHighscores(m_reverse_sort);

    ModalDialog::dismiss();
    HighScoreSelection::getInstance()->refresh();
}   // onConfirm

// ----------------------------------------------------------------------------
/** Change the sort order if a column was clicked.
 *  \param column_id ID of the column that was clicked.
 */

void HighScoreSelection::onColumnClicked(int column_id, bool sort_desc, bool sort_default)
{
    // Begin by resorting the list to default
    defaultSort();

    if (sort_default)
    {
        loadList();
        return;
    }

    if (column_id == 0)
        Highscores::setSortOrder(Highscores::SO_TRACK);
    else if (column_id == 1)
        Highscores::setSortOrder(Highscores::SO_DIFF);
    else if (column_id == 2)
        Highscores::setSortOrder(Highscores::SO_KART_NUM);
    else if (column_id == 3)
        Highscores::setSortOrder(Highscores::SO_LAPS);
    else if (column_id == 4)
        Highscores::setSortOrder(Highscores::SO_REV);
    else
        assert(0);

    m_reverse_sort = sort_desc;
    highscore_manager->sortHighscores(sort_desc);

    loadList();
}   // onColumnClicked

// ----------------------------------------------------------------------------
/** Apply the default sorting to the high score list
 */

void HighScoreSelection::defaultSort()
{
    m_reverse_sort = false;
    Highscores::setSortOrder(Highscores::SO_DEFAULT);
    highscore_manager->sortHighscores(false);
}   // defaultSort
