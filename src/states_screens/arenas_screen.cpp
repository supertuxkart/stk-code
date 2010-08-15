//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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
#include "graphics/irr_driver.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "io/file_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/arenas_screen.hpp"
#include "states_screens/dialogs/track_info_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::video;

DEFINE_SCREEN_SINGLETON( ArenasScreen );

const char* ALL_ARENA_GROUPS_ID = "all";


// ------------------------------------------------------------------------------------------------------

ArenasScreen::ArenasScreen() : Screen("arenas.stkgui")
{
}

// ------------------------------------------------------------------------------------------------------

void ArenasScreen::loadedFromFile()
{
    // Dynamically add tabs
    RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
    assert( tabs != NULL );
    
    tabs->clearAllChildren();
    
    const std::vector<std::string>& groups = track_manager->getAllArenaGroups();
    const int group_amount = groups.size();

    // add standard group first
    for (int n=0; n<group_amount; n++)
    {
        if (groups[n] == DEFAULT_GROUP_NAME)
        {
            //FIXME: group name not translated
            tabs->addTextChild( stringw(groups[n].c_str()).c_str(), groups[n]);
            break;
        }
    }
    
    // add others after
    for (int n=0; n<group_amount; n++)
    {
        if (groups[n] != DEFAULT_GROUP_NAME)
        {
            //FIXME: group name not translated
            tabs->addTextChild( stringw(groups[n].c_str()).c_str(), groups[n]);
        }
    }
    
    if (group_amount > 1)
    {
        //I18N: name of the tab that will show arenas from all groups
        tabs->addTextChild( _("All") , ALL_ARENA_GROUPS_ID);
    }
    
}   // ArenasScreen

// ------------------------------------------------------------------------------------------------------

void ArenasScreen::init()
{
    Screen::init();
    buildTrackList();
    
    // select something by default for the game master
    DynamicRibbonWidget* w = this->getWidget<DynamicRibbonWidget>("tracks");
    assert( w != NULL );
    w->setSelection(w->getItems()[0].m_code_name, PLAYER_ID_GAME_MASTER, true); 
}   // init

// ------------------------------------------------------------------------------------------------------

void ArenasScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "tracks")
    { 
        DynamicRibbonWidget* w2 = dynamic_cast<DynamicRibbonWidget*>(widget);
        if (w2 == NULL) return;

        const std::string selection = w2->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (UserConfigParams::m_verbosity>=5)
            std::cout << "Clicked on arena " << selection.c_str() << std::endl;

        
        if (selection == "random_track")
        {
            RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
            assert( tabs != NULL );
            
            const std::vector<int>& curr_group = track_manager->getArenasInGroup(
                    tabs->getSelectionIDString(PLAYER_ID_GAME_MASTER) );
            
            RandomGenerator random;
            const int randomID = random.get(curr_group.size());
            
            Track* clickedTrack = track_manager->getTrack( curr_group[randomID] );
            if (clickedTrack != NULL)
            {
                ITexture* screenshot = irr_driver->getTexture( clickedTrack->getScreenshotFile().c_str() );
                
                new TrackInfoDialog( clickedTrack->getIdent(), clickedTrack->getName().c_str(),
                                    screenshot, 0.8f, 0.7f);
            }
            
        }    
        else if (selection == "locked")
        {
            unlock_manager->playLockSound();
        }
        else if (selection == RibbonWidget::NO_ITEM_ID)
        {
        }
        else
        {
            Track* clickedTrack = track_manager->getTrack(selection);
            if (clickedTrack != NULL)
            {
                ITexture* screenshot = irr_driver->getTexture( clickedTrack->getScreenshotFile().c_str() );

                new TrackInfoDialog( clickedTrack->getIdent(), clickedTrack->getName().c_str(),
                    screenshot, 0.8f, 0.7f);
            }   // clickedTrack !=  NULL
        }   // if random_track
        
    }
    else if (name == "trackgroups")
    {
        buildTrackList();
    }
    
}   // eventCallback

// ------------------------------------------------------------------------------------------------------

void ArenasScreen::buildTrackList()
{
    DynamicRibbonWidget* w = this->getWidget<DynamicRibbonWidget>("tracks");
    assert( w != NULL );
    
    // Re-build track list everytime (accounts for locking changes, etc.)
    w->clearItems();
    
    RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
    assert( tabs != NULL );
    const std::string curr_group_name = tabs->getSelectionIDString(0);
    
    if (curr_group_name == ALL_ARENA_GROUPS_ID)
    {
        const int trackAmount = track_manager->getNumberOfTracks();
        
        for (int n=0; n<trackAmount; n++)
        {
            Track* curr = track_manager->getTrack(n);
            if (!curr->isArena()) continue;
            
            if (unlock_manager->isLocked(curr->getIdent()))
            {
                w->addItem( _("Locked : solve active challenges to gain access to more!"),
                           "locked", curr->getScreenshotFile(), LOCKED_BADGE );
            }
            else
            {
                w->addItem( curr->getName(), curr->getIdent(), curr->getScreenshotFile(), 0,
                           IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE );
            }
        }
        
    }
    else
    {
        const std::vector<int>& currArenas = track_manager->getArenasInGroup(curr_group_name);
        const int trackAmount = currArenas.size();

        for (int n=0; n<trackAmount; n++)
        {
            Track* curr = track_manager->getTrack(currArenas[n]);
            if (!curr->isArena()) continue;
            
            if (unlock_manager->isLocked(curr->getIdent()))
            {
                w->addItem( _("Locked : solve active challenges to gain access to more!"),
                           "locked", curr->getScreenshotFile(), LOCKED_BADGE );
            }
            else
            {
                w->addItem( curr->getName(), curr->getIdent(), curr->getScreenshotFile(), 0,
                           IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE );
            }
        }
    }
    w->addItem(_("Random Arena"), "random_track", "/gui/track_random.png");
    w->updateItemDisplay();  
    
    assert(w->getItems().size() > 0);
}

// ------------------------------------------------------------------------------------------------------

void ArenasScreen::setFocusOnTrack(const std::string& trackName)
{
    DynamicRibbonWidget* w = this->getWidget<DynamicRibbonWidget>("tracks");
    assert( w != NULL );
    
    w->setSelection(trackName, PLAYER_ID_GAME_MASTER, true); 
}   // setFOxuOnTrack

// ------------------------------------------------------------------------------------------------------

