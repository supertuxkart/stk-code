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

#include "states_screens/dialogs/high_score_info_dialog.hpp"

#include "config/player_manager.hpp"
#include "guiengine/CGUISpriteBank.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "race/highscores.hpp"
#include "race/highscore_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/high_score_selection.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::core;

// -----------------------------------------------------------------------------
HighScoreInfoDialog::HighScoreInfoDialog(Highscores* highscore)
                      : ModalDialog(0.75f,0.75f)
{
    m_self_destroy         = false;

    m_hs = highscore;

    loadFromFile("high_score_info_dialog.stkgui");

    Track* track = track_manager->getTrack(m_hs->m_track);

    m_track_screenshot_widget = getWidget<IconButtonWidget>("track_screenshot");
    m_track_screenshot_widget->setFocusable(false);
    m_track_screenshot_widget->m_tab_stop = false;

    // temporary icon, will replace it just after (but it will be shown if the given icon is not found)
    m_track_screenshot_widget->m_properties[PROP_ICON] = "gui/icons/main_help.png";

    irr::video::ITexture* image = STKTexManager::getInstance()
        ->getTexture(track->getScreenshotFile(),
        "While loading screenshot for track '%s':", track->getFilename());
    if(!image)
    {
        image = STKTexManager::getInstance()->getTexture("main_help.png",
            "While loading screenshot for track '%s':", track->getFilename());
    }
    if (image != NULL)
        m_track_screenshot_widget->setImage(image);

    // TODO : small refinement, add the possibility to tab stops for lists
    //        to make this unselectable by keyboard/mouse
    m_high_score_entries = getWidget<GUIEngine::ListWidget>("high_score_entries");
    assert(m_high_score_entries != NULL);

    /* Used to display kart icons for the entries */
    irr::gui::STKModifiedSpriteBank *icon_bank = HighScoreSelection::getInstance()->getIconBank();
    int icon_height = GUIEngine::getFontHeight() * 3 / 2;
                                                    
    icon_bank->setScale(icon_height/128.0f);
    icon_bank->setTargetIconSize(128, 128);
    m_high_score_entries->setIcons(icon_bank, (int)icon_height);

    updateHighscoreEntries();

    m_high_score_label = getWidget<LabelWidget>("name");

    m_back_widget = getWidget<IconButtonWidget>("back");

    // Remove is enabled only when a non-empty entry is selected
    m_remove_widget = getWidget<IconButtonWidget>("remove");
    m_remove_widget->setVisible(false);

    m_remove_all_widget = getWidget<IconButtonWidget>("remove-all");

    m_action_widget = getWidget<RibbonWidget>("actions");

    m_action_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    m_action_widget->select("back", PLAYER_ID_GAME_MASTER);
}   // HighScoreInfoDialog
// -----------------------------------------------------------------------------
HighScoreInfoDialog::~HighScoreInfoDialog()
{
}   // ~HighScoreInfoDialog

// -----------------------------------------------------------------------------
void HighScoreInfoDialog::updateHighscoreEntries()
{
    m_high_score_entries->clear();

    const int amount = m_hs->getNumberEntries();

    std::string kart_name;
    core::stringw name;
    float time;

    int time_precision = RaceManager::get()->currentModeTimePrecision();

    // Fill highscore entries
    for (int n = 0; n < m_hs->HIGHSCORE_LEN; n++)
    {
        irr::core::stringw line;
        int icon = -1;

        // Check if this entry is filled or still empty
        if (n < amount)
        {
            m_hs->getEntry(n, kart_name, name, &time);

            std::string time_string = StringUtils::timeToString(time, time_precision);

            for(unsigned int i=0; i<kart_properties_manager->getNumberOfKarts(); i++)
            {
                const KartProperties* prop = kart_properties_manager->getKartById(i);
                if (kart_name == prop->getIdent())
                {
                    icon = i;
                    break;
                }
            }

            line = name + "    " + core::stringw(time_string.c_str());
        }
        else
        {
            //I18N: for empty highscores entries
            line = _("(Empty)");
        }

        if (icon == -1)
        {
            icon = HighScoreSelection::getInstance()->getUnknownKartIcon();
            //Log::warn("HighScoreInfoDialog", "Kart not found, using default icon.");
        }

        std::vector<GUIEngine::ListWidget::ListCell> row;

        row.push_back(GUIEngine::ListWidget::ListCell(line.c_str(), icon, 5, false));
        m_high_score_entries->addItem(StringUtils::toString(n), row);
    }
} // updateHighscoreEntries

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation
    HighScoreInfoDialog::processEvent(const std::string& event_source)
{
    if (event_source == "high_score_entries")
    {
        m_remove_widget->setVisible(true);
    }

    if (event_source == "actions")
    {
        const std::string& selection =
                m_action_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if(selection == "remove")
        {
            // First set values for comparison
            //m_compare_replay_uid = m_rd.m_replay_uid;

            //m_compare_ghost = true;

            //refreshMainScreen();

            // Now quit the dialog
            //m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == "remove-all")
        {
            //std::string fn = m_rd.m_filename;
            //ModalDialog::dismiss();

            //dynamic_cast<HighScoreSelection*>(GUIEngine::getCurrentScreen())
            //    ->onDeleteReplay(fn);
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
bool HighScoreInfoDialog::onEscapePressed()
{
    if (m_back_widget->isActivated())
        m_self_destroy = true;
    return false;
}   // onEscapePressed

// -----------------------------------------------------------------------------
void HighScoreInfoDialog::onUpdate(float dt)
{
    if (m_self_destroy)
    {
        ModalDialog::clearWindow();
        ModalDialog::dismiss();
        return;
    }
}   // onUpdate
/*
void HighScoreInfoDialog::refreshMainScreen()
{
    HighScoreSelection::getInstance()->setCompare(m_compare_ghost);
    HighScoreSelection::getInstance()->setCompareReplayUid(m_compare_replay_uid);

    // Refresh the list to have only compatible replays
    dynamic_cast<HighScoreSelection*>(GUIEngine::getCurrentScreen())
        ->refresh();
}
*/