//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015  Marianne Gagnon
//            (C) 2014-2015  Joerg Henrichs, konstin
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

#include "states_screens/gp_info_screen.hpp"

#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/saved_grand_prix.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "io/file_manager.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/grand_prix_data.hpp"
#include "race/race_manager.hpp"
#include "race/highscore_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include "karts/kart_properties_manager.hpp"
#include "karts/kart_properties.hpp"

#include <IGUIEnvironment.h>
#include <IGUIStaticText.h>

#include <algorithm>

using irr::gui::IGUIStaticText;
using namespace GUIEngine;

/** Constructor, initialised some variables which might be used before
 *  loadedFromFile is called.
 */
GPInfoScreen::GPInfoScreen() : Screen("gp_info.stkgui")
{
    m_curr_time = 0.0f;
    // Necessary to test if loadedFroMFile() was executed (in setGP)
    m_reverse_spinner   = NULL;
    m_max_num_tracks = 0;
}   // GPInfoScreen

// ----------------------------------------------------------------------------
/** Called when the stkgui file is read. It stores the pointer to various
 *  widgets and adds the right names for reverse mode.
 */
void GPInfoScreen::loadedFromFile()
{
    // The group spinner is filled in init every time the screen is shown
    // (since the groups can change if addons are added/deleted).
    m_group_spinner      = getWidget<SpinnerWidget>("group-spinner");
    m_reverse_spinner    = getWidget<SpinnerWidget>("reverse-spinner");
    m_reverse_spinner->addLabel(_("Default"));
    m_reverse_spinner->addLabel(_("None"));
    m_reverse_spinner->addLabel(_("All"));
    m_reverse_spinner->addLabel(_("Random"));

    m_num_tracks_spinner = getWidget<SpinnerWidget>("track-spinner");
    // Only init the number of tracks here, this way the previously selected
    // number of tracks will be the default.

    m_ai_kart_spinner = getWidget<SpinnerWidget>("ai-spinner");

    m_time_target_spinner = getWidget<SpinnerWidget>("time-target-spinner");
    
    GUIEngine::IconButtonWidget* screenshot = getWidget<IconButtonWidget>("screenshot");
    screenshot->setFocusable(false);
    screenshot->m_tab_stop = false;

    m_highscore_list = getWidget<ListWidget>("highscore-entries");

    m_icon_bank = new irr::gui::STKModifiedSpriteBank(GUIEngine::getGUIEnv());
    for(unsigned int i=0; i < kart_properties_manager->getNumberOfKarts(); i++)
    {
        const KartProperties* prop = kart_properties_manager->getKartById(i);
        m_icon_bank->addTextureAsSprite(prop->getIconMaterial()->getTexture());
    }
    video::ITexture* kart_not_found = irr_driver->getTexture(file_manager->getAsset(FileManager::GUI_ICON, "random_kart.png"));
    m_unknown_kart_icon = m_icon_bank->addTextureAsSprite(kart_not_found);

    
}   // loadedFromFile

// ----------------------------------------------------------------------------
/** Sets the GP to be displayed. If the identifier is 'random', no gp info
 *  will be loaded.
 */
void GPInfoScreen::setGP(const std::string &gp_ident)
{
    if(gp_ident!=GrandPrixData::getRandomGPID()){
        m_gp = *grand_prix_manager->getGrandPrix(gp_ident);
    }
    else
    {
        // Doesn't matter what kind of GP we create, it just gets the
        // right id ("random").
        m_gp.createRandomGP(1, "standard",
                            m_reverse_spinner ? getReverse()
                                              : GrandPrixData::GP_NO_REVERSE);
    }
}   // setGP

