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

#include "states_screens/tracks_and_gp_screen.hpp"

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "io/file_manager.hpp"
#include "race/grand_prix_data.hpp"
#include "race/grand_prix_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/track_info_screen.hpp"
#include "states_screens/gp_info_screen.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <iostream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::video;

static const char ALL_TRACK_GROUPS_ID[] = "all";

// -----------------------------------------------------------------------------

void TracksAndGPScreen::eventCallback(Widget* widget, const std::string& name,
                                 const int playerID)
{
    // -- track selection screen
    if (name == "tracks")
    {
        DynamicRibbonWidget* w2 = dynamic_cast<DynamicRibbonWidget*>(widget);
        if(!w2) return;

        std::string selection = w2->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (UserConfigParams::logGUI())
        {
            Log::info("TracksAndGPScreen", "Clicked on track '%s'.",
                       selection.c_str());
        }

        UserConfigParams::m_last_track = selection;
        if (selection == "locked" && RaceManager::get()->getNumLocalPlayers() == 1)
        {
            unlock_manager->playLockSound();
            return;
        }
        else if (selection == RibbonWidget::NO_ITEM_ID)
        {
            return;
        }

        if (selection == "random_track")
        {
            if (m_random_track_list.empty()) return;

            selection = m_random_track_list.front();
            m_random_track_list.pop_front();
            m_random_track_list.push_back(selection);

        }   // selection=="random_track"
        Track *track = track_manager->getTrack(selection);

        if (track)
        {
            TrackInfoScreen::getInstance()->setTrack(track);
            TrackInfoScreen::getInstance()->push();
        }   // if clicked_track

    }   // name=="tracks"
    else if (name == "gps")
    {
        DynamicRibbonWidget* gps_widget = dynamic_cast<DynamicRibbonWidget*>(widget);
        const std::string &selection =
                       gps_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "locked" && RaceManager::get()->getNumLocalPlayers()==1)
        {
            unlock_manager->playLockSound();
        }
        else
        {
            const GrandPrixData* gp_data=grand_prix_manager->getGrandPrix(selection);
            if(gp_data->containsUnavailableTracks() && RaceManager::get()->getNumLocalPlayers()==1)
            {
                new MessageDialog(_("You cannot play this Grand Prix because it contains tracks that aren't unlocked!"));
            }
            else
            {
                GPInfoScreen *gpis = GPInfoScreen::getInstance();
                gpis->setGP(selection);
                gpis->push();
            }
        }
    }
    else if (name == "rand-gp")
    {
        // Random GP
        GPInfoScreen::getInstance()->setGP(GrandPrixData::getRandomGPID());
        GPInfoScreen::getInstance()->push();
    }
    else if (name == "trackgroups")
    {
        RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
        UserConfigParams::m_last_used_track_group = tabs->getSelectionIDString(0);
        buildTrackList();
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}   // eventCallback

// -----------------------------------------------------------------------------

void TracksAndGPScreen::beforeAddingWidget()
{
    Screen::init();
    RibbonWidget* tabs = getWidget<RibbonWidget>("trackgroups");
    tabs->clearAllChildren();

    const std::vector<std::string>& groups = track_manager->getAllTrackGroups();
    const int group_amount = (int)groups.size();

    if (group_amount > 1)
    {
        //I18N: name of the tab that will show tracks from all groups
        tabs->addTextChild( _("All"), ALL_TRACK_GROUPS_ID );
    }

    // Make group names being picked up by gettext
#define FOR_GETTEXT_ONLY(x)
    //I18N: track group name
    FOR_GETTEXT_ONLY( _("All") )
    //I18N: track group name
    FOR_GETTEXT_ONLY( _("Standard") )
    //I18N: track group name
    FOR_GETTEXT_ONLY( _("Add-Ons") )

    // Add other groups after
    for (int n=0; n<group_amount; n++)
    {
        if (groups[n] == "standard") // Fix capitalization (#4622)
            tabs->addTextChild( _("Standard") , groups[n]);
        else // Try to translate group names
            tabs->addTextChild( _(groups[n].c_str()) , groups[n]);
    } // for n<group_amount

    DynamicRibbonWidget* tracks_widget = getWidget<DynamicRibbonWidget>("tracks");

    // Avoid too many items shown at the same time
    tracks_widget->setItemCountHint(std::min((int)track_manager->getNumberOfTracks()+1, 20));
}   // beforeAddingWidget

// -----------------------------------------------------------------------------

