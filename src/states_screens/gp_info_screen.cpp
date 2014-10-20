//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2014  Marianne Gagnon
//                2014       Joerg Henrichs, konstin
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
#include "graphics/irr_driver.hpp"
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
#include "states_screens/tracks_screen.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>
#include <IGUIStaticText.h>

#include <algorithm>

using irr::gui::IGUIStaticText;
using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( GPInfoScreen );

/** Constructor, initialised some variables which might be used before
 *  loadedFromFile is called.
 */
GPInfoScreen::GPInfoScreen() : Screen("gp_info.stkgui")
{
    m_curr_time = 0.0f;
    // Necessary to test if loadedFroMFile() was executed (in setGP)
    m_reverse_spinner   = NULL;
    m_screenshot_widget = NULL;
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
}   // loadedFromFile

// ----------------------------------------------------------------------------
/** Sets the GP to be displayed. If the identifier is 'random', no gp info
 *  will be loaded.
 */
void GPInfoScreen::setGP(const std::string &gp_ident)
{
    if(gp_ident!="random")
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
            race_manager->getNumLocalPlayers());

        RibbonWidget* ribbonButtons = getWidget<RibbonWidget>("buttons");
        int id_continue_button = ribbonButtons->findItemNamed("continue");
        ribbonButtons->setItemVisible(id_continue_button, saved_gp != NULL);
        ribbonButtons->setLabel(id_continue_button, _("Continue saved GP"));
    }
    else
    {
        RibbonWidget* ribbonButtons = getWidget<RibbonWidget>("buttons");
        int id_continue_button = ribbonButtons->findItemNamed("continue");
        ribbonButtons->setItemVisible(id_continue_button, true);
        ribbonButtons->setLabel(id_continue_button, _("Reload"));
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
        getWidget<LabelWidget>("name")->setText(_("Random Grand Prix"), false);
        std::string restart = file_manager->getAsset(FileManager::GUI, "restart.png");

        // We have to recreate the group spinner, but a new group might have
        // been added or deleted since the last time this screen was shown.
        m_group_spinner->clearLabels();
        m_group_spinner->addLabel("all");
        int index_standard=0;
        const std::vector<std::string>& groups = track_manager->getAllTrackGroups();
        for (unsigned int i = 0; i < groups.size(); i++)
        {
            m_group_spinner->addLabel(stringw(groups[i].c_str()));
            if (groups[i] == "standard")
                index_standard = i + 1;
        }
        // Try to keep a previously selected group value
        if(m_group_spinner->getValue() >= (int)groups.size())
        {
            m_group_spinner->setValue(index_standard);
            m_group_name = "standard";
        }
        else
            m_group_name = stringc(m_group_spinner->getStringValue().c_str()).c_str();

        // If there are more tracks selected atm as in the group (which can
        // happen if the group has been changed since last time this screen
        // was shown), adjust it:
        int max_num_tracks = m_group_name=="all"
                           ? track_manager->getNumberOfRaceTracks()
                           : (int)track_manager->getTracksInGroup(m_group_name).size();
        m_num_tracks_spinner->setMax(max_num_tracks);
        if(m_num_tracks_spinner->getValue() > max_num_tracks)
        {
            m_num_tracks_spinner->setValue(max_num_tracks);
        }

        // Now create the random GP:
        m_gp.createRandomGP(m_num_tracks_spinner->getValue(),
                            m_group_name, getReverse(), true);
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
        m_ai_kart_spinner->setActivated();

        // Avoid negative numbers (which can happen if e.g. the number of karts
        // in a previous race was lower than the number of players now.
        int num_ai = UserConfigParams::m_num_karts - race_manager->getNumLocalPlayers();
        if (num_ai < 0) num_ai = 0;
        m_ai_kart_spinner->setValue(num_ai);
        race_manager->setNumKarts(num_ai + race_manager->getNumLocalPlayers());
        m_ai_kart_spinner->setMax(stk_config->m_max_karts - race_manager->getNumLocalPlayers());
        // A ftl reace needs at least three karts to make any sense
        if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER)
        {
            m_ai_kart_spinner->setMin(3-race_manager->getNumLocalPlayers());
        }
        else
            m_ai_kart_spinner->setMin(0);

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
        list->addItem(s, translations->fribidize(track->getName()));
    }
}   // addTracks

// ----------------------------------------------------------------------------
/** Creates a screenshot widget in the placeholder of the GUI.
 */
void GPInfoScreen::addScreenshot()
{
    Widget* screenshot_div = getWidget("screenshot_div");

    if(!m_screenshot_widget)
    {
        m_screenshot_widget = new IconButtonWidget(
            IconButtonWidget::SCALE_MODE_KEEP_CUSTOM_ASPECT_RATIO,
            false, false, IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
        m_widgets.push_back(m_screenshot_widget);
    }
    // images are saved squared, but must be stretched to 4:3
    m_screenshot_widget->setCustomAspectRatio(4.0f / 3.0f);
    m_screenshot_widget->m_x = screenshot_div->m_x;
    m_screenshot_widget->m_y = screenshot_div->m_y;
    m_screenshot_widget->m_w = screenshot_div->m_w;
    m_screenshot_widget->m_h = screenshot_div->m_h;


    // Temporary icon, will replace it just after
    // (but it will be shown if the given icon is not found)
    m_screenshot_widget->m_properties[PROP_ICON] = "gui/main_help.png";
    m_screenshot_widget->add();

    const Track *track = track_manager->getTrack(m_gp.getTrackId(0));
    video::ITexture* screenshot = irr_driver->getTexture(track->getScreenshotFile(),
                                    "While loading screenshot for track '%s':",
                                           track->getFilename()            );
    if (screenshot != NULL)
        m_screenshot_widget->setImage(screenshot);
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
        else if (button == "start" || button == "continue")
        {
            // Normal GP: start/continue a saved GP
            m_gp.changeReverse(getReverse());
            race_manager->startGP(m_gp, false, (button == "continue"));
        }
    }   // name=="buttons"
    else if (name=="group-spinner")
    {
        m_group_name = stringc(m_group_spinner->getStringValue()).c_str();

        // Update the maximum for the number of tracks since it's depending on
        // the current track. The current value in the Number-of-tracks-spinner
        // has to be updated, since otherwise the displayed (and used) value
        // can be bigger than the maximum. (Might be a TODO to fix this)
        int max_num_tracks = m_group_name=="all"
                           ? track_manager->getNumberOfRaceTracks()
                           : (int)track_manager->getTracksInGroup(m_group_name).size();
        m_num_tracks_spinner->setMax(max_num_tracks);
        if (m_num_tracks_spinner->getValue() > max_num_tracks)
            m_num_tracks_spinner->setValue(max_num_tracks);
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
        UserConfigParams::m_num_karts = race_manager->getNumLocalPlayers() + num_ai;
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
    m_screenshot_widget->setImage(file, IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    m_screenshot_widget->m_properties[PROP_ICON] = file;
}   // onUpdate
