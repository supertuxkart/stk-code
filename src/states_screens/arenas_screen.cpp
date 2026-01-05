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

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "io/file_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/arenas_screen.hpp"
#include "states_screens/track_info_screen.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/random_generator.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <iostream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::video;

static const char ALL_ARENA_GROUPS_ID[] = "all";


// -----------------------------------------------------------------------------

ArenasScreen::ArenasScreen() : Screen("arenas.stkgui")
{
}

// -----------------------------------------------------------------------------

void ArenasScreen::loadedFromFile()
{
    m_widgets.bind(this);
}

// -----------------------------------------------------------------------------

void ArenasScreen::beforeAddingWidget()
{
    // Add user-defined group to track groups
    track_manager->setFavoriteTrackStatus(PlayerManager::getCurrentPlayer()->getFavoriteTrackStatus());

    m_widgets.favorite->setState(false);

    // Dynamically add tabs
    m_widgets.trackgroups->clearAllChildren();

    bool soccer_mode = RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_SOCCER;
    const std::vector<std::string>& groups = track_manager->getAllArenaGroups(soccer_mode);
    const int group_amount = (int)groups.size();

    if (group_amount > 1)
    {
        //I18N: name of the tab that will show arenas from all groups
        m_widgets.trackgroups->addTextChild( _("All"), ALL_ARENA_GROUPS_ID );
    }

    // Make group names being picked up by gettext
#define FOR_GETTEXT_ONLY(x)
    //I18N: track group name
    FOR_GETTEXT_ONLY( _("All") )
    //I18N: track group name
    FOR_GETTEXT_ONLY( _("Favorites") )
    //I18N: track group name
    FOR_GETTEXT_ONLY( _("Standard") )
    //I18N: track group name
    FOR_GETTEXT_ONLY( _("Add-Ons") )

    // Add other groups after
    for (int n=0; n<group_amount; n++)
    {
        if (groups[n] == "standard") // Fix capitalization (#4622)
            m_widgets.trackgroups->addTextChild( _("Standard") , groups[n]);
        else // Try to translate group names
            m_widgets.trackgroups->addTextChild( _(groups[n].c_str()) , groups[n]);
    } // for n<group_amount

    int num_of_arenas=0;
    for (unsigned int n=0; n<track_manager->getNumberOfTracks(); n++) //iterate through tracks to find how many are arenas
    {
        Track* temp = track_manager->getTrack(n);
        if (soccer_mode)
        {
            if(temp->isSoccer() && (temp->hasNavMesh() ||
                RaceManager::get()->getNumLocalPlayers() > 1 ||
                UserConfigParams::m_artist_debug_mode))
                num_of_arenas++;
        }
        else
        {
            if(temp->isArena() && (temp->hasNavMesh()  ||
                RaceManager::get()->getNumLocalPlayers() > 1 ||
                UserConfigParams::m_artist_debug_mode))
                num_of_arenas++;
        }
    }

    // Set the item hint to that number to prevent weird formatting
    // Avoid too many items shown at the same time
    m_widgets.tracks->setItemCountHint(std::min(num_of_arenas + 1, 30));
}

// -----------------------------------------------------------------------------

void ArenasScreen::init()
{
    m_random_arena_list.clear();
    Screen::init();

    m_widgets.search->clearListeners();
    m_widgets.search->addListener(this);

    buildTrackList();
    // select something by default for the game master
    m_widgets.tracks->setSelection(m_widgets.tracks->getItems()[0].m_code_name, PLAYER_ID_GAME_MASTER, true);
}   // init

// -----------------------------------------------------------------------------

void ArenasScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (widget == m_widgets.tracks)
    {
        std::string selection = m_widgets.tracks->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (UserConfigParams::logGUI())
            Log::info("ArenasScreen", "Clicked on arena %s", selection.c_str());


        if (selection == "random_track")
        {
            if (m_random_arena_list.empty()) return;

            selection = m_random_arena_list.front();
            m_random_arena_list.pop_front();
            m_random_arena_list.push_back(selection);
        }
        else if (selection == "locked")
        {
            unlock_manager->playLockSound();
            return;
        }
        else if (selection == RibbonWidget::NO_ITEM_ID)
        {
            return;
        }

        Track* clicked_track = track_manager->getTrack(selection);
        if (clicked_track)
        {
            // In favorite edit mode, switch the status of the selected track
            if (m_widgets.favorite->getState())
            {
                if(PlayerManager::getCurrentPlayer()->isFavoriteTrack(clicked_track->getIdent()))
                    PlayerManager::getCurrentPlayer()->removeFavoriteTrack(clicked_track->getIdent());
                else
                    PlayerManager::getCurrentPlayer()->addFavoriteTrack(clicked_track->getIdent());

                buildTrackList();
            }
            else
            {
                TrackInfoScreen::getInstance()->setTrack(clicked_track);
                TrackInfoScreen::getInstance()->push();
            }
        }   // clickedTrack !=  NULL

    }
    else if (widget == m_widgets.trackgroups)
    {
        buildTrackList();
    }
    else if (widget == m_widgets.back)
    {
        StateManager::get()->escapePressed();
    }

}   // eventCallback