void TracksAndGPScreen::init()
{
    DynamicRibbonWidget* gps_widget    = getWidget<DynamicRibbonWidget>("gps");
    DynamicRibbonWidget* tracks_widget = getWidget<DynamicRibbonWidget>("tracks");
    assert(tracks_widget != NULL);

    m_search_box = getWidget<TextBoxWidget>("search");
    m_search_box->clearListeners();
    m_search_box->addListener(this);

    // Reset GP list everytime (accounts for locking changes, etc.)
    gps_widget->clearItems();
    gps_widget->setMaxLabelLength(30);

    // Ensure that no GP and no track is NULL
    grand_prix_manager->checkConsistency();

    // Build GP list
    const int gpAmount = grand_prix_manager->getNumberOfGrandPrix();
    for (int n=0; n<gpAmount; n++)
    {
        const GrandPrixData* gp = grand_prix_manager->getGrandPrix(n);
        const std::vector<std::string> tracks = gp->getTrackNames(true);

        //Skip epmpty GPs
        if (gp->getNumberOfTracks()==0)
            continue;

        std::vector<std::string> screenshots;
        for (unsigned int t=0; t<tracks.size(); t++)
        {
            const Track* curr = track_manager->getTrack(tracks[t]);
            screenshots.push_back(curr->getScreenshotFile());
        }
        assert(screenshots.size() > 0);

        if (PlayerManager::getCurrentPlayer()->isLocked(gp->getId()) &&
            RaceManager::get()->getNumLocalPlayers() == 1)
        {
            gps_widget->addAnimatedItem(_("Locked!"), "locked",
                                        screenshots, 1.5f,
                                        LOCKED_BADGE | TROPHY_BADGE,
                                        IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
        }
        else
        {
            gps_widget->addAnimatedItem(gp->getName(),
                                        gp->getId(), screenshots, 1.5f,
                                        TROPHY_BADGE,
                                        IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
        }
    }

    gps_widget->updateItemDisplay();

    RibbonWidget* tabs = getWidget<RibbonWidget>("trackgroups");
    tabs->select(UserConfigParams::m_last_used_track_group, PLAYER_ID_GAME_MASTER);

    buildTrackList();

    // select old track for the game master (if found)
    STKTexManager::getInstance()->setTextureErrorMessage(
              "While loading screenshot in track screen for last track '%s':",
              UserConfigParams::m_last_track);
    if (!tracks_widget->setSelection(UserConfigParams::m_last_track,
                                     PLAYER_ID_GAME_MASTER, true))
    {
        tracks_widget->setSelection(0, PLAYER_ID_GAME_MASTER, true);
    }
    STKTexManager::getInstance()->unsetTextureErrorMessage();
}   // init

// -----------------------------------------------------------------------------
/** Rebuild the list of tracks and GPs. This need to be recomputed e.g. to
 *  take unlocked tracks into account.
 */
void TracksAndGPScreen::buildTrackList()
{
    DynamicRibbonWidget* tracks_widget = this->getWidget<DynamicRibbonWidget>("tracks");
    RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");

    // Reset track list everytime (accounts for locking changes, etc.)
    tracks_widget->clearItems();
    m_random_track_list.clear();

    const std::string& curr_group_name = tabs->getSelectionIDString(0);

    const int track_amount = (int)track_manager->getNumberOfTracks();

    // First build a list of all tracks to be displayed
    // (e.g. exclude arenas, ...)
    PtrVector<Track, REF> tracks;
    for (int n = 0; n < track_amount; n++)
    {
        Track* curr = track_manager->getTrack(n);
        if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_EASTER_EGG
            && !curr->hasEasterEggs())
            continue;
        core::stringw search_text = m_search_box->getText();
        search_text.make_lower();
        if (!search_text.empty() &&
            curr->getName().make_lower().find(search_text.c_str()) == -1)
            continue;
        if (curr->isArena() || curr->isSoccer()||curr->isInternal()) continue;
        if (curr_group_name != ALL_TRACK_GROUPS_ID &&
            !curr->isInGroup(curr_group_name)) continue;

        tracks.push_back(curr);
    }   // for n<track_amount

    tracks.insertionSort();
    for (unsigned int i = 0; i < tracks.size(); i++)
    {
        Track *curr = tracks.get(i);
        if (PlayerManager::getCurrentPlayer()->isLocked(curr->getIdent()) &&
            RaceManager::get()->getNumLocalPlayers() == 1)
        {
            tracks_widget->addItem(
                _("Locked: solve active challenges to gain access to more!"),
                "locked", curr->getScreenshotFile(), LOCKED_BADGE,
                IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
        }
        else
        {
            tracks_widget->addItem(curr->getName(),
                curr->getIdent(),
                curr->getScreenshotFile(), 0,
                IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
            m_random_track_list.push_back(curr->getIdent());
        }
    }

    tracks_widget->addItem(_("Random Track"), "random_track",
                           "/gui/icons/track_random.png", 0 /* no badge */,
                           IconButtonWidget::ICON_PATH_TYPE_RELATIVE);

    tracks_widget->updateItemDisplay();
    std::random_shuffle( m_random_track_list.begin(), m_random_track_list.end() );
}   // buildTrackList

// -----------------------------------------------------------------------------

void TracksAndGPScreen::setFocusOnTrack(const std::string& trackName)
{
    DynamicRibbonWidget* tracks_widget = this->getWidget<DynamicRibbonWidget>("tracks");

    // only the game master can select tracks,
    // so it's safe to use 'PLAYER_ID_GAME_MASTER'
    tracks_widget->setSelection(trackName, PLAYER_ID_GAME_MASTER, true);
}   // setFocusOnTrack

// -----------------------------------------------------------------------------

void TracksAndGPScreen::setFocusOnGP(const std::string& gpName)
{
    DynamicRibbonWidget* gps_widget = getWidget<DynamicRibbonWidget>("gps");

    // only the game master can select tracks/GPs,
    // so it's safe to use 'PLAYER_ID_GAME_MASTER'
    gps_widget->setSelection(gpName, PLAYER_ID_GAME_MASTER, true);
}   // setFocusOnGP

// -----------------------------------------------------------------------------