// ----------------------------------------------------------------------------
/** Converts the currently selected reverse status into a value of type
*  GPReverseType .
*/
GrandPrixData::GPReverseType GPInfoScreen::getReverse() const
{
    switch (m_reverse_spinner->getValue())
    {
    case 0: return GrandPrixData::GP_DEFAULT_REVERSE; break;
    case 1: return GrandPrixData::GP_NO_REVERSE;      break;
    case 2: return GrandPrixData::GP_ALL_REVERSE;     break;
    case 3: return GrandPrixData::GP_RANDOM_REVERSE;  break;
    default: assert(false);
    }   // switch
    // Avoid compiler warning
    return GrandPrixData::GP_DEFAULT_REVERSE;
}   // getReverse
// ----------------------------------------------------------------------------
void GPInfoScreen::beforeAddingWidget()
{
    bool random = m_gp.isRandomGP();
    if (!random)
    {
        // Check if there is a saved GP:
        SavedGrandPrix* saved_gp = SavedGrandPrix::getSavedGP(
            StateManager::get()->getActivePlayerProfile(0)->getUniqueID(),
            m_gp.getId(),
            RaceManager::get()->getMinorMode(),
            RaceManager::get()->getNumLocalPlayers());
            
        int tracks = (int)m_gp.getTrackNames(true).size();
        bool continue_visible = saved_gp && saved_gp->getNextTrack() > 0 &&
                                            saved_gp->getNextTrack() < tracks;

        RibbonWidget* ribbonButtons = getWidget<RibbonWidget>("buttons");
        int id_continue_button = ribbonButtons->findItemNamed("continue");
        ribbonButtons->setItemVisible(id_continue_button, continue_visible);
        ribbonButtons->setLabel(id_continue_button, _("Continue saved GP"));
        getWidget<IconButtonWidget>("continue")->setImage("gui/icons/green_check.png");
    }
    else
    {
        RibbonWidget* ribbonButtons = getWidget<RibbonWidget>("buttons");
        int id_continue_button = ribbonButtons->findItemNamed("continue");
        ribbonButtons->setItemVisible(id_continue_button, true);
        ribbonButtons->setLabel(id_continue_button, _("Reload"));
        getWidget<IconButtonWidget>("continue")->setImage("gui/icons/restart.png");
    }
}

// ----------------------------------------------------------------------------
/** Called before the screen is shown. It adds the screenshot icon, and
 *  initialises all widgets depending on GP mode (random or not), if a saved
 *  GP is available etc.
 */
