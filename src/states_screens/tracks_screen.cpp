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
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "race/grand_prix_data.hpp"
#include "race/grand_prix_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/tracks_screen.hpp"
#include "states_screens/dialogs/gp_info_dialog.hpp"
#include "states_screens/dialogs/track_info_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::video;

const char* ALL_TRACK_GROUPS_ID = "all";

DEFINE_SCREEN_SINGLETON( TracksScreen );

// -----------------------------------------------------------------------------------------------

TracksScreen::TracksScreen() : Screen("tracks.stkgui")
{
    // Dynamically add tabs
    // FIXME: it's not very well documented that RibbonWidgets can have dynamically generated contents
    RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
    assert( tabs != NULL );
    
    tabs->m_children.clearAndDeleteAll();
    
    const std::vector<std::string>& groups = track_manager->getAllGroups();
    
    const int group_amount = groups.size();
    for (int n=0; n<group_amount; n++)
    {
        ButtonWidget* item = new ButtonWidget();
        item->m_text = groups[n].c_str(); // FIXME: i18n ?
        item->m_properties[PROP_ID] = groups[n];
        tabs->m_children.push_back(item);
    }
    
    if (group_amount > 1)
    {
        ButtonWidget* item = new ButtonWidget();
        //I18N: name of the tab that will show tracks from all groups
        item->m_text = _("All");
        item->m_properties[PROP_ID] = ALL_TRACK_GROUPS_ID;
        tabs->m_children.push_back(item);
    }
}

// -----------------------------------------------------------------------------------------------

void TracksScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    // -- track seelction screen
    if (name == "tracks")
    {
        DynamicRibbonWidget* w2 = dynamic_cast<DynamicRibbonWidget*>(widget);
        if (w2 != NULL)
        {
            const std::string selection = w2->getSelectionIDString(GUI_PLAYER_ID);
            if(UserConfigParams::m_verbosity>=5)
                std::cout << "Clicked on track " << selection.c_str() 
                          << std::endl;
            
            if (selection == "random_track")
            {
                RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
                assert( tabs != NULL );
                
                const std::vector<int>& curr_group = track_manager->getTracksInGroup( tabs->getSelectionIDString(0) );
                
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
            else if (selection == DynamicRibbonWidget::NO_ITEM_ID)
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
                }
            }
        }
    }
    else if (name == "gps")
    {
        DynamicRibbonWidget* gps_widget = dynamic_cast<DynamicRibbonWidget*>(widget);
        if (gps_widget != NULL)
        {
            std::string selection = gps_widget->getSelectionIDString(GUI_PLAYER_ID);
            
            if (selection == "locked")
            {
                unlock_manager->playLockSound();
            }
            else
            {
                new GPInfoDialog( selection, 0.8f, 0.7f );
            }
        }
        else
        {
            assert(false);
        }
    }
    else if (name == "trackgroups")
    {
        buildTrackList();
    }
    
}

// -----------------------------------------------------------------------------------------------

