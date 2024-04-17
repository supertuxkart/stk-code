//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#include "states_screens/easter_egg_screen.hpp"

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "io/file_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/track_info_screen.hpp"
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

EasterEggScreen::EasterEggScreen() : Screen("easter_egg.stkgui")
{
}

// -----------------------------------------------------------------------------

void EasterEggScreen::loadedFromFile()
{
}

// -----------------------------------------------------------------------------

void EasterEggScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    // -- track selection screen
    if (name == "tracks")
    {
        DynamicRibbonWidget* w2 = dynamic_cast<DynamicRibbonWidget*>(widget);
        if (w2 != NULL)
        {
            const std::string selection = w2->getSelectionIDString(PLAYER_ID_GAME_MASTER);
            if(UserConfigParams::logGUI())
                Log::info("EasterEggScreen", "Clicked on track %s", selection.c_str());

            UserConfigParams::m_last_track = selection;

            if (selection == "random_track")
            {
#ifdef DEBUG
                RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
                assert( tabs != NULL );
#endif
                if (m_random_track_list.empty()) return;

                std::string track = m_random_track_list.front();
                m_random_track_list.pop_front();
                m_random_track_list.push_back(track);
                Track* clicked_track = track_manager->getTrack( track );


                if (clicked_track != NULL)
                {
                    TrackInfoScreen::getInstance()->setTrack(clicked_track);
                    TrackInfoScreen::getInstance()->push();
                }

            }
            else if (selection == "locked")
            {
                unlock_manager->playLockSound();
            }
            else if (selection != RibbonWidget::NO_ITEM_ID)
            {
                Track* clicked_track = track_manager->getTrack(selection);
                if (clicked_track != NULL)
                {
                    TrackInfoScreen::getInstance()->setTrack(clicked_track);
                    TrackInfoScreen::getInstance()->push();
                }
            }
        }
    }
    else if (name == "trackgroups")
    {
        RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
        assert( tabs != NULL );
        UserConfigParams::m_last_used_track_group = tabs->getSelectionIDString(0);
        buildTrackList();
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}

// -----------------------------------------------------------------------------

void EasterEggScreen::beforeAddingWidget()
{
    Screen::init();
    // Dynamically add tabs
    RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
    assert( tabs != NULL );

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
    
    int num_of_arenas=0;
    for (unsigned int n=0; n<track_manager->getNumberOfTracks(); n++) //iterate through tracks to find how many are arenas
    {
        Track* temp = track_manager->getTrack(n);
        if(temp->hasEasterEggs())
            num_of_arenas++;
    }

    DynamicRibbonWidget* tracks_widget = this->getWidget<DynamicRibbonWidget>("tracks");
    assert( tracks_widget != NULL );

    // Set the item hint to that number to prevent weird formatting
    // Avoid too many items shown at the same time
    tracks_widget->setItemCountHint(std::min(num_of_arenas + 1, 30));
}

// -----------------------------------------------------------------------------

void EasterEggScreen::init()
{
    DynamicRibbonWidget* tracks_widget = this->getWidget<DynamicRibbonWidget>("tracks");
    assert( tracks_widget != NULL );

    RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
    assert( tabs != NULL );
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
}

// -----------------------------------------------------------------------------

void EasterEggScreen::buildTrackList()
{
    DynamicRibbonWidget* tracks_widget = this->getWidget<DynamicRibbonWidget>("tracks");
    assert( tracks_widget != NULL );

    RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
    assert( tabs != NULL );

    // Reset track list everytime (accounts for locking changes, etc.)
    tracks_widget->clearItems();
    m_random_track_list.clear();

    const std::string curr_group_name = tabs->getSelectionIDString(0);

    // Build track list
    if (curr_group_name == ALL_TRACK_GROUPS_ID)
    {
        const int trackAmount = (int)track_manager->getNumberOfTracks();

        for (int n=0; n<trackAmount; n++)
        {
            Track* curr = track_manager->getTrack( n );
            if(RaceManager::get()->getMinorMode()==RaceManager::MINOR_MODE_EASTER_EGG
                && !curr->hasEasterEggs())
                continue;
            if (curr->isArena() || curr->isSoccer()) continue;
            if (curr->isInternal()) continue;

            if (PlayerManager::getCurrentPlayer()->isLocked(curr->getIdent()))
            {
                tracks_widget->addItem( _("Locked : solve active challenges to gain access to more!"),
                                       "locked", curr->getScreenshotFile(), LOCKED_BADGE,
                                       IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
            }
            else
            {
                tracks_widget->addItem(curr->getName(), curr->getIdent(),
                                       curr->getScreenshotFile(), 0,
                                       IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE );
                m_random_track_list.push_back(curr->getIdent());
            }
        }

    }
    else
    {
        const std::vector<int>& curr_group = track_manager->getTracksInGroup( curr_group_name );
        const int trackAmount = (int)curr_group.size();

        for (int n=0; n<trackAmount; n++)
        {
            Track* curr = track_manager->getTrack( curr_group[n] );
            if(RaceManager::get()->getMinorMode()==RaceManager::MINOR_MODE_EASTER_EGG
                && !curr->hasEasterEggs())
                continue;
            if (curr->isArena()) continue;
            if (curr->isSoccer()) continue;
            if (curr->isInternal()) continue;

            if (PlayerManager::getCurrentPlayer()->isLocked(curr->getIdent()))
            {
                tracks_widget->addItem( _("Locked : solve active challenges to gain access to more!"),
                                       "locked", curr->getScreenshotFile(), LOCKED_BADGE,
                                       IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
            }
            else
            {
                tracks_widget->addItem(curr->getName(), curr->getIdent(),
                                       curr->getScreenshotFile(), 0 /* no badge */,
                                       IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE );
                m_random_track_list.push_back(curr->getIdent());
            }
        }
    }

    tracks_widget->addItem(_("Random Track"), "random_track", "/gui/icons/track_random.png",
                           0 /* no badge */, IconButtonWidget::ICON_PATH_TYPE_RELATIVE);

    tracks_widget->updateItemDisplay();
    std::random_shuffle( m_random_track_list.begin(), m_random_track_list.end() );
}

// -----------------------------------------------------------------------------

void EasterEggScreen::setFocusOnTrack(const std::string& trackName)
{
    DynamicRibbonWidget* tracks_widget = this->getWidget<DynamicRibbonWidget>("tracks");
    assert( tracks_widget != NULL );

    // only the game master can select tracks, so it's safe to use 'PLAYER_ID_GAME_MASTER'
    tracks_widget->setSelection(trackName, PLAYER_ID_GAME_MASTER, true);
}

// -----------------------------------------------------------------------------