void GPInfoScreen::init()
{
    Screen::init();
    m_curr_time = 0.0f;

    bool random = m_gp.isRandomGP();

    getWidget<LabelWidget  >("track-text"   )->setVisible(random);
    m_num_tracks_spinner->setVisible(random);
    getWidget<LabelWidget  >("group-text"   )->setVisible(random);
    m_group_spinner->setVisible(random);

    m_time_target_spinner->setVisible(RaceManager::get()->isLapTrialMode());
    getWidget<LabelWidget>("time-target-text")->setVisible(RaceManager::get()->isLapTrialMode());
    if (RaceManager::get()->isLapTrialMode())
        m_time_target_spinner->setValue(UserConfigParams::m_lap_trial_time_limit);

    // Number of AIs
    // -------------
    const bool has_AI = RaceManager::get()->hasAI();
    m_ai_kart_spinner->setVisible(has_AI);
    getWidget<LabelWidget>("ai-text")->setVisible(has_AI);
    if (has_AI)
    {
        const int local_players = RaceManager::get()->getNumLocalPlayers();
        int min_ai = 0;
        int num_ai = int(UserConfigParams::m_num_karts_per_gamemode
            [RaceManager::MAJOR_MODE_GRAND_PRIX]) - local_players;

        // A ftl reace needs at least three karts to make any sense
        if (RaceManager::get()->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER)
        {
            min_ai = std::max(0, 3 - local_players);
        }
        
        num_ai = std::max(min_ai, num_ai);
        
        m_ai_kart_spinner->setActive(true);
        m_ai_kart_spinner->setValue(num_ai);
        m_ai_kart_spinner->setMax(stk_config->m_max_karts - local_players);
        m_ai_kart_spinner->setMin(min_ai);
    }   // has_AI

    m_reverse_spinner->setValue( UserConfigParams::m_gp_reverse );

    if (random)
    {
        RibbonWidget *rb = getWidget<RibbonWidget>("buttons");
        rb->setLabel(1,_(L"Reload") );
        std::string restart = file_manager->getAsset(FileManager::GUI_ICON, "restart.png");

        // We have to recreate the group spinner, but a new group might have
        // been added or deleted since the last time this screen was shown.
        const std::vector<std::string>& groups = track_manager->getAllTrackGroups();
        m_group_names.clear();
        m_group_names.push_back("all"); // Add "all" group as first group
        for (unsigned int i = 0; i < groups.size(); i++)  // Add rest of groups
            m_group_names.push_back(groups[i]);

        m_group_spinner->clearLabels();
        int index_standard = 0; // Index value of "standard" category

        for (unsigned int i = 0; i < m_group_names.size(); i++)
        {
            if (m_group_names[i] == "all")
            {
                // Fix capitalization (#4622)
                m_group_spinner->addLabel( _("All") );
            }
            else if (m_group_names[i] == "standard")
            {
                // Set index value of "Standard" category
                index_standard = i + 1;
                // Fix capitalization (#4622)
                m_group_spinner->addLabel( _("Standard") );
            }
            else
            {
                m_group_spinner->addLabel(_(m_group_names[i].c_str()));
            }
        }

        // Try to keep a previously selected group value
        if (m_group_spinner->getValue() >= (int)groups.size())
        {
            m_group_spinner->setValue(index_standard);
            m_group_name = "standard";
        }
        else
            m_group_name = stringc(m_group_names[m_group_spinner->getValue()].c_str()).c_str();

        m_num_tracks_spinner->setValue( UserConfigParams::m_rand_gp_num_tracks );
        m_max_num_tracks = getMaxNumTracks(m_group_name);

        m_num_tracks_spinner->setMax(m_max_num_tracks);
        if (m_num_tracks_spinner->getValue() > m_max_num_tracks ||
            m_num_tracks_spinner->getValue() < 1)
        {
            m_num_tracks_spinner->setValue(m_max_num_tracks);
        }

        // Now create the random GP:
        m_gp.createRandomGP(m_num_tracks_spinner->getValue(),
                            m_group_name, getReverse(), true);

        getWidget<LabelWidget>("name")->setText(m_gp.getName(), false);
    }
    else
    {
        getWidget<LabelWidget>("name")->setText(m_gp.getName(), false);
        m_gp.checkConsistency();

        int icon_height = GUIEngine::getFontHeight();
        int row_height = GUIEngine::getFontHeight() * 1.2f;
        m_icon_bank->setScale(icon_height/128.0f);
        m_icon_bank->setTargetIconSize(128,128);
        m_highscore_list->setIcons(m_icon_bank,row_height);
        RaceManager::get()->setNumKarts(RaceManager::get()->getNumLocalPlayers() + m_ai_kart_spinner->getValue());
        // We don't save highscores for random gps so load highscores here
        updateHighscores();
    }

    addTracks();
    addScreenshot();

    RibbonWidget* bt_start = getWidget<GUIEngine::RibbonWidget>("buttons");
    bt_start->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
}   // init

// ----------------------------------------------------------------------------
/** Updates the list of tracks shown.
 */
void GPInfoScreen::addTracks()
{
    const std::vector<std::string> tracks = m_gp.getTrackNames(true);

    ListWidget *list = getWidget<ListWidget>("tracks");
    list->clear();
    for (unsigned int i = 0; i < (unsigned int)tracks.size(); i++)
    {
        const Track *track = track_manager->getTrack(tracks[i]);
        std::string s = StringUtils::toString(i);
        list->addItem(s, track->getName());
    }
}   // addTracks

// ----------------------------------------------------------------------------
/** Creates a screenshot widget in the placeholder of the GUI.
 */
