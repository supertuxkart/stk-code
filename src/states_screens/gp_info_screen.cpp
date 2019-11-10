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
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

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
    m_reverse_spinner->setValue(0);

    m_num_tracks_spinner = getWidget<SpinnerWidget>("track-spinner");
    // Only init the number of tracks here, this way the previously selected
    // number of tracks will be the default.
    m_num_tracks_spinner->setValue(1);

    m_ai_kart_spinner = getWidget<SpinnerWidget>("ai-spinner");
    
    GUIEngine::IconButtonWidget* screenshot = getWidget<IconButtonWidget>("screenshot");
    screenshot->setFocusable(false);
    screenshot->m_tab_stop = false;
}   // loadedFromFile

// ----------------------------------------------------------------------------
/** Sets the GP to be displayed. If the identifier is 'random', no gp info
 *  will be loaded.
 */
void GPInfoScreen::setGP(const std::string &gp_ident)
{
    if(gp_ident!=GrandPrixData::getRandomGPID())
        m_gp = *grand_prix_manager->getGrandPrix(gp_ident);
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
            race_manager->getMinorMode(),
            race_manager->getNumLocalPlayers());
            
        int tracks = (int)m_gp.getTrackNames().size();
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


    if(random)
    {
        RibbonWidget *rb = getWidget<RibbonWidget>("buttons");
        rb->setLabel(1,_(L"Reload") );
        std::string restart = file_manager->getAsset(FileManager::GUI_ICON, "restart.png");

        // We have to recreate the group spinner, but a new group might have
        // been added or deleted since the last time this screen was shown.
        const std::vector<std::string>& groups = track_manager->getAllTrackGroups();
        m_group_names.clear();
        m_group_names.push_back("all");
        for (unsigned int i = 0; i < groups.size(); i++)
            m_group_names.push_back(groups[i]);
        m_group_spinner->clearLabels();
        int index_standard=0;
        for (unsigned int i = 0; i < m_group_names.size(); i++)
        {
            m_group_spinner->addLabel(_(m_group_names[i].c_str()));
            if (m_group_names[i] == "standard")
                index_standard = i + 1;
        }
        // Try to keep a previously selected group value
        if(m_group_spinner->getValue() >= (int)groups.size())
        {
            m_group_spinner->setValue(index_standard);
            m_group_name = "standard";
        }
        else
            m_group_name = stringc(m_group_names[m_group_spinner->getValue()].c_str()).c_str();

        m_max_num_tracks = getMaxNumTracks(m_group_name);

        m_num_tracks_spinner->setMax(m_max_num_tracks);
        if(m_num_tracks_spinner->getValue() > m_max_num_tracks ||
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
    }

    // Number of AIs
    // -------------
    const bool has_AI = race_manager->hasAI();
    m_ai_kart_spinner->setVisible(has_AI);
    getWidget<LabelWidget>("ai-text")->setVisible(has_AI);

    if (has_AI)
    {
        const int local_players = race_manager->getNumLocalPlayers();
        int min_ai = 0;
        int num_ai = int(UserConfigParams::m_num_karts_per_gamemode
            [RaceManager::MAJOR_MODE_GRAND_PRIX]) - local_players;

        // A ftl reace needs at least three karts to make any sense
        if (race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER)
        {
            min_ai = std::max(0, 3 - local_players);
        }
        
        num_ai = std::max(min_ai, num_ai);
        
        m_ai_kart_spinner->setActive(true);
        m_ai_kart_spinner->setValue(num_ai);
        m_ai_kart_spinner->setMax(stk_config->m_max_karts - local_players);
        m_ai_kart_spinner->setMin(min_ai);
    }   // has_AI

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
    const std::vector<std::string> tracks = m_gp.getTrackNames();

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
            const int local_players = race_manager->getNumLocalPlayers();
            const bool has_AI = race_manager->hasAI();
            const int num_ai = has_AI ? m_ai_kart_spinner->getValue() : 0;
            
            race_manager->setNumKarts(local_players + num_ai);
            UserConfigParams::m_num_karts_per_gamemode[RaceManager::MAJOR_MODE_GRAND_PRIX] = local_players + num_ai;
            
            m_gp.changeReverse(getReverse());
            race_manager->startGP(m_gp, false, false);
        }
        else if (button == "continue")
        {
            // Normal GP: continue a saved GP
            m_gp.changeReverse(getReverse());
            race_manager->startGP(m_gp, false, true);
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
        m_gp.changeTrackNumber(m_num_tracks_spinner->getValue(), m_group_name);
        addTracks();
    }
    else if (name=="ai-spinner")
    {
        const int num_ai = m_ai_kart_spinner->getValue();
        race_manager->setNumKarts( race_manager->getNumLocalPlayers() + num_ai );
        UserConfigParams::m_num_karts_per_gamemode[RaceManager::MAJOR_MODE_GRAND_PRIX] = race_manager->getNumLocalPlayers() + num_ai;
    }
    else if(name=="back")
    {
        StateManager::get()->escapePressed();
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
}