void TracksScreen::init()
{
    DynamicRibbonWidget* gps_widget = this->getWidget<DynamicRibbonWidget>("gps");
    assert( gps_widget != NULL );
    
    DynamicRibbonWidget* tracks_widget = this->getWidget<DynamicRibbonWidget>("tracks");
    assert( tracks_widget != NULL );
    
    RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
    assert( tabs != NULL );
       
    // Reset GP list everytime (accounts for locking changes, etc.)
    gps_widget->clearItems();
    
    // Build GP list
    const int gpAmount = grand_prix_manager->getNumberOfGrandPrix();
    for (int n=0; n<gpAmount; n++)
    {
        const GrandPrixData* gp = grand_prix_manager->getGrandPrix(n);
        
        std::vector<std::string> tracks = gp->getTracks();
        
        std::vector<std::string> sshot_files;
        for (unsigned int t=0; t<tracks.size(); t++)
        {
            Track* curr = track_manager->getTrack(tracks[t]);
            if (curr == NULL)
            {
                std::cerr << "/!\\ WARNING: Grand Prix '" << gp->getId() << "' refers to track '"
                          << tracks[t] << "', which does not exist.\n";
            }
            else
            {
                sshot_files.push_back(curr->getScreenshotFile());
            }
        }
        if (sshot_files.size() == 0)
        {
            std::cerr << "/!\\ WARNING: Grand Prix '" << gp->getId() << "' does not contain any valid track.\n";
            sshot_files.push_back("gui/main_help.png");
        }
        
        if (unlock_manager->isLocked(gp->getId()))
        {
            gps_widget->addAnimatedItem( _("Locked!"),
                                         "locked", sshot_files, 1.5f, LOCKED_BADGE | TROPHY_BADGE,
                                         IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
        }
        else
        {
            gps_widget->addAnimatedItem( gp->getName(), gp->getId(), sshot_files, 1.5f, TROPHY_BADGE,
                                         IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE );
        }
    }
    gps_widget->updateItemDisplay();
    
    buildTrackList();
       
    // FIXME: don't hardcode player 0?
    tracks_widget->setSelection(tracks_widget->getItems()[0].m_code_name, 0, true);
}

// -----------------------------------------------------------------------------------------------

void TracksScreen::tearDown()
{
}

// -----------------------------------------------------------------------------------------------

void TracksScreen::buildTrackList()
{
    DynamicRibbonWidget* tracks_widget = this->getWidget<DynamicRibbonWidget>("tracks");
    assert( tracks_widget != NULL );
    
    RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
    assert( tabs != NULL );
    
    // Reset track list everytime (accounts for locking changes, etc.)
    tracks_widget->clearItems();
    
    const std::string curr_group_name = tabs->getSelectionIDString(0);
    
    // Build track list
    if (curr_group_name == ALL_TRACK_GROUPS_ID)
    {
        const int trackAmount = track_manager->getNumberOfTracks();
        
        for (int n=0; n<trackAmount; n++)
        {
            Track* curr = track_manager->getTrack( n );
            if (curr->isArena()) continue;
            
            if (unlock_manager->isLocked(curr->getIdent()))
            {
                tracks_widget->addItem( _("Locked : solve active challenges to gain access to more!"),
                                       "locked", curr->getScreenshotFile(), true,
                                       IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
            }
            else
            {
                tracks_widget->addItem( curr->getName(), curr->getIdent(), curr->getScreenshotFile(),
                                       0, IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE );
            }
        }
        
    }
    else
    {
        //FIXME: don't hardcode player 0?
        const std::vector<int>& curr_group = track_manager->getTracksInGroup( curr_group_name );
        const int trackAmount = curr_group.size();
        
        for (int n=0; n<trackAmount; n++)
        {
            Track* curr = track_manager->getTrack( curr_group[n] );
            if (curr->isArena()) continue;
            
            if (unlock_manager->isLocked(curr->getIdent()))
            {
                tracks_widget->addItem( _("Locked : solve active challenges to gain access to more!"),
                                       "locked", curr->getScreenshotFile(), true,
                                       IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
            }
            else
            {
                tracks_widget->addItem( curr->getName(), curr->getIdent(), curr->getScreenshotFile(),
                                       0, IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE );
            }
        }
    }
    
    tracks_widget->addItem(_("Random Track"), "random_track", "/gui/track_random.png",
                           0, IconButtonWidget::ICON_PATH_TYPE_RELATIVE);
    
    tracks_widget->updateItemDisplay();       
}

// -----------------------------------------------------------------------------------------------

void TracksScreen::setFocusOnTrack(const std::string& trackName)
{
    DynamicRibbonWidget* tracks_widget = this->getWidget<DynamicRibbonWidget>("tracks");
    assert( tracks_widget != NULL );
    
    // FIXME: don't hardcode player 0?
    tracks_widget->setSelection(trackName, 0, true); 
}

// -----------------------------------------------------------------------------------------------

void TracksScreen::setFocusOnGP(const std::string& gpName)
{
    DynamicRibbonWidget* gps_widget = this->getWidget<DynamicRibbonWidget>("gps");
    assert( gps_widget != NULL );
    
    // FIXME: don't hardcode player 0?
    gps_widget->setSelection(gpName, 0, true); 
}

// -----------------------------------------------------------------------------------------------