void GPInfoScreen::addScreenshot()
{
    GUIEngine::IconButtonWidget* screenshot = getWidget<IconButtonWidget>("screenshot");

    // Temporary icon, will replace it just after
    // (but it will be shown if the given icon is not found)
    screenshot->m_properties[PROP_ICON] = "gui/icons/main_help.png";

    const Track *track = track_manager->getTrack(m_gp.getTrackId(0));
    video::ITexture* image = STKTexManager::getInstance()
        ->getTexture(track->getScreenshotFile(),
        "While loading screenshot for track '%s':", track->getFilename());
    if (image != NULL)
        screenshot->setImage(image);
}   // addScreenShot

// ----------------------------------------------------------------------------
/** Handle user input.
 */
void GPInfoScreen::eventCallback(Widget *, const std::string &name,
                                 const int player_id)
{
    if(name=="buttons")
    {
        const std::string &button = getWidget<RibbonWidget>("buttons")
                                  ->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        // The continue button becomes a 'reload' button in random GP:
        if(button=="continue" && m_gp.isRandomGP())
        {
            // Create a new GP:
            m_gp.createRandomGP(m_num_tracks_spinner->getValue(),
                                m_group_name, getReverse(),
                                /*new tracks*/ true );
            addTracks();
        }
        else if (button == "start")
        {
            // Normal GP: start GP
            const int local_players = RaceManager::get()->getNumLocalPlayers();
            const bool has_AI = RaceManager::get()->hasAI();
            const int num_ai = has_AI ? m_ai_kart_spinner->getValue() : 0;
            
            RaceManager::get()->setNumKarts(local_players + num_ai);
            UserConfigParams::m_num_karts_per_gamemode[RaceManager::MAJOR_MODE_GRAND_PRIX] = local_players + num_ai;
            
            if (RaceManager::get()->isLapTrialMode())
            {
                RaceManager::get()->setGPTimeTarget(static_cast<int>(m_time_target_spinner->getValue()) * 60);
            }

            m_gp.changeReverse(getReverse());
            RaceManager::get()->startGP(m_gp, false, false);
        }
        else if (button == "continue")
        {
            // Normal GP: continue a saved GP
            m_gp.changeReverse(getReverse());
            RaceManager::get()->startGP(m_gp, false, true);
        }
    }   // name=="buttons"
    else if (name=="group-spinner")
    {
        m_group_name = stringc(m_group_names[m_group_spinner->getValue()].c_str()).c_str();

        m_max_num_tracks = getMaxNumTracks(m_group_name);

        m_num_tracks_spinner->setMax(m_max_num_tracks);
        if (m_num_tracks_spinner->getValue() > m_max_num_tracks)
            m_num_tracks_spinner->setValue(m_max_num_tracks);
        // Create a new (i.e. with new tracks) random gp, since the old
        // tracks might not all belong to the newly selected group.

        m_gp.createRandomGP(m_num_tracks_spinner->getValue(), m_group_name,
                            getReverse(),  /*new_tracks*/true);
        addTracks();
    }
    else if (name=="track-spinner")
    {
        const int num_tranks = m_num_tracks_spinner->getValue();
        m_gp.changeTrackNumber(num_tranks, m_group_name);
        UserConfigParams::m_rand_gp_num_tracks = num_tranks;
        addTracks();
    }
    else if (name=="ai-spinner")
    {
        const int num_ai = m_ai_kart_spinner->getValue();
        RaceManager::get()->setNumKarts( RaceManager::get()->getNumLocalPlayers() + num_ai );
        UserConfigParams::m_num_karts_per_gamemode[RaceManager::MAJOR_MODE_GRAND_PRIX] = RaceManager::get()->getNumLocalPlayers() + num_ai;
        updateHighscores();
    }
    else if(name=="back")
    {
        StateManager::get()->escapePressed();
    }
    else if(name=="reverse-spinner")
    {
        const int reverse = m_reverse_spinner->getValue();
        UserConfigParams::m_gp_reverse = reverse;
        updateHighscores();
    }
    else if(name == "time-target-spinner")
    {
        UserConfigParams::m_lap_trial_time_limit = m_time_target_spinner->getValue();
        updateHighscores();
    }

}   // eventCallback