// ------------------------------------------------------------------------------------------------------

void ArenasScreen::buildTrackList()
{
    // Add user-defined group to track groups
    track_manager->setFavoriteTrackStatus(PlayerManager::getCurrentPlayer()->getFavoriteTrackStatus());

    // Re-build track list everytime (accounts for locking changes, etc.)
    m_widgets.tracks->clearItems();

    const std::string curr_group_name = m_widgets.trackgroups->getSelectionIDString(0);

    bool soccer_mode = RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_SOCCER;
    bool arenas_have_navmesh = false;
    PtrVector<Track, REF> tracks;

    m_random_arena_list.clear();

    if (curr_group_name == ALL_ARENA_GROUPS_ID)
    {
        const int track_amount = (int)track_manager->getNumberOfTracks();

        for (int n=0; n<track_amount; n++)
        {
            Track* curr = track_manager->getTrack(n);
            
            core::stringw search_text = m_widgets.search->getText();
            search_text.make_lower();
            if (!search_text.empty() &&
                curr->getName().make_lower().find(search_text.c_str()) == -1)
                continue;
            
            if (soccer_mode)
            {
                if(curr->isSoccer() && curr->hasNavMesh() && !arenas_have_navmesh)
                    arenas_have_navmesh = true;

                if(!curr->isSoccer()                     ||
                  (!(curr->hasNavMesh()                  ||
                  RaceManager::get()->getNumLocalPlayers() > 1 ||
                  UserConfigParams::m_artist_debug_mode)))
                {
                    continue;
                }
            }
            else
            {
                if(curr->isArena() && curr->hasNavMesh() && !arenas_have_navmesh)
                    arenas_have_navmesh = true;

                if(!curr->isArena()                      ||
                  (!(curr->hasNavMesh()                  ||
                  RaceManager::get()->getNumLocalPlayers() > 1 ||
                  UserConfigParams::m_artist_debug_mode)))
                {
                    continue;
                }
            }
            tracks.push_back(curr);
        }
    }
    else
    {
        const std::vector<int>& currArenas = track_manager->getArenasInGroup(curr_group_name, soccer_mode);
        const int track_amount = (int)currArenas.size();

        for (int n=0; n<track_amount; n++)
        {
            Track* curr = track_manager->getTrack(currArenas[n]);

            core::stringw search_text = m_widgets.search->getText();
            search_text.make_lower();
            if (!search_text.empty() &&
                curr->getName().make_lower().find(search_text.c_str()) == -1)
                continue;
            
            if (soccer_mode)
            {
                if(curr->isSoccer() && curr->hasNavMesh() && !arenas_have_navmesh)
                    arenas_have_navmesh = true;

                if(!curr->isSoccer()                     ||
                  (!(curr->hasNavMesh()                  ||
                  RaceManager::get()->getNumLocalPlayers() > 1 ||
                  UserConfigParams::m_artist_debug_mode)))
                {
                    continue;
                }
            }
            else
            {
                if(curr->isArena() && curr->hasNavMesh() && !arenas_have_navmesh)
                    arenas_have_navmesh = true;

                if(!curr->isArena()                      ||
                  (!(curr->hasNavMesh()                  ||
                  RaceManager::get()->getNumLocalPlayers() > 1 ||
                  UserConfigParams::m_artist_debug_mode)))
                {
                    continue;
                }
            }
            tracks.push_back(curr);
        }
    }
    tracks.insertionSort();

    for (unsigned int i = 0; i < tracks.size(); i++)
    {
        Track *curr = tracks.get(i);
        if (PlayerManager::getCurrentPlayer()->isLocked(curr->getIdent()))
        {
            m_widgets.tracks->addItem( _("Locked : solve active challenges to gain access to more!"),
                        "locked", curr->getScreenshotFile(), LOCKED_BADGE );
        }
        else if (PlayerManager::getCurrentPlayer()->isFavoriteTrack(curr->getIdent()))
        {
            m_widgets.tracks->addItem(curr->getName(), curr->getIdent(),
                curr->getScreenshotFile(), HEART_BADGE,
                IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
            m_random_arena_list.push_back(curr->getIdent());
        }
        else
        {
            m_widgets.tracks->addItem(curr->getName(), curr->getIdent(), curr->getScreenshotFile(), 0,
                        IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE );
            m_random_arena_list.push_back(curr->getIdent());
        }
    }

    if (arenas_have_navmesh || RaceManager::get()->getNumLocalPlayers() > 1 ||
        UserConfigParams::m_artist_debug_mode)
        m_widgets.tracks->addItem(_("Random Arena"), "random_track", "/gui/icons/track_random.png");
    m_widgets.tracks->updateItemDisplay();

    std::shuffle( m_random_arena_list.begin(), m_random_arena_list.end(),
                  RandomGenerator::getGenerator());
}

// ------------------------------------------------------------------------------------------------------

void ArenasScreen::setFocusOnTrack(const std::string& trackName)
{
    m_widgets.tracks->setSelection(trackName, PLAYER_ID_GAME_MASTER, true);
}   // setFocusOnTrack

// ------------------------------------------------------------------------------------------------------