// ----------------------------------------------------------------------------
/** Called every update. Used to cycle the screenshots.
 *  \param dt Time step size.
 */
void GPInfoScreen::onUpdate(float dt)
{
    if (dt == 0)
        return; // if nothing changed, return right now

    m_curr_time += dt;
    int frame_after = (int)(m_curr_time / 1.5f);

    const std::vector<std::string> tracks = m_gp.getTrackNames();
    if (frame_after >= (int)tracks.size())
    {
        frame_after = 0;
        m_curr_time = 0;
    }

    Track* track = track_manager->getTrack(tracks[frame_after]);
    std::string file = track->getScreenshotFile();
    GUIEngine::IconButtonWidget* screenshot = getWidget<IconButtonWidget>("screenshot");
    screenshot->setImage(file, IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    screenshot->m_properties[PROP_ICON] = file;
}   // onUpdate

/** Get number of available tracks for random GPs 
 */
int GPInfoScreen::getMaxNumTracks(std::string group)
{
    int max_num_tracks = 0;

    if (group == "all")
    {
        for (unsigned int i = 0; i < track_manager->getNumberOfTracks(); i++)
        {
            std::string id = track_manager->getTrack(i)->getIdent();
    
            if (!PlayerManager::getCurrentPlayer()->isLocked(id) &&
                track_manager->getTrack(i)->isRaceTrack())
            {
                max_num_tracks++;
            }
        }
    }
    else
    {
        std::vector<int> tracks = track_manager->getTracksInGroup(group);
        
        for (unsigned int i = 0; i < tracks.size(); i++)
        {
            std::string id = track_manager->getTrack(tracks[i])->getIdent();
            
            if (!PlayerManager::getCurrentPlayer()->isLocked(id) &&
                track_manager->getTrack(tracks[i])->isRaceTrack())
            {
                max_num_tracks++;
            }               
        }
    }
    
    return max_num_tracks;
}   // getNumTracks

// -----------------------------------------------------------------------
void GPInfoScreen::updateHighscores()
{
    if(m_gp.isRandomGP())
        return;
    const Highscores::HighscoreType type = "HST_GRANDPRIX";
    Highscores* highscores = highscore_manager->getGPHighscores(
                                                                RaceManager::get()->getNumberOfKarts(),
                                                                RaceManager::get()->getDifficulty(),
                                                                m_gp.getId(),
                                                                RaceManager::get()->isLapTrialMode() ? m_time_target_spinner->getValue() * 60 : 0,
                                                                getReverse(),
                                                                RaceManager::get()->getMinorMode());
    m_highscore_list->clear();
    int count = highscores->getNumberEntries();
    std::string kart;
    irr::core::stringw name;
    float time;
    for(int i=0; i<Highscores::HIGHSCORE_LEN; i++)
    {
        irr::core::stringw line;
        int icon = -1;
        if(i < count)
        {
            highscores->getEntry(i, kart, name, &time);

            std::string highscore_string;
            if (RaceManager::get()->isLapTrialMode())
                highscore_string = std::to_string(static_cast<int>(time));
            else
                highscore_string = StringUtils::timeToString(time);

            for(unsigned int n=0; n<kart_properties_manager->getNumberOfKarts(); n++)
            {
                const KartProperties* prop = kart_properties_manager->getKartById(n);
                if(kart == prop->getIdent())
                {
                    icon = n;
                    break;
                }
            }
            line = name + "    " + irr::core::stringw(highscore_string.c_str());
        }
        else
        {
            line = _("(Empty)");
        }
        if(icon == -1)
        {
            icon = m_unknown_kart_icon;
        }
        std::vector<ListWidget::ListCell> row;
        row.push_back(ListWidget::ListCell(line, icon, 1, false));
        m_highscore_list->addItem(StringUtils::toString(i),row);
    }
}   // updateHighscores

// -----------------------------------------------------------------------
void GPInfoScreen::unloaded()
{
    delete m_icon_bank;
    m_icon_bank = NULL;
}   // unloaded
